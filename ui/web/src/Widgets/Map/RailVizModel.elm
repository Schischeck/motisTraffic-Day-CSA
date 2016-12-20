module Widgets.Map.RailVizModel exposing (..)

import Widgets.Map.Port exposing (..)
import Math.Vector2 as Vector2 exposing (Vec2, vec2)
import Math.Vector3 as Vector3 exposing (Vec3, vec3)
import Data.Connection.Types exposing (Station, Position, TripId)
import Time exposing (Time)
import WebGL exposing (..)
import Debounce


type alias RVTrain =
    { names : List String
    , currentSegment : List ( Vec2, Float )
    , departureTime : Time
    , arrivalTime : Time
    , scheduledDepartureTime : Time
    , scheduledArrivalTime : Time
    , departureStation : RVStation
    , arrivalStation : RVStation
    , pathLength : Float
    , currentSubSegment : Maybe CurrentSubSegment
    , pickId : Int
    , pickColor : Vec3
    , trips : List TripId
    }


type alias RVStation =
    { station : Station
    , pos : Vec2
    }


type alias CurrentSubSegment =
    { startDistance : Float
    , startPoint : Vec2
    , endPoint : Vec2
    , length : Float
    , nextSubSegmentIndex : Int
    }


type alias Model =
    { mapInfo : MapInfo
    , texture : Maybe WebGL.Texture
    , time : Time
    , systemTime : Time
    , timeOffset : Float
    , remoteAddress : String
    , allTrains : List RVTrain
    , filteredTrains : List RVTrain
    , filterTrips : Maybe (List TripId)
    , hoveredTrain : Maybe Int
    , nextUpdate : Maybe Time
    , debounce : Debounce.State
    , mouseX : Int
    , mouseY : Int
    , zoomOverride : Maybe Float
    }


applyFilter : Maybe (List TripId) -> List RVTrain -> List RVTrain
applyFilter filterTrips allTrains =
    case filterTrips of
        Just trips ->
            let
                isFiltered train =
                    List.any (\trip -> List.member trip trips) train.trips
            in
                List.filter isFiltered allTrains

        Nothing ->
            allTrains
