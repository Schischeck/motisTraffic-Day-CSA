module Widgets.Data.Connection exposing (..)

import Json.Decode as Decode exposing ((:=))
import Json.Decode.Extra exposing ((|:), withDefault, maybeNull)
import Date exposing (Date, fromTime)


type alias Connection =
    { stops : List Stop
    , transports : List Move
    , attributes : List Attribute
    }


type alias Stop =
    { station : Station
    , arrival : EventInfo
    , departure : EventInfo
    , interchange : Bool
    }


type alias Station =
    { id : String
    , name : String
    , pos : Position
    }


type alias Position =
    { lat : Float
    , lng : Float
    }


type alias EventInfo =
    { time : Maybe Date
    , schedule_time : Maybe Date
    , track : String
    , reason : TimestampReason
    }


type TimestampReason
    = Schedule
    | Is
    | Propagation
    | Forecast


type Move
    = Transport TransportInfo
    | Walk WalkInfo


type alias TransportInfo =
    { range : Range
    , category_name : String
    , train_nr : Maybe Int
    , line_id : String
    , name : String
    , provider : String
    , direction : String
    }


type alias WalkInfo =
    { range : Range
    , mumo_id : Int
    , price : Maybe Int
    , mumo_type : String
    }


type alias Attribute =
    { range : Range
    , code : String
    , text : String
    }


type alias Range =
    { from : Int
    , to : Int
    }


decodeRoutingResponse : Decode.Decoder (List Connection)
decodeRoutingResponse =
    Decode.at [ "content", "connections" ] (Decode.list decodeConnection)


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
        |: ("interchange" := Decode.bool |> withDefault False)


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


decodeDate : Decode.Decoder Date
decodeDate =
    Decode.int `Decode.andThen` (Decode.succeed << Date.fromTime << toFloat << \i -> i * 1000)
