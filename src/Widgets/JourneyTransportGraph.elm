module Widgets.JourneyTransportGraph
    exposing
        ( Model
        , Msg
        , init
        , update
        , hideTooltips
        , view
        )

import String
import Html exposing (Html, div)
import Html.Attributes
import Html.Lazy
import Svg exposing (..)
import Svg.Attributes exposing (..)
import Svg.Events exposing (..)
import Date exposing (Date)
import Date.Extra.Duration as Duration exposing (DeltaRecord)
import Data.Journey.Types as Journey exposing (Journey, Train, JourneyWalk)
import Data.Connection.Types exposing (Stop)
import Widgets.Helpers.ConnectionUtil exposing (..)
import Util.List exposing (last)
import Util.DateFormat exposing (formatTime)
import Localization.Base exposing (..)


-- MODEL


type alias Model =
    { displayParts : List DisplayPart
    , totalWidth : Int
    , hover : Maybe DisplayPart
    }


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


init : Int -> Journey -> Model
init totalWidth journey =
    let
        parts =
            journeyParts journey |> layoutParts totalWidth LongName
    in
        { displayParts = parts
        , totalWidth = totalWidth
        , hover = Nothing
        }



-- UPDATE


type Msg
    = MouseOver DisplayPart
    | MouseOut DisplayPart


update : Msg -> Model -> Model
update msg model =
    case msg of
        MouseOver part ->
            { model | hover = Just part }

        MouseOut part ->
            case model.hover of
                Just hoveredPart ->
                    if hoveredPart == part then
                        { model | hover = Nothing }
                    else
                        model

                _ ->
                    model


hideTooltips : Model -> Model
hideTooltips model =
    { model | hover = Nothing }



-- VIEW


view : Localization -> Model -> Html Msg
view locale model =
    Html.Lazy.lazy2 graphView locale model


graphView : Localization -> Model -> Html Msg
graphView locale model =
    div [ class "transport-graph" ]
        [ transportsView locale model ]


transportsView : Localization -> Model -> Svg Msg
transportsView locale model =
    let
        isHovered displayPart =
            case model.hover of
                Just hoveredPart ->
                    hoveredPart == displayPart

                Nothing ->
                    False

        renderedParts =
            List.map
                (\p -> partView locale model.totalWidth (isHovered p) p)
                model.displayParts
    in
        svg
            [ width (toString model.totalWidth)
            , height (toString totalHeight)
            , viewBox <| "0 0 " ++ (toString model.totalWidth) ++ " " ++ (toString totalHeight)
            ]
            [ g [] (List.map fst renderedParts)
            , destinationView model.totalWidth
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


partView : Localization -> Int -> Bool -> DisplayPart -> ( Svg Msg, Svg Msg )
partView locale totalWidth tooltipVisible displayPart =
    let
        { part, position, barLength, nameDisplayType } =
            displayPart

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

        tooltipVisiblity =
            if tooltipVisible then
                "visible"
            else
                "hidden"

        tooltipTransportName =
            if part.icon == "walk" then
                locale.t.connections.walk
            else
                part.longName

        tooltip =
            g []
                [ g
                    [ visibility tooltipVisiblity
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
                                    [ Html.span [] [ Html.text tooltipTransportName ] ]
                                ]
                            ]
                        ]
                    ]
                , rect
                    [ x (position |> toString)
                    , y "0"
                    , width (position + partWidth |> toString)
                    , height (circleRadius * 2 |> toString)
                    , class "tooltipTrigger"
                    , onMouseOver (MouseOver displayPart)
                    , onMouseOut (MouseOut displayPart)
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
