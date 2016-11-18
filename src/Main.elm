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
import Data.Connection.Types exposing (Station, Position, TripId, Connection)
import Data.Journey.Types exposing (toJourney)
import Data.Lookup.Request exposing (encodeTripToConnection)
import Data.Lookup.Decode exposing (decodeTripToConnectionResponse)
import Util.Core exposing ((=>))
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
import String
import Navigation exposing (Location)
import UrlParser
import Routes exposing (..)
import Debounce
import Maybe.Extra exposing (isJust)


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
    , selectedConnection : Maybe ConnectionDetails.State
    , scheduleInfo : Maybe ScheduleInfo
    , locale : Localization
    , apiEndpoint : String
    , currentRoutingRequest : Maybe RoutingRequest
    , debounce : Debounce.State
    , connectionListScrollPos : Float
    }


init : ProgramFlags -> Location -> ( Model, Cmd Msg )
init flags _ =
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
            Map.init

        ( fromLocationModel, fromLocationCmd ) =
            Typeahead.init remoteAddress "Bahra An der Brücke, Bad Gottleuba-Berggießhübel"

        ( toLocationModel, toLocationCmd ) =
            Typeahead.init remoteAddress "Frankfurt(M)Hauptwache"
    in
        ( { fromLocation = fromLocationModel
          , toLocation = toLocationModel
          , fromTransports = TagList.init
          , toTransports = TagList.init
          , date = dateModel
          , time = timeModel
          , searchDirection = Forward
          , map = mapModel
          , connections = Connections.init remoteAddress
          , selectedConnection = Nothing
          , scheduleInfo = Nothing
          , locale = locale
          , apiEndpoint = remoteAddress
          , currentRoutingRequest = Nothing
          , debounce = Debounce.init
          , connectionListScrollPos = 0
          }
        , Cmd.batch
            [ Cmd.map DateUpdate dateCmd
            , Cmd.map TimeUpdate timeCmd
            , Cmd.map MapUpdate mapCmd
            , Cmd.map FromLocationUpdate fromLocationCmd
            , Cmd.map ToLocationUpdate toLocationCmd
            , requestScheduleInfo remoteAddress
            , Navigation.modifyUrl (toUrl Connections)
            ]
        )



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
    | CloseConnectionDetails
    | SelectTrip Int
    | TripToConnectionError Int Int ApiError
    | TripToConnectionResponse Int Int Connection
    | ScheduleInfoError ApiError
    | ScheduleInfoResponse ScheduleInfo
    | Deb (Debounce.Msg Msg)
    | SetLocale Localization
    | NavigateTo Route
    | ReplaceLocation Route


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
                    case model.selectedConnection of
                        Just state ->
                            let
                                ( m_, c_ ) =
                                    ConnectionDetails.update msg_ state
                            in
                                ( Just m_, c_ )

                        Nothing ->
                            Nothing ! []
            in
                ( { model | selectedConnection = m }, Cmd.map ConnectionDetailsUpdate c )

        CloseConnectionDetails ->
            closeSelectedConnection model

        SelectTrip idx ->
            loadTrip model idx

        TripToConnectionError connIdx tripIdx err ->
            -- TODO
            let
                _ =
                    Debug.log "TripToConnectionError" err
            in
                model ! []

        TripToConnectionResponse connIdx tripIdx connection ->
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


loadTrip : Model -> Int -> ( Model, Cmd Msg )
loadTrip model idx =
    case model.selectedConnection of
        Just cdm ->
            let
                journey =
                    ConnectionDetails.getJourney cdm

                trip =
                    (journey.trains !! idx) |> Maybe.andThen .trip
            in
                case trip of
                    Just tripId ->
                        model ! [ sendTripRequest model.apiEndpoint 0 idx tripId ]

                    Nothing ->
                        model ! []

        Nothing ->
            model ! []



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.batch
        [ Sub.map FromTransportsUpdate (TagList.subscriptions model.fromTransports)
        , Sub.map ToTransportsUpdate (TagList.subscriptions model.toTransports)
        , Sub.map MapUpdate (Map.subscriptions model.map)
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
            case model.selectedConnection of
                Nothing ->
                    searchView model

                Just c ->
                    detailsView model.locale c
    in
        div [ class "overlay" ] <|
            [ div [ id "overlay-content" ]
                content
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


detailsView : Localization -> ConnectionDetails.State -> List (Html Msg)
detailsView locale state =
    [ ConnectionDetails.view detailsConfig locale state ]


detailsConfig : ConnectionDetails.Config Msg
detailsConfig =
    ConnectionDetails.Config
        { internalMsg = ConnectionDetailsUpdate
        , selectTripMsg = SelectTrip
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


sendTripRequest : String -> Int -> Int -> TripId -> Cmd Msg
sendTripRequest remoteAddress connIdx tripIdx tripId =
    Api.sendRequest
        remoteAddress
        decodeTripToConnectionResponse
        (TripToConnectionError connIdx tripIdx)
        (TripToConnectionResponse connIdx tripIdx)
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

        ConnectionFullTripDetails ->
            NoOp


selectConnection : Model -> Int -> ( Model, Cmd Msg )
selectConnection model idx =
    let
        realIndex =
            idx - model.connections.indexOffset

        journey =
            model.connections.journeys !! realIndex

        ( newConnections, _ ) =
            Connections.update Connections.ResetNew model.connections
    in
        case journey of
            Just j ->
                { model
                    | selectedConnection =
                        Maybe.map (ConnectionDetails.init False) journey
                    , connections = newConnections
                }
                    ! [ MapConnectionOverlay.showOverlay model.locale j ]

            Nothing ->
                update (ReplaceLocation Connections) model


closeSelectedConnection : Model -> ( Model, Cmd Msg )
closeSelectedConnection model =
    { model | selectedConnection = Nothing }
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
            | selectedConnection = Just (ConnectionDetails.init True tripJourney)
        }
            ! [ Navigation.newUrl (toUrl ConnectionFullTripDetails)
              , MapConnectionOverlay.showOverlay model.locale journey
              , Task.attempt noop <| Scroll.toTop "overlay-content"
              , Task.attempt noop <| Scroll.toTop "connection-journey"
              ]
