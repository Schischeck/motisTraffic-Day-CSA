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
    }


init : ( Model, Cmd Msg )
init =
    let
        ( calendarModel, calendarCmd ) =
            Calendar.init
    in
        ( { calendar = calendarModel }, Cmd.map CalendarUpdate calendarCmd )



-- UPDATE


type Msg
    = Reset
    | CalendarUpdate Calendar.Msg


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



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.map CalendarUpdate (Calendar.subscriptions model.calendar)



-- VIEW


view : Model -> Html Msg
view model =
    App.map CalendarUpdate (Calendar.view model.calendar)
