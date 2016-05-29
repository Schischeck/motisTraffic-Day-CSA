module Main exposing (..)

import Widgets.TimeInput as TimeInput
import Widgets.TagList as TagList
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
    , taglist : TagList.Model
    , timeinput : TimeInput.Model
    }


init : ( Model, Cmd Msg )
init =
    let
        ( calendarModel, calendarCmd ) =
            Calendar.init

        ( typeaheadModel, typeaheadCmd ) =
            Typeahead.init

        ( taglistModel, taglistCmd ) =
            TagList.init

        ( timeinputModel, timeinputCmd ) =
            TimeInput.init
    in
        ( { calendar = calendarModel
          , typeahead = typeaheadModel
          , taglist = taglistModel
          , timeinput = timeinputModel
          }
        , Cmd.map CalendarUpdate calendarCmd
        )



-- UPDATE


type Msg
    = Reset
    | CalendarUpdate Calendar.Msg
    | TypeaheadUpdate Typeahead.Msg
    | TagListUpdate TagList.Msg
    | TimeInputUpdate TimeInput.Msg


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
                ( { model | calendar = calModel }, Cmd.map CalendarUpdate calCmd )

        TypeaheadUpdate m ->
            let
                ( taModel, taCmd ) =
                    Typeahead.update m model.typeahead
            in
                ( { model | typeahead = taModel }, Cmd.map TypeaheadUpdate taCmd )

        TagListUpdate m ->
            let
                ( tlModel, calCmd ) =
                    TagList.update m model.taglist
            in
                ( { model | taglist = tlModel }, Cmd.none )

        TimeInputUpdate m ->
            let
                ( tiModel, calCmd ) =
                    ( TimeInput.update m model.timeinput, Cmd.none )
            in
                ( { model | timeinput = tiModel }, Cmd.none )



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.batch
        [ Sub.map CalendarUpdate (Calendar.subscriptions model.calendar)
        , Sub.map TypeaheadUpdate (Typeahead.subscriptions model.typeahead)
        , Sub.map TagListUpdate (TagList.subscriptions model.taglist)
        ]



-- VIEW


view : Model -> Html Msg
view model =
    div []
        [ App.map CalendarUpdate (Calendar.view model.calendar)
        , App.map TypeaheadUpdate (Typeahead.view model.typeahead)
        , App.map TagListUpdate (TagList.view model.taglist)
        , App.map TimeInputUpdate (TimeInput.view model.timeinput)
        ]
