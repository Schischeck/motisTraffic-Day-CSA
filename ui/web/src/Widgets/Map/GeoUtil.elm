module Widgets.Map.GeoUtil exposing (..)

import Math.Vector2 as Vector2 exposing (Vec2, vec2)
import Data.Connection.Types exposing (Position)


latLngToWorldCoord : Float -> Float -> ( Float, Float )
latLngToWorldCoord lat lng =
    let
        initialResolution =
            2 * pi * 6378137 / 256

        originShift =
            pi * 6378137

        mx =
            lng * originShift / 180

        my1 =
            (logBase e (tan ((90 + lat) * pi / 360))) / (pi / 180)

        my =
            my1 * originShift / 180

        x =
            (mx + originShift) / initialResolution

        y =
            (my + originShift) / initialResolution
    in
        ( x, 256 - y )


positionToVec2 : Position -> Vec2
positionToVec2 pos =
    let
        ( x, y ) =
            latLngToWorldCoord pos.lat pos.lng
    in
        vec2 x y
