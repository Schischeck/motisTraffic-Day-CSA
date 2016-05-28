module Widgets.Calendar exposing (Model, Msg, init, subscriptions, update, view)

import Html exposing (..)
import Html.Events exposing (onClick, onInput, onWithOptions)
import Html.Attributes exposing (..)
import Date exposing (Date, Day, day, month, year, dayOfWeek)
import Date.Extra.Duration as Duration
import Date.Extra.Core exposing (lastOfMonthDate, intToMonth, monthToInt, toFirstOfMonth, isoDayOfWeek)
import Date.Extra.Utils exposing (dayList)
import Date.Extra.Compare as Compare
import Date.Extra.Create exposing (dateFromFields)
import Task
import String
import Array
import Mouse
import Json.Decode as Json
import Widgets.Input as Input


-- MODEL


type alias Model =
    { conf : DateConfig
    , today : Date
    , date : Date
    , inputStr : String
    , visible : Bool
    }


init : ( Model, Cmd Msg )
init =
    ( emptyModel, getCurrentDate )


emptyModel : Model
emptyModel =
    { conf = enDateConfig
    , today = Date.fromTime 0
    , date = Date.fromTime 0
    , visible = False
    , inputStr = ""
    }



-- UPDATE


type Msg
    = NoOp
    | InitDate Date
    | NewDate Date
    | DateInput String
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
        NoOp ->
            model

        InitDate d ->
            { model | date = d, inputStr = formatDate model.conf d, today = d }

        NewDate d ->
            { model | date = d, inputStr = formatDate model.conf d, visible = False }

        DateInput str ->
            case parseDate model.conf str of
                Nothing ->
                    { model | inputStr = str }

                Just date ->
                    { model | date = date, inputStr = str }

        NewDateError _ ->
            model

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


onPreventDefault : String -> msg -> Html.Attribute msg
onPreventDefault event msg =
    onWithOptions event { stopPropagation = True, preventDefault = False } (Json.succeed msg)


weekDays : DateConfig -> List (Html Msg)
weekDays conf =
    dayListForMonthView (Date.fromTime 0) (Date.fromTime 0)
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
            ]
        ]
        [ text (toString (day date.day)) ]


calendarDays : Date -> Date -> List (Html Msg)
calendarDays today date =
    dayListForMonthView today date |> List.map calendarDay


monthView : DateConfig -> Date -> Html Msg
monthView conf date =
    div [ class "month" ]
        [ i [ class "icon", onClick PrevMonth ] [ text "\xE314" ]
        , span [ class "month-name" ] [ text (monthAndYearStr conf date) ]
        , i [ class "icon", onClick NextMonth ] [ text "\xE315" ]
        ]


view : Model -> Html Msg
view model =
    div []
        [ Input.view
            [ onPreventDefault "mousedown" ToggleVisibility
            , onInput DateInput
            , value model.inputStr
            ]
        , div
            [ classList
                [ ( "paper", True )
                , ( "calendar", True )
                , ( "hide", not model.visible )
                ]
            , onStop "mousedown" NoOp
            ]
            [ monthView model.conf model.date
            , ul [ class "weekdays" ] (weekDays model.conf)
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


formatDate : DateConfig -> Date -> String
formatDate conf d =
    Array.repeat 3 0
        |> Array.set conf.yearPos (year d)
        |> Array.set conf.monthPos (monthToInt (month d))
        |> Array.set conf.dayPos (day d)
        |> Array.toList
        |> List.map toString
        |> String.join conf.seperator


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


monthName : DateConfig -> Date -> String
monthName conf date =
    List.drop (monthToInt (month date)) conf.monthNames
        |> List.head
        |> Maybe.withDefault ""


weekDayName : DateConfig -> Date -> String
weekDayName conf date =
    List.drop (isoDayOfWeek (dayOfWeek date) - 1) conf.weekDayNames
        |> List.head
        |> Maybe.withDefault ""


monthAndYearStr : DateConfig -> Date -> String
monthAndYearStr conf d =
    [ monthName conf d, toString (year d) ]
        |> String.join " "


type alias DateConfig =
    { seperator : String
    , yearPos : Int
    , monthPos : Int
    , dayPos : Int
    , weekDayNames : List String
    , monthNames : List String
    }


enDateConfig : DateConfig
enDateConfig =
    { seperator = "/"
    , yearPos = 2
    , monthPos = 1
    , dayPos = 0
    , weekDayNames = [ "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" ]
    , monthNames =
        [ "January"
        , "February"
        , "March"
        , "April"
        , "May"
        , "June"
        , "July"
        , "August"
        , "September"
        , "October"
        , "November"
        , "December"
        ]
    }


deDateConfig : DateConfig
deDateConfig =
    { seperator = "."
    , yearPos = 0
    , monthPos = 1
    , dayPos = 2
    , weekDayNames = [ "Mo", "Di", "Mi", "Do", "Fr", "Sa", "So" ]
    , monthNames =
        [ "Januar"
        , "Februar"
        , "März"
        , "April"
        , "Mai"
        , "Juni"
        , "Juli"
        , "August"
        , "September"
        , "Oktober"
        , "November"
        , "Dezember"
        ]
    }


nthToken : Int -> String -> String -> Maybe String
nthToken pos splitToken str =
    String.split splitToken str
        |> List.drop pos
        |> List.head


intNthToken : Int -> String -> String -> Maybe Int
intNthToken pos splitToken str =
    case nthToken pos splitToken str of
        Just str ->
            String.toInt str |> Result.toMaybe

        Nothing ->
            Nothing


toDate : Int -> Int -> Int -> Date
toDate year month day =
    dateFromFields year (intToMonth month) day 0 0 0 0


parseDate : DateConfig -> String -> Maybe Date
parseDate conf str =
    let
        year =
            intNthToken conf.yearPos conf.seperator str

        month =
            intNthToken conf.monthPos conf.seperator str

        day =
            intNthToken conf.dayPos conf.seperator str
    in
        Maybe.map3 toDate year month day
