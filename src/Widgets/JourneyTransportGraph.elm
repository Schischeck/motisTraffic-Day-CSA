module Widgets.JourneyTransportGraph exposing (view)

import String
import Html exposing (Html, div)
import Html.Attributes
import Html.Lazy
import Svg exposing (..)
import Svg.Attributes exposing (..)
import Date exposing (Date)
import Date.Extra.Duration as Duration exposing (DeltaRecord)
import Data.Journey.Types as Journey exposing (Journey, Train, JourneyWalk)
import Data.Connection.Types exposing (Stop)
import Widgets.Helpers.ConnectionUtil exposing (..)
import Util.Core exposing ((=>))
import Util.List exposing (last)
import Util.DateFormat exposing (formatTime)


type alias Part =
    { icon : String
    , colorClass : String
    , duration : Int
    , longName : String
    , shortName : String
    , departureStation : String
    , departureTime : Date
    , arrivalStation : String
    , arrivalTime : Date
    }


type alias DisplayPart =
    { part : Part
    , position : Float
    , barLength : Float
    , nameDisplayType : NameDisplayType
    }


type NameDisplayType
    = LongName
    | NoName



-- VIEW


view : Int -> String -> Journey -> Html msg
view totalWidth idBase journey =
    Html.Lazy.lazy3 graphView totalWidth idBase journey


graphView : Int -> String -> Journey -> Html msg
graphView totalWidth idBase journey =
    div [ class "transport-graph" ]
        [ transportsView totalWidth idBase journey ]


transportsView : Int -> String -> Journey -> Svg msg
transportsView totalWidth idBase journey =
    let
        parts =
            journeyParts journey |> layoutParts totalWidth LongName

        renderedParts =
            List.map (partView totalWidth idBase) parts
    in
        svg
            [ width (toString totalWidth)
            , height (toString totalHeight)
            , viewBox <| "0 0 " ++ (toString totalWidth) ++ " " ++ (toString totalHeight)
            ]
            [ g [] (List.map fst renderedParts)
            , destinationView totalWidth
            , g [] (List.map snd renderedParts)
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


partView : Int -> String -> DisplayPart -> ( Svg msg, Svg msg )
partView totalWidth idBase { part, position, barLength, nameDisplayType } =
    let
        radius =
            toString circleRadius

        partWidth =
            (circleRadius * 2) + barLength

        lineEnd =
            position + partWidth + (destinationRadius / 2)

        trainName =
            case nameDisplayType of
                LongName ->
                    [ text'
                        [ x (toString <| position)
                        , y (toString <| textOffset + textHeight)
                        , textAnchor "start"
                        , class "train-name"
                        ]
                        [ text part.longName ]
                    ]

                NoName ->
                    []

        elementId =
            idBase ++ "_" ++ (position |> floor |> toString)

        graphPart =
            g
                [ class <| "train-class-" ++ part.colorClass ]
            <|
                [ line
                    [ x1 (toString <| position)
                    , y1 radius
                    , x2 (toString <| lineEnd)
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
                    ++ trainName

        tooltipX =
            Basics.min position ((toFloat totalWidth) - tooltipWidth)

        tooltip =
            g []
                [ g
                    [ visibility "hidden"
                    , class "tooltip"
                    ]
                    [ switch []
                        [ foreignObject
                            [ x (tooltipX |> toString)
                            , y (textOffset - 5 |> toString)
                            , width (tooltipWidth |> toString)
                            , height "50"
                            ]
                            [ Html.div
                                [ Html.Attributes.class "tooltip" ]
                                [ Html.div [ Html.Attributes.class "stations" ]
                                    [ Html.div [ Html.Attributes.class "departure" ]
                                        [ Html.span [ Html.Attributes.class "station" ]
                                            [ text part.departureStation ]
                                        , Html.span [ Html.Attributes.class "time" ]
                                            [ text (formatTime part.departureTime) ]
                                        ]
                                    , Html.div [ Html.Attributes.class "arrival" ]
                                        [ Html.span [ Html.Attributes.class "station" ]
                                            [ text part.arrivalStation ]
                                        , Html.span [ Html.Attributes.class "time" ]
                                            [ text (formatTime part.arrivalTime) ]
                                        ]
                                    ]
                                , Html.div [ Html.Attributes.class "transport-name" ]
                                    [ Html.span [] [ Html.text part.longName ] ]
                                ]
                            ]
                        ]
                    , set
                        [ attributeName "visibility"
                        , from "hidden"
                        , to "visible"
                        , begin (elementId ++ ".mouseover")
                        , end (elementId ++ ".mouseout")
                        ]
                        []
                    ]
                , rect
                    [ x (position |> toString)
                    , y "0"
                    , width (position + partWidth |> toString)
                    , height (circleRadius * 2 |> toString)
                    , class "background"
                    , id elementId
                    ]
                    []
                ]
    in
        ( graphPart, tooltip )


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

        departure =
            List.head train.stops

        arrival =
            last train.stops

        ( departureStation, departureTime ) =
            Maybe.map departureInfo departure
                |> Maybe.withDefault ( "", Date.fromTime 0 )

        ( arrivalStation, arrivalTime ) =
            Maybe.map arrivalInfo arrival
                |> Maybe.withDefault ( "", Date.fromTime 0 )

        base =
            { icon = "train"
            , colorClass = "0"
            , duration = trainDuration train
            , longName = ""
            , shortName = ""
            , departureStation = departureStation
            , departureTime = departureTime
            , arrivalStation = arrivalStation
            , arrivalTime = arrivalTime
            }
    in
        case transport of
            Just t ->
                { base
                    | icon = trainIcon t.class
                    , colorClass = toString t.class
                    , longName = longTransportNameWithoutIcon t
                    , shortName = shortTransportName t
                }

            Nothing ->
                base


walkPart : JourneyWalk -> Part
walkPart walk =
    let
        ( departureStation, departureTime ) =
            departureInfo walk.from

        ( arrivalStation, arrivalTime ) =
            arrivalInfo walk.to
    in
        { icon = "walk"
        , colorClass = "walk"
        , duration = deltaRecordToMinutes walk.duration
        , longName = ""
        , shortName = ""
        , departureStation = departureStation
        , departureTime = departureTime
        , arrivalStation = arrivalStation
        , arrivalTime = arrivalTime
        }


departureInfo : Stop -> ( String, Date )
departureInfo stop =
    let
        station =
            stop.station.name

        time =
            stop.departure.schedule_time
                |> Maybe.withDefault (Date.fromTime 0)
    in
        ( station, time )


arrivalInfo : Stop -> ( String, Date )
arrivalInfo stop =
    let
        station =
            stop.station.name

        time =
            stop.arrival.schedule_time
                |> Maybe.withDefault (Date.fromTime 0)
    in
        ( station, time )


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


layoutParts : Int -> NameDisplayType -> List Part -> List DisplayPart
layoutParts totalWidth nameDisplayType parts =
    let
        totalDuration =
            getTotalDuration parts |> toFloat

        partCount =
            List.length parts

        baseBarLength =
            2

        basePartSize =
            circleRadius * 2

        destinationWidth =
            destinationRadius * 2

        avgCharWidth =
            7

        requiredBaseBarLength part =
            case nameDisplayType of
                LongName ->
                    baseBarLength
                        + (Basics.max
                            0
                            (avgCharWidth * (String.length part.longName) - basePartSize)
                          )

                NoName ->
                    baseBarLength

        totalBaseBarLength =
            parts
                |> List.map requiredBaseBarLength
                |> List.sum

        scaleLineSpace =
            totalWidth
                - (partCount * basePartSize)
                - totalBaseBarLength
                - destinationWidth
                |> toFloat

        getBarLength part =
            (requiredBaseBarLength part |> toFloat)
                + (((toFloat part.duration) / totalDuration) * scaleLineSpace)

        layout part ( pos, results ) =
            let
                displayPart =
                    { part = part
                    , position = pos
                    , barLength = getBarLength part
                    , nameDisplayType = nameDisplayType
                    }

                nextPos =
                    pos + basePartSize + displayPart.barLength
            in
                ( nextPos, results ++ [ displayPart ] )

        ( _, displayParts ) =
            List.foldl layout ( 0, [] ) parts
    in
        if scaleLineSpace <= 0 && nameDisplayType /= NoName then
            layoutParts totalWidth NoName parts
        else
            displayParts


iconSize : number
iconSize =
    16


circleRadius : number
circleRadius =
    12


iconOffset : Float
iconOffset =
    ((circleRadius * 2) - iconSize) / 2


destinationRadius : number
destinationRadius =
    6


textOffset : number
textOffset =
    circleRadius * 2 + 4


textHeight : number
textHeight =
    12


totalHeight : number
totalHeight =
    textOffset + textHeight


tooltipWidth : number
tooltipWidth =
    240
