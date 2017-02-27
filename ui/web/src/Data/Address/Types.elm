module Data.Address.Types exposing (..)

import Data.Connection.Types exposing (Position)


type alias AddressRequest =
    { input : String
    }


type alias AddressResponse =
    { guesses : List Address }


type alias Address =
    { pos : Position
    , name : String
    , type_ : String
    , regions : List String
    }
