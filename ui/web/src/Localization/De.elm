module Localization.De exposing (..)

import Localization.Base exposing (..)
import Util.DateFormat exposing (..)


deLocalization : Localization
deLocalization =
    { t = deTranslations
    , dateConfig = deDateConfig
    }


deTranslations : Translations
deTranslations =
    { search =
        { search = "Suchen"
        , start = "Start"
        , destination = "Ziel"
        , date = "Datum"
        , time = "Uhrzeit"
        , startTransports = "Verkehrsmittel am Start"
        , destinationTransports = "Verkehrsmittel am Ziel"
        , departure = "Abfahrt"
        , arrival = "Ankunft"
        , trainNr = "Zugnummer"
        }
    , connections =
        { timeHeader = "Zeit"
        , durationHeader = "Dauer"
        , transportsHeader = "Verkehrsmittel"
        , scheduleRange =
            \begin end ->
                "Auskunft von "
                    ++ (formatDate deDateConfig begin)
                    ++ " bis "
                    ++ (formatDate deDateConfig end)
                    ++ " möglich"
        , loading = "Verbindungen suchen..."
        , noResults = "Keine Verbindungen gefunden"
        , extendBefore = "Früher"
        , extendAfter = "Später"
        , interchanges =
            \count ->
                case count of
                    0 ->
                        "Keine Umstiege"

                    1 ->
                        "1 Umstieg"

                    _ ->
                        (toString count) ++ " Umstiege"
        , walkDuration = \duration -> duration ++ " Fußweg"
        , interchangeDuration = \duration -> duration ++ " Umstieg"
        , arrivalTrack = \track -> "Ankunft Gleis " ++ track
        , track = "Gleis"
        , tripIntermediateStops =
            \count ->
                case count of
                    0 ->
                        "Fahrt ohne Zwischenhalt"

                    1 ->
                        "Fahrt 1 Station"

                    _ ->
                        "Fahrt " ++ (toString count) ++ " Stationen"
        , tripWalk = \duration -> "Fußweg (" ++ duration ++ ")"
        , provider = "Betreiber"
        , walk = "Fußweg"
        , trainNr = "Zugnummer"
        , lineId = "Linie"
        }
    , station =
        { direction = "Richtung"
        , noDepartures = "Keine Abfahrten im gewählten Zeitraum"
        , noArrivals = "Keine Ankünfte im gewählten Zeitraum"
        , loading = "Laden..."
        , trackAbbr = "Gl."
        }
    , railViz =
        { trainColors = "Zugfarben"
        , delayColors = "Verspätung"
        , classColors = "Kategorie"
        , simActive = "Simulationsmodus aktiv"
        }
    , errors =
        { journeyDateNotInSchedule = "Zeitraum nicht im Fahrplan"
        , internalError = \msg -> "Interner Fehler (" ++ msg ++ ")"
        , timeout = "Zeitüberschreitung"
        , network = "Netzwerkfehler"
        , http = \code -> "HTTP-Fehler " ++ (toString code)
        , decode = \msg -> "Ungültige Antwort (" ++ msg ++ ")"
        , moduleNotFound = "Modul nicht geladen"
        }
    , trips =
        { noResults = "Keine passenden Züge gefunden"
        }
    , misc =
        { permalink = "Permalink" }
    , simTime =
        { simMode = "Simulationsmodus"
        }
    }
