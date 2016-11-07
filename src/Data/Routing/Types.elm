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
    }
