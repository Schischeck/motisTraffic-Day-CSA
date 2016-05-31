module Main exposing (..)

import Widgets.TimeInput as TimeInput
import Widgets.TagList as TagList
import Widgets.Calendar as Calendar
import Widgets.Typeahead as Typeahead
import Html exposing (..)
import Html.Attributes exposing (..)
import Html.App as App


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
    }


init : ( Model, Cmd Msg )
init =
    let
        ( dateModel, dateCmd ) =
            Calendar.init

        ( timeModel, timeCmd ) =
            TimeInput.init
    in
        ( { fromLocation = Typeahead.init
          , toLocation = Typeahead.init
          , fromTransports = TagList.init
          , toTransports = TagList.init
          , date = dateModel
          , time = timeModel
          }
        , Cmd.batch
            [ Cmd.map DateUpdate dateCmd
            , Cmd.map TimeUpdate timeCmd
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


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        Reset ->
            ( model, Cmd.none )

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



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.batch
        [ Sub.map FromLocationUpdate (Typeahead.subscriptions model.fromLocation)
        , Sub.map ToLocationUpdate (Typeahead.subscriptions model.toLocation)
        , Sub.map FromTransportsUpdate (TagList.subscriptions model.fromTransports)
        , Sub.map ToTransportsUpdate (TagList.subscriptions model.toTransports)
        ]



-- VIEW


view : Model -> Html Msg
view model =
    div [ class "app" ]
        [ div [ id "map" ] []
        , div [ class "overlay" ]
            [ div [ id "header" ]
                [ h1 [ class "disable-select" ] [ text "Motis Project" ]
                , i [ class "icon" ] [ text "\xE2BF" ]
                ]
            , div [ id "search" ]
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
            ]
        ]
