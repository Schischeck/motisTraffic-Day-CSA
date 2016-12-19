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
        , at
        )
import Json.Decode.Pipeline exposing (decode, required, optional, hardcoded)
import Util.Json exposing (decodeDate)
import Data.Connection.Decode exposing (decodePosition, decodeTripId, decodeStation)


decodeRailVizTrainsResponse : JD.Decoder RailVizTrainsResponse
decodeRailVizTrainsResponse =
    at [ "content" ] decodeRailVizTrainsResponseContent


decodeRailVizTrainsResponseContent : JD.Decoder RailVizTrainsResponse
decodeRailVizTrainsResponseContent =
    decode RailVizTrainsResponse
        |> required "trains" (list decodeRailVizTrain)
        |> required "routes" (list decodeRailVizRoute)
        |> required "stations" (list decodeStation)


decodeRailVizTrain : JD.Decoder RailVizTrain
decodeRailVizTrain =
    decode RailVizTrain
        |> required "names" (list string)
        |> required "d_time" decodeDate
        |> required "a_time" decodeDate
        |> required "sched_d_time" decodeDate
        |> required "sched_a_time" decodeDate
        |> optional "route_index" int 0
        |> optional "segment_index" int 0
        |> required "trip" (list decodeTripId)


decodeRailVizRoute : JD.Decoder RailVizRoute
decodeRailVizRoute =
    decode RailVizRoute
        |> required "segments" (list decodeRailVizSegment)


decodeRailVizSegment : JD.Decoder RailVizSegment
decodeRailVizSegment =
    decode RailVizSegment
        |> required "from_station_id" string
        |> required "to_station_id" string
        |> required "coordinates" decodePolyline


decodePolyline : JD.Decoder Polyline
decodePolyline =
    decode Polyline
        |> required "coordinates" (list float)
