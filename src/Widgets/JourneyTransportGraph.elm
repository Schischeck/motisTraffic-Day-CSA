module Widgets.JourneyTransportGraph exposing (view)

import Html exposing (Html, div)
import Html.Lazy
import Svg exposing (..)
import Svg.Attributes exposing (..)
import String
import Date exposing (Date)
import Date.Extra.Duration as Duration exposing (DeltaRecord)
import Data.Connection.Types as Connection exposing (Connection, Stop)
import Data.Journey.Types as Journey exposing (Journey, Train, JourneyWalk)
import Widgets.Helpers.ConnectionUtil exposing (..)
import Util.List exposing (last)


type alias Part =
    { icon : String
    , colorClass : String
    , duration : Int
    , longName : String
    , shortName : String
    }


type alias DisplayPart =
    { part : Part
    , position : Float
    , barLength : Float
    }



-- VIEW


view : Int -> Journey -> Html msg
view totalWidth journey =
    Html.Lazy.lazy2 graphView totalWidth journey


graphView : Int -> Journey -> Html msg
graphView totalWidth journey =
    div [ class "transport-graph" ]
        [ transportsView totalWidth journey ]


transportsView : Int -> Journey -> Svg msg
transportsView totalWidth journey =
    let
        parts =
            journeyParts journey |> layoutParts totalWidth
    in
        svg
            [ width (toString totalWidth)
            , height (toString totalHeight)
            , viewBox <| "0 0 " ++ (toString totalWidth) ++ " " ++ (toString totalHeight)
            ]
            [ g [] (List.map partView parts)
            , destinationView totalWidth
            ]


destinationView : Int -> Svg msg
destinationView totalWidth =
    g [ class "destination" ]
        [ circle
            [ cx (toString <| totalWidth - destinationRadius)
            , cy (toString circleRadius)
            , r (toString destinationRadius)
            ]
            []
        ]


partView : DisplayPart -> Svg msg
partView { part, position, barLength } =
    let
        radius =
            toString circleRadius
    in
        g [ class <| "train-class-" ++ part.colorClass ]
            [ line
                [ x1 (toString <| position)
                , y1 radius
                , x2 (toString <| position + (circleRadius * 2) + barLength)
                , y2 radius
                , class "train-line"
                ]
                []
            , circle
                [ cx (toString <| position + circleRadius)
                , cy radius
                , r radius
                , class "train-circle"
                ]
                []
            , use
                [ xlinkHref <| "#" ++ part.icon
                , class "train-icon"
                , x (toString <| position + iconOffset)
                , y (toString <| iconOffset)
                , width (toString <| iconSize)
                , height (toString <| iconSize)
                ]
                []
            ]


getTotalDuration : List Part -> Int
getTotalDuration parts =
    List.map .duration parts |> List.sum


journeyParts : Journey -> List Part
journeyParts { trains, leadingWalk, trailingWalk } =
    let
        lw =
            case leadingWalk of
                Just walk ->
                    [ walkPart walk ]

                Nothing ->
                    []

        tw =
            case trailingWalk of
                Just walk ->
                    [ walkPart walk ]

                Nothing ->
                    []

        trainParts =
            List.map trainPart trains
    in
        lw ++ trainParts ++ tw


trainPart : Train -> Part
trainPart train =
    let
        transport =
            List.head train.transports

        base =
            { icon = "train"
            , colorClass = "0"
            , duration = trainDuration train
            , longName = ""
            , shortName = ""
            }
    in
        case transport of
            Just t ->
                { base
                    | icon = trainIcon t.class
                    , colorClass = toString t.class
                    , longName = longTransportName t
                    , shortName = shortTransportName t
                }

            Nothing ->
                base


walkPart : JourneyWalk -> Part
walkPart walk =
    { icon = "walk"
    , colorClass = "walk"
    , duration = deltaRecordToMinutes walk.duration
    , longName = ""
    , shortName = ""
    }


deltaRecordToMinutes : DeltaRecord -> Int
deltaRecordToMinutes dr =
    dr.minute + 60 * dr.hour + 1440 * dr.day


walkDuration : JourneyWalk -> Int
walkDuration walk =
    deltaRecordToMinutes walk.duration


trainDuration : Train -> Int
trainDuration { stops } =
    let
        departure : Maybe Date
        departure =
            (List.head stops) `Maybe.andThen` (.departure >> .schedule_time)

        arrival : Maybe Date
        arrival =
            (last stops) `Maybe.andThen` (.arrival >> .schedule_time)

        minutesBetween : Date -> Date -> Int
        minutesBetween from to =
            deltaRecordToMinutes <| Duration.diff to from
    in
        Maybe.map2 minutesBetween departure arrival |> Maybe.withDefault 0


layoutParts : Int -> List Part -> List DisplayPart
layoutParts totalWidth parts =
    let
        totalDuration =
            getTotalDuration parts |> toFloat

        partCount =
            List.length parts

        basePartSize =
            circleRadius * 2

        destinationWidth =
            destinationRadius * 2

        totalLineSpace =
            totalWidth
                - (partCount * basePartSize)
                - destinationWidth
                |> toFloat

        getBarLength part =
            ((toFloat part.duration) / totalDuration) * totalLineSpace

        layout part ( pos, results ) =
            let
                displayPart =
                    { part = part
                    , position = pos
                    , barLength = getBarLength part
                    }

                nextPos =
                    pos + basePartSize + displayPart.barLength
            in
                ( nextPos, results ++ [ displayPart ] )

        ( _, displayParts ) =
            List.foldl layout ( 0, [] ) parts
    in
        displayParts


iconSize : number
iconSize =
    16


circleRadius : number
circleRadius =
    12


totalHeight : number
totalHeight =
    circleRadius * 2


iconOffset : Int
iconOffset =
    ((circleRadius * 2) - iconSize) // 2


destinationRadius : number
destinationRadius =
    6
