module Widgets.Calendar exposing (Model, Msg, init, subscriptions, update, view)

import Html exposing (..)
import Html.Events exposing (onClick, onWithOptions)
import Html.Attributes exposing (..)
import Date exposing (Date, Day, day, month, year, dayOfWeek)
import Date.Extra.Format exposing (format)
import Date.Extra.Duration as Duration
import Date.Extra.Core exposing (lastOfMonthDate, toFirstOfMonth, isoDayOfWeek)
import Date.Extra.Utils exposing (dayList)
import Date.Extra.Config.Config_en_us exposing (config)
import Date.Extra.Compare as Compare
import Task
import String
import Mouse
import Json.Decode as Json
import Widgets.Input as Input


-- MODEL


type alias Model =
    { today : Date
    , date : Date
    , visible : Bool
    }


init : ( Model, Cmd Msg )
init =
    ( emptyModel, getCurrentDate )


emptyModel : Model
emptyModel =
    { today = Date.fromTime 0
    , date = Date.fromTime 0
    , visible = False
    }



-- UPDATE


type Msg
    = Reset
    | InitDate Date
    | NewDate Date
    | NewDateError String
    | PrevMonth
    | NextMonth
    | ToggleVisibility
    | Click


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    ( updateModel msg model, Cmd.none )


updateModel : Msg -> Model -> Model
updateModel msg model =
    case msg of
        Reset ->
            model

        InitDate d ->
            { model | date = d, today = d }

        NewDate d ->
            { model | date = d, visible = False }

        NewDateError _ ->
            model

        PrevMonth ->
            { model | date = (Duration.add Duration.Month -1 model.date) }

        NextMonth ->
            { model | date = (Duration.add Duration.Month 1 model.date) }

        ToggleVisibility ->
            { model | visible = not model.visible }

        Click ->
            { model | visible = False }



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    if model.visible then
        Mouse.downs (\_ -> Click)
    else
        Sub.none



-- VIEW


onStop : String -> msg -> Html.Attribute msg
onStop event msg =
    onWithOptions event { stopPropagation = True, preventDefault = True } (Json.succeed msg)


weekDays : List (Html Msg)
weekDays =
    dayListForMonthView (Date.fromTime 0) (Date.fromTime 0)
        |> List.take 7
        |> List.map (\d -> li [] [ text (toString (dayOfWeek d.day)) ])


calendarDay : CalendarDay -> Html Msg
calendarDay date =
    li
        [ onClick (NewDate date.day)
        , classList
            [ ( "out-of-month", not date.inMonth )
            , ( "in-month", date.inMonth )
            , ( "today", date.today )
            , ( "selected", date.selected )
            ]
        ]
        [ text (toString (day date.day)) ]


calendarDays : Date -> Date -> List (Html Msg)
calendarDays today date =
    dayListForMonthView today date |> List.map calendarDay


monthView : Date -> Html Msg
monthView date =
    div [ class "month" ]
        [ i [ class "icon", onClick PrevMonth ] [ text "\xE314" ]
        , span [ class "month-name" ] [ text (monthAndYearStr date) ]
        , i [ class "icon", onClick NextMonth ] [ text "\xE315" ]
        ]


view : Model -> Html Msg
view model =
    div []
        [ Input.view
            [ onStop "mousedown" ToggleVisibility
            , value (formatDate model.date)
            ]
        , div
            [ classList
                [ ( "paper", True )
                , ( "calendar", True )
                , ( "hide", not model.visible )
                ]
            , onStop "mousedown" Reset
            ]
            [ monthView model.date
            , ul [ class "weekdays" ] weekDays
            , ul [ class "calendardays" ] (calendarDays model.today model.date)
            ]
        ]



-- DATE


type alias CalendarDay =
    { day : Date
    , inMonth : Bool
    , today : Bool
    , selected : Bool
    }


getCurrentDate : Cmd Msg
getCurrentDate =
    Task.perform NewDateError InitDate Date.now


formatDate : Date -> String
formatDate d =
    format config "%d.%m.%Y" d


daysInSixWeeks : Int
daysInSixWeeks =
    42


dayListForMonthView : Date -> Date -> List CalendarDay
dayListForMonthView today selected =
    let
        firstOfMonth =
            toFirstOfMonth selected
            
        lastOfMonth =
            lastOfMonthDate selected

        daysToStart =
            isoDayOfWeek (dayOfWeek firstOfMonth) - 1

        first =
            Duration.add Duration.Day -daysToStart firstOfMonth
    in
        dayList daysInSixWeeks first
            |> List.map
                (\date ->
                    { day = date
                    , inMonth = Compare.is3 Compare.BetweenOpen date firstOfMonth lastOfMonth
                    , today = Compare.is Compare.Same today date
                    , selected = Compare.is Compare.Same selected date
                    }
                )


monthAndYearStr : Date -> String
monthAndYearStr d =
    [ toString (month d), toString (year d) ]
        |> String.join " "
