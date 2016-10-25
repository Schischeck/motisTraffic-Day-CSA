module Util.DateFormat exposing (..)

import Date exposing (Date, Day, day, month, year, dayOfWeek)
import Date.Extra.Duration as Duration exposing (DeltaRecord)
import Date.Extra.Core exposing (monthToInt, isoDayOfWeek)
import String
import Util.StringSplit exposing (nthToken, intNthToken)
import Util.Date exposing (toDate)
import Array


twoDigits : Int -> String
twoDigits =
    toString >> String.pad 2 '0'


formatTime : Date -> String
formatTime d =
    (Date.hour d |> twoDigits) ++ ":" ++ (Date.minute d |> twoDigits)


durationText : DeltaRecord -> String
durationText dr =
    if dr.hour > 0 then
        (toString dr.hour) ++ "h " ++ (toString dr.minute) ++ "min"
    else
        (toString dr.minute) ++ "min"


type alias DateConfig =
    { seperator : String
    , shortFormatTrailingSeperator : String
    , yearPos : Int
    , monthPos : Int
    , dayPos : Int
    , weekDayNames : List String
    , monthNames : List String
    }


enDateConfig : DateConfig
enDateConfig =
    { seperator = "/"
    , shortFormatTrailingSeperator = ""
    , yearPos = 0
    , monthPos = 1
    , dayPos = 2
    , weekDayNames = [ "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" ]
    , monthNames =
        [ ""
        , "January"
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
    , shortFormatTrailingSeperator = "."
    , yearPos = 2
    , monthPos = 1
    , dayPos = 0
    , weekDayNames = [ "Mo", "Di", "Mi", "Do", "Fr", "Sa", "So" ]
    , monthNames =
        [ ""
        , "Januar"
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


formatDate : DateConfig -> Date -> String
formatDate conf d =
    Array.repeat 3 0
        |> Array.set conf.yearPos (year d)
        |> Array.set conf.monthPos (monthToInt (month d))
        |> Array.set conf.dayPos (day d)
        |> Array.toList
        |> List.map toString
        |> String.join conf.seperator


formatShortDate : DateConfig -> Date -> String
formatShortDate conf date =
    let
        d =
            toString (day date)

        m =
            toString (monthToInt (month date))
    in
        if conf.dayPos < conf.monthPos then
            d ++ conf.seperator ++ d
        else
            m ++ conf.seperator ++ d


formatShortDateTime : DateConfig -> Date -> String
formatShortDateTime conf date =
    (formatShortDate conf date)
        ++ conf.shortFormatTrailingSeperator
        ++ " "
        ++ (formatTime date)


formatDateTime : DateConfig -> Date -> String
formatDateTime conf date =
    (formatDate conf date) ++ " " ++ (formatTime date)
