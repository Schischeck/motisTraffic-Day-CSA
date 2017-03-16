module Widgets.Map.ConnectionDetails
    exposing
        ( setConnectionFilter
        )

import Widgets.Map.Port exposing (..)
import Widgets.Map.RailViz exposing (mapId)
import Data.Journey.Types exposing (..)
import Data.Connection.Types exposing (..)
import Util.List exposing ((!!), last, dropEnd)
import List.Extra
import Maybe.Extra exposing (maybeToList)
import Date exposing (Date)


setConnectionFilter : Journey -> Cmd msg
setConnectionFilter journey =
    let
        ( connectionFilter, bounds ) =
            buildConnectionFilter journey
    in
        Cmd.batch
            [ mapSetConnectionFilter connectionFilter
            , mapFitBounds bounds
            ]


buildConnectionFilter : Journey -> ( RVConnectionFilter, MapFitBounds )
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

        bounds =
            (interchangeStops ++ intermediateStops ++ walkStops)
                |> List.map (.station >> .pos)
                |> List.map (\pos -> [ pos.lat, pos.lng ])
                |> List.Extra.unique
    in
        ( { trains = List.map buildRVTrain journey.trains
          , walks = List.map buildRVWalk journey.walks
          , interchangeStations = interchangeStations
          , intermediateStations = intermediateStations
          }
        , { mapId = mapId
          , coords = bounds
          }
        )


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
        { departureStation = dep.station
        , arrivalStation = arr.station
        , scheduledDepartureTime = toTime dep.departure.schedule_time
        , scheduledArrivalTime = toTime arr.arrival.schedule_time
        }


buildRVWalk : JourneyWalk -> RVConnectionWalk
buildRVWalk walk =
    { departureStation = walk.from.station
    , arrivalStation = walk.to.station
    , polyline = walk.polyline
    }
