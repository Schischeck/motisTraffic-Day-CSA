module Main exposing (..)

import Widgets.TimeInput as TimeInput
import Widgets.TagList as TagList
import Widgets.Calendar as Calendar
import Widgets.Typeahead as Typeahead
import Widgets.Map as Map
import Widgets.Connections as Connections
import Widgets.ConnectionDetails as ConnectionDetails
import Widgets.ConnectionUtil exposing (Journey)
import Html exposing (..)
import Html.Attributes exposing (..)
import Html.Events exposing (..)
import Html.App as App
import Date
import Date.Extra.Create exposing (dateFromFields)


remoteAddress : String
remoteAddress =
    "http://localhost:8081"


main =
    App.program
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
    , map : Map.Model
    , connections : Connections.Model
    , selectedConnection : Maybe ConnectionDetails.State
    }


init : ( Model, Cmd Msg )
init =
    let
        ( dateModel, dateCmd ) =
            Calendar.init

        ( timeModel, timeCmd ) =
            TimeInput.init

        ( mapModel, mapCmd ) =
            Map.init
    in
        ( { fromLocation = Typeahead.init remoteAddress "Luisenplatz, Darmstadt"
          , toLocation = Typeahead.init remoteAddress "Frankfurt(Main)Hbf"
          , fromTransports = TagList.init
          , toTransports = TagList.init
          , date = dateModel
          , time = timeModel
          , map = mapModel
          , connections = Connections.init remoteAddress
          , selectedConnection = Nothing
          }
        , Cmd.batch
            [ Cmd.map DateUpdate dateCmd
            , Cmd.map TimeUpdate timeCmd
            , Cmd.map MapUpdate mapCmd
            ]
        )



-- UPDATE


type Msg
    = Reset
    | FromLocationUpdate Typeahead.Msg
    | ToLocationUpdate Typeahead.Msg
    | FromTransportsUpdate TagList.Msg
    | ToTransportsUpdate TagList.Msg
    | DateUpdate Calendar.Msg
    | TimeUpdate TimeInput.Msg
    | MapUpdate Map.Msg
    | ConnectionsUpdate Connections.Msg
    | SearchConnections
    | SelectConnection Journey
    | SetConnectionDetailsState ConnectionDetails.State
    | CloseConnectionDetails


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
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
                ( { model | fromLocation = m }, Cmd.map FromLocationUpdate c )

        ToLocationUpdate msg' ->
            let
                ( m, c ) =
                    Typeahead.update msg' model.toLocation
            in
                ( { model | toLocation = m }, Cmd.map ToLocationUpdate c )

        FromTransportsUpdate msg' ->
            ( { model | fromTransports = TagList.update msg' model.fromTransports }, Cmd.none )

        ToTransportsUpdate msg' ->
            ( { model | toTransports = TagList.update msg' model.toTransports }, Cmd.none )

        DateUpdate msg' ->
            ( { model | date = Calendar.update msg' model.date }, Cmd.none )

        TimeUpdate msg' ->
            ( { model | time = TimeInput.update msg' model.time }, Cmd.none )

        ConnectionsUpdate msg' ->
            let
                ( m, c ) =
                    Connections.update msg' model.connections
            in
                ( { model | connections = m }, Cmd.map ConnectionsUpdate c )

        SearchConnections ->
            let
                ( m, c ) =
                    Connections.update
                        (Connections.Search
                            { from = model.fromLocation.input
                            , to = model.toLocation.input
                            , date = (combineDateTime model.date.date model.time.date)
                            }
                        )
                        model.connections
            in
                ( { model | connections = m }, Cmd.map ConnectionsUpdate c )

        SelectConnection journey ->
            ( { model | selectedConnection = Just journey }, Cmd.none )

        SetConnectionDetailsState state ->
            ( { model | selectedConnection = Just state }, Cmd.none )

        CloseConnectionDetails ->
            ( { model | selectedConnection = Nothing }, Cmd.none )


combineDateTime : Date.Date -> Date.Date -> Date.Date
combineDateTime date time =
    dateFromFields (Date.year date)
        (Date.month date)
        (Date.day date)
        (Date.hour time)
        (Date.minute time)
        (Date.second time)
        (Date.millisecond time)



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
                    detailsView c
    in
        div [ class "app" ] <|
            [ App.map MapUpdate (Map.view model.map)
            , div [ class "overlay" ] <|
                [ div [ id "header" ]
                    [ h1 [ class "disable-select" ] [ text "Motis Project" ]
                    , i [ class "icon" ] [ text "\xE2BF" ]
                    ]
                , div [ class "overlay-content" ]
                    content
                ]
            ]


searchView : Model -> List (Html Msg)
searchView model =
    [ div [ id "search" ]
        [ div [ class "pure-g gutters" ]
            [ div [ class "pure-u-1 pure-u-sm-3-5" ]
                [ App.map FromLocationUpdate (Typeahead.view (Just "\xE8B4") model.fromLocation) ]
            , div [ class "pure-u-1 pure-u-sm-2-5" ]
                [ App.map FromTransportsUpdate (TagList.view model.fromTransports) ]
            ]
        , div [ class "pure-g gutters" ]
            [ div [ class "pure-u-1 pure-u-sm-3-5" ]
                [ App.map ToLocationUpdate (Typeahead.view (Just "\xE8B4") model.toLocation) ]
            , div [ class "pure-u-1 pure-u-sm-2-5" ]
                [ App.map ToTransportsUpdate (TagList.view model.toTransports) ]
            ]
        , div [ class "pure-g gutters" ]
            [ div [ class "pure-u-1 pure-u-sm-1-2" ]
                [ App.map DateUpdate (Calendar.view model.date) ]
            , div [ class "pure-u-1 pure-u-sm-1-2" ]
                [ App.map TimeUpdate (TimeInput.view model.time) ]
            ]
        ]
    , div []
        [ a
            [ class "gb-button gb-button-medium gb-button-PRIMARY_COLOR disable-select"
            , onClick SearchConnections
            ]
            [ text "Search" ]
        ]
    , div [ id "connections" ]
        [ Connections.view connectionConfig model.connections ]
    ]


connectionConfig : Connections.Config Msg
connectionConfig =
    Connections.Config
        { internalMsg = ConnectionsUpdate
        , selectMsg = SelectConnection
        }


detailsView : ConnectionDetails.State -> List (Html Msg)
detailsView state =
    [ ConnectionDetails.view detailsConfig state ]


detailsConfig : ConnectionDetails.Config Msg
detailsConfig =
    ConnectionDetails.Config
        { internalMsg = SetConnectionDetailsState
        , closeMsg = CloseConnectionDetails
        }
