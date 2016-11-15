module Widgets.MapConnectionOverlay
    exposing
        ( hideOverlay
        , showOverlay
        )

import Port exposing (..)
import Data.Journey.Types exposing (..)
import Data.Connection.Types exposing (..)
import Util.List exposing ((!!))
import Maybe.Extra


hideOverlay : Cmd msg
hideOverlay =
    mapClearOverlays "map"


showOverlay : Journey -> Cmd msg
showOverlay journey =
    let
        trainLines =
            List.map trainPolyline journey.trains

        leadingWalkLines =
            journey.leadingWalk
                |> Maybe.map walkPolyline
                |> Maybe.Extra.maybeToList

        trailingWalkLines =
            journey.trailingWalk
                |> Maybe.map walkPolyline
                |> Maybe.Extra.maybeToList

        stops =
            stopCircles journey.connection.stops
    in
        mapSetOverlays
            { mapId = "map"
            , overlays =
                leadingWalkLines
                    ++ trainLines
                    ++ trailingWalkLines
                    ++ stops
            }


trainPolyline : Train -> MapOverlay
trainPolyline train =
    { shape = "polyline"
    , latlngs = List.map stopLatLng train.stops
    , options =
        { defaultOptions
            | color = trainColor train
        }
    }


walkPolyline : JourneyWalk -> MapOverlay
walkPolyline walk =
    { shape = "polyline"
    , latlngs = List.map stopLatLng [ walk.from, walk.to ]
    , options =
        { defaultOptions
            | color = walkColor
        }
    }


stopCircles : List Stop -> List MapOverlay
stopCircles stops =
    let
        interchangeOptions =
            { defaultOptions
                | color = "red"
                , fill = True
                , fillColor = Just "red"
                , radius = Just 2
            }

        intermediateOptions =
            { defaultOptions
                | color = "#777"
                , fill = True
                , fillColor = Just "#777"
                , radius = Just 2
            }

        stopCircle stop =
            let
                options =
                    if stop.exit || stop.enter then
                        interchangeOptions
                    else
                        intermediateOptions
            in
                { shape = "circleMarker"
                , latlngs = [ stopLatLng stop ]
                , options = options
                }
    in
        List.map stopCircle stops


stopLatLng : Stop -> ( Float, Float )
stopLatLng stop =
    ( stop.station.pos.lat, stop.station.pos.lng )


defaultOptions : MapOverlayOptions
defaultOptions =
    { color = "#777"
    , fill = False
    , fillColor = Nothing
    , radius = Nothing
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
