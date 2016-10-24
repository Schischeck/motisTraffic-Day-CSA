module Data.Connection.Types exposing (..)

import Date exposing (Date)
import Date.Extra.Duration as Duration exposing (DeltaRecord)
import Set exposing (Set)
import Util.List exposing (..)


type alias Connection =
    { stops : List Stop
    , transports : List Move
    , attributes : List Attribute
    }


type alias Stop =
    { station : Station
    , arrival : EventInfo
    , departure : EventInfo
    , leave : Bool
    , enter : Bool
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
    , class : Int
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


departureEvent : Connection -> Maybe EventInfo
departureEvent connection =
    List.head connection.stops |> Maybe.map .departure


arrivalEvent : Connection -> Maybe EventInfo
arrivalEvent connection =
    last connection.stops |> Maybe.map .arrival


departureTime : Connection -> Maybe Date
departureTime connection =
    departureEvent connection `Maybe.andThen` .schedule_time


arrivalTime : Connection -> Maybe Date
arrivalTime connection =
    arrivalEvent connection `Maybe.andThen` .schedule_time


duration : Connection -> Maybe DeltaRecord
duration connection =
    Maybe.map2 Duration.diff (arrivalTime connection) (departureTime connection)


interchanges : Connection -> Int
interchanges connection =
    Basics.max 0 ((List.length <| List.filter .enter connection.stops) - 1)


transportsForRange : Connection -> Int -> Int -> List TransportInfo
transportsForRange connection from to =
    let
        checkMove : Move -> Maybe TransportInfo
        checkMove move =
            case move of
                Transport transport ->
                    if transport.range.from < to && transport.range.to > from then
                        Just transport
                    else
                        Nothing

                Walk _ ->
                    Nothing
    in
        List.filterMap checkMove connection.transports


transportCategories : Connection -> Set String
transportCategories connection =
    let
        category : Move -> Maybe String
        category move =
            case move of
                Transport t ->
                    Just t.category_name

                Walk _ ->
                    Nothing
    in
        List.filterMap category connection.transports |> Set.fromList
