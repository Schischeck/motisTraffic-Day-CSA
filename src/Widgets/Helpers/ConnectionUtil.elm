module Widgets.Helpers.ConnectionUtil exposing (..)

import Date.Extra.Duration as Duration exposing (DeltaRecord)
import Html exposing (Html, div, ul, li, text, span, i)
import Html.Attributes exposing (..)
import Svg
import Svg.Attributes exposing (xlinkHref)
import Data.Connection.Types as Connection exposing (..)
import String


useLineId : Int -> Bool
useLineId class =
    class == 5 || class == 6


longTransportName : TransportInfo -> String
longTransportName transport =
    if useLineId transport.class then
        transport.line_id
    else
        transport.name


shortTransportName : TransportInfo -> String
shortTransportName transport =
    let
        train_nr =
            Maybe.withDefault 0 transport.train_nr
    in
        if useLineId transport.class then
            transport.line_id
        else if String.length transport.name < 7 then
            transport.name
        else if String.isEmpty transport.line_id && train_nr == 0 then
            transport.name
        else if String.isEmpty transport.line_id then
            toString train_nr
        else
            transport.line_id


trainIcon : Int -> String
trainIcon class =
    case class of
        0 ->
            "train"

        1 ->
            "train"

        2 ->
            "train"

        3 ->
            "train"

        4 ->
            "train"

        5 ->
            "sbahn"

        6 ->
            "ubahn"

        7 ->
            "tram"

        _ ->
            "bus"


type TransportViewMode
    = LongName
    | ShortName
    | IconOnly
    | IconOnlyNoSep


trainBox : TransportViewMode -> TransportInfo -> Html msg
trainBox viewMode t =
    let
        icon =
            Svg.svg
                [ Svg.Attributes.class "train-icon" ]
                [ Svg.use
                    [ xlinkHref <| "#" ++ (trainIcon t.class) ]
                    []
                ]

        name =
            case viewMode of
                LongName ->
                    longTransportName t

                ShortName ->
                    shortTransportName t

                IconOnly ->
                    ""

                IconOnlyNoSep ->
                    ""
    in
        div [ class <| "train-box train-class-" ++ (toString t.class) ]
            [ icon
            , if String.isEmpty name then
                text ""
              else
                span [ class "train-name" ] [ text name ]
            ]


walkBox : Html msg
walkBox =
    div [ class <| "train-box train-class-walk" ]
        [ Svg.svg
            [ Svg.Attributes.class "train-icon" ]
            [ Svg.use
                [ xlinkHref <| "#walk" ]
                []
            ]
        ]


isDelayed : DeltaRecord -> Bool
isDelayed dr =
    dr.minute > 0 || dr.hour > 0 || dr.day > 0


zeroDelay : DeltaRecord -> Bool
zeroDelay dr =
    dr.minute == 0 && dr.hour == 0 && dr.day == 0


delayText : DeltaRecord -> String
delayText dr =
    let
        str =
            abs >> toString
    in
        if dr.day /= 0 then
            (str dr.day) ++ "d " ++ (str dr.hour) ++ "h " ++ (str dr.minute) ++ "min"
        else if dr.hour /= 0 then
            (str dr.hour) ++ "h " ++ (str dr.minute) ++ "min"
        else
            str dr.minute


delay : EventInfo -> Html msg
delay event =
    let
        diff =
            Maybe.map2 Duration.diff event.time event.schedule_time
    in
        case diff of
            Just d ->
                let
                    delayed =
                        isDelayed d
                in
                    span
                        [ class <|
                            if delayed then
                                "delay pos-delay"
                            else
                                "delay neg-delay"
                        ]
                        [ text <|
                            (if delayed || (zeroDelay d) then
                                "+"
                             else
                                "-"
                            )
                                ++ (delayText d)
                        ]

            Nothing ->
                span [ class "delay" ] []
