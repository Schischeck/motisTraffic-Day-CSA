module Widgets.DateHeaders exposing (..)

import Html exposing (Html, div, ul, li, text, span, i, a)
import Html.Attributes exposing (..)
import Date exposing (Date)
import Util.DateFormat exposing (..)
import Util.Date exposing (isSameDay, unixTime)
import Localization.Base exposing (..)


dateHeader : Localization -> Date -> Html msg
dateHeader { dateConfig } date =
    div [ class "date-header divider" ] [ span [] [ text <| formatDate dateConfig date ] ]



-- b = Html msg
--   | (String, Html msg) for keyed nodes


withDateHeaders :
    (a -> Date)
    -> (a -> b)
    -> (Date -> b)
    -> List a
    -> List b
withDateHeaders getDate renderElement renderDateHeader elements =
    let
        f element ( lastDate, result ) =
            let
                currentDate =
                    getDate element

                base =
                    if not (isSameDay currentDate lastDate) then
                        result ++ [ renderDateHeader currentDate ]
                    else
                        result
            in
                ( currentDate, base ++ [ renderElement element ] )

        ( _, result ) =
            List.foldl f ( Date.fromTime 0, [] ) elements
    in
        result
