module Data.Intermodal.Types exposing (..)

import Data.Connection.Types exposing (Connection, Station, Position)
import Data.Routing.Types exposing (SearchDirection, Interval)


type alias IntermodalRoutingRequest =
    { start : IntermodalStart
    , startModes : List Mode
    , destination : IntermodalDestination
    , destinationModes : List Mode
    , searchDir : SearchDirection
    }


type IntermodalStart
    = IntermodalPretripStart IntermodalPretripStartInfo
    | PretripStart PretripStartInfo


type alias IntermodalPretripStartInfo =
    { position : Position
    , interval : Interval
    }


type alias PretripStartInfo =
    { station : Station
    , interval : Interval
    , minConnectionCount : Int
    , extendIntervalEarlier : Bool
    , extendIntervalLater : Bool
    }


type IntermodalDestination
    = InputStation Station
    | InputPosition Position


type Mode
    = Foot FootModeInfo
    | Bike BikeModeInfo


type alias FootModeInfo =
    { maxDuration : Int }


type alias BikeModeInfo =
    { maxDuration : Int }


type alias IntermodalRoutingResponse =
    { connections : List Connection
    }
