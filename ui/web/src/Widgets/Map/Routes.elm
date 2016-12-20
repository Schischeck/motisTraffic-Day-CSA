module Widgets.Map.Routes exposing (..)

import Widgets.Map.RailVizModel exposing (..)
import Widgets.Map.GeoUtil exposing (..)
import Data.RailViz.Types exposing (..)
import Math.Vector2 as Vector2 exposing (Vec2, vec2)
import Math.Vector4 as Vector4 exposing (Vec4, vec4)
import Math.Matrix4 exposing (Mat4, scale, translate, makeOrtho2D)
import WebGL exposing (..)
import List.Extra


-- MODEL


mesh : List RailVizRoute -> Drawable RouteVertex
mesh routes =
    Lines (List.concatMap routeLines routes)


routeLines : RailVizRoute -> List ( RouteVertex, RouteVertex )
routeLines route =
    List.concatMap segmentLines route.segments


segmentLines : RailVizSegment -> List ( RouteVertex, RouteVertex )
segmentLines seg =
    let
        latLngToVec2 lat lng =
            let
                ( x, y ) =
                    latLngToWorldCoord lat lng
            in
                vec2 x y

        convertCoords : List Float -> Maybe Vec2
        convertCoords l =
            case l of
                [ lat, lng ] ->
                    Just (latLngToVec2 lat lng)

                _ ->
                    let
                        _ =
                            Debug.log "RailViz: Invalid Polyline (1)" <|
                                seg
                    in
                        Nothing

        coords : List Vec2
        coords =
            List.Extra.groupsOf 2 seg.coordinates.coordinates
                |> List.filterMap convertCoords
    in
        coords
            |> List.Extra.groupsOfWithStep 2 1
            |> List.filterMap
                (\l ->
                    case l of
                        [ a, b ] ->
                            Just ( RouteVertex a, RouteVertex b )

                        _ ->
                            Nothing
                )



-- SHADERS


lineColor : Vec4
lineColor =
    vec4 0.4 0.4 0.4 1.0


vertexShader : Shader { attr | aCoords : Vec2 } { unif | uPerspective : Mat4, uZoom : Float } {}
vertexShader =
    [glsl|
attribute vec2 aCoords;
uniform mat4 uPerspective;
uniform float uZoom;

void main() {
    gl_Position = uPerspective * vec4(aCoords, 0.0, 1.0);
    gl_PointSize = uZoom;
}
|]


fragmentShader : Shader {} { unif | uColor : Vec4 } {}
fragmentShader =
    [glsl|
precision mediump float;
uniform vec4 uColor;

void main () {
    gl_FragColor = uColor;
}
|]
