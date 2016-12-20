module Widgets.Map.Trains exposing (..)

import Widgets.Map.RailVizModel exposing (..)
import Math.Vector2 as Vector2 exposing (Vec2, vec2)
import Math.Vector3 as Vector3 exposing (Vec3, vec3)
import Math.Matrix4 exposing (Mat4, scale, translate, makeOrtho2D)
import WebGL exposing (..)
import Time exposing (Time)
import Bitwise
import Util.List exposing ((!!))


-- MODEL


type alias Vertex =
    { c1 : Vec2
    , c2 : Vec2
    , p : Float
    , pickColor : Vec3
    , col : Vec3
    }


mesh : Time -> List RVTrain -> Drawable Vertex
mesh currentTime trains =
    Points (List.filterMap (getTrainPosition currentTime) trains)


getTrainPosition : Time -> RVTrain -> Maybe Vertex
getTrainPosition currentTime train =
    if currentTime < train.departureTime || currentTime > train.arrivalTime then
        Nothing
    else
        case train.currentSubSegment of
            Just currentSubSegment ->
                let
                    progress =
                        trainProgress currentTime train

                    dist =
                        progress * train.pathLength

                    subSegmentPos =
                        dist - currentSubSegment.startDistance

                    p =
                        subSegmentPos / currentSubSegment.length
                in
                    Just <| Vertex currentSubSegment.startPoint currentSubSegment.endPoint p train.pickColor (trainColor train)

            Nothing ->
                Just <| Vertex train.departureStation.pos train.arrivalStation.pos 1.0 train.pickColor (trainColor train)


toPickColor : Int -> Vec3
toPickColor pickId =
    let
        tf x =
            (toFloat x) / 255.0

        r =
            tf (Bitwise.and pickId 255)

        g =
            tf (Bitwise.and (Bitwise.shiftRightZfBy 8 pickId) 255)

        b =
            tf (Bitwise.and (Bitwise.shiftRightZfBy 16 pickId) 255)
    in
        vec3 r g b


toPickId : Maybe ( Int, Int, Int, Int ) -> Maybe Int
toPickId color =
    case color of
        Just ( r, g, b, a ) ->
            if a == 0 then
                Nothing
            else
                Just (r + (Bitwise.shiftLeftBy 8 g) + (Bitwise.shiftLeftBy 16 b))

        Nothing ->
            Nothing


trainProgress : Time -> RVTrain -> Float
trainProgress currentTime train =
    ((currentTime - train.departureTime) / (train.arrivalTime - train.departureTime))


trainColor : RVTrain -> Vec3
trainColor train =
    let
        delay =
            ceiling ((train.departureTime - train.scheduledDepartureTime) / 60000)
    in
        if delay <= 3 then
            vec3 0.27 0.82 0.29
        else if delay <= 5 then
            vec3 0.99 0.93 0.0
        else if delay <= 10 then
            vec3 1.0 0.4 0.0
        else if delay <= 15 then
            vec3 1.0 0.19 0.28
        else
            vec3 0.64 0.0 0.04


updateCurrentSubSegment : Time -> RVTrain -> RVTrain
updateCurrentSubSegment currentTime train =
    let
        progress =
            trainProgress currentTime train

        distance =
            progress * train.pathLength

        findSubSegment start startDist vectors idx dist =
            case vectors of
                ( vec, vecLen ) :: rest ->
                    let
                        endPt =
                            Vector2.add start vec
                    in
                        if vecLen < dist then
                            findSubSegment
                                endPt
                                (startDist + vecLen)
                                rest
                                (idx + 1)
                                (dist - vecLen)
                        else
                            { train
                                | currentSubSegment =
                                    Just
                                        { startDistance = startDist
                                        , startPoint = start
                                        , endPoint = endPt
                                        , length = vecLen
                                        , nextSubSegmentIndex = idx + 1
                                        }
                            }

                [] ->
                    { train | currentSubSegment = Nothing }
    in
        case train.currentSubSegment of
            Just currentSubSegment ->
                let
                    d =
                        distance - currentSubSegment.startDistance
                in
                    if d >= 0 && d <= currentSubSegment.length then
                        train
                    else
                        findSubSegment
                            currentSubSegment.endPoint
                            (currentSubSegment.startDistance + currentSubSegment.length)
                            (List.drop currentSubSegment.nextSubSegmentIndex train.currentSegment)
                            currentSubSegment.nextSubSegmentIndex
                            (d - currentSubSegment.length)

            Nothing ->
                findSubSegment
                    train.departureStation.pos
                    0
                    train.currentSegment
                    0
                    distance


getHoveredTrain : Model -> Maybe RVTrain
getHoveredTrain model =
    model.hoveredTrain
        |> Maybe.andThen (\pickId -> model.allTrains !! pickId)



-- SHADERS


vertexShader : Shader { attr | c1 : Vec2, c2 : Vec2, p : Float, col : Vec3 } { unif | perspective : Mat4, zoom : Float } { vCol : Vec3 }
vertexShader =
    [glsl|
attribute vec2 c1, c2;
attribute float p;
attribute vec3 col;
uniform mat4 perspective;
uniform float zoom;
varying vec3 vCol;

void main() {
    vec4 c1p = perspective * vec4(c1, 0.0, 1.0);
    vec4 c2p = perspective * vec4(c2, 0.0, 1.0);
    gl_Position = c1p + p * (c2p - c1p);
    gl_PointSize = zoom;
    vCol = col;
}
|]


fragmentShader : Shader {} { u | texture : Texture } { vCol : Vec3 }
fragmentShader =
    [glsl|
precision mediump float;
uniform sampler2D texture;
varying vec3 vCol;

void main () {
    gl_FragColor = vec4(vCol, 1.0) * texture2D(texture, gl_PointCoord);
}
|]


offscreenVertexShader : Shader { attr | c1 : Vec2, c2 : Vec2, p : Float, pickColor : Vec3 } { unif | perspective : Mat4, zoom : Float } { vPickColor : Vec3 }
offscreenVertexShader =
    [glsl|
precision highp float;
attribute vec2 c1, c2;
attribute float p;
attribute vec3 pickColor;
uniform mat4 perspective;
uniform float zoom;
varying vec3 vPickColor;

void main() {
    vec4 c1p = perspective * vec4(c1, 0.0, 1.0);
    vec4 c2p = perspective * vec4(c2, 0.0, 1.0);
    gl_Position = c1p + p * (c2p - c1p);
    gl_PointSize = zoom;

    vPickColor = pickColor;
}
|]


offscreenFragmentShader : Shader {} { u | texture : Texture } { vPickColor : Vec3 }
offscreenFragmentShader =
    [glsl|
precision highp float;
uniform sampler2D texture;
varying vec3 vPickColor;

void main () {
    vec4 tex = texture2D(texture, gl_PointCoord);
    if (tex.w == 0.0) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    } else {
        gl_FragColor = vec4(vPickColor, 1.0);
    }
}
|]
