port module Port exposing (..)

import Data.Connection.Types exposing (TripId)


-- see also: Widgets.Map.Port


port setRoutingResponses : (List ( String, String ) -> msg) -> Sub msg


port showStationDetails : (String -> msg) -> Sub msg


port showTripDetails : (TripId -> msg) -> Sub msg


port setRailVizFilter : Maybe (List TripId) -> Cmd msg


port setTimeOffset : Float -> Cmd msg


port setSimulationTime : (Float -> msg) -> Sub msg
