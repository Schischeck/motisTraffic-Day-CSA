module Data.Connection.Decode
    exposing
        ( decodeConnection
        , decodeStation
        )

import Data.Connection.Types exposing (..)
import Json.Decode as Decode exposing ((:=))
import Json.Decode.Extra exposing ((|:), withDefault, maybeNull)
import Util.Json exposing (decodeDate)


decodeConnection : Decode.Decoder Connection
decodeConnection =
    Decode.succeed Connection
        |: ("stops" := Decode.list decodeStop)
        |: ("transports" := Decode.list decodeMove)
        |: ("attributes" := Decode.list decodeAttribute)


decodeStop : Decode.Decoder Stop
decodeStop =
    Decode.succeed Stop
        |: ("station" := decodeStation)
        |: ("arrival" := decodeEventInfo)
        |: ("departure" := decodeEventInfo)
        |: ("exit" := Decode.bool |> withDefault False)
        |: ("enter" := Decode.bool |> withDefault False)


decodeStation : Decode.Decoder Station
decodeStation =
    Decode.succeed Station
        |: ("id" := Decode.string)
        |: ("name" := Decode.string)
        |: ("pos" := decodePosition)


decodePosition : Decode.Decoder Position
decodePosition =
    Decode.succeed Position
        |: ("lat" := Decode.float)
        |: ("lng" := Decode.float)


decodeEventInfo : Decode.Decoder EventInfo
decodeEventInfo =
    Decode.succeed EventInfo
        |: ("time" := decodeDate |> Decode.maybe)
        |: ("schedule_time" := decodeDate |> Decode.maybe)
        |: ("track" := Decode.string)
        |: ("reason" := decodeTimestampReason |> withDefault Schedule)


decodeMove : Decode.Decoder Move
decodeMove =
    let
        move : String -> Decode.Decoder Move
        move move_type =
            case move_type of
                "Transport" ->
                    Decode.object1 Transport ("move" := decodeTransportInfo)

                "Walk" ->
                    Decode.object1 Walk ("move" := decodeWalkInfo)

                _ ->
                    Decode.fail ("move type " ++ move_type ++ " not supported")
    in
        ("move_type" := Decode.string) `Decode.andThen` move


decodeTransportInfo : Decode.Decoder TransportInfo
decodeTransportInfo =
    Decode.succeed TransportInfo
        |: ("range" := decodeRange)
        |: ("category_name" := Decode.string)
        |: ("clasz" := Decode.int |> withDefault 0)
        |: ("train_nr" := Decode.int |> Decode.maybe)
        |: ("line_id" := Decode.string)
        |: ("name" := Decode.string)
        |: ("provider" := Decode.string)
        |: ("direction" := Decode.string)


decodeWalkInfo : Decode.Decoder WalkInfo
decodeWalkInfo =
    Decode.succeed WalkInfo
        |: ("range" := decodeRange)
        |: ("mumo_id" := Decode.int)
        |: ("price" := Decode.int |> Decode.maybe)
        |: ("mumo_type" := Decode.string)


decodeAttribute : Decode.Decoder Attribute
decodeAttribute =
    Decode.succeed Attribute
        |: ("range" := decodeRange)
        |: ("code" := Decode.string)
        |: ("text" := Decode.string)


decodeRange : Decode.Decoder Range
decodeRange =
    Decode.succeed Range
        |: ("from" := Decode.int)
        |: ("to" := Decode.int)


decodeTimestampReason : Decode.Decoder TimestampReason
decodeTimestampReason =
    let
        decodeToType string =
            case string of
                "SCHEDULE" ->
                    Result.Ok Schedule

                "IS" ->
                    Result.Ok Is

                "REPAIR" ->
                    Result.Ok Is

                "PROPAGATION" ->
                    Result.Ok Propagation

                "FORECAST" ->
                    Result.Ok Forecast

                _ ->
                    Result.Err ("Not valid pattern for decoder to TimestampReason. Pattern: " ++ (toString string))
    in
        Decode.customDecoder Decode.string decodeToType
