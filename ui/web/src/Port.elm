port module Port exposing (..)


type alias MapInfo =
    { scale : Float
    , zoom : Float
    , pixelBounds : MapPixelBounds
    , geoBounds : MapGeoBounds
    , railVizBounds : MapGeoBounds
    }


type alias MapPixelBounds =
    { north : Float
    , west : Float
    , width : Float
    , height : Float
    }


type alias MapGeoBounds =
    { north : Float
    , west : Float
    , south : Float
    , east : Float
    }


type alias MapOverlays =
    { mapId : String
    , overlays : List MapOverlay
    }


type alias MapOverlay =
    { shape : String
    , latlngs : List ( Float, Float )
    , options : MapOverlayOptions
    , tooltip : Maybe String
    }


type alias MapOverlayOptions =
    { color : String
    , fill : Bool
    , fillColor : Maybe String
    , radius : Maybe Int
    , weight : Maybe Int
    , fillOpacity : Maybe Float
    }


port mapInit : String -> Cmd msg


port mapLoaded : (String -> msg) -> Sub msg


port mapUpdate : (MapInfo -> msg) -> Sub msg


port mapSetOverlays : MapOverlays -> Cmd msg


port mapClearOverlays : String -> Cmd msg


port setRoutingResponses : (List ( String, String ) -> msg) -> Sub msg



-- Map: Picking


type alias MapMouseUpdate =
    { x : Float
    , y : Float
    , color : Maybe ( Int, Int, Int, Int )
    }


port mapMouseMove : (MapMouseUpdate -> msg) -> Sub msg


port mapMouseDown : (MapMouseUpdate -> msg) -> Sub msg


port mapMouseUp : (MapMouseUpdate -> msg) -> Sub msg


port mapMouseOut : (MapMouseUpdate -> msg) -> Sub msg
