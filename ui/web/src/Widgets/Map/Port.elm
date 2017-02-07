port module Widgets.Map.Port exposing (..)

import Time exposing (Time)
import Data.Connection.Types exposing (TripId)


type alias MapInfo =
    { scale : Float
    , zoom : Float
    , pixelBounds : MapPixelBounds
    , geoBounds : MapGeoBounds
    , railVizBounds : MapGeoBounds
    }


type alias MapPixelBounds =
    { north : Float
    , west : Float
    , width : Float
    , height : Float
    }


type alias MapGeoBounds =
    { north : Float
    , west : Float
    , south : Float
    , east : Float
    }


type alias MapTooltip =
    { mouseX : Int
    , mouseY : Int
    , hoveredTrain : Maybe RVTrain
    , hoveredStation : Maybe String
    }


type alias RVTrain =
    { names : List String
    , departureTime : Time
    , arrivalTime : Time
    , scheduledDepartureTime : Time
    , scheduledArrivalTime : Time
    , hasDepartureDelayInfo : Bool
    , hasArrivalDelayInfo : Bool
    , departureStation : String
    , arrivalStation : String
    }


type alias MapFlyLocation =
    { mapId : String
    , lat : Float
    , lng : Float
    }


type alias MapFitBounds =
    { mapId : String
    , coords : List (List Float)
    }


type alias RVConnectionFilter =
    { trains : List RVConnectionTrain
    , walks : List RVConnectionWalk
    , interchangeStations : List String
    , intermediateStations : List String
    }


type alias RVConnectionTrain =
    { sections : List RVConnectionSection
    , trip : Maybe TripId
    }


type alias RVConnectionSection =
    { departureStation : RVConnectionStation
    , arrivalStation : RVConnectionStation
    , scheduledDepartureTime : Time
    , scheduledArrivalTime : Time
    }


type alias RVConnectionWalk =
    { departureStation : RVConnectionStation
    , arrivalStation : RVConnectionStation
    }


type alias RVConnectionStation =
    { id : String
    , lat : Float
    , lng : Float
    }


port mapInit : String -> Cmd msg


port mapUpdate : (MapInfo -> msg) -> Sub msg


port mapSetTooltip : (MapTooltip -> msg) -> Sub msg


port mapFlyTo : MapFlyLocation -> Cmd msg


port mapFitBounds : MapFitBounds -> Cmd msg


port mapUseTrainClassColors : Bool -> Cmd msg


port mapSetConnectionFilter : RVConnectionFilter -> Cmd msg
