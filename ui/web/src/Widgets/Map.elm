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
import Html.Attributes exposing (id, class, classList)
import Port exposing (..)
import Math.Vector2 as Vector2 exposing (Vec2, vec2)
import Math.Vector3 as Vector3 exposing (Vec3, vec3)
import Math.Matrix4 exposing (Mat4, scale, translate, makeOrtho2D)
import WebGL exposing (..)
import Time exposing (Time)
import Date exposing (Date)
import AnimationFrame
import Task exposing (..)
import Html exposing (Html)
import Data.Connection.Types exposing (Station, Position, TripId)
import Random
import Bitwise
import Maybe.Extra exposing (isJust, isNothing)
import Util.Core exposing ((=>))
import Util.List exposing ((!!))
import List.Extra
import Data.RailViz.Types exposing (..)
import Data.RailViz.Request
import Data.RailViz.Decode exposing (decodeRailVizTrainsResponse)
import Util.Api as Api
    exposing
        ( ApiError(..)
        , MotisErrorInfo(..)
        , ModuleErrorInfo(..)
        , RoutingErrorInfo(..)
        , MotisErrorDetail
        )
import Debounce
import Navigation
import Routes exposing (..)


-- MODEL


type alias Vertex =
    { c1 : Vec2
    , c2 : Vec2
    , p : Float
    , pickColor : Vec3
    }


type alias RVTrain =
    { names : List String
    , currentSegment : List ( Vec2, Float )
    , departureTime : Time
    , arrivalTime : Time
    , scheduledDepartureTime : Time
    , scheduledArrivalTime : Time
    , departureStation : RVStation
    , arrivalStation : RVStation
    , pathLength : Float
    , currentSubSegment : Maybe CurrentSubSegment
    , pickId : Int
    , pickColor : Vec3
    , trips : List TripId
    }


type alias RVStation =
    { station : Station
    , pos : Vec2
    }


type alias CurrentSubSegment =
    { startDistance : Float
    , startPoint : Vec2
    , endPoint : Vec2
    , length : Float
    , nextSubSegmentIndex : Int
    }


type alias Model =
    { mapInfo : MapInfo
    , texture : Maybe WebGL.Texture
    , time : Time
    , remoteAddress : String
    , rvTrains : List RVTrain
    , hoveredTrain : Maybe Int
    , nextUpdate : Maybe Time
    , debounce : Debounce.State
    }


init : String -> ( Model, Cmd Msg )
init remoteAddress =
    { mapInfo =
        { scale = 0
        , zoom = 0
        , pixelBounds = { north = 0, west = 0, width = 0, height = 0 }
        , geoBounds = { north = 0, west = 0, south = 0, east = 0 }
        , railVizBounds = { north = 0, west = 0, south = 0, east = 0 }
        }
    , texture = Nothing
    , time = 0.0
    , remoteAddress = remoteAddress
    , rvTrains = []
    , hoveredTrain = Nothing
    , nextUpdate = Nothing
    , debounce = Debounce.init
    }
        ! [ mapInit "map" ]


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
                    Just <| Vertex currentSubSegment.startPoint currentSubSegment.endPoint p train.pickColor

            Nothing ->
                Just <| Vertex train.departureStation.pos train.arrivalStation.pos 1.0 train.pickColor


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
    | MapUpdate MapInfo
    | TextureError WebGL.Error
    | TextureLoaded WebGL.Texture
    | GenerateDemoTrains Time
    | MouseMove Port.MapMouseUpdate
    | MouseDown Port.MapMouseUpdate
    | MouseUp Port.MapMouseUpdate
    | MouseOut Port.MapMouseUpdate
    | ReceiveResponse RailVizTrainsRequest RailVizTrainsResponse
    | ReceiveError RailVizTrainsRequest ApiError
    | CheckUpdate Time
    | RequestUpdate
    | Deb (Debounce.Msg Msg)


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        Animate t ->
            { model
                | time = t
                , rvTrains = List.map (updateCurrentSubSegment t) model.rvTrains
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
            let
                model_ =
                    { model | mapInfo = m_ }
            in
                model_ ! [ Debounce.debounceCmd debounceCfg <| RequestUpdate ]

        TextureError err ->
            ( model, Cmd.none )

        TextureLoaded texture ->
            ( { model | texture = Just texture }, Cmd.none )

        MouseMove mapMouseUpdate ->
            let
                model_ =
                    handleMapMouseUpdate mapMouseUpdate model
            in
                model_ ! []

        MouseDown mapMouseUpdate ->
            let
                model_ =
                    handleMapMouseUpdate mapMouseUpdate model

                selectedTrain =
                    model_.hoveredTrain
                        |> Maybe.andThen (\pickId -> model_.rvTrains !! pickId)

                tripId =
                    selectedTrain
                        |> Maybe.map .trips
                        |> Maybe.andThen List.head

                cmds =
                    tripId
                        |> Maybe.map (\tripId -> Navigation.newUrl (toUrl (tripDetailsRoute tripId)))
                        |> Maybe.Extra.maybeToList

                _ =
                    Debug.log "MouseDown" selectedTrain
            in
                model_ ! cmds

        MouseUp mapMouseUpdate ->
            let
                model_ =
                    handleMapMouseUpdate mapMouseUpdate model
            in
                model_ ! []

        MouseOut mapMouseUpdate ->
            let
                model_ =
                    handleMapMouseUpdate mapMouseUpdate model
            in
                model_ ! []

        ReceiveResponse request response ->
            let
                _ =
                    Debug.log "RailViz response"
                        ((toString (List.length response.trains))
                            ++ " trains, "
                            ++ (toString (List.length response.routes))
                            ++ " routes, "
                            ++ (toString (List.length response.stations))
                            ++ " stations"
                        )
            in
                (handleRailVizTrainsResponse response model) ! []

        ReceiveError request msg_ ->
            let
                _ =
                    Debug.log "RailViz error" msg_
            in
                model ! []

        CheckUpdate time ->
            case model.nextUpdate of
                Just nextUpdate ->
                    if nextUpdate <= time then
                        model ! [ Debounce.debounceCmd debounceCfg <| RequestUpdate ]
                    else
                        model ! []

                Nothing ->
                    model ! []

        RequestUpdate ->
            sendTrainRequest model

        Deb a ->
            Debounce.update debounceCfg a model


debounceCfg : Debounce.Config Model Msg
debounceCfg =
    Debounce.config
        .debounce
        (\model s -> { model | debounce = s })
        Deb
        500


handleMapMouseUpdate : MapMouseUpdate -> Model -> Model
handleMapMouseUpdate mapMouseUpdate model =
    let
        pickId =
            toPickId mapMouseUpdate.color
    in
        { model | hoveredTrain = pickId }


handleRailVizTrainsResponse : RailVizTrainsResponse -> Model -> Model
handleRailVizTrainsResponse response model =
    let
        rvStations =
            List.map toRVStation response.stations

        getSegment routeIdx segmentIdx =
            response.routes
                !! routeIdx
                |> Maybe.andThen (\r -> r.segments !! segmentIdx)

        getStation id =
            List.Extra.find (\s -> s.station.id == id) rvStations

        buildTrain : RailVizTrain -> Int -> RVStation -> RVStation -> RailVizSegment -> RVTrain
        buildTrain train pickId depStation arrStation segment =
            let
                rvSeg =
                    convertSegment segment

                totalLength =
                    List.foldr (\( _, l ) s -> s + l) 0.0 rvSeg
            in
                { names = train.names
                , currentSegment = rvSeg
                , departureTime = Date.toTime train.depTime
                , arrivalTime = Date.toTime train.arrTime
                , scheduledDepartureTime = Date.toTime train.scheduledDepTime
                , scheduledArrivalTime = Date.toTime train.scheduledArrTime
                , departureStation = depStation
                , arrivalStation = arrStation
                , pathLength = totalLength
                , currentSubSegment = Nothing
                , pickId = pickId
                , pickColor = toPickColor pickId
                , trips = train.trip
                }

        tryBuildTrain : Int -> RailVizTrain -> Maybe RVTrain
        tryBuildTrain pickId train =
            let
                seg =
                    getSegment train.routeIndex train.segmentIndex

                depStation =
                    seg |> Maybe.map .fromStationId |> Maybe.andThen getStation

                arrStation =
                    seg |> Maybe.map .toStationId |> Maybe.andThen getStation
            in
                Maybe.map3
                    (buildTrain train pickId)
                    depStation
                    arrStation
                    seg

        rvTrains =
            response.trains
                |> List.indexedMap tryBuildTrain
                |> List.filterMap identity
    in
        { model
            | rvTrains = rvTrains
        }


convertSegment : RailVizSegment -> List ( Vec2, Float )
convertSegment seg =
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

        toOffsets : Vec2 -> ( Vec2, List Vec2 ) -> ( Vec2, List Vec2 )
        toOffsets pt ( last, result ) =
            ( pt, (Vector2.sub pt last) :: result )
    in
        case coords of
            firstPt :: pts ->
                pts
                    |> List.foldl toOffsets ( firstPt, [] )
                    |> Tuple.second
                    |> List.map (\v -> ( v, Vector2.length v ))

            _ ->
                let
                    _ =
                        Debug.log "RailViz: Invalid Polyline (2)" <|
                            seg
                in
                    []



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.batch
        [ mapLoaded MapLoaded
        , mapUpdate MapUpdate
        , AnimationFrame.times Animate
        , mapMouseMove MouseMove
        , mapMouseDown MouseDown
        , mapMouseUp MouseUp
        , mapMouseOut MouseOut
        , Time.every (5 * Time.second) CheckUpdate
        ]



-- VIEW


view : Model -> Html Msg
view model =
    div [ id "map" ]
        [ overlay
            [ classList
                [ "leaflet-overlay" => True
                , "train-hover" => isJust model.hoveredTrain
                ]
            ]
            model
        ]


overlay : List (Html.Attribute Msg) -> Model -> Html Msg
overlay attributes model =
    let
        toHtml =
            WebGL.toHtmlWith
                [ Enable Blend
                , Disable DepthTest
                , BlendFunc ( SrcAlpha, OneMinusSrcAlpha )
                ]
                attributes
    in
        case model.texture of
            Nothing ->
                toHtml [] []

            Just tex ->
                let
                    buffer =
                        (mesh model.time model.rvTrains)

                    renderable =
                        render vertexShader
                            fragmentShader
                            buffer
                            { texture = tex
                            , perspective = perspective model
                            , zoom = model.mapInfo.zoom
                            }

                    offscreenRenderable =
                        render offscreenVertexShader
                            offscreenFragmentShader
                            buffer
                            { texture = tex
                            , perspective = perspective model
                            , zoom = model.mapInfo.zoom
                            }
                in
                    toHtml [ renderable ] [ offscreenRenderable ]


perspective : Model -> Mat4
perspective { mapInfo } =
    makeOrtho2D 0.0 mapInfo.pixelBounds.width mapInfo.pixelBounds.height 0.0
        |> scale (vec3 mapInfo.scale mapInfo.scale mapInfo.scale)
        |> translate
            (vec3
                -(mapInfo.pixelBounds.west / mapInfo.scale)
                -(mapInfo.pixelBounds.north / mapInfo.scale)
                0
            )



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


offscreenVertexShader : Shader { attr | c1 : Vec2, c2 : Vec2, p : Float, pickColor : Vec3 } { unif | perspective : Mat4, zoom : Float } { vPickColor : Vec3 }
offscreenVertexShader =
    [glsl|
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
precision mediump float;
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



-- REQUESTS


sendTrainRequest : Model -> ( Model, Cmd Msg )
sendTrainRequest model =
    let
        startTime =
            model.time

        endTime =
            startTime + (120 * 1000)

        nextUpdate =
            Just (endTime - 10)

        bounds =
            model.mapInfo.railVizBounds
    in
        { model | nextUpdate = nextUpdate }
            ! [ sendRequest model.remoteAddress
                    { corner1 =
                        { lat = bounds.north
                        , lng = bounds.west
                        }
                    , corner2 =
                        { lat = bounds.south
                        , lng = bounds.east
                        }
                    , startTime = Date.fromTime startTime
                    , endTime = Date.fromTime endTime
                    , maxTrains = 1000
                    }
              ]


sendRequest : String -> RailVizTrainsRequest -> Cmd Msg
sendRequest remoteAddress request =
    Api.sendRequest
        remoteAddress
        decodeRailVizTrainsResponse
        (ReceiveError request)
        (ReceiveResponse request)
        (Data.RailViz.Request.encodeRequest request)



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
