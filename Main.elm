module Main exposing (..)

import Widgets.Calendar as Calendar
import Widgets.Typeahead as Typeahead
import Html exposing (..)
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
    { calendar : Calendar.Model
    , typeahead : Typeahead.Model
    }


init : ( Model, Cmd Msg )
init =
    let
        ( calendarModel, calendarCmd ) =
            Calendar.init

        ( typeaheadModel, typeaheadCmd ) =
            Typeahead.init
    in
        ( { calendar = calendarModel
          , typeahead = typeaheadModel
          }
        , Cmd.map CalendarUpdate calendarCmd
        )



-- UPDATE


type Msg
    = Reset
    | CalendarUpdate Calendar.Msg
    | TypeaheadUpdate Typeahead.Msg


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        Reset ->
            ( model, Cmd.none )

        CalendarUpdate m ->
            let
                ( calModel, calCmd ) =
                    Calendar.update m model.calendar
            in
                ( { model | calendar = calModel }, Cmd.none )

        TypeaheadUpdate m ->
            let
                ( taModel, calCmd ) =
                    Typeahead.update m model.typeahead
            in
                ( { model | typeahead = taModel }, Cmd.none )



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.batch
        [ Sub.map CalendarUpdate (Calendar.subscriptions model.calendar)
        , Sub.map TypeaheadUpdate (Typeahead.subscriptions model.typeahead)
        ]



-- VIEW


view : Model -> Html Msg
view model =
    div []
        [ App.map CalendarUpdate (Calendar.view model.calendar)
        , App.map TypeaheadUpdate (Typeahead.view model.typeahead)
        ]
