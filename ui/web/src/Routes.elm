module Routes exposing (..)

import UrlParser exposing (Parser, (</>), s, int, string, map, oneOf, parseHash, top)


type Route
    = Connections
    | ConnectionDetails Int
    | ConnectionFullTripDetails Int Int


urlParser : Parser (Route -> a) a
urlParser =
    oneOf
        [ map Connections top
        , map ConnectionDetails (s "connection" </> int)
        , map ConnectionFullTripDetails (s "connection" </> int </> s "trip" </> int)
        ]


toUrl : Route -> String
toUrl route =
    case route of
        Connections ->
            "#/"

        ConnectionDetails idx ->
            "#/connection/" ++ toString idx

        ConnectionFullTripDetails connIdx tripIdx ->
            "#/connection/" ++ toString connIdx ++ "/trip/" ++ toString tripIdx
