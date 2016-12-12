module Main exposing (..)

import Widgets.TimeInput as TimeInput
import Widgets.TagList as TagList
import Widgets.Calendar as Calendar
import Widgets.Typeahead as Typeahead
import Widgets.Map as Map
import Widgets.MapConnectionOverlay as MapConnectionOverlay
import Widgets.Connections as Connections
import Widgets.ConnectionDetails as ConnectionDetails
import Data.ScheduleInfo.Types exposing (ScheduleInfo)
import Data.ScheduleInfo.Request as ScheduleInfo
import Data.ScheduleInfo.Decode exposing (decodeScheduleInfoResponse)
import Data.Routing.Types exposing (RoutingRequest, SearchDirection(..))
import Data.Routing.Request as RoutingRequest
import Data.Routing.Decode exposing (decodeRoutingResponse)
import Data.Connection.Types exposing (Station, Position, TripId, Connection)
import Data.Journey.Types exposing (toJourney)
import Data.Lookup.Request exposing (encodeTripToConnection)
import Data.Lookup.Decode exposing (decodeTripToConnectionResponse)
import Util.List exposing ((!!))
import Util.Api as Api exposing (ApiError(..))
import Util.Date exposing (combineDateTime)
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
import Maybe.Extra exposing (isJust)
import Port
import Json.Decode as Decode
import Date exposing (Date)
import Time exposing (Time)


type alias ProgramFlags =
    { apiEndpoint : String }


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
    , map : Map.Model
    , connections : Connections.Model
    , connectionDetails : Maybe ConnectionDetails.State
    , selectedConnectionIdx : Maybe Int
    , selectedTripIdx : Maybe Int
    , scheduleInfo : Maybe ScheduleInfo
    , locale : Localization
    , apiEndpoint : String
    , currentRoutingRequest : Maybe RoutingRequest
    , debounce : Debounce.State
    , connectionListScrollPos : Float
    , currentTime : Date
    }


init : ProgramFlags -> Location -> ( Model, Cmd Msg )
init flags initialLocation =
    let
        locale =
            deLocalization

        remoteAddress =
            flags.apiEndpoint

        ( dateModel, dateCmd ) =
            Calendar.init locale.dateConfig

        ( timeModel, timeCmd ) =
            TimeInput.init

        ( mapModel, mapCmd ) =
            Map.init remoteAddress

        ( fromLocationModel, fromLocationCmd ) =
            Typeahead.init remoteAddress "Willy-Brandt-Platz, Darmstadt"

        ( toLocationModel, toLocationCmd ) =
            Typeahead.init remoteAddress "Frankfurt(M)Hauptwache"

        initialModel =
            { fromLocation = fromLocationModel
            , toLocation = toLocationModel
            , fromTransports = TagList.init
            , toTransports = TagList.init
            , date = dateModel
            , time = timeModel
            , searchDirection = Forward
            , map = mapModel
            , connections = Connections.init remoteAddress
            , connectionDetails = Nothing
            , selectedConnectionIdx = Nothing
            , selectedTripIdx = Nothing
            , scheduleInfo = Nothing
            , locale = locale
            , apiEndpoint = remoteAddress
            , currentRoutingRequest = Nothing
            , debounce = Debounce.init
            , connectionListScrollPos = 0
            , currentTime = Date.fromTime 0
            }

        ( model, cmds ) =
            update (locationToMsg initialLocation) initialModel
    in
        model
            ! [ Cmd.map DateUpdate dateCmd
              , Cmd.map TimeUpdate timeCmd
              , Cmd.map MapUpdate mapCmd
              , Cmd.map FromLocationUpdate fromLocationCmd
              , Cmd.map ToLocationUpdate toLocationCmd
              , requestScheduleInfo remoteAddress
              , Task.perform UpdateCurrentTime Time.now
              , cmds
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
    | MapUpdate Map.Msg
    | ConnectionsUpdate Connections.Msg
    | SearchConnections
    | PrepareSelectConnection Int
    | SelectConnection Int
    | StoreConnectionListScrollPos Msg Float
    | ConnectionDetailsUpdate ConnectionDetails.Msg
    | ConnectionDetailsGoBack
    | CloseConnectionDetails
    | PrepareSelectTrip Int
    | LoadTrip TripId
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
                    Map.update msg_ model.map
            in
                ( { model | map = m }, Cmd.map MapUpdate c )

        FromLocationUpdate msg_ ->
            let
                ( m, c ) =
                    Typeahead.update msg_ model.fromLocation
            in
                checkRoutingRequest ( { model | fromLocation = m }, Cmd.map FromLocationUpdate c )

        ToLocationUpdate msg_ ->
            let
                ( m, c ) =
                    Typeahead.update msg_ model.toLocation
            in
                checkRoutingRequest ( { model | toLocation = m }, Cmd.map ToLocationUpdate c )

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

        CloseConnectionDetails ->
            closeSelectedConnection model

        ConnectionDetailsGoBack ->
            case model.selectedTripIdx of
                Just _ ->
                    case model.selectedConnectionIdx of
                        Just connIdx ->
                            update (NavigateTo (ConnectionDetails connIdx))
                                { model | selectedTripIdx = Nothing }

                        Nothing ->
                            update (NavigateTo Connections)
                                { model | selectedTripIdx = Nothing }

                Nothing ->
                    update (NavigateTo Connections) model

        PrepareSelectTrip tripIdx ->
            selectConnectionTrip model tripIdx

        LoadTrip tripId ->
            loadTripById model tripId

        TripToConnectionError tripId err ->
            -- TODO
            let
                _ =
                    Debug.log "TripToConnectionError" err
            in
                model ! []

        TripToConnectionResponse tripId connection ->
            showFullTripConnection model connection

        ScheduleInfoError _ ->
            let
                ( newConnections, c ) =
                    Connections.update (Connections.UpdateScheduleInfo Nothing) model.connections

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
                ( m, c ) =
                    Connections.update (Connections.UpdateScheduleInfo (Just si)) model.connections

                newDate =
                    Calendar.update (Calendar.SetValidRange (Just ( si.begin, si.end ))) model.date
            in
                ( { model
                    | scheduleInfo = Just si
                    , connections = m
                    , date = newDate
                  }
                , Cmd.map ConnectionsUpdate c
                )

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
                update (NavigateTo (tripDetailsRoute tripId))
                    { model
                        | selectedTripIdx = Just tripIdx
                    }

            Nothing ->
                model ! []


loadTripById : Model -> TripId -> ( Model, Cmd Msg )
loadTripById model tripId =
    let
        selectedJourney =
            model.selectedConnectionIdx
                |> Maybe.andThen (Connections.getJourney model.connections)

        selectedTrip =
            Maybe.map2 (\j i -> j.trains !! i) selectedJourney model.selectedTripIdx
                |> Maybe.Extra.join
                |> Maybe.andThen .trip

        isConnectionTrip =
            Maybe.Extra.unwrap False (\t -> t == tripId) selectedTrip

        model_ =
            if isConnectionTrip then
                model
            else
                { model
                    | selectedConnectionIdx = Nothing
                    , selectedTripIdx = Nothing
                }
    in
        model_ ! [ sendTripRequest model.apiEndpoint tripId ]



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.batch
        [ Sub.map FromTransportsUpdate (TagList.subscriptions model.fromTransports)
        , Sub.map ToTransportsUpdate (TagList.subscriptions model.toTransports)
        , Sub.map MapUpdate (Map.subscriptions model.map)
        , Port.setRoutingResponses SetRoutingResponses
        , Time.every (2 * Time.second) UpdateCurrentTime
        ]



-- VIEW


view : Model -> Html Msg
view model =
    div [ class "app" ] <|
        [ Html.map MapUpdate (Map.view model.map)
        , lazy overlayView model
        ]


overlayView : Model -> Html Msg
overlayView model =
    let
        content =
            case model.connectionDetails of
                Nothing ->
                    searchView model

                Just c ->
                    detailsView model.locale model.currentTime c
    in
        div [ class "overlay-container" ]
            [ div [ class "overlay" ] <|
                [ div [ id "overlay-content" ]
                    content
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


connectionConfig : Connections.Config Msg
connectionConfig =
    Connections.Config
        { internalMsg = ConnectionsUpdate
        , selectMsg = PrepareSelectConnection
        }


detailsView : Localization -> Date -> ConnectionDetails.State -> List (Html Msg)
detailsView locale currentTime state =
    [ ConnectionDetails.view detailsConfig locale currentTime state ]


detailsConfig : ConnectionDetails.Config Msg
detailsConfig =
    ConnectionDetails.Config
        { internalMsg = ConnectionDetailsUpdate
        , selectTripMsg = PrepareSelectTrip
        , goBackMsg = ConnectionDetailsGoBack
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
                { model
                    | connectionDetails =
                        Maybe.map (ConnectionDetails.init False) journey
                    , connections = newConnections
                    , selectedConnectionIdx = Just idx
                    , selectedTripIdx = Nothing
                }
                    ! [ MapConnectionOverlay.showOverlay model.locale j ]

            Nothing ->
                update (ReplaceLocation Connections) model


closeSelectedConnection : Model -> ( Model, Cmd Msg )
closeSelectedConnection model =
    { model
        | connectionDetails = Nothing
        , selectedConnectionIdx = Nothing
        , selectedTripIdx = Nothing
    }
        ! [ Task.attempt noop <| Scroll.toY "connections" model.connectionListScrollPos
          , MapConnectionOverlay.hideOverlay
          ]


showFullTripConnection : Model -> Connection -> ( Model, Cmd Msg )
showFullTripConnection model connection =
    let
        journey =
            toJourney connection

        tripJourney =
            { journey | isSingleCompleteTrip = True }
    in
        { model
            | connectionDetails = Just (ConnectionDetails.init True tripJourney)
        }
            ! [ MapConnectionOverlay.showOverlay model.locale journey
              , Task.attempt noop <| Scroll.toTop "overlay-content"
              , Task.attempt noop <| Scroll.toTop "connection-journey"
              ]
