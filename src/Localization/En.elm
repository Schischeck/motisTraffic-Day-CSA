module Localization.En exposing (..)

import Localization.Base exposing (..)
import Util.DateFormat exposing (..)


enLocalization : Localization
enLocalization =
    { t = enTranslations
    , dateConfig = enDateConfig
    }


enTranslations : Translations
enTranslations =
    { search =
        { search = "Search"
        , start = "Start"
        , destination = "Destination"
        , date = "Date"
        , time = "Time"
        , startTransports = "Transports at the start"
        , destinationTransports = "Transports at the destination"
        }
    , connections =
        { timeHeader = "Time"
        , durationHeader = "Duration"
        , transportsHeader = "Transports"
        , scheduleRange =
            \begin end ->
                "Auskunft von "
                    ++ (formatDate enDateConfig begin)
                    ++ " bis "
                    ++ (formatDate enDateConfig end)
                    ++ " möglich"
        , loading = "Searching..."
        , noResults = "No connections found"
        , errors =
            { journeyDateNotInSchedule = "Date not in schedule"
            , internalError = \msg -> "Internal error (" ++ msg ++ ")"
            , timeout = "Timeout"
            , network = "Network error"
            , http = \code -> "HTTP error " ++ (toString code)
            , decode = \msg -> "Invalid response (" ++ msg ++ ")"
            }
        , extendBefore = "Earlier"
        , extendAfter = "Later"
        , interchanges =
            \count ->
                case count of
                    0 ->
                        "No interchanges"

                    1 ->
                        "1 interchange"

                    _ ->
                        (toString count) ++ " interchanges"
        , walkDuration = \duration -> duration ++ " walk"
        , interchangeDuration = \duration -> duration ++ " interchange"
        , arrivalTrack = \track -> "Arrival track " ++ track
        , track = "Track"
        , tripIntermediateStops =
            \count ->
                case count of
                    0 ->
                        "No intermediate stops"

                    1 ->
                        "1 intermediate stop"

                    _ ->
                        (toString count) ++ " intermediate stops"
        , tripWalk = \duration -> "Walk (" ++ duration ++ ")"
        }
    }
