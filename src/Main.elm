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
import Data.Routing.Request as RoutingRequest exposing (RoutingRequest)
import Util.List exposing ((!!))
import Util.Api as Api exposing (ApiError(..))
import Util.Date exposing (combineDateTime)
import Localization.Base exposing (..)
import Localization.De exposing (..)
import Localization.En exposing (..)
import Html exposing (..)
import Html.Attributes exposing (..)
import Html.App as App
import Html.Lazy exposing (..)
import Dom.Scroll as Scroll
import Task
import String
import Navigation
import Debounce


remoteAddress : String
remoteAddress =
    "http://localhost:8081"


main : Program Never
main =
    Navigation.program urlParser
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
    , map : Map.Model
    , connections : Connections.Model
    , selectedConnection : Maybe ConnectionDetails.State
    , scheduleInfo : Maybe ScheduleInfo
    , locale : Localization
    , currentRoutingRequest : Maybe RoutingRequest
    , debounce : Debounce.State
    }


init : Result String Route -> ( Model, Cmd Msg )
init _ =
    let
        ( dateModel, dateCmd ) =
            Calendar.init

        ( timeModel, timeCmd ) =
            TimeInput.init

        ( mapModel, mapCmd ) =
            Map.init
    in
        ( { fromLocation =
                Typeahead.init remoteAddress "Luisenplatz, Darmstadt"
          , toLocation = Typeahead.init remoteAddress "Hamburg Berliner Tor"
          , fromTransports = TagList.init
          , toTransports = TagList.init
          , date = dateModel
          , time = timeModel
          , map = mapModel
          , connections = Connections.init remoteAddress
          , selectedConnection = Nothing
          , scheduleInfo = Nothing
          , locale = deLocalization
          , currentRoutingRequest = Nothing
          , debounce = Debounce.init
          }
        , Cmd.batch
            [ Cmd.map DateUpdate dateCmd
            , Cmd.map TimeUpdate timeCmd
            , Cmd.map MapUpdate mapCmd
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
    | MapUpdate Map.Msg
    | ConnectionsUpdate Connections.Msg
    | SearchConnections
    | SelectConnection Int
    | ConnectionDetailsUpdate ConnectionDetails.Msg
    | CloseConnectionDetails
    | ScheduleInfoError ApiError
    | ScheduleInfoResponse ScheduleInfo
    | Deb (Debounce.Msg Msg)


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
                }
                    ! [ Cmd.map ConnectionsUpdate c ]

        SelectConnection idx ->
            let
                ( m, c ) =
                    selectConnection True model idx

                noop =
                    \_ -> NoOp
            in
                m
                    ! [ c
                      , Task.perform noop noop <| Scroll.toTop "overlay-content"
                      , Task.perform noop noop <| Scroll.toTop "connection-journey"
                      ]

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
            closeSelectedConnection model

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
    RoutingRequest.initialRequest
        model.fromLocation.input
        model.toLocation.input
        (combineDateTime model.date.date model.time.date)


checkRoutingRequest : ( Model, Cmd Msg ) -> ( Model, Cmd Msg )
checkRoutingRequest ( model, cmds ) =
    let
        newRoutingRequest =
            buildRoutingRequest model

        requestChanged =
            case model.currentRoutingRequest of
                Just currentRequest ->
                    newRoutingRequest /= currentRequest

                Nothing ->
                    True
    in
        if requestChanged then
            model ! [ cmds, Debounce.debounceCmd debounceCfg <| SearchConnections ]
        else
            ( model, cmds )


debounceCfg : Debounce.Config Model Msg
debounceCfg =
    Debounce.config
        .debounce
        (\model s -> { model | debounce = s })
        Deb
        300



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
    let
        content =
            case model.selectedConnection of
                Nothing ->
                    searchView model

                Just c ->
                    detailsView model.locale c
    in
        div [ class "app" ] <|
            [ App.map MapUpdate (Map.view model.map)
            , div [ class "overlay" ] <|
                [ div [ id "overlay-content" ]
                    content
                ]
            ]


searchView : Model -> List (Html Msg)
searchView model =
    [ div [ id "search" ]
        [ div [ class "pure-g gutters" ]
            [ div [ class "pure-u-1 pure-u-sm-3-5" ]
                [ App.map FromLocationUpdate (Typeahead.view model.locale.t.search.start (Just "\xE8B4") model.fromLocation) ]
            , div [ class "pure-u-1 pure-u-sm-2-5" ]
                [ App.map FromTransportsUpdate (TagList.view model.locale.t.search.startTransports model.fromTransports) ]
            ]
        , div [ class "pure-g gutters" ]
            [ div [ class "pure-u-1 pure-u-sm-3-5" ]
                [ App.map ToLocationUpdate (Typeahead.view model.locale.t.search.destination (Just "\xE8B4") model.toLocation) ]
            , div [ class "pure-u-1 pure-u-sm-2-5" ]
                [ App.map ToTransportsUpdate (TagList.view model.locale.t.search.destinationTransports model.toTransports) ]
            ]
        , div [ class "pure-g gutters" ]
            [ div [ class "pure-u-1 pure-u-sm-1-2" ]
                [ App.map DateUpdate (Calendar.view model.locale.t.search.date model.date) ]
            , div [ class "pure-u-1 pure-u-sm-1-2" ]
                [ App.map TimeUpdate (TimeInput.view model.locale.t.search.time model.time) ]
            ]
        ]
    , div [ id "connections" ]
        [ lazy3 Connections.view connectionConfig model.locale model.connections ]
    ]


connectionConfig : Connections.Config Msg
connectionConfig =
    Connections.Config
        { internalMsg = ConnectionsUpdate
        , selectMsg = SelectConnection
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
                    { model | selectedConnection = Nothing } ! []

                ConnectionDetails idx ->
                    selectConnection False model idx

        Err _ ->
            { model | selectedConnection = Nothing } ! [ Navigation.newUrl (toUrl Connections) ]


selectConnection : Bool -> Model -> Int -> ( Model, Cmd Msg )
selectConnection updateUrl model idx =
    let
        realIndex =
            idx - model.connections.indexOffset

        journey =
            model.connections.journeys !! realIndex
    in
        case journey of
            Just j ->
                { model | selectedConnection = Maybe.map ConnectionDetails.init journey }
                    ! (if updateUrl then
                        [ Navigation.newUrl (toUrl (ConnectionDetails idx)) ]
                       else
                        []
                      )

            Nothing ->
                closeSelectedConnection model


closeSelectedConnection : Model -> ( Model, Cmd Msg )
closeSelectedConnection model =
    { model | selectedConnection = Nothing }
        ! [ Navigation.newUrl (toUrl Connections) ]
