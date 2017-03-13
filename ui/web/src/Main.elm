module Main exposing (..)

import Widgets.TimeInput as TimeInput
import Widgets.TagList as TagList exposing (Tag(..))
import Widgets.Calendar as Calendar
import Widgets.Typeahead as Typeahead
import Widgets.Map.RailViz as RailViz
import Widgets.Map.ConnectionDetails as MapConnectionDetails
import Widgets.Connections as Connections
import Widgets.ConnectionDetails as ConnectionDetails
import Widgets.StationEvents as StationEvents
import Widgets.TripSearch as TripSearch
import Widgets.SimTimePicker as SimTimePicker
import Data.ScheduleInfo.Types exposing (ScheduleInfo)
import Data.ScheduleInfo.Request as ScheduleInfo
import Data.ScheduleInfo.Decode exposing (decodeScheduleInfoResponse)
import Data.Routing.Types exposing (SearchDirection(..))
import Data.Intermodal.Types as Intermodal exposing (IntermodalRoutingRequest)
import Data.Intermodal.Request as IntermodalRoutingRequest exposing (IntermodalLocation(..))
import Data.Routing.Decode exposing (decodeRoutingResponse)
import Data.Connection.Types exposing (Station, Position, TripId, Connection)
import Data.Journey.Types exposing (Journey, toJourney)
import Data.Lookup.Request exposing (encodeTripToConnection)
import Data.Lookup.Decode exposing (decodeTripToConnectionResponse)
import Util.List exposing ((!!))
import Util.Api as Api exposing (ApiError(..))
import Util.Date exposing (combineDateTime, unixTime)
import Util.Core exposing ((=>))
import Date.Extra.Compare
import Localization.Base exposing (..)
import Localization.De exposing (..)
import Localization.En exposing (..)
import Html exposing (..)
import Html.Attributes exposing (..)
import Html.Events exposing (..)
import Html.Lazy exposing (..)
import Dom
import Dom.Scroll as Scroll
import Http exposing (encodeUri)
import Task
import Navigation exposing (Location)
import UrlParser
import Routes exposing (..)
import Debounce
import Maybe.Extra exposing (isJust, orElse)
import Port
import Json.Encode
import Json.Decode as Decode
import Date exposing (Date)
import Time exposing (Time)


type alias ProgramFlags =
    { apiEndpoint : String
    , currentTime : Time
    , simulationTime : Maybe Time
    , language : String
    , motisParam : Maybe String
    , timeParam : Maybe String
    , langParam : Maybe String
    , fromLocation : Maybe String
    , toLocation : Maybe String
    , fromTransports : Maybe String
    , toTransports : Maybe String
    }


main : Program ProgramFlags Model Msg
main =
    Navigation.programWithFlags locationToMsg
        { init = init
        , view = view
        , update = update
        , subscriptions = subscriptions
        }



-- MODEL


type alias Model =
    { fromLocation : Typeahead.Model
    , toLocation : Typeahead.Model
    , fromTransports : TagList.Model
    , toTransports : TagList.Model
    , date : Calendar.Model
    , time : TimeInput.Model
    , searchDirection : SearchDirection
    , railViz : RailViz.Model
    , connections : Connections.Model
    , connectionDetails : Maybe ConnectionDetails.State
    , tripDetails : Maybe ConnectionDetails.State
    , stationEvents : Maybe StationEvents.Model
    , tripSearch : TripSearch.Model
    , subView : Maybe SubView
    , selectedConnectionIdx : Maybe Int
    , scheduleInfo : Maybe ScheduleInfo
    , locale : Localization
    , apiEndpoint : String
    , currentRoutingRequest : Maybe IntermodalRoutingRequest
    , debounce : Debounce.State
    , connectionListScrollPos : Float
    , currentTime : Date
    , timeOffset : Float
    , overlayVisible : Bool
    , stationSearch : Typeahead.Model
    , programFlags : ProgramFlags
    , simTimePicker : SimTimePicker.Model
    , updateSearchTime : Bool
    }


type SubView
    = TripDetailsView
    | StationEventsView
    | TripSearchView


init : ProgramFlags -> Location -> ( Model, Cmd Msg )
init flags initialLocation =
    let
        locale =
            getLocale flags.language

        remoteAddress =
            flags.apiEndpoint

        fromLocation =
            flags.fromLocation
                |> Maybe.withDefault ""

        toLocation =
            flags.toLocation
                |> Maybe.withDefault ""

        ( dateModel, dateCmd ) =
            Calendar.init locale.dateConfig

        ( timeModel, timeCmd ) =
            TimeInput.init False

        ( mapModel, mapCmd ) =
            RailViz.init remoteAddress

        ( fromLocationModel, fromLocationCmd ) =
            Typeahead.init remoteAddress fromLocation

        ( toLocationModel, toLocationCmd ) =
            Typeahead.init remoteAddress toLocation

        ( stationSearchModel, stationSearchCmd ) =
            Typeahead.init remoteAddress ""

        ( tripSearchModel, tripSearchCmd ) =
            TripSearch.init remoteAddress locale

        ( simTimePickerModel, simTimePickerCmd ) =
            SimTimePicker.init locale

        initialModel =
            { fromLocation = fromLocationModel
            , toLocation = toLocationModel
            , fromTransports = TagList.init flags.fromTransports
            , toTransports = TagList.init flags.toTransports
            , date = dateModel
            , time = timeModel
            , searchDirection = Forward
            , railViz = mapModel
            , connections = Connections.init remoteAddress
            , connectionDetails = Nothing
            , tripDetails = Nothing
            , stationEvents = Nothing
            , tripSearch = tripSearchModel
            , subView = Nothing
            , selectedConnectionIdx = Nothing
            , scheduleInfo = Nothing
            , locale = locale
            , apiEndpoint = remoteAddress
            , currentRoutingRequest = Nothing
            , debounce = Debounce.init
            , connectionListScrollPos = 0
            , currentTime = Date.fromTime flags.currentTime
            , timeOffset = 0
            , overlayVisible = True
            , stationSearch = stationSearchModel
            , programFlags = flags
            , simTimePicker = simTimePickerModel
            , updateSearchTime = isJust flags.simulationTime
            }

        ( model1, cmd1 ) =
            update (locationToMsg initialLocation) initialModel

        ( model2, cmd2 ) =
            case flags.simulationTime of
                Just time ->
                    update (SetSimulationTime time) model1

                Nothing ->
                    ( model1, Cmd.none )
    in
        model2
            ! [ Cmd.map DateUpdate dateCmd
              , Cmd.map TimeUpdate timeCmd
              , Cmd.map MapUpdate mapCmd
              , Cmd.map FromLocationUpdate fromLocationCmd
              , Cmd.map ToLocationUpdate toLocationCmd
              , Cmd.map StationSearchUpdate stationSearchCmd
              , Cmd.map TripSearchUpdate tripSearchCmd
              , Cmd.map SimTimePickerUpdate simTimePickerCmd
              , requestScheduleInfo remoteAddress
              , Task.perform UpdateCurrentTime Time.now
              , cmd1
              , cmd2
              ]



-- UPDATE


type Msg
    = NoOp
    | Reset
    | FromLocationUpdate Typeahead.Msg
    | ToLocationUpdate Typeahead.Msg
    | FromTransportsUpdate TagList.Msg
    | ToTransportsUpdate TagList.Msg
    | DateUpdate Calendar.Msg
    | TimeUpdate TimeInput.Msg
    | SearchDirectionUpdate SearchDirection
    | SwitchInputs
    | MapUpdate RailViz.Msg
    | ConnectionsUpdate Connections.Msg
    | SearchConnections
    | PrepareSelectConnection Int
    | SelectConnection Int
    | StoreConnectionListScrollPos Msg Float
    | ConnectionDetailsUpdate ConnectionDetails.Msg
    | TripDetailsUpdate ConnectionDetails.Msg
    | ConnectionDetailsGoBack
    | TripDetailsGoBack
    | CloseConnectionDetails
    | PrepareSelectTrip Int
    | LoadTrip TripId
    | SelectTripId TripId
    | TripToConnectionError TripId ApiError
    | TripToConnectionResponse TripId Connection
    | ScheduleInfoError ApiError
    | ScheduleInfoResponse ScheduleInfo
    | Deb (Debounce.Msg Msg)
    | SetLocale Localization
    | NavigateTo Route
    | ReplaceLocation Route
    | SetRoutingResponses (List ( String, String ))
    | UpdateCurrentTime Time
    | SetSimulationTime Time
    | SetTimeOffset Time Time
    | StationEventsUpdate StationEvents.Msg
    | PrepareSelectStation Station (Maybe Date)
    | SelectStation String (Maybe Date)
    | StationEventsGoBack
    | ShowStationDetails String
    | ToggleOverlay
    | CloseSubOverlay
    | StationSearchUpdate Typeahead.Msg
    | HandleRailVizError Json.Encode.Value
    | ClearRailVizError
    | TripSearchUpdate TripSearch.Msg
    | ShowTripSearch
    | ToggleTripSearch
    | HandleRailVizPermalink Float Float Float Date
    | SimTimePickerUpdate SimTimePicker.Msg


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        NoOp ->
            ( model, Cmd.none )

        Reset ->
            ( model, Cmd.none )

        MapUpdate msg_ ->
            let
                ( railViz_, railVizCmd ) =
                    RailViz.update msg_ model.railViz

                ( model1, cmd1 ) =
                    ( { model | railViz = railViz_ }, Cmd.map MapUpdate railVizCmd )

                ( model2, cmd2 ) =
                    case msg_ of
                        RailViz.ToggleSimTimePicker ->
                            update (SimTimePickerUpdate (SimTimePicker.Toggle (getCurrentDate model1) (model1.timeOffset /= 0))) model1

                        _ ->
                            model1 ! []
            in
                model2 ! [ cmd1, cmd2 ]

        FromLocationUpdate msg_ ->
            let
                ( m, c ) =
                    Typeahead.update msg_ model.fromLocation
            in
                { model | fromLocation = m }
                    ! [ Cmd.map FromLocationUpdate c ]
                    |> checkTypeaheadUpdate msg_

        ToLocationUpdate msg_ ->
            let
                ( m, c ) =
                    Typeahead.update msg_ model.toLocation
            in
                { model | toLocation = m }
                    ! [ Cmd.map ToLocationUpdate c ]
                    |> checkTypeaheadUpdate msg_

        FromTransportsUpdate msg_ ->
            { model | fromTransports = TagList.update msg_ model.fromTransports }
                ! []
                |> checkRoutingRequest

        ToTransportsUpdate msg_ ->
            { model | toTransports = TagList.update msg_ model.toTransports }
                ! []
                |> checkRoutingRequest

        DateUpdate msg_ ->
            { model | date = Calendar.update msg_ model.date }
                ! []
                |> checkRoutingRequest

        TimeUpdate msg_ ->
            { model | time = TimeInput.update msg_ model.time }
                ! []
                |> checkRoutingRequest

        SearchDirectionUpdate dir ->
            { model | searchDirection = dir }
                ! []
                |> checkRoutingRequest

        SwitchInputs ->
            { model
                | fromLocation = model.toLocation
                , toLocation = model.fromLocation
            }
                ! []
                |> checkRoutingRequest

        ConnectionsUpdate msg_ ->
            let
                ( m, c ) =
                    Connections.update msg_ model.connections
            in
                ( { model | connections = m }, Cmd.map ConnectionsUpdate c )

        SearchConnections ->
            let
                routingRequest =
                    buildRoutingRequest model

                fromName =
                    Typeahead.getSelectedSuggestion model.fromLocation
                        |> Maybe.map Typeahead.getShortSuggestionName

                toName =
                    Typeahead.getSelectedSuggestion model.toLocation
                        |> Maybe.map Typeahead.getShortSuggestionName

                searchReq =
                    Connections.Search
                        Connections.ReplaceResults
                        routingRequest
                        fromName
                        toName

                ( m, c ) =
                    Connections.update searchReq model.connections
            in
                { model
                    | connections = m
                    , currentRoutingRequest = Just routingRequest
                    , connectionListScrollPos = 0
                }
                    ! [ Cmd.map ConnectionsUpdate c ]

        PrepareSelectConnection idx ->
            let
                selectMsg =
                    NavigateTo (ConnectionDetails idx)
            in
                model
                    ! [ Task.attempt
                            (\r ->
                                case r of
                                    Ok pos ->
                                        StoreConnectionListScrollPos selectMsg pos

                                    Err _ ->
                                        selectMsg
                            )
                            (Scroll.y "connections")
                      ]

        SelectConnection idx ->
            let
                ( m, c ) =
                    selectConnection model idx
            in
                m
                    ! [ c
                      , Task.attempt noop <| Scroll.toTop "overlay-content"
                      , Task.attempt noop <| Scroll.toTop "connection-journey"
                      ]

        StoreConnectionListScrollPos msg_ pos ->
            let
                newModel =
                    { model | connectionListScrollPos = pos }
            in
                update msg_ newModel

        ConnectionDetailsUpdate msg_ ->
            let
                ( m, c ) =
                    case model.connectionDetails of
                        Just state ->
                            let
                                ( m_, c_ ) =
                                    ConnectionDetails.update msg_ state
                            in
                                ( Just m_, c_ )

                        Nothing ->
                            Nothing ! []
            in
                { model | connectionDetails = m }
                    ! [ Cmd.map ConnectionDetailsUpdate c ]

        TripDetailsUpdate msg_ ->
            let
                ( m, c ) =
                    case model.tripDetails of
                        Just state ->
                            let
                                ( m_, c_ ) =
                                    ConnectionDetails.update msg_ state
                            in
                                ( Just m_, c_ )

                        Nothing ->
                            Nothing ! []
            in
                { model | tripDetails = m }
                    ! [ Cmd.map TripDetailsUpdate c ]

        CloseConnectionDetails ->
            closeSelectedConnection model

        ConnectionDetailsGoBack ->
            update (NavigateTo Connections) model

        TripDetailsGoBack ->
            model ! [ Navigation.back 1 ]

        PrepareSelectTrip tripIdx ->
            selectConnectionTrip model tripIdx

        LoadTrip tripId ->
            loadTripById model tripId

        SelectTripId tripId ->
            update (NavigateTo (tripDetailsRoute tripId)) model

        TripToConnectionError tripId err ->
            setFullTripError model tripId err

        TripToConnectionResponse tripId connection ->
            setFullTripConnection model tripId connection

        ScheduleInfoError err ->
            let
                ( connections_, _ ) =
                    Connections.update (Connections.SetError err) model.connections

                ( newConnections, c ) =
                    Connections.update (Connections.UpdateScheduleInfo Nothing) connections_

                newDate =
                    Calendar.update (Calendar.SetValidRange Nothing) model.date
            in
                { model
                    | scheduleInfo = Nothing
                    , connections = newConnections
                    , date = newDate
                }
                    ! [ Cmd.map ConnectionsUpdate c ]

        ScheduleInfoResponse si ->
            let
                ( connections_, connCmd ) =
                    Connections.update (Connections.UpdateScheduleInfo (Just si)) model.connections

                newDate =
                    Calendar.update (Calendar.SetValidRange (Just ( si.begin, si.end ))) model.date

                ( newTripSearch, _ ) =
                    TripSearch.update (TripSearch.UpdateScheduleInfo si) model.tripSearch

                ( newSimTimePicker, _ ) =
                    SimTimePicker.update (SimTimePicker.UpdateScheduleInfo si) model.simTimePicker

                model1 =
                    { model
                        | scheduleInfo = Just si
                        , connections = connections_
                        , date = newDate
                        , tripSearch = newTripSearch
                        , simTimePicker = newSimTimePicker
                    }

                currentDate =
                    getCurrentDate model1

                currentTimeInSchedule =
                    Date.Extra.Compare.is3 Date.Extra.Compare.BetweenOpen currentDate si.begin si.end

                ( model2, cmd1 ) =
                    if currentTimeInSchedule then
                        ( model1, Cmd.none )
                    else
                        update
                            (SetSimulationTime (Date.toTime (combineDateTime si.begin currentDate)))
                            { model1 | updateSearchTime = True }
            in
                model2 ! [ Cmd.map ConnectionsUpdate connCmd, cmd1 ]

        Deb a ->
            Debounce.update debounceCfg a model

        SetLocale newLocale ->
            let
                date_ =
                    Calendar.update (Calendar.SetDateConfig newLocale.dateConfig) model.date

                ( tripSearch_, _ ) =
                    TripSearch.update (TripSearch.SetLocale newLocale) model.tripSearch

                ( simTimePicker_, _ ) =
                    SimTimePicker.update (SimTimePicker.SetLocale newLocale) model.simTimePicker
            in
                { model
                    | locale = newLocale
                    , date = date_
                    , tripSearch = tripSearch_
                    , simTimePicker = simTimePicker_
                }
                    ! []

        NavigateTo route ->
            model ! [ Navigation.newUrl (toUrl route) ]

        ReplaceLocation route ->
            model ! [ Navigation.modifyUrl (toUrl route) ]

        SetRoutingResponses files ->
            let
                parsed =
                    List.map
                        (\( name, json ) -> ( name, Decode.decodeString decodeRoutingResponse json ))
                        files

                valid =
                    List.filterMap
                        (\( name, result ) ->
                            case result of
                                Ok routingResponse ->
                                    Just ( name, routingResponse )

                                Err _ ->
                                    Nothing
                        )
                        parsed

                errors =
                    List.filterMap
                        (\( name, result ) ->
                            case result of
                                Err msg_ ->
                                    Just ( name, msg_ )

                                Ok _ ->
                                    Nothing
                        )
                        parsed
            in
                update
                    (ConnectionsUpdate (Connections.SetRoutingResponses valid))
                    model

        UpdateCurrentTime time ->
            { model | currentTime = Date.fromTime time } ! []

        SetSimulationTime simulationTime ->
            model
                ! [ Task.perform (SetTimeOffset simulationTime) Time.now ]

        SetTimeOffset simulationTime currentTime ->
            let
                offset =
                    simulationTime - currentTime

                model1 =
                    { model
                        | currentTime = Date.fromTime currentTime
                        , timeOffset = offset
                        , updateSearchTime = False
                    }

                newDate =
                    getCurrentDate model1

                updateSearchTime =
                    model.updateSearchTime

                ( model2, cmds1 ) =
                    update (MapUpdate (RailViz.SetTimeOffset offset)) model1

                ( model3, cmds2 ) =
                    if updateSearchTime then
                        update (DateUpdate (Calendar.InitDate True newDate)) model2
                    else
                        model2 ! []

                ( model4, cmds3 ) =
                    if updateSearchTime then
                        update (TimeUpdate (TimeInput.InitDate True newDate)) model3
                    else
                        model3 ! []

                ( model5, cmds4 ) =
                    if updateSearchTime then
                        update (TripSearchUpdate (TripSearch.SetTime newDate)) model4
                    else
                        model4 ! []
            in
                model5
                    ! [ cmds1
                      , cmds2
                      , cmds3
                      , cmds4
                      , Port.setTimeOffset offset
                      ]

        StationEventsUpdate msg_ ->
            let
                ( m, c ) =
                    case model.stationEvents of
                        Just state ->
                            let
                                ( m_, c_ ) =
                                    StationEvents.update msg_ state
                            in
                                ( Just m_, c_ )

                        Nothing ->
                            Nothing ! []
            in
                { model | stationEvents = m }
                    ! [ Cmd.map StationEventsUpdate c ]

        PrepareSelectStation station maybeDate ->
            case maybeDate of
                Just date ->
                    update (NavigateTo (StationEventsAt station.id date)) model

                Nothing ->
                    update (NavigateTo (StationEvents station.id)) model

        SelectStation stationId maybeDate ->
            let
                ( model_, cmds_ ) =
                    closeSubOverlay model

                date =
                    Maybe.withDefault (getCurrentDate model_) maybeDate

                ( m, c ) =
                    StationEvents.init model_.apiEndpoint stationId date
            in
                { model_
                    | stationEvents = Just m
                    , subView = Just StationEventsView
                    , overlayVisible = True
                }
                    ! [ cmds_, Cmd.map StationEventsUpdate c ]

        StationEventsGoBack ->
            model ! [ Navigation.back 1 ]

        ShowStationDetails id ->
            update (NavigateTo (StationEvents id)) model

        ToggleOverlay ->
            { model | overlayVisible = not model.overlayVisible } ! []

        CloseSubOverlay ->
            let
                ( model1, cmds1 ) =
                    closeSubOverlay model

                route =
                    case model1.selectedConnectionIdx of
                        Nothing ->
                            Connections

                        Just idx ->
                            ConnectionDetails idx

                ( model2, cmds2 ) =
                    update (NavigateTo route) model1
            in
                model2 ! [ cmds1, cmds2 ]

        StationSearchUpdate msg_ ->
            let
                ( m, c1 ) =
                    Typeahead.update msg_ model.stationSearch

                c2 =
                    case msg_ of
                        Typeahead.ItemSelected ->
                            case Typeahead.getSelectedSuggestion m of
                                Just (Typeahead.StationSuggestion station) ->
                                    Navigation.newUrl (toUrl (StationEvents station.id))

                                Just (Typeahead.AddressSuggestion address) ->
                                    RailViz.flyTo address.pos Nothing True

                                Nothing ->
                                    Cmd.none

                        _ ->
                            Cmd.none
            in
                { model | stationSearch = m } ! [ Cmd.map StationSearchUpdate c1, c2 ]

        HandleRailVizError json ->
            let
                apiError =
                    case Decode.decodeValue Api.decodeErrorResponse json of
                        Ok value ->
                            MotisError value

                        Err msg ->
                            case Decode.decodeValue Decode.string json of
                                Ok errType ->
                                    case errType of
                                        "NetworkError" ->
                                            NetworkError

                                        "TimeoutError" ->
                                            TimeoutError

                                        _ ->
                                            DecodeError errType

                                Err msg_ ->
                                    DecodeError msg_

                ( railVizModel, _ ) =
                    RailViz.update (RailViz.SetApiError (Just apiError)) model.railViz
            in
                { model | railViz = railVizModel } ! []

        ClearRailVizError ->
            let
                ( railVizModel, _ ) =
                    RailViz.update (RailViz.SetApiError Nothing) model.railViz
            in
                { model | railViz = railVizModel } ! []

        TripSearchUpdate msg_ ->
            let
                ( m, c ) =
                    TripSearch.update msg_ model.tripSearch
            in
                { model | tripSearch = m }
                    ! [ Cmd.map TripSearchUpdate c ]

        ShowTripSearch ->
            let
                model1 =
                    { model
                        | subView = Just TripSearchView
                        , overlayVisible = True
                    }

                ( model2, cmd1 ) =
                    setRailVizFilter model1 Nothing
            in
                model2
                    ! [ cmd1
                      , Task.attempt noop (Dom.focus "trip-search-trainnr-input")
                      ]

        ToggleTripSearch ->
            case model.subView of
                Just TripSearchView ->
                    update (NavigateTo Connections) model

                _ ->
                    update (NavigateTo TripSearchRoute) model

        HandleRailVizPermalink lat lng zoom date ->
            let
                model1 =
                    { model | overlayVisible = False }

                ( model2, cmd1 ) =
                    update (SetSimulationTime (Date.toTime date)) model1

                pos =
                    { lat = lat, lng = lng }

                cmd2 =
                    RailViz.flyTo pos (Just zoom) False
            in
                model2 ! [ cmd1, cmd2 ]

        SimTimePickerUpdate msg_ ->
            let
                ( m, c ) =
                    SimTimePicker.update msg_ model.simTimePicker

                ( model1, cmd1 ) =
                    ( { model | simTimePicker = m }, Cmd.map SimTimePickerUpdate c )

                ( model2, cmd2 ) =
                    case msg_ of
                        SimTimePicker.SetSimulationTime ->
                            let
                                pickedTime =
                                    SimTimePicker.getSelectedSimTime model1.simTimePicker
                            in
                                update (SetSimulationTime pickedTime) model1

                        SimTimePicker.DisableSimMode ->
                            let
                                currentTime =
                                    Date.toTime model1.currentTime
                            in
                                update (SetTimeOffset currentTime currentTime) model1

                        _ ->
                            model1 ! []
            in
                model2 ! [ cmd1, cmd2 ]


buildRoutingRequest : Model -> IntermodalRoutingRequest
buildRoutingRequest model =
    let
        default =
            IntermodalStation (Station "" "" (Position 0 0))

        toIntermodalLocation typeahead =
            case (Typeahead.getSelectedSuggestion typeahead) of
                Just (Typeahead.StationSuggestion s) ->
                    IntermodalStation s

                Just (Typeahead.AddressSuggestion a) ->
                    IntermodalPosition a.pos

                Nothing ->
                    default

        tagToMode tag =
            case tag of
                WalkTag o ->
                    Just (Intermodal.Foot { maxDuration = o.maxDuration })

                BikeTag o ->
                    Just (Intermodal.Bike { maxDuration = o.maxDuration })

                CarTag o ->
                    Nothing

        fromLocation =
            toIntermodalLocation model.fromLocation

        toLocation =
            toIntermodalLocation model.toLocation

        fromModes =
            TagList.getSelectedTags model.fromTransports
                |> List.filterMap tagToMode

        toModes =
            TagList.getSelectedTags model.toTransports
                |> List.filterMap tagToMode

        minConnectionCount =
            5
    in
        IntermodalRoutingRequest.initialRequest
            minConnectionCount
            fromLocation
            toLocation
            fromModes
            toModes
            (combineDateTime model.date.date model.time.date)
            model.searchDirection


isCompleteQuery : Model -> Bool
isCompleteQuery model =
    let
        fromLocation =
            Typeahead.getSelectedSuggestion model.fromLocation

        toLocation =
            Typeahead.getSelectedSuggestion model.toLocation
    in
        isJust fromLocation && isJust toLocation


checkRoutingRequest : ( Model, Cmd Msg ) -> ( Model, Cmd Msg )
checkRoutingRequest ( model, cmds ) =
    let
        completeQuery =
            isCompleteQuery model

        newRoutingRequest =
            buildRoutingRequest model

        requestChanged =
            case model.currentRoutingRequest of
                Just currentRequest ->
                    newRoutingRequest /= currentRequest

                Nothing ->
                    True

        fromLocation =
            Typeahead.saveSelection model.fromLocation

        toLocation =
            Typeahead.saveSelection model.toLocation

        fromTransports =
            TagList.saveSelections model.fromTransports

        toTransports =
            TagList.saveSelections model.toTransports
    in
        if completeQuery && requestChanged then
            model
                ! [ cmds
                  , Debounce.debounceCmd debounceCfg <| SearchConnections
                  , Port.localStorageSet ("motis.routing.from_location" => fromLocation)
                  , Port.localStorageSet ("motis.routing.to_location" => toLocation)
                  , Port.localStorageSet ("motis.routing.from_transports" => fromTransports)
                  , Port.localStorageSet ("motis.routing.to_transports" => toTransports)
                  ]
        else
            ( model, cmds )


checkTypeaheadUpdate : Typeahead.Msg -> ( Model, Cmd Msg ) -> ( Model, Cmd Msg )
checkTypeaheadUpdate msg ( model, cmds ) =
    let
        model_ =
            case msg of
                Typeahead.StationSuggestionsError err ->
                    let
                        ( m, _ ) =
                            Connections.update (Connections.SetError err) model.connections
                    in
                        { model | connections = m }

                _ ->
                    model
    in
        checkRoutingRequest ( model_, cmds )


debounceCfg : Debounce.Config Model Msg
debounceCfg =
    Debounce.config
        .debounce
        (\model s -> { model | debounce = s })
        Deb
        700


noop : a -> Msg
noop =
    \_ -> NoOp


selectConnectionTrip : Model -> Int -> ( Model, Cmd Msg )
selectConnectionTrip model tripIdx =
    let
        journey =
            model.selectedConnectionIdx
                |> Maybe.andThen (Connections.getJourney model.connections)

        trip =
            journey
                |> Maybe.andThen (\j -> j.trains !! tripIdx)
                |> Maybe.andThen .trip
    in
        case trip of
            Just tripId ->
                update (NavigateTo (tripDetailsRoute tripId)) model

            Nothing ->
                model ! []


loadTripById : Model -> TripId -> ( Model, Cmd Msg )
loadTripById model tripId =
    let
        tripDetails =
            ConnectionDetails.init True True (Just tripId) Nothing
    in
        { model
            | stationEvents = Nothing
            , tripDetails = Just tripDetails
            , subView = Just TripDetailsView
            , overlayVisible = True
        }
            ! [ sendTripRequest model.apiEndpoint tripId
              , Task.attempt noop <| Scroll.toTop "sub-overlay-content"
              , Task.attempt noop <| Scroll.toTop "sub-connection-journey"
              ]


setFullTripConnection : Model -> TripId -> Connection -> ( Model, Cmd Msg )
setFullTripConnection model tripId connection =
    let
        journey =
            toJourney connection

        tripJourney =
            { journey | isSingleCompleteTrip = True }

        ( tripDetails, _ ) =
            case model.tripDetails of
                Just td ->
                    ConnectionDetails.update (ConnectionDetails.SetJourney tripJourney True) td

                Nothing ->
                    ConnectionDetails.init True True (Just tripId) (Just tripJourney) ! []

        ( model_, cmds ) =
            setRailVizFilter model (Just [ tripId ])
    in
        { model_
            | tripDetails = Just tripDetails
            , subView = Just TripDetailsView
        }
            ! [ MapConnectionDetails.setConnectionFilter tripJourney
              , Task.attempt noop <| Scroll.toTop "sub-overlay-content"
              , Task.attempt noop <| Scroll.toTop "sub-connection-journey"
              , cmds
              ]


setFullTripError : Model -> TripId -> ApiError -> ( Model, Cmd Msg )
setFullTripError model tripId error =
    case model.tripDetails of
        Just td ->
            let
                ( tripDetails, _ ) =
                    ConnectionDetails.update (ConnectionDetails.SetApiError error) td
            in
                setRailVizFilter { model | tripDetails = Just tripDetails } Nothing

        Nothing ->
            model ! []


getCurrentTime : Model -> Time
getCurrentTime model =
    (Date.toTime model.currentTime) + model.timeOffset


getCurrentDate : Model -> Date
getCurrentDate model =
    Date.fromTime (getCurrentTime model)


getLocale : String -> Localization
getLocale language =
    case String.toLower language of
        "de" ->
            deLocalization

        "en" ->
            enLocalization

        _ ->
            deLocalization



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.batch
        [ Sub.map FromTransportsUpdate (TagList.subscriptions model.fromTransports)
        , Sub.map ToTransportsUpdate (TagList.subscriptions model.toTransports)
        , Sub.map MapUpdate (RailViz.subscriptions model.railViz)
        , Port.setRoutingResponses SetRoutingResponses
        , Port.showStationDetails ShowStationDetails
        , Port.showTripDetails SelectTripId
        , Port.setSimulationTime SetSimulationTime
        , Port.handleRailVizError HandleRailVizError
        , Port.clearRailVizError (always ClearRailVizError)
        , Time.every (2 * Time.second) UpdateCurrentTime
        ]



-- VIEW


view : Model -> Html Msg
view model =
    let
        permalink =
            getPermalink model
    in
        div [ class "app" ] <|
            [ Html.map MapUpdate (RailViz.view model.locale permalink model.railViz)
            , lazy overlayView model
            , lazy stationSearchView model
            , lazy simTimePickerView model
            ]


overlayView : Model -> Html Msg
overlayView model =
    let
        mainOverlayContent =
            case model.connectionDetails of
                Nothing ->
                    searchView model

                Just c ->
                    connectionDetailsView model.locale (getCurrentDate model) c

        subOverlayContent =
            case model.subView of
                Just TripDetailsView ->
                    Maybe.map (tripDetailsView model.locale (getCurrentDate model)) model.tripDetails

                Just StationEventsView ->
                    Maybe.map (stationView model.locale) model.stationEvents

                Just TripSearchView ->
                    Just (tripSearchView model.locale model.tripSearch)

                Nothing ->
                    Nothing

        subOverlay =
            subOverlayContent
                |> Maybe.map
                    (\c ->
                        div [ class "sub-overlay" ]
                            [ div [ id "sub-overlay-content" ] c
                            , div [ id "sub-overlay-close", onClick CloseSubOverlay ]
                                [ i [ class "icon" ] [ text "close" ] ]
                            ]
                    )
                |> Maybe.withDefault (div [ class "sub-overlay hidden" ] [ div [ id "sub-overlay-content" ] [] ])
    in
        div
            [ classList
                [ "overlay-container" => True
                , "hidden" => not model.overlayVisible
                ]
            ]
            [ div [ class "overlay" ]
                [ div [ id "overlay-content" ]
                    mainOverlayContent
                , subOverlay
                ]
            , div [ class "overlay-tabs" ]
                [ div [ class "overlay-toggle", onClick ToggleOverlay ]
                    [ i [ class "icon" ] [ text "arrow_drop_down" ] ]
                , div
                    [ classList
                        [ "trip-search-toggle" => True
                        , "enabled" => (model.subView == Just TripSearchView)
                        ]
                    , onClick ToggleTripSearch
                    ]
                    [ i [ class "icon" ] [ text "train" ] ]
                ]
            ]


searchView : Model -> List (Html Msg)
searchView model =
    [ div [ id "search" ]
        [ div [ class "pure-g gutters" ]
            [ div [ class "pure-u-1 pure-u-sm-14-24 from-location" ]
                [ Html.map FromLocationUpdate <|
                    Typeahead.view 1 model.locale.t.search.start (Just "place") model.fromLocation
                , (swapLocationsView model)
                ]
            , div [ class "pure-u-1 pure-u-sm-10-24" ]
                [ Html.map FromTransportsUpdate <|
                    TagList.view model.locale model.locale.t.search.startTransports model.fromTransports
                ]
            ]
        , div [ class "pure-g gutters" ]
            [ div [ class "pure-u-1 pure-u-sm-14-24 to-location" ]
                [ Html.map ToLocationUpdate <|
                    Typeahead.view 2 model.locale.t.search.destination (Just "place") model.toLocation
                ]
            , div [ class "pure-u-1 pure-u-sm-10-24" ]
                [ Html.map ToTransportsUpdate <|
                    TagList.view model.locale model.locale.t.search.destinationTransports model.toTransports
                ]
            ]
        , div [ class "pure-g gutters" ]
            [ div [ class "pure-u-1 pure-u-sm-10-24" ]
                [ Html.map DateUpdate <|
                    Calendar.view 3 model.locale.t.search.date model.date
                ]
            , div [ class "pure-u-1 pure-u-sm-10-24" ]
                [ Html.map TimeUpdate <|
                    TimeInput.view 4 model.locale.t.search.time model.time
                ]
            , div [ class "pure-u-1 pure-u-sm-4-24 time-option" ]
                (searchDirectionView model)
            ]
        ]
    , div [ id "connections" ]
        [ lazy3 Connections.view connectionConfig model.locale model.connections ]
    ]


searchDirectionView : Model -> List (Html Msg)
searchDirectionView model =
    [ div []
        [ input
            [ type_ "radio"
            , id "search-forward"
            , name "time-option"
            , checked (model.searchDirection == Forward)
            , onClick (SearchDirectionUpdate Forward)
            ]
            []
        , label [ for "search-forward" ] [ text model.locale.t.search.departure ]
        ]
    , div []
        [ input
            [ type_ "radio"
            , id "search-backward"
            , name "time-option"
            , checked (model.searchDirection == Backward)
            , onClick (SearchDirectionUpdate Backward)
            ]
            []
        , label [ for "search-backward" ] [ text model.locale.t.search.arrival ]
        ]
    ]


swapLocationsView : Model -> Html Msg
swapLocationsView model =
    div
        [ class "swap-locations-btn" ]
        [ label
            [ class "gb-button gb-button-small gb-button-circle gb-button-outline gb-button-PRIMARY_COLOR disable-select" ]
            [ input
                [ type_ "checkbox"
                , onClick SwitchInputs
                ]
                []
            , i [ class "icon" ] [ text "swap_vert" ]
            ]
        ]


stationSearchView : Model -> Html Msg
stationSearchView model =
    div
        [ id "station-search"
        , classList
            [ "overlay-hidden" => not model.overlayVisible
            ]
        ]
        [ Html.map StationSearchUpdate <|
            Typeahead.view 10 "" (Just "place") model.stationSearch
        ]


simTimePickerView : Model -> Html Msg
simTimePickerView model =
    div [ class "sim-time-picker-container" ]
        [ Html.map SimTimePickerUpdate <|
            SimTimePicker.view model.locale model.simTimePicker
        ]


connectionConfig : Connections.Config Msg
connectionConfig =
    Connections.Config
        { internalMsg = ConnectionsUpdate
        , selectMsg = PrepareSelectConnection
        }


connectionDetailsView : Localization -> Date -> ConnectionDetails.State -> List (Html Msg)
connectionDetailsView locale currentTime state =
    [ ConnectionDetails.view connectionDetailsConfig locale currentTime state ]


connectionDetailsConfig : ConnectionDetails.Config Msg
connectionDetailsConfig =
    ConnectionDetails.Config
        { internalMsg = ConnectionDetailsUpdate
        , selectTripMsg = PrepareSelectTrip
        , selectStationMsg = PrepareSelectStation
        , goBackMsg = ConnectionDetailsGoBack
        }


tripDetailsView : Localization -> Date -> ConnectionDetails.State -> List (Html Msg)
tripDetailsView locale currentTime state =
    [ ConnectionDetails.view tripDetailsConfig locale currentTime state ]


tripDetailsConfig : ConnectionDetails.Config Msg
tripDetailsConfig =
    ConnectionDetails.Config
        { internalMsg = TripDetailsUpdate
        , selectTripMsg = PrepareSelectTrip
        , selectStationMsg = PrepareSelectStation
        , goBackMsg = TripDetailsGoBack
        }


stationView : Localization -> StationEvents.Model -> List (Html Msg)
stationView locale model =
    [ StationEvents.view stationConfig locale model ]


stationConfig : StationEvents.Config Msg
stationConfig =
    StationEvents.Config
        { internalMsg = StationEventsUpdate
        , selectTripMsg = SelectTripId
        , goBackMsg = StationEventsGoBack
        }


tripSearchView : Localization -> TripSearch.Model -> List (Html Msg)
tripSearchView locale model =
    [ TripSearch.view tripSearchConfig locale model ]


tripSearchConfig : TripSearch.Config Msg
tripSearchConfig =
    TripSearch.Config
        { internalMsg = TripSearchUpdate
        , selectTripMsg = SelectTripId
        , selectStationMsg = PrepareSelectStation
        }


getPermalink : Model -> String
getPermalink model =
    let
        date =
            getCurrentDate model

        urlBase =
            getBaseUrl model.programFlags date
    in
        case model.subView of
            Just TripDetailsView ->
                case (Maybe.andThen ConnectionDetails.getTripId model.tripDetails) of
                    Just tripId ->
                        urlBase
                            ++ toUrl
                                (TripDetails
                                    tripId.station_id
                                    tripId.train_nr
                                    tripId.time
                                    tripId.target_station_id
                                    tripId.target_time
                                    tripId.line_id
                                )

                    Nothing ->
                        urlBase

            Just StationEventsView ->
                case model.stationEvents of
                    Just stationEvents ->
                        toUrl
                            (StationEventsAt
                                (StationEvents.getStationId stationEvents)
                                date
                            )

                    Nothing ->
                        urlBase

            Just TripSearchView ->
                urlBase ++ (toUrl TripSearchRoute)

            Nothing ->
                RailViz.getMapPermalink model.railViz


getBaseUrl : ProgramFlags -> Date -> String
getBaseUrl flags date =
    let
        timestamp =
            unixTime date

        params1 =
            [ "time" => (toString timestamp) ]

        params2 =
            case flags.langParam of
                Just lang ->
                    ("lang" => lang) :: params1

                Nothing ->
                    params1

        params3 =
            case flags.motisParam of
                Just motis ->
                    ("motis" => motis) :: params2

                Nothing ->
                    params2

        urlBase =
            params3
                |> List.map (\( k, v ) -> (encodeUri k) ++ "=" ++ (encodeUri v))
                |> String.join "&"
                |> \s -> "?" ++ s
    in
        urlBase



-- REQUESTS


requestScheduleInfo : String -> Cmd Msg
requestScheduleInfo remoteAddress =
    Api.sendRequest
        remoteAddress
        decodeScheduleInfoResponse
        ScheduleInfoError
        ScheduleInfoResponse
        ScheduleInfo.request


sendTripRequest : String -> TripId -> Cmd Msg
sendTripRequest remoteAddress tripId =
    Api.sendRequest
        remoteAddress
        decodeTripToConnectionResponse
        (TripToConnectionError tripId)
        (TripToConnectionResponse tripId)
        (encodeTripToConnection tripId)



-- NAVIGATION


locationToMsg : Location -> Msg
locationToMsg location =
    case (UrlParser.parseHash urlParser location) of
        Just route ->
            routeToMsg route

        Nothing ->
            ReplaceLocation Connections


routeToMsg : Route -> Msg
routeToMsg route =
    case route of
        Connections ->
            CloseConnectionDetails

        ConnectionDetails idx ->
            SelectConnection idx

        TripDetails station trainNr time targetStation targetTime lineId ->
            LoadTrip
                { station_id = station
                , train_nr = trainNr
                , time = time
                , target_station_id = targetStation
                , target_time = targetTime
                , line_id = lineId
                }

        StationEvents stationId ->
            SelectStation stationId Nothing

        StationEventsAt stationId date ->
            SelectStation stationId (Just date)

        SimulationTime time ->
            SetSimulationTime (Date.toTime time)

        TripSearchRoute ->
            ShowTripSearch

        RailVizPermalink lat lng zoom date ->
            HandleRailVizPermalink lat lng zoom date


selectConnection : Model -> Int -> ( Model, Cmd Msg )
selectConnection model idx =
    let
        journey =
            Connections.getJourney model.connections idx

        ( newConnections, _ ) =
            Connections.update Connections.ResetNew model.connections
    in
        case journey of
            Just j ->
                let
                    trips =
                        journeyTrips j

                    ( model_, cmds ) =
                        setRailVizFilter model (Just trips)
                in
                    { model_
                        | connectionDetails =
                            Maybe.map (ConnectionDetails.init False False Nothing) (Just journey)
                        , connections = newConnections
                        , selectedConnectionIdx = Just idx
                        , tripDetails = Nothing
                        , stationEvents = Nothing
                        , subView = Nothing
                    }
                        ! [ MapConnectionDetails.setConnectionFilter j
                          , cmds
                          ]

            Nothing ->
                update (ReplaceLocation Connections) model


closeSelectedConnection : Model -> ( Model, Cmd Msg )
closeSelectedConnection model =
    let
        ( model_, cmds ) =
            setRailVizFilter model Nothing
    in
        { model_
            | connectionDetails = Nothing
            , selectedConnectionIdx = Nothing
            , tripDetails = Nothing
            , stationEvents = Nothing
            , subView = Nothing
        }
            ! [ Task.attempt noop <| Scroll.toY "connections" model.connectionListScrollPos
              , cmds
              ]


closeSubOverlay : Model -> ( Model, Cmd Msg )
closeSubOverlay model =
    let
        ( model_, cmds ) =
            setRailVizFilter model Nothing
    in
        ( { model_
            | tripDetails = Nothing
            , stationEvents = Nothing
            , subView = Nothing
          }
        , cmds
        )


setRailVizFilter : Model -> Maybe (List TripId) -> ( Model, Cmd Msg )
setRailVizFilter model filterTrips =
    model ! [ Port.setRailVizFilter filterTrips ]


journeyTrips : Journey -> List TripId
journeyTrips journey =
    journey.connection.trips
        |> List.map .id
