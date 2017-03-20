module Data.Connection.Request exposing (..)

import Data.Connection.Types exposing (..)
import Json.Encode as Encode exposing (string, int)
import Util.Core exposing ((=>))


-- for local storage


encodeStation : Station -> Encode.Value
encodeStation station =
    Encode.object
        [ "id" => Encode.string station.id
        , "name" => Encode.string station.name
        , "pos" => encodePosition station.pos
        ]


encodePosition : Position -> Encode.Value
encodePosition pos =
    Encode.object
        [ "lat" => Encode.float pos.lat
        , "lng" => Encode.float pos.lng
        ]
