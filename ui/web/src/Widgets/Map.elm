module Widgets.Map
    exposing
        ( Model
        , Msg
        , init
        , view
        , update
        , subscriptions
        )

import Html exposing (Html, Attribute, div, text)
import Html.Attributes exposing (id, class)
import Port exposing (..)
import Math.Vector2 as Vector2 exposing (Vec2, vec2)
import Math.Vector3 as Vector3 exposing (Vec3, vec3)
import Math.Matrix4 exposing (Mat4, scale, translate, makeOrtho2D)
import WebGL exposing (..)
import Time exposing (Time)
import AnimationFrame
import Task exposing (..)
import Html exposing (Html)
import Data.Connection.Types exposing (Station, Position)
import Random


-- MODEL


type alias Vertex =
    { c1 : Vec2
    , c2 : Vec2
    , p : Float
    }


type alias RVTrain =
    { currentEdge : List ( Vec2, Float )
    , departureTime : Time
    , arrivalTime : Time
    , departureStation : RVStation
    , arrivalStation : RVStation
    , pathLength : Float
    , currentSegment : Maybe CurrentSegment
    }


type alias RVStation =
    { station : Station
    , pos : Vec2
    }


type alias CurrentSegment =
    { startDistance : Float
    , startPoint : Vec2
    , endPoint : Vec2
    , length : Float
    , nextSegmentIndex : Int
    }


type alias Model =
    { map : Map
    , texture : Maybe WebGL.Texture
    , time : Time
    , rvTrains : List RVTrain
    }


init : ( Model, Cmd Msg )
init =
    { map =
        { width = 0
        , height = 0
        , scale = 0
        , zoom = 0
        , north = 0
        , west = 0
        }
    , texture = Nothing
    , time = 0.0
    , rvTrains = []
    }
        ! [ mapInit "map"
          , Task.perform GenerateDemoTrains Time.now
          ]


mesh : Time -> List RVTrain -> Drawable Vertex
mesh currentTime trains =
    Points (List.map (getTrainPosition currentTime) trains)


getTrainPosition : Time -> RVTrain -> Vertex
getTrainPosition currentTime train =
    if currentTime <= train.departureTime then
        Vertex train.departureStation.pos train.arrivalStation.pos 0.0
    else if currentTime >= train.arrivalTime then
        Vertex train.departureStation.pos train.arrivalStation.pos 1.0
    else
        case train.currentSegment of
            Just currentSegment ->
                let
                    progress =
                        trainProgress currentTime train

                    dist =
                        progress * train.pathLength

                    segmentPos =
                        dist - currentSegment.startDistance

                    p =
                        segmentPos / currentSegment.length
                in
                    Vertex currentSegment.startPoint currentSegment.endPoint p

            Nothing ->
                Vertex train.departureStation.pos train.arrivalStation.pos 1.0


trainProgress : Time -> RVTrain -> Float
trainProgress currentTime train =
    ((currentTime - train.departureTime) / (train.arrivalTime - train.departureTime))


updateCurrentSegment : Time -> RVTrain -> RVTrain
updateCurrentSegment currentTime train =
    let
        progress =
            trainProgress currentTime train

        distance =
            progress * train.pathLength

        findSegment start startDist vectors idx dist =
            case vectors of
                ( vec, vecLen ) :: rest ->
                    let
                        endPt =
                            Vector2.add start vec
                    in
                        if vecLen < dist then
                            findSegment
                                endPt
                                (startDist + vecLen)
                                rest
                                (idx + 1)
                                (dist - vecLen)
                        else
                            { train
                                | currentSegment =
                                    Just
                                        { startDistance = startDist
                                        , startPoint = start
                                        , endPoint = endPt
                                        , length = vecLen
                                        , nextSegmentIndex = idx + 1
                                        }
                            }

                [] ->
                    { train | currentSegment = Nothing }
    in
        case train.currentSegment of
            Just currentSegment ->
                let
                    d =
                        distance - currentSegment.startDistance
                in
                    if d >= 0 && d <= currentSegment.length then
                        train
                    else
                        findSegment
                            currentSegment.endPoint
                            (currentSegment.startDistance + currentSegment.length)
                            (List.drop currentSegment.nextSegmentIndex train.currentEdge)
                            currentSegment.nextSegmentIndex
                            (d - currentSegment.length)

            Nothing ->
                findSegment
                    train.departureStation.pos
                    0
                    train.currentEdge
                    0
                    distance


positionToVec2 : Position -> Vec2
positionToVec2 pos =
    let
        ( x, y ) =
            latLngToWorldCoord pos.lat pos.lng
    in
        vec2 x y


toRVStation : Station -> RVStation
toRVStation station =
    { station = station
    , pos = positionToVec2 station.pos
    }



-- UPDATE


type Msg
    = Animate Time
    | MapLoaded String
    | MapUpdate Map
    | TextureError WebGL.Error
    | TextureLoaded WebGL.Texture
    | GenerateDemoTrains Time


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        Animate t ->
            { model
                | time = t
                , rvTrains = List.map (updateCurrentSegment t) model.rvTrains
            }
                ! []

        MapLoaded _ ->
            ( model
            , Task.attempt
                (\r ->
                    case r of
                        Ok v ->
                            TextureLoaded v

                        Err v ->
                            TextureError v
                )
                (WebGL.loadTexture "circle.png")
            )

        MapUpdate m_ ->
            ( { model | map = m_ }, Cmd.none )

        TextureError err ->
            ( model, Cmd.none )

        TextureLoaded texture ->
            ( { model | texture = Just texture }, Cmd.none )

        GenerateDemoTrains t ->
            ( { model
                | time = t
                , rvTrains = generateDemoTrains t (Random.initialSeed (floor t)) 1000
              }
            , Cmd.none
            )



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.batch
        [ mapLoaded MapLoaded
        , mapUpdate MapUpdate
        , AnimationFrame.times Animate
        ]



-- VIEW


view : Model -> Html Msg
view model =
    div [ id "map" ] [ overlay [ class "leaflet-overlay" ] model ]


overlay : List (Html.Attribute Msg) -> Model -> Html Msg
overlay attributes model =
    (case model.texture of
        Nothing ->
            []

        Just tex ->
            [ render vertexShader
                fragmentShader
                (mesh model.time model.rvTrains)
                { texture = tex
                , perspective = perspective model
                , zoom = model.map.zoom
                }
            ]
    )
        |> WebGL.toHtmlWith
            [ Enable Blend
            , Disable DepthTest
            , BlendFunc ( SrcAlpha, OneMinusSrcAlpha )
            ]
            attributes


perspective : Model -> Mat4
perspective { map } =
    makeOrtho2D 0.0 map.width map.height 0.0
        |> scale (vec3 map.scale map.scale map.scale)
        |> translate (vec3 -(map.west / map.scale) -(map.north / map.scale) 0)



-- SHADERS


vertexShader : Shader { attr | c1 : Vec2, c2 : Vec2, p : Float } { unif | perspective : Mat4, zoom : Float } {}
vertexShader =
    [glsl|
attribute vec2 c1, c2;
attribute float p;
uniform mat4 perspective;
uniform float zoom;

void main() {
    vec4 c1p = perspective * vec4(c1, 0.0, 1.0);
    vec4 c2p = perspective * vec4(c2, 0.0, 1.0);
    gl_Position = c1p + p * (c2p - c1p);
    gl_PointSize = zoom;
}
|]


fragmentShader : Shader {} { u | texture : Texture } {}
fragmentShader =
    [glsl|
precision mediump float;
uniform sampler2D texture;

void main () {
    gl_FragColor = texture2D(texture, gl_PointCoord);
}
|]



-- DEMO TRAIN GENERATION


generateDemoTrains : Time -> Random.Seed -> Int -> List RVTrain
generateDemoTrains currentTime seed count =
    if count > 0 then
        let
            ( train, nextSeed ) =
                generateDemoTrain currentTime seed
        in
            train :: (generateDemoTrains currentTime nextSeed (count - 1))
    else
        []


generateDemoTrain : Time -> Random.Seed -> ( RVTrain, Random.Seed )
generateDemoTrain currentTime seed0 =
    let
        topLeft =
            { lat = 49.89921, lng = 8.61696 }

        bottomRight =
            { lat = 49.85258, lng = 8.69334 }

        ( depPosition, seed1 ) =
            randomPosition seed0 topLeft bottomRight

        ( arrPosition, seed2 ) =
            randomPosition seed1 topLeft bottomRight

        depStation =
            toRVStation { id = "", name = "", pos = depPosition }

        arrStation =
            toRVStation { id = "", name = "", pos = arrPosition }

        depTime =
            currentTime

        ( duration, seed3 ) =
            Random.step (Random.float 60 300) seed2

        arrTime =
            currentTime + (duration * Time.second)

        ( edge, seed4 ) =
            generateEdge depStation arrStation 10 seed3

        totalLength =
            List.foldr (\( _, l ) s -> s + l) 0.0 edge

        train =
            { currentEdge = edge
            , departureTime = depTime
            , arrivalTime = arrTime
            , departureStation = depStation
            , arrivalStation = arrStation
            , pathLength = totalLength
            , currentSegment = Nothing
            }

        nextSeed =
            seed4
    in
        ( train, nextSeed )


generateEdge :
    RVStation
    -> RVStation
    -> Int
    -> Random.Seed
    -> ( List ( Vec2, Float ), Random.Seed )
generateEdge from to parts seed =
    let
        topLeft =
            ( min (Vector2.getX from.pos) (Vector2.getX to.pos) - 0.02
            , min (Vector2.getY from.pos) (Vector2.getY to.pos) - 0.02
            )

        bottomRight =
            ( max (Vector2.getX from.pos) (Vector2.getX to.pos) + 0.02
            , max (Vector2.getY from.pos) (Vector2.getY to.pos) + 0.02
            )

        ( ctrl1, seed1 ) =
            randomPoint seed topLeft bottomRight

        ( ctrl2, seed2 ) =
            randomPoint seed1 topLeft bottomRight

        w =
            ( from.pos, ctrl1, ctrl2, to.pos )

        delta =
            1.0 / (toFloat parts)

        points =
            List.range 1 parts
                |> List.map (\i -> (toFloat i) * delta)
                |> List.map (cbezier w)

        edgeParts =
            points
                |> List.foldl
                    (\point ( last, result ) ->
                        ( point, (Vector2.sub point last) :: result )
                    )
                    ( from.pos, [] )
                |> Tuple.second
                |> List.map (\v -> ( v, Vector2.length v ))
    in
        ( edgeParts, seed2 )


randomPoint : Random.Seed -> ( Float, Float ) -> ( Float, Float ) -> ( Vec2, Random.Seed )
randomPoint seed ( x1, y1 ) ( x2, y2 ) =
    let
        ( x, seed1 ) =
            Random.step (Random.float x1 x2) seed

        ( y, seed2 ) =
            Random.step (Random.float y1 y2) seed1
    in
        ( vec2 x y, seed2 )


cbezier : ( Vec2, Vec2, Vec2, Vec2 ) -> Float -> Vec2
cbezier ( w0, w1, w2, w3 ) t =
    let
        t2 =
            t * t

        t3 =
            t2 * t

        mt =
            1.0 - t

        mt2 =
            mt * mt

        mt3 =
            mt2 * mt
    in
        (Vector2.scale mt3 w0)
            |> Vector2.add (Vector2.scale (3 * mt2 * t) w1)
            |> Vector2.add (Vector2.scale (3 * mt * t2) w2)
            |> Vector2.add (Vector2.scale t3 w3)


randomPosition : Random.Seed -> Position -> Position -> ( Position, Random.Seed )
randomPosition seed a b =
    let
        ( lat, seed1 ) =
            Random.step (Random.float (min a.lat b.lat) (max a.lat b.lat)) seed

        ( lng, seed2 ) =
            Random.step (Random.float (min a.lng b.lng) (max a.lng b.lng)) seed1
    in
        ( { lat = lat, lng = lng }, seed2 )



-- UTIL


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
