module Widgets.Map.ConnectionOverlay
    exposing
        ( hideOverlay
        , showOverlay
        )

import Widgets.Map.Port exposing (..)
import Widgets.Map.RailViz exposing (mapId)
import Data.Journey.Types exposing (..)
import Data.Connection.Types exposing (..)
import Localization.Base exposing (..)
import Util.DateFormat exposing (..)
import Widgets.Helpers.ConnectionUtil exposing (..)
import Util.List exposing ((!!))


hideOverlay : Cmd msg
hideOverlay =
    mapClearOverlays mapId


showOverlay : Localization -> Journey -> Cmd msg
showOverlay locale journey =
    let
        trainLines =
            List.map trainPolyline journey.trains

        walkLines =
            List.map (walkPolyline locale) journey.walks

        stops =
            stopCircles journey.connection.stops
    in
        mapSetOverlays
            { mapId = mapId
            , overlays =
                trainLines
                    ++ walkLines
                    ++ stops
            }


trainPolyline : Train -> MapOverlay
trainPolyline train =
    { shape = "polyline"
    , latlngs = List.map stopLatLng train.stops
    , options =
        { defaultOptions
            | color = trainColor train
            , weight = Just 5
        }
    , tooltip =
        train.transports
            |> List.head
            |> Maybe.map longTransportNameWithoutIcon
    }


walkPolyline : Localization -> JourneyWalk -> MapOverlay
walkPolyline locale walk =
    { shape = "polyline"
    , latlngs = List.map stopLatLng [ walk.from, walk.to ]
    , options =
        { defaultOptions
            | color = walkColor
            , weight = Just 5
        }
    , tooltip =
        Just (locale.t.connections.walkDuration (durationText walk.duration))
    }


stopCircles : List Stop -> List MapOverlay
stopCircles stops =
    let
        interchangeOptions =
            { defaultOptions
                | color = "black"
                , fill = True
                , fillColor = Just "white"
                , radius = Just 5
                , weight = Just 2
                , fillOpacity = Just 0.8
            }

        stopCircle stop =
            { shape = "circleMarker"
            , latlngs = [ stopLatLng stop ]
            , options = interchangeOptions
            , tooltip = Just stop.station.name
            }

        lastIndex =
            List.length stops - 1
    in
        stops
            |> List.indexedMap (,)
            |> List.filter
                (\( i, s ) -> s.exit || s.enter || i == 0 || i == lastIndex)
            |> List.map (\( i, s ) -> stopCircle s)


stopLatLng : Stop -> ( Float, Float )
stopLatLng stop =
    ( stop.station.pos.lat, stop.station.pos.lng )


defaultOptions : MapOverlayOptions
defaultOptions =
    { color = "#777"
    , fill = False
    , fillColor = Nothing
    , radius = Nothing
    , weight = Nothing
    , fillOpacity = Nothing
    }


classColors : List String
classColors =
    [ "#9c27b0"
    , "#e91e63"
    , "#1a237e"
    , "#f44336"
    , "#f44336"
    , "#4caf50"
    , "#3f51b5"
    , "#ff9800"
    , "#ff9800"
    , "#9e9e9e"
    ]


defaultClassColor : String
defaultClassColor =
    "#9e9e9e"


walkColor : String
walkColor =
    "#333"


transportColor : TransportInfo -> String
transportColor transport =
    classColors
        !! transport.class
        |> Maybe.withDefault "red"


trainColor : Train -> String
trainColor train =
    List.head train.transports
        |> Maybe.map transportColor
        |> Maybe.withDefault "blue"
