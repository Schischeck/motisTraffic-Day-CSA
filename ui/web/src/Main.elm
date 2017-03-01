module Main exposing (..)

import Widgets.TimeInput as TimeInput
import Widgets.TagList as TagList
import Widgets.Calendar as Calendar
import Widgets.Typeahead as Typeahead
import Widgets.Map.RailViz as RailViz
import Widgets.Map.ConnectionDetails as MapConnectionDetails
import Widgets.Connections as Connections
import Widgets.ConnectionDetails as ConnectionDetails
import Widgets.StationEvents as StationEvents
import Widgets.TripSearch as TripSearch
import Data.ScheduleInfo.Types exposing (ScheduleInfo)
import Data.ScheduleInfo.Request as ScheduleInfo
import Data.ScheduleInfo.Decode exposing (decodeScheduleInfoResponse)
import Data.Routing.Types exposing (RoutingRequest, SearchDirection(..))
import Data.Routing.Request as RoutingRequest
import Data.Routing.Decode exposing (decodeRoutingResponse)
import Data.Connection.Types exposing (Station, Position, TripId, Connection)
import Data.Journey.Types exposing (Journey, toJourney)
import Data.Lookup.Request exposing (encodeTripToConnection)
import Data.Lookup.Decode exposing (decodeTripToConnectionResponse)
import Util.List exposing ((!!))
import Util.Api as Api exposing (ApiError(..))
import Util.Date exposing (combineDateTime)
import Util.Core exposing ((=>))
import Date.Extra.Compare
import Localization.Base exposing (..)
import Localization.De exposing (..)
import Localization.En exposing (..)
import Html exposing (..)
import Html.Attributes exposing (..)
import Html.Events exposing (..)
import Html.Lazy exposing (..)
import Dom.Scroll as Scroll
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
    , currentRoutingRequest : Maybe RoutingRequest
    , debounce : Debounce.State
    , connectionListScrollPos : Float
    , currentTime : Date
    , timeOffset : Float
    , overlayVisible : Bool
    , stationSearch : Typeahead.Model
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

        ( dateModel, dateCmd ) =
            Calendar.init locale.dateConfig

        ( timeModel, timeCmd ) =
            TimeInput.init

        ( mapModel, mapCmd ) =
            RailViz.init remoteAddress

        ( fromLocationModel, fromLocationCmd ) =
            Typeahead.init remoteAddress "Willy-Brandt-Platz, Darmstadt"

        ( toLocationModel, toLocationCmd ) =
            Typeahead.init remoteAddress "Frankfurt(M)Hauptwache"

        ( stationSearchModel, stationSearchCmd ) =
            Typeahead.init remoteAddress ""

        ( tripSearchModel, tripSearchCmd ) =
            TripSearch.init remoteAddress locale

        initialModel =
            { fromLocation = fromLocationModel
            , toLocation = toLocationModel
            , fromTransports = TagList.init
            , toTransports = TagList.init
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


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        NoOp ->
            ( model, Cmd.none )

        Reset ->
            ( model, Cmd.none )

        MapUpdate msg_ ->
            let
                ( m, c ) =
                    RailViz.update msg_ model.railViz
            in
                ( { model | railViz = m }, Cmd.map MapUpdate c )

        FromLocationUpdate msg_ ->
            let
                ( m, c ) =
                    Typeahead.update msg_ model.fromLocation
            in
                checkTypeaheadUpdate msg_ ( { model | fromLocation = m }, Cmd.map FromLocationUpdate c )

        ToLocationUpdate msg_ ->
            let
                ( m, c ) =
                    Typeahead.update msg_ model.toLocation
            in
                checkTypeaheadUpdate msg_ ( { model | toLocation = m }, Cmd.map ToLocationUpdate c )

        FromTransportsUpdate msg_ ->
            ( { model | fromTransports = TagList.update msg_ model.fromTransports }, Cmd.none )

        ToTransportsUpdate msg_ ->
            ( { model | toTransports = TagList.update msg_ model.toTransports }, Cmd.none )

        DateUpdate msg_ ->
            checkRoutingRequest ( { model | date = Calendar.update msg_ model.date }, Cmd.none )

        TimeUpdate msg_ ->
            checkRoutingRequest ( { model | time = TimeInput.update msg_ model.time }, Cmd.none )

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

                ( m, c ) =
                    Connections.update
                        (Connections.Search Connections.ReplaceResults routingRequest)
                        model.connections
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
            -- TODO
            let
                _ =
                    Debug.log "TripToConnectionError" err
            in
                setRailVizFilter model Nothing

        TripToConnectionResponse tripId connection ->
            showFullTripConnection model tripId connection

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

                model1 =
                    { model
                        | scheduleInfo = Just si
                        , connections = connections_
                        , date = newDate
                        , tripSearch = newTripSearch
                    }

                currentDate =
                    getCurrentDate model1

                currentTimeInSchedule =
                    Date.Extra.Compare.is3 Date.Extra.Compare.BetweenOpen currentDate si.begin si.end

                ( model2, cmd1 ) =
                    if currentTimeInSchedule then
                        ( model1, Cmd.none )
                    else
                        update (SetSimulationTime (Date.toTime (combineDateTime si.begin currentDate))) model1
            in
                model2 ! [ Cmd.map ConnectionsUpdate connCmd, cmd1 ]

        Deb a ->
            Debounce.update debounceCfg a model

        SetLocale newLocale ->
            { model
                | locale = newLocale
                , date = Calendar.update (Calendar.SetDateConfig newLocale.dateConfig) model.date
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
                    }

                _ =
                    Debug.log "SetSimulationTime" ( offset, model1.currentTime, getCurrentDate model1 )

                newDate =
                    getCurrentDate model1

                ( model2, cmds1 ) =
                    update (MapUpdate (RailViz.SetTimeOffset offset)) model1

                ( model3, cmds2 ) =
                    update (DateUpdate (Calendar.InitDate True newDate)) model2

                ( model4, cmds3 ) =
                    update (TimeUpdate (TimeInput.InitDate True newDate)) model3

                ( model5, cmds4 ) =
                    update (TripSearchUpdate (TripSearch.SetTime newDate)) model4
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
                                    RailViz.flyTo address.pos Nothing

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
                model_ =
                    { model
                        | subView = Just TripSearchView
                        , overlayVisible = True
                    }
            in
                setRailVizFilter model_ Nothing

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
                    RailViz.flyTo pos (Just zoom)
            in
                model2 ! [ cmd1, cmd2 ]


buildRoutingRequest : Model -> RoutingRequest
buildRoutingRequest model =
    let
        guessStation input =
            Station "" input (Position 0 0)

        getStation typeaheadModel =
            Typeahead.getSelectedStation typeaheadModel
                |> Maybe.withDefault (guessStation typeaheadModel.input)

        fromStation =
            getStation model.fromLocation

        toStation =
            getStation model.toLocation

        minConnectionCount =
            5
    in
        RoutingRequest.initialRequest
            minConnectionCount
            fromStation
            toStation
            (combineDateTime model.date.date model.time.date)
            model.searchDirection


isCompleteQuery : Model -> Bool
isCompleteQuery model =
    let
        fromStation =
            Typeahead.getSelectedStation model.fromLocation

        toStation =
            Typeahead.getSelectedStation model.toLocation
    in
        isJust fromStation && isJust toStation


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
    in
        if completeQuery && requestChanged then
            model ! [ cmds, Debounce.debounceCmd debounceCfg <| SearchConnections ]
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
    { model
        | stationEvents = Nothing
        , overlayVisible = True
    }
        ! [ sendTripRequest model.apiEndpoint tripId ]


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
    div [ class "app" ] <|
        [ Html.map MapUpdate (RailViz.view model.locale model.railViz)
        , lazy overlayView model
        , lazy stationSearchView model
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
            [ div [ class "pure-u-1 pure-u-sm-1-2 from-location" ]
                [ Html.map FromLocationUpdate <|
                    Typeahead.view 1 model.locale.t.search.start (Just "place") model.fromLocation
                , (swapLocationsView model)
                ]
            , div [ class "pure-u-1 pure-u-sm-1-2" ]
                [ Html.map DateUpdate <|
                    Calendar.view 3 model.locale.t.search.date model.date
                ]
            ]
        , div [ class "pure-g gutters" ]
            [ div [ class "pure-u-1 pure-u-sm-12-24 to-location" ]
                [ Html.map ToLocationUpdate <|
                    Typeahead.view 2 model.locale.t.search.destination (Just "place") model.toLocation
                ]
            , div [ class "pure-u-1 pure-u-sm-9-24" ]
                [ Html.map TimeUpdate <|
                    TimeInput.view 4 model.locale.t.search.time model.time
                ]
            , div [ class "pure-u-1 pure-u-sm-3-24 time-option" ]
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
                            Maybe.map (ConnectionDetails.init False False) journey
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


showFullTripConnection : Model -> TripId -> Connection -> ( Model, Cmd Msg )
showFullTripConnection model tripId connection =
    let
        journey =
            toJourney connection

        tripJourney =
            { journey | isSingleCompleteTrip = True }

        ( model_, cmds ) =
            setRailVizFilter model (Just [ tripId ])
    in
        { model_
            | tripDetails = Just (ConnectionDetails.init True True tripJourney)
            , subView = Just TripDetailsView
        }
            ! [ MapConnectionDetails.setConnectionFilter tripJourney
              , Task.attempt noop <| Scroll.toTop "sub-overlay-content"
              , Task.attempt noop <| Scroll.toTop "sub-connection-journey"
              , cmds
              ]


setRailVizFilter : Model -> Maybe (List TripId) -> ( Model, Cmd Msg )
setRailVizFilter model filterTrips =
    model ! [ Port.setRailVizFilter filterTrips ]


journeyTrips : Journey -> List TripId
journeyTrips journey =
    journey.connection.trips
        |> List.map .id
