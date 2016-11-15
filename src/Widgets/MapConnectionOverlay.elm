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
    in
        mapSetOverlays
            { mapId = "map"
            , overlays =
                leadingWalkLines
                    ++ trainLines
                    ++ trailingWalkLines
            }


trainPolyline : Train -> MapOverlay
trainPolyline train =
    { shape = "polyline"
    , latlngs = stopLatlngs train.stops
    , options = { color = trainColor train }
    }


walkPolyline : JourneyWalk -> MapOverlay
walkPolyline walk =
    { shape = "polyline"
    , latlngs = stopLatlngs [ walk.from, walk.to ]
    , options = { color = walkColor }
    }


stopLatlngs : List Stop -> List ( Float, Float )
stopLatlngs stops =
    let
        coords stop =
            ( stop.station.pos.lat, stop.station.pos.lng )
    in
        List.map coords stops


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
