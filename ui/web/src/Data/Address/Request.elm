module Data.Address.Request exposing (..)

import Data.Address.Types exposing (..)
import Json.Encode as Encode exposing (string, int)
import Util.Core exposing ((=>))


encodeAddressRequest : String -> Encode.Value
encodeAddressRequest input =
    Encode.object
        [ "destination"
            => Encode.object
                [ "type" => Encode.string "Module"
                , "target" => Encode.string "/address"
                ]
        , "content_type" => Encode.string "AddressRequest"
        , "content"
            => Encode.object
                [ "input" => Encode.string input ]
        ]
