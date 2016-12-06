port module Port exposing (..)


type alias Map =
    { width : Float
    , height : Float
    , scale : Float
    , zoom : Float
    , north : Float
    , west : Float
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


port mapUpdate : (Map -> msg) -> Sub msg


port mapSetOverlays : MapOverlays -> Cmd msg


port mapClearOverlays : String -> Cmd msg


port setRoutingResponses : (List ( String, String ) -> msg) -> Sub msg



-- Map: Picking


type alias MapMouseUpdate =
    { x : Float
    , y : Float
    , color : Float
    }


port mapMouseMove : (MapMouseUpdate -> msg) -> Sub msg


port mapMouseDown : (MapMouseUpdate -> msg) -> Sub msg


port mapMouseUp : (MapMouseUpdate -> msg) -> Sub msg
