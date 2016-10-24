module Util.Date exposing (..)

import Date exposing (Date)
import Date.Extra.Create exposing (dateFromFields)


unixTime : Date -> Int
unixTime d =
    (floor (Date.toTime d)) // 1000


combineDateTime : Date.Date -> Date.Date -> Date.Date
combineDateTime date time =
    dateFromFields (Date.year date)
        (Date.month date)
        (Date.day date)
        (Date.hour time)
        (Date.minute time)
        (Date.second time)
        (Date.millisecond time)
