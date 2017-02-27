module Data.Routing.Types exposing (..)

import Date exposing (Date)
import Data.Connection.Types exposing (Connection, Station)


type alias RoutingResponse =
    { connections : List Connection
    , intervalStart : Date
    , intervalEnd : Date
    }


type alias RoutingRequest =
    { from : Station
    , to : Station
    , intervalStart : Int
    , intervalEnd : Int
    , minConnectionCount : Int
    , searchDirection : SearchDirection
    , extendIntervalEarlier : Bool
    , extendIntervalLater : Bool
    }


type SearchDirection
    = Forward
    | Backward


type alias Interval =
    { begin : Int
    , end : Int
    }
