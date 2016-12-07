module Data.RailViz.Decode exposing (decodeRailVizTrainsResponse)

import Data.RailViz.Types exposing (..)
import Json.Decode as JD
    exposing
        ( list
        , string
        , int
        , float
        , bool
        , field
        , nullable
        , succeed
        , fail
        )
import Json.Decode.Pipeline exposing (decode, required, optional, hardcoded)
import Util.Json exposing (decodeDate)
import Data.Connection.Decode exposing (decodePosition, decodeTripId)


decodeRailVizTrainsResponse : JD.Decoder RailVizTrainsResponse
decodeRailVizTrainsResponse =
    decode RailVizTrainsResponse
        |> required "trains" (list decodeRailVizTrain)
        |> required "routes" (list decodeRailVizRoute)
        |> required "stations" (list decodePosition)


decodeRailVizTrain : JD.Decoder RailVizTrain
decodeRailVizTrain =
    decode RailVizTrain
        |> required "d_time" decodeDate
        |> required "a_time" decodeDate
        |> required "sched_d_time" decodeDate
        |> required "sched_a_time" decodeDate
        |> required "route_index" int
        |> required "segment_index" int
        |> required "trip" (list decodeTripId)


decodeRailVizRoute : JD.Decoder RailVizRoute
decodeRailVizRoute =
    decode RailVizRoute
        |> required "segments" (list decodeRailVizSegment)


decodeRailVizSegment : JD.Decoder RailVizSegment
decodeRailVizSegment =
    decode RailVizSegment
        |> required "from_station_idx" int
        |> required "to_station_idx" int
        |> required "coordinates" decodePolyline


decodePolyline : JD.Decoder Polyline
decodePolyline =
    decode Polyline
        |> required "coordinates" (list float)
