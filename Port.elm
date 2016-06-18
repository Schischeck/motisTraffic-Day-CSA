port module Port exposing (..)

type alias Map =
    { width : Float
    , height : Float
    , scale : Float
    , zoom : Float
    , north : Float
    , west : Float
    }


port mapInit : String -> Cmd msg


port mapLoaded : (String -> msg) -> Sub msg


port mapUpdate : (Map -> msg) -> Sub msg