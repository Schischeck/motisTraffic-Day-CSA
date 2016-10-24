module Util.Date exposing (..)

import Date exposing (Date)


unixTime : Date -> Int
unixTime d =
    (floor (Date.toTime d)) // 1000
