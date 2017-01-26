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
    | TripDetails String Int Int String Int String
    | StationEvents String
    | SimulationTime Date


urlParser : Parser (Route -> a) a
urlParser =
    oneOf
        [ map Connections top
        , map ConnectionDetails (s "connection" </> int)
        , map TripDetails (s "trip" </> string </> int </> int </> string </> int </> encodedString)
        , map StationEvents (s "station" </> string)
        , map SimulationTime (s "time" </> date)
        ]


date : Parser (Date -> a) a
date =
    oneOf
        [ unixTimestamp
        , nativeDate
        ]


unixTimestamp : Parser (Date -> a) a
unixTimestamp =
    custom "UNIX_TIMESTAMP" (String.toFloat >> Result.map (\u -> Date.fromTime (u * 1000.0)))


nativeDate : Parser (Date -> a) a
nativeDate =
    custom "DATE_STR" Date.fromString


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

        TripDetails station trainNr time targetStation targetTime lineId ->
            "#/trip/"
                ++ station
                ++ "/"
                ++ toString trainNr
                ++ "/"
                ++ toString time
                ++ "/"
                ++ targetStation
                ++ "/"
                ++ toString targetTime
                ++ "/"
                ++ Http.encodeUri lineId

        StationEvents stationId ->
            "#/station/" ++ stationId

        SimulationTime time ->
            "#/time/" ++ dateToUrl time


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
