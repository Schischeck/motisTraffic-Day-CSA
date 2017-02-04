module Widgets.Map.ConnectionOverlay
    exposing
        ( hideOverlay
        , showOverlay
        )

import Widgets.Map.Port exposing (..)
import Widgets.Map.RailViz exposing (mapId)
import Data.Journey.Types exposing (..)
import Data.Connection.Types exposing (..)
import Localization.Base exposing (..)
import Util.DateFormat exposing (..)
import Widgets.Helpers.ConnectionUtil exposing (..)
import Util.List exposing ((!!), last, dropEnd)
import List.Extra
import Maybe.Extra exposing (maybeToList)
import Date exposing (Date)


hideOverlay : Cmd msg
hideOverlay =
    mapClearOverlays mapId


showOverlay : Localization -> Journey -> Cmd msg
showOverlay locale journey =
    let
        trainLines =
            List.map trainPolyline journey.trains

        walkLines =
            List.map (walkPolyline locale) journey.walks

        stops =
            stopCircles journey.connection.stops
    in
        Cmd.batch
            [ mapSetOverlays
                { mapId = mapId
                , overlays =
                    trainLines
                        ++ walkLines
                        ++ stops
                }
            , mapSetConnectionFilter (buildConnectionFilter journey)
            ]


buildConnectionFilter : Journey -> RVConnectionFilter
buildConnectionFilter journey =
    let
        interchangeStops =
            List.concatMap
                (\train ->
                    (maybeToList (List.head train.stops) ++ (maybeToList (last train.stops)))
                )
                journey.trains

        walkStops =
            List.concatMap (\walk -> [ walk.from, walk.to ]) journey.walks

        interchangeStations =
            List.map (.station >> .id) (interchangeStops ++ walkStops)
                |> List.Extra.unique

        intermediateStops =
            List.concatMap (\train -> dropEnd 1 (List.drop 1 train.stops)) journey.trains

        intermediateStations =
            List.map (.station >> .id) (intermediateStops)
                |> List.Extra.unique
                |> List.filter (\station -> not (List.member station interchangeStations))
    in
        { trains = List.map buildRVTrain journey.trains
        , walks = List.map buildRVWalk journey.walks
        , interchangeStations = interchangeStations
        , intermediateStations = intermediateStations
        }


buildRVTrain : Train -> RVConnectionTrain
buildRVTrain train =
    let
        trainSections =
            List.Extra.groupsOfWithStep 2 1 train.stops

        buildSection stops =
            case stops of
                [ dep, arr ] ->
                    Just (buildRVSection dep arr)

                _ ->
                    Debug.log ("buildRVTrain: Invalid section list: " ++ toString train) Nothing
    in
        { sections = List.filterMap buildSection trainSections
        , trip = train.trip
        }


buildRVSection : Stop -> Stop -> RVConnectionSection
buildRVSection dep arr =
    let
        toTime maybeDate =
            maybeDate
                |> Maybe.map Date.toTime
                |> Maybe.withDefault 0
    in
        { departureStation = dep.station.id
        , arrivalStation = arr.station.id
        , scheduledDepartureTime = toTime dep.departure.schedule_time
        , scheduledArrivalTime = toTime arr.arrival.schedule_time
        }


buildRVWalk : JourneyWalk -> RVConnectionWalk
buildRVWalk walk =
    { departureStation = walk.from.station.id
    , arrivalStation = walk.to.station.id
    }


trainPolyline : Train -> MapOverlay
trainPolyline train =
    { shape = "polyline"
    , latlngs = List.map stopLatLng train.stops
    , options =
        { defaultOptions
            | color = trainColor train
            , weight = Just 5
        }
    , tooltip =
        train.transports
            |> List.head
            |> Maybe.map longTransportNameWithoutIcon
    }


walkPolyline : Localization -> JourneyWalk -> MapOverlay
walkPolyline locale walk =
    { shape = "polyline"
    , latlngs = List.map stopLatLng [ walk.from, walk.to ]
    , options =
        { defaultOptions
            | color = walkColor
            , weight = Just 5
        }
    , tooltip =
        Just (locale.t.connections.walkDuration (durationText walk.duration))
    }


stopCircles : List Stop -> List MapOverlay
stopCircles stops =
    let
        interchangeOptions =
            { defaultOptions
                | color = "black"
                , fill = True
                , fillColor = Just "white"
                , radius = Just 5
                , weight = Just 2
                , fillOpacity = Just 0.8
            }

        stopCircle stop =
            { shape = "circleMarker"
            , latlngs = [ stopLatLng stop ]
            , options = interchangeOptions
            , tooltip = Just stop.station.name
            }

        lastIndex =
            List.length stops - 1
    in
        stops
            |> List.indexedMap (,)
            |> List.filter
                (\( i, s ) -> s.exit || s.enter || i == 0 || i == lastIndex)
            |> List.map (\( i, s ) -> stopCircle s)


stopLatLng : Stop -> ( Float, Float )
stopLatLng stop =
    ( stop.station.pos.lat, stop.station.pos.lng )


defaultOptions : MapOverlayOptions
defaultOptions =
    { color = "#777"
    , fill = False
    , fillColor = Nothing
    , radius = Nothing
    , weight = Nothing
    , fillOpacity = Nothing
    }


classColors : List String
classColors =
    [ "#9c27b0"
    , "#e91e63"
    , "#1a237e"
    , "#f44336"
    , "#f44336"
    , "#4caf50"
    , "#3f51b5"
    , "#ff9800"
    , "#ff9800"
    , "#9e9e9e"
    ]


defaultClassColor : String
defaultClassColor =
    "#9e9e9e"


walkColor : String
walkColor =
    "#333"


transportColor : TransportInfo -> String
transportColor transport =
    classColors
        !! transport.class
        |> Maybe.withDefault "red"


trainColor : Train -> String
trainColor train =
    List.head train.transports
        |> Maybe.map transportColor
        |> Maybe.withDefault "blue"
