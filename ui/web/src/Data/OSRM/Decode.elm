module Data.OSRM.Decode exposing (decodeOSRMViaRouteResponse)

import Data.OSRM.Types exposing (..)
import Data.RailViz.Decode exposing (decodePolyline)
import Json.Decode as Decode exposing (string, int, float)
import Json.Decode.Pipeline exposing (decode, required, optional, hardcoded)


decodeOSRMViaRouteResponse : Decode.Decoder OSRMViaRouteResponse
decodeOSRMViaRouteResponse =
    Decode.at [ "content" ] decodeOSRMViaRouteResponseContent


decodeOSRMViaRouteResponseContent : Decode.Decoder OSRMViaRouteResponse
decodeOSRMViaRouteResponseContent =
    decode OSRMViaRouteResponse
        |> optional "time" int 0
        |> optional "distance" float 0.0
        |> required "polyline" decodePolyline
