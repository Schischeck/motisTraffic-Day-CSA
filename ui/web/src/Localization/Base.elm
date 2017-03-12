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
        , start : String
        , destination : String
        , date : String
        , time : String
        , startTransports : String
        , destinationTransports : String
        , departure : String
        , arrival : String
        , trainNr : String
        , maxDuration : String
        }
    , connections :
        { timeHeader : String
        , durationHeader : String
        , transportsHeader : String
        , scheduleRange : Date -> Date -> String
        , loading : String
        , noResults : String
        , extendBefore : String
        , extendAfter : String
        , interchanges : Int -> String
        , walkDuration : String -> String
        , interchangeDuration : String -> String
        , arrivalTrack : String -> String
        , track : String
        , tripIntermediateStops : Int -> String
        , tripWalk : String -> String
        , tripBike : String -> String
        , provider : String
        , walk : String
        , bike : String
        , trainNr : String
        , lineId : String
        }
    , station :
        { direction : String
        , noDepartures : String
        , noArrivals : String
        , loading : String
        , trackAbbr : String
        }
    , railViz :
        { trainColors : String
        , delayColors : String
        , classColors : String
        , simActive : String
        }
    , errors :
        { journeyDateNotInSchedule : String
        , internalError : String -> String
        , timeout : String
        , network : String
        , http : Int -> String
        , decode : String -> String
        , moduleNotFound : String
        }
    , trips :
        { noResults : String
        }
    , misc :
        { permalink : String }
    , simTime :
        { simMode : String
        }
    }
