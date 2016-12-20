module Widgets.Map.RailVizHandler exposing (..)

import Widgets.Map.RailVizModel exposing (..)
import Widgets.Map.GeoUtil exposing (..)
import Widgets.Map.Trains as Trains
import Data.RailViz.Types exposing (..)
import Math.Vector2 as Vector2 exposing (Vec2, vec2)
import Data.Connection.Types exposing (Station, Position, TripId)
import Date exposing (Date)
import List.Extra
import Util.List exposing ((!!))


handleRailVizTrainsResponse : RailVizTrainsResponse -> Model -> Model
handleRailVizTrainsResponse response model =
    let
        rvStations =
            List.map toRVStation response.stations

        getSegment routeIdx segmentIdx =
            response.routes
                !! routeIdx
                |> Maybe.andThen (\r -> r.segments !! segmentIdx)

        getStation id =
            List.Extra.find (\s -> s.station.id == id) rvStations

        buildTrain : RailVizTrain -> Int -> RVStation -> RVStation -> RailVizSegment -> RVTrain
        buildTrain train pickId depStation arrStation segment =
            let
                rvSeg =
                    convertSegment segment

                totalLength =
                    List.foldr (\( _, l ) s -> s + l) 0.0 rvSeg
            in
                { names = train.names
                , currentSegment = rvSeg
                , departureTime = Date.toTime train.depTime
                , arrivalTime = Date.toTime train.arrTime
                , scheduledDepartureTime = Date.toTime train.scheduledDepTime
                , scheduledArrivalTime = Date.toTime train.scheduledArrTime
                , departureStation = depStation
                , arrivalStation = arrStation
                , pathLength = totalLength
                , currentSubSegment = Nothing
                , pickId = pickId
                , pickColor = Trains.toPickColor pickId
                , trips = train.trip
                }

        tryBuildTrain : Int -> RailVizTrain -> Maybe RVTrain
        tryBuildTrain pickId train =
            let
                seg =
                    getSegment train.routeIndex train.segmentIndex

                depStation =
                    seg |> Maybe.map .fromStationId |> Maybe.andThen getStation

                arrStation =
                    seg |> Maybe.map .toStationId |> Maybe.andThen getStation
            in
                Maybe.map3
                    (buildTrain train pickId)
                    depStation
                    arrStation
                    seg

        allTrains =
            response.trains
                |> List.indexedMap tryBuildTrain
                |> List.filterMap identity

        filteredTrains =
            applyFilter model.filterTrips allTrains
    in
        { model
            | allTrains = allTrains
            , filteredTrains = filteredTrains
            , stations = rvStations
        }


convertSegment : RailVizSegment -> List ( Vec2, Float )
convertSegment seg =
    let
        latLngToVec2 lat lng =
            let
                ( x, y ) =
                    latLngToWorldCoord lat lng
            in
                vec2 x y

        convertCoords : List Float -> Maybe Vec2
        convertCoords l =
            case l of
                [ lat, lng ] ->
                    Just (latLngToVec2 lat lng)

                _ ->
                    let
                        _ =
                            Debug.log "RailViz: Invalid Polyline (1)" <|
                                seg
                    in
                        Nothing

        coords : List Vec2
        coords =
            List.Extra.groupsOf 2 seg.coordinates.coordinates
                |> List.filterMap convertCoords

        toOffsets : Vec2 -> ( Vec2, List Vec2 ) -> ( Vec2, List Vec2 )
        toOffsets pt ( last, result ) =
            ( pt, (Vector2.sub pt last) :: result )
    in
        case coords of
            firstPt :: pts ->
                pts
                    |> List.foldl toOffsets ( firstPt, [] )
                    |> Tuple.second
                    |> List.map (\v -> ( v, Vector2.length v ))

            _ ->
                let
                    _ =
                        Debug.log "RailViz: Invalid Polyline (2)" <|
                            seg
                in
                    []


toRVStation : Station -> RVStation
toRVStation station =
    { station = station
    , pos = positionToVec2 station.pos
    }
