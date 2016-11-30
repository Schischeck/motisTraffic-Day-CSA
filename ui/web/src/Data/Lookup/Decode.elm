module Data.Lookup.Decode exposing (decodeTripToConnectionResponse)

import Data.Connection.Types exposing (Connection)
import Data.Connection.Decode exposing (decodeConnection)
import Json.Decode as Decode


decodeTripToConnectionResponse : Decode.Decoder Connection
decodeTripToConnectionResponse =
    Decode.at [ "content" ] decodeConnection
