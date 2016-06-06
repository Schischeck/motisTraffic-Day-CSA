port module Port exposing (..)


port initMap : String -> Cmd msg


port mapLoaded : (String -> msg) -> Sub msg
