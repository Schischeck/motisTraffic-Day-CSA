module Main exposing (..)

import Widgets.TimeInput as TimeInput
import Widgets.TagList as TagList
import Widgets.Calendar as Calendar
import Widgets.Typeahead as Typeahead
import Widgets.Map as Map
import Widgets.Connections as Connections
import Widgets.ConnectionDetails as ConnectionDetails
import Data.ScheduleInfo.Types exposing (ScheduleInfo)
import Data.ScheduleInfo.Request as ScheduleInfo
import Data.ScheduleInfo.Decode exposing (decodeScheduleInfoResponse)
import Data.Routing.Types exposing (RoutingRequest, SearchDirection(..))
import Data.Routing.Request as RoutingRequest
import Data.Connection.Types exposing (Station, Position)
import Util.List exposing ((!!))
import Util.Api as Api exposing (ApiError(..))
import Util.Date exposing (combineDateTime)
import Localization.Base exposing (..)
import Localization.De exposing (..)
import Localization.En exposing (..)
import Html exposing (..)
import Html.Attributes exposing (..)
import Html.Events exposing (..)
import Html.App as App
import Html.Lazy exposing (..)
import Dom.Scroll as Scroll
import Task
import String
import Navigation
import Debounce
import Maybe.Extra exposing (isJust)


type alias ProgramFlags =
    { apiEndpoint : String }


main : Program ProgramFlags
main =
    Navigation.programWithFlags urlParser
        { init = init
        , view = view
        , update = update
        , urlUpdate = urlUpdate
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


init : ProgramFlags -> Result String Route -> ( Model, Cmd Msg )
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
            Typeahead.init remoteAddress "Luisenplatz, Darmstadt"

        ( toLocationModel, toLocationCmd ) =
            Typeahead.init remoteAddress "Hamburg Berliner Tor"
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
    | MapUpdate Map.Msg
    | ConnectionsUpdate Connections.Msg
    | SearchConnections
    | PrepareSelectConnection Int
    | SelectConnection Int
    | StoreConnectionListScrollPos Msg Float
    | ConnectionDetailsUpdate ConnectionDetails.Msg
    | CloseConnectionDetails
    | ScheduleInfoError ApiError
    | ScheduleInfoResponse ScheduleInfo
    | Deb (Debounce.Msg Msg)
    | SetLocale Localization


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        NoOp ->
            ( model, Cmd.none )

        Reset ->
            ( model, Cmd.none )

        MapUpdate msg' ->
            let
                ( m, c ) =
                    Map.update msg' model.map
            in
                ( { model | map = m }, Cmd.map MapUpdate c )

        FromLocationUpdate msg' ->
            let
                ( m, c ) =
                    Typeahead.update msg' model.fromLocation
            in
                checkRoutingRequest ( { model | fromLocation = m }, Cmd.map FromLocationUpdate c )

        ToLocationUpdate msg' ->
            let
                ( m, c ) =
                    Typeahead.update msg' model.toLocation
            in
                checkRoutingRequest ( { model | toLocation = m }, Cmd.map ToLocationUpdate c )

        FromTransportsUpdate msg' ->
            ( { model | fromTransports = TagList.update msg' model.fromTransports }, Cmd.none )

        ToTransportsUpdate msg' ->
            ( { model | toTransports = TagList.update msg' model.toTransports }, Cmd.none )

        DateUpdate msg' ->
            checkRoutingRequest ( { model | date = Calendar.update msg' model.date }, Cmd.none )

        TimeUpdate msg' ->
            checkRoutingRequest ( { model | time = TimeInput.update msg' model.time }, Cmd.none )

        SearchDirectionUpdate dir ->
            { model | searchDirection = dir }
                ! []
                |> checkRoutingRequest

        ConnectionsUpdate msg' ->
            let
                ( m, c ) =
                    Connections.update msg' model.connections
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
                    SelectConnection idx
            in
                model
                    ! [ Task.perform
                            (always selectMsg)
                            (StoreConnectionListScrollPos selectMsg)
                            (Scroll.y "connections")
                      ]

        SelectConnection idx ->
            let
                ( m, c ) =
                    selectConnection True model idx
            in
                m
                    ! [ c
                      , Task.perform noop noop <| Scroll.toTop "overlay-content"
                      , Task.perform noop noop <| Scroll.toTop "connection-journey"
                      ]

        StoreConnectionListScrollPos msg' pos ->
            let
                newModel =
                    { model | connectionListScrollPos = pos }
            in
                update msg' newModel

        ConnectionDetailsUpdate msg' ->
            let
                ( m, c ) =
                    case model.selectedConnection of
                        Just state ->
                            let
                                ( m', c' ) =
                                    ConnectionDetails.update msg' state
                            in
                                ( Just m', c' )

                        Nothing ->
                            Nothing ! []
            in
                ( { model | selectedConnection = m }, Cmd.map ConnectionDetailsUpdate c )

        CloseConnectionDetails ->
            closeSelectedConnection True model

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


requestScheduleInfo : String -> Cmd Msg
requestScheduleInfo remoteAddress =
    Api.sendRequest
        remoteAddress
        decodeScheduleInfoResponse
        ScheduleInfoError
        ScheduleInfoResponse
        ScheduleInfo.request


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
        [ App.map MapUpdate (Map.view model.map)
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
            [ div [ class "pure-u-1 pure-u-sm-1-2" ]
                [ App.map FromLocationUpdate <|
                    Typeahead.view 1 model.locale.t.search.start (Just "place") model.fromLocation
                ]
            , div [ class "pure-u-1 pure-u-sm-1-2" ]
                [ App.map DateUpdate <|
                    Calendar.view 3 model.locale.t.search.date model.date
                ]
            ]
        , div [ class "pure-g gutters" ]
            [ div [ class "pure-u-1 pure-u-sm-12-24" ]
                [ App.map ToLocationUpdate <|
                    Typeahead.view 2 model.locale.t.search.destination (Just "place") model.toLocation
                ]
            , div [ class "pure-u-1 pure-u-sm-9-24" ]
                [ App.map TimeUpdate <|
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
            [ type' "radio"
            , id "search-forward"
            , name "time-option"
            , checked (model.searchDirection == Forward)
            , onClick (SearchDirectionUpdate Forward)
            ]
            []
        , label [ for "search-forward" ] [ text "Abfahrt" ]
        ]
    , div []
        [ input
            [ type' "radio"
            , id "search-backward"
            , name "time-option"
            , checked (model.searchDirection == Backward)
            , onClick (SearchDirectionUpdate Backward)
            ]
            []
        , label [ for "search-backward" ] [ text "Ankunft" ]
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
        , closeMsg = CloseConnectionDetails
        }



-- NAVIGATION


type Route
    = Connections
    | ConnectionDetails Int


toUrl : Route -> String
toUrl route =
    case route of
        Connections ->
            "#!"

        ConnectionDetails idx ->
            "#!" ++ toString idx


fromUrl : String -> Result String Route
fromUrl url =
    let
        path =
            String.dropLeft 2 url

        int =
            String.toInt path
    in
        case int of
            Ok idx ->
                Ok (ConnectionDetails idx)

            Err _ ->
                Ok Connections


urlParser : Navigation.Parser (Result String Route)
urlParser =
    Navigation.makeParser (fromUrl << .hash)


urlUpdate : Result String Route -> Model -> ( Model, Cmd Msg )
urlUpdate result model =
    case result of
        Ok route ->
            case route of
                Connections ->
                    closeSelectedConnection False model

                ConnectionDetails idx ->
                    selectConnection False model idx

        Err _ ->
            closeSelectedConnection False model


selectConnection : Bool -> Model -> Int -> ( Model, Cmd Msg )
selectConnection updateUrl model idx =
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
                    | selectedConnection = Maybe.map ConnectionDetails.init journey
                    , connections = newConnections
                }
                    ! (if updateUrl then
                        [ Navigation.newUrl (toUrl (ConnectionDetails idx)) ]
                       else
                        []
                      )

            Nothing ->
                closeSelectedConnection updateUrl model


closeSelectedConnection : Bool -> Model -> ( Model, Cmd Msg )
closeSelectedConnection updateUrl model =
    { model | selectedConnection = Nothing }
        ! ([ Task.perform noop noop <| Scroll.toY "connections" model.connectionListScrollPos ]
            ++ if updateUrl then
                [ Navigation.newUrl (toUrl Connections) ]
               else
                []
          )
