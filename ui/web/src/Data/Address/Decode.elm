module Data.Address.Decode exposing (decodeAddressResponse)

import Data.Address.Types exposing (..)
import Data.Connection.Types exposing (Position)
import Data.Connection.Decode exposing (decodePosition)
import Json.Decode as Decode
    exposing
        ( Decoder
        , list
        , string
        , int
        , float
        , bool
        , field
        , nullable
        , succeed
        , fail
        )
import Json.Decode.Pipeline exposing (decode, required, optional, hardcoded, requiredAt)


decodeAddressResponse : Decoder AddressResponse
decodeAddressResponse =
    decode AddressResponse
        |> requiredAt [ "content", "guesses" ] (list decodeAddress)


decodeAddress : Decoder Address
decodeAddress =
    decode Address
        |> required "pos" decodePosition
        |> required "name" string
        |> required "type" string
        |> required "regions" (list string)
