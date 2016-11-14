module Data.Connection.Decode
    exposing
        ( decodeConnection
        , decodeStation
        )

import Data.Connection.Types exposing (..)
import Json.Decode as Decode
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


decodeConnection : Decode.Decoder Connection
decodeConnection =
    decode Connection
        |> required "stops" (list decodeStop)
        |> required "transports" (list decodeMove)
        |> required "attributes" (list decodeAttribute)


decodeStop : Decode.Decoder Stop
decodeStop =
    decode Stop
        |> required "station" decodeStation
        |> required "arrival" decodeEventInfo
        |> required "departure" decodeEventInfo
        |> optional "exit" bool False
        |> optional "enter" bool False


decodeStation : Decode.Decoder Station
decodeStation =
    decode Station
        |> required "id" string
        |> required "name" string
        |> required "pos" decodePosition


decodePosition : Decode.Decoder Position
decodePosition =
    decode Position
        |> required "lat" float
        |> required "lng" float


decodeEventInfo : Decode.Decoder EventInfo
decodeEventInfo =
    decode EventInfo
        |> optional "time" (nullable decodeDate) Nothing
        |> optional "schedule_time" (nullable decodeDate) Nothing
        |> required "track" string
        |> optional "reason" decodeTimestampReason Schedule


decodeMove : Decode.Decoder Move
decodeMove =
    let
        move : String -> Decode.Decoder Move
        move move_type =
            case move_type of
                "Transport" ->
                    decode Transport
                        |> required "move" decodeTransportInfo

                "Walk" ->
                    decode Walk
                        |> required "move" decodeWalkInfo

                _ ->
                    Decode.fail ("move type " ++ move_type ++ " not supported")
    in
        (field "move_type" string) |> Decode.andThen move


decodeTransportInfo : Decode.Decoder TransportInfo
decodeTransportInfo =
    decode TransportInfo
        |> required "range" decodeRange
        |> required "category_name" string
        |> optional "clasz" int 0
        |> optional "train_nr" (nullable int) Nothing
        |> required "line_id" string
        |> required "name" string
        |> required "provider" string
        |> required "direction" string


decodeWalkInfo : Decode.Decoder WalkInfo
decodeWalkInfo =
    decode WalkInfo
        |> required "range" decodeRange
        |> required "mumo_id" int
        |> optional "price" (nullable int) Nothing
        |> required "mumo_type" string


decodeAttribute : Decode.Decoder Attribute
decodeAttribute =
    decode Attribute
        |> required "range" decodeRange
        |> required "code" string
        |> required "text" string


decodeRange : Decode.Decoder Range
decodeRange =
    decode Range
        |> required "from" int
        |> required "to" int


decodeTimestampReason : Decode.Decoder TimestampReason
decodeTimestampReason =
    let
        decodeToType string =
            case string of
                "SCHEDULE" ->
                    succeed Schedule

                "IS" ->
                    succeed Is

                "REPAIR" ->
                    succeed Is

                "PROPAGATION" ->
                    succeed Propagation

                "FORECAST" ->
                    succeed Forecast

                _ ->
                    fail ("Not valid pattern for decoder to TimestampReason. Pattern: " ++ (toString string))
    in
        Decode.string |> Decode.andThen decodeToType
