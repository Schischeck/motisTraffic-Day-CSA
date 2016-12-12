module Routes
    exposing
        ( Route(..)
        , urlParser
        , toUrl
        , tripDetailsRoute
        , routeToTripId
        )

import UrlParser exposing (Parser, (</>), s, int, string, map, oneOf, parseHash, top, custom)
import Date exposing (Date)
import Data.Connection.Types exposing (TripId)
import Http


type Route
    = Connections
    | ConnectionDetails Int
    | ConnectionFullTripDetails Int Int
    | TripDetails String Int Date String Date String


urlParser : Parser (Route -> a) a
urlParser =
    oneOf
        [ map Connections top
        , map ConnectionDetails (s "connection" </> int)
        , map ConnectionFullTripDetails (s "connection" </> int </> s "trip" </> int)
        , map TripDetails (s "trip" </> string </> int </> date </> string </> date </> encodedString)
        ]


date : Parser (Date -> a) a
date =
    custom "DATE" (String.toFloat >> Result.map (\u -> Date.fromTime (u * 1000.0)))


encodedString : Parser (String -> a) a
encodedString =
    custom "ENCODED_STRING" (Http.decodeUri >> Result.fromMaybe "decodeUri error")


dateToUrl : Date -> String
dateToUrl d =
    toString (round ((Date.toTime d) / 1000.0))


toUrl : Route -> String
toUrl route =
    case route of
        Connections ->
            "#/"

        ConnectionDetails idx ->
            "#/connection/" ++ toString idx

        ConnectionFullTripDetails connIdx tripIdx ->
            "#/connection/" ++ toString connIdx ++ "/trip/" ++ toString tripIdx

        TripDetails station trainNr time targetStation targetTime lineId ->
            "#/trip/"
                ++ station
                ++ "/"
                ++ toString trainNr
                ++ "/"
                ++ dateToUrl time
                ++ "/"
                ++ targetStation
                ++ "/"
                ++ dateToUrl targetTime
                ++ "/"
                ++ Http.encodeUri lineId


tripDetailsRoute : TripId -> Route
tripDetailsRoute trip =
    TripDetails
        trip.station_id
        trip.train_nr
        trip.time
        trip.target_station_id
        trip.target_time
        trip.line_id


routeToTripId : Route -> Maybe TripId
routeToTripId route =
    case route of
        TripDetails station trainNr time targetStation targetTime lineId ->
            Just
                { station_id = station
                , train_nr = trainNr
                , time = time
                , target_station_id = targetStation
                , target_time = targetTime
                , line_id = lineId
                }

        _ ->
            Nothing
