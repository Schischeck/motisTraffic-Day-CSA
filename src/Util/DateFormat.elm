module Util.DateFormat exposing (..)

import Date exposing (Date)
import Date.Extra.Duration as Duration exposing (DeltaRecord)
import String


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
