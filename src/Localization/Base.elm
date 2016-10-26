module Localization.Base exposing (..)

import Date exposing (Date)
import Util.DateFormat exposing (DateConfig)


type alias Localization =
    { t : Translations
    , dateConfig : DateConfig
    }


type alias Translations =
    { search :
        { search : String
        }
    , connections :
        { timeHeader : String
        , durationHeader : String
        , transportsHeader : String
        , scheduleRange : Date -> Date -> String
        , loading : String
        , noResults : String
        , errors :
            { journeyDateNotInSchedule : String
            , internalError : String -> String
            , timeout : String
            , network : String
            , http : Int -> String
            , decode : String -> String
            }
        , extendBefore : String
        , extendAfter : String
        , interchanges : Int -> String
        , walkDuration : String -> String
        , interchangeDuration : String -> String
        , arrivalTrack : String -> String
        , track : String
        , tripIntermediateStops : Int -> String
        , tripWalk : String -> String
        }
    }
