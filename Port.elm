port module Port exposing (..)


port mapInit : String -> Cmd msg


port mapLoaded : (String -> msg) -> Sub msg


port mapResizeWidth : (Float -> msg) -> Sub msg


port mapResizeHeight : (Float -> msg) -> Sub msg


port mapZoom : (Float -> msg) -> Sub msg


port mapNorth : (Float -> msg) -> Sub msg


port mapWest : (Float -> msg) -> Sub msg
