module Data.RailViz.Types exposing (..)

import Date exposing (Date)
import Data.Connection.Types exposing (Position, TripId)


type alias RailVizTrainsRequest =
    { corner1 : Position
    , corner2 : Position
    , startTime : Date
    , endTime : Date
    }


type alias RailVizTrainsResponse =
    { trains : List RailVizTrain
    , routes : List RailVizRoute
    , stations : List Position
    }


type alias RailVizTrain =
    { depTime : Date
    , arrTime : Date
    , scheduledDepTime : Date
    , scheduledArrTime : Date
    , routeIndex : Int
    , segmentIndex : Int
    , trip : List TripId
    }


type alias RailVizSegment =
    { fromStationIdx : Int
    , toStationIdx : Int
    , coordinates : Polyline
    }


type alias RailVizRoute =
    { segments : List RailVizSegment }


type alias Polyline =
    { coordinates : List Float }
