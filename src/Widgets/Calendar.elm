module Widgets.Calendar exposing (Model, Msg(..), init, update, view)

import Html exposing (..)
import Html.Events exposing (onClick, onInput, onFocus, onBlur, onWithOptions)
import Html.Attributes exposing (..)
import Html.Lazy exposing (lazy)
import Date exposing (Date, Day, day, month, year, dayOfWeek)
import Date.Extra.Duration as Duration
import Date.Extra.Core exposing (lastOfMonthDate, toFirstOfMonth, isoDayOfWeek)
import Date.Extra.Utils exposing (dayList)
import Date.Extra.Compare as Compare
import Task
import Util.View exposing (onStopAll, onStopPropagation)
import Util.DateFormat exposing (..)
import Widgets.Input as Input
import Widgets.Button as Button


-- MODEL


type alias Model =
    { conf : DateConfig
    , today : Date
    , date : Date
    , inputStr : String
    , visible : Bool
    , inputWidget : Input.Model
    , validRange : Maybe ( Date, Date )
    }


init : ( Model, Cmd Msg )
init =
    ( emptyModel, getCurrentDate )


emptyModel : Model
emptyModel =
    { conf = deDateConfig
    , today = Date.fromTime 0
    , date = Date.fromTime 0
    , visible = False
    , inputStr = ""
    , inputWidget = Input.init
    , validRange = Nothing
    }



-- UPDATE


type Msg
    = NoOp
    | InitDate Date
    | NewDate Date
    | DateInput String
    | NewDateError String
    | PrevDay
    | NextDay
    | PrevMonth
    | NextMonth
    | ToggleVisibility
    | InputUpdate Input.Msg
    | SetValidRange (Maybe ( Date, Date ))


update : Msg -> Model -> Model
update msg model =
    case msg of
        NoOp ->
            model

        InputUpdate msg' ->
            let
                updated =
                    case msg' of
                        Input.Focus ->
                            { model | visible = True }

                        Input.Blur ->
                            { model | visible = False }
            in
                { updated | inputWidget = Input.update msg' model.inputWidget }

        InitDate d ->
            { model
                | date = d
                , inputStr = formatDate model.conf d
                , today = d
            }

        NewDate d ->
            { model
                | date = d
                , inputStr = formatDate model.conf d
                , visible = False
            }

        DateInput str ->
            case parseDate model.conf str of
                Nothing ->
                    { model | inputStr = str }

                Just date ->
                    { model | date = date, inputStr = str }

        NewDateError _ ->
            model

        PrevDay ->
            let
                newDate =
                    Duration.add Duration.Day -1 model.date
            in
                { model
                    | date = newDate
                    , inputStr = formatDate model.conf newDate
                }

        NextDay ->
            let
                newDate =
                    Duration.add Duration.Day 1 model.date
            in
                { model
                    | date = newDate
                    , inputStr = formatDate model.conf newDate
                }

        PrevMonth ->
            let
                newDate =
                    Duration.add Duration.Month -1 model.date
            in
                { model
                    | date = newDate
                    , inputStr = formatDate model.conf newDate
                }

        NextMonth ->
            let
                newDate =
                    Duration.add Duration.Month 1 model.date
            in
                { model
                    | date = newDate
                    , inputStr = formatDate model.conf newDate
                }

        ToggleVisibility ->
            model

        SetValidRange range ->
            { model | validRange = range }



-- VIEW


weekDays : DateConfig -> List (Html Msg)
weekDays conf =
    dayListForMonthView Nothing (Date.fromTime 0) (Date.fromTime 0)
        |> List.take 7
        |> List.map (\d -> li [] [ text (weekDayName conf d.day) ])


calendarDay : CalendarDay -> Html Msg
calendarDay date =
    li
        [ onClick (NewDate date.day)
        , classList
            [ ( "out-of-month", not date.inMonth )
            , ( "in-month", date.inMonth )
            , ( "today", date.today )
            , ( "selected", date.selected )
            , ( "valid-day", Maybe.withDefault False date.valid )
            , ( "invalid-day", not <| Maybe.withDefault True date.valid )
            ]
        ]
        [ text (toString (day date.day)) ]


calendarDays : Maybe ( Date, Date ) -> Date -> Date -> List (Html Msg)
calendarDays validRange today date =
    List.map calendarDay (dayListForMonthView validRange today date)


monthView : DateConfig -> Date -> Html Msg
monthView conf date =
    div [ class "month" ]
        [ i [ class "icon", onClick PrevMonth ] [ text "\xE314" ]
        , span [ class "month-name" ] [ text (monthAndYearStr conf date) ]
        , i [ class "icon", onClick NextMonth ] [ text "\xE315" ]
        ]


dayButtons : Html Msg
dayButtons =
    div [ class "day-buttons" ]
        [ div [] [ Button.view [ onStopAll "mousedown" PrevDay ] [ i [ class "icon" ] [ text "\xE314" ] ] ]
        , div [] [ Button.view [ onStopAll "mousedown" NextDay ] [ i [ class "icon" ] [ text "\xE315" ] ] ]
        ]


calendarView : Model -> Html Msg
calendarView model =
    div []
        [ Input.view InputUpdate
            [ onInput DateInput
            , value model.inputStr
            ]
            (Just [ dayButtons ])
            (Just "\xE878")
            model.inputWidget
        , div
            [ classList
                [ ( "paper", True )
                , ( "calendar", True )
                , ( "hide", not model.visible )
                ]
            , onStopAll "mousedown" NoOp
            ]
            [ monthView model.conf model.date
            , ul [ class "weekdays" ] (weekDays model.conf)
            , ul [ class "calendardays" ] (calendarDays model.validRange model.today model.date)
            ]
        ]


view : Model -> Html Msg
view =
    lazy calendarView



-- DATE


type alias CalendarDay =
    { day : Date
    , inMonth : Bool
    , today : Bool
    , selected : Bool
    , valid : Maybe Bool
    }


getCurrentDate : Cmd Msg
getCurrentDate =
    Task.perform NewDateError InitDate Date.now


daysInSixWeeks : Int
daysInSixWeeks =
    42


dayListForMonthView : Maybe ( Date, Date ) -> Date -> Date -> List CalendarDay
dayListForMonthView validRange today selected =
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
                    , valid = Maybe.map2 isValidDay validRange (Just date)
                    }
                )


isValidDay : ( Date, Date ) -> Date -> Bool
isValidDay ( begin, end ) day =
    Compare.is3 Compare.BetweenOpen day begin end
