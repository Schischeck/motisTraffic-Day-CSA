module Widgets.Map.RailViz
    exposing
        ( Msg(SetFilter, SetTimeOffset)
        , init
        , view
        , update
        , subscriptions
        , mapId
        )

import Widgets.Map.RailVizModel exposing (..)
import Widgets.Map.RailVizHandler exposing (..)
import Widgets.Map.Picking exposing (..)
import Widgets.Map.Trains as Trains
import Widgets.Map.Stations as Stations
import Widgets.Map.Routes as Routes
import Widgets.Map.Port as Port exposing (..)
import Html exposing (Html, Attribute, div, text, span)
import Html.Attributes exposing (id, class, classList, style)
import Math.Vector3 as Vector3 exposing (Vec3, vec3)
import Math.Matrix4 exposing (Mat4, scale, translate, makeOrtho2D)
import WebGL exposing (..)
import Time exposing (Time)
import Date exposing (Date)
import AnimationFrame
import Task exposing (..)
import Html exposing (Html)
import Data.Connection.Types exposing (Station, Position, TripId)
import Maybe.Extra exposing (isJust, isNothing)
import Util.Core exposing ((=>))
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
import Util.DateFormat exposing (formatTime, formatDateTimeWithSeconds)
import Localization.Base exposing (..)


-- MODEL


init : String -> ( Model, Cmd Msg )
init remoteAddress =
    { mapInfo =
        { scale = 0
        , zoom = 0
        , pixelBounds = { north = 0, west = 0, width = 0, height = 0 }
        , geoBounds = { north = 0, west = 0, south = 0, east = 0 }
        , railVizBounds = { north = 0, west = 0, south = 0, east = 0 }
        }
    , textures = Nothing
    , time = 0
    , systemTime = 0
    , timeOffset = 0
    , remoteAddress = remoteAddress
    , fullData =
        { trains = []
        , stations = []
        , stationsDrawable = Points []
        , routesDrawable = Lines []
        }
    , filteredData = Nothing
    , hoveredPickId = Nothing
    , nextUpdate = Nothing
    , debounce = Debounce.init
    , mouseX = 0
    , mouseY = 0
    }
        ! [ Task.perform SetTime Time.now
          , mapInit mapId
          ]


filteredZoomMin : number
filteredZoomMin =
    13



-- UPDATE


type Msg
    = Animate Time
    | MapLoaded String
    | MapUpdate MapInfo
    | TextureError WebGL.Error
    | TextureLoaded ( WebGL.Texture, WebGL.Texture )
    | MouseMove Port.MapMouseUpdate
    | MouseDown Port.MapMouseUpdate
    | MouseUp Port.MapMouseUpdate
    | MouseOut Port.MapMouseUpdate
    | TrainsReceiveResponse RailVizTrainsRequest RailVizTrainsResponse
    | TrainsReceiveError RailVizTrainsRequest ApiError
    | TripsReceiveResponse RailVizTripsRequest RailVizTrainsResponse
    | TripsReceiveError RailVizTripsRequest ApiError
    | CheckUpdate Time
    | RequestUpdate
    | Deb (Debounce.Msg Msg)
    | SetFilter (Maybe (List TripId))
    | SetTime Time
    | SetTimeOffset Float


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        Animate t ->
            let
                simTime =
                    t + model.timeOffset

                model_ =
                    { model
                        | systemTime = t
                        , time = simTime
                    }

                updateData data =
                    { data
                        | trains =
                            List.map
                                (Trains.updateCurrentSubSegment simTime)
                                data.trains
                    }
            in
                case model_.filteredData of
                    Just data ->
                        { model_
                            | filteredData = Just (updateData data)
                        }
                            ! []

                    Nothing ->
                        { model_
                            | fullData = updateData model_.fullData
                        }
                            ! []

        MapLoaded _ ->
            model
                ! [ Task.attempt
                        (\r ->
                            case r of
                                Ok v ->
                                    TextureLoaded v

                                Err v ->
                                    TextureError v
                        )
                        fetchTextures
                  ]

        MapUpdate m_ ->
            let
                model_ =
                    { model | mapInfo = m_ }
            in
                model_ ! [ Debounce.debounceCmd debounceCfg <| RequestUpdate ]

        TextureError err ->
            let
                _ =
                    Debug.log "Error loading texture" err
            in
                model ! []

        TextureLoaded textures ->
            { model
                | textures = Just textures
            }
                ! []

        MouseMove mapMouseUpdate ->
            handleMapMouseUpdate mapMouseUpdate model

        MouseDown mapMouseUpdate ->
            let
                ( model_, cmds1 ) =
                    handleMapMouseUpdate mapMouseUpdate model

                selectedTrain =
                    getHoveredTrain model_

                tripId =
                    selectedTrain
                        |> Maybe.map .trips
                        |> Maybe.andThen List.head

                cmds2 =
                    tripId
                        |> Maybe.map (\tripId -> Navigation.newUrl (toUrl (tripDetailsRoute tripId)))
                        |> Maybe.withDefault Cmd.none

                selectedStation =
                    getHoveredStation model_

                cmds3 =
                    selectedStation
                        |> Maybe.map (\rvs -> Navigation.newUrl (toUrl (StationEvents rvs.station.id rvs.station.name)))
                        |> Maybe.withDefault Cmd.none
            in
                model_ ! [ cmds1, cmds2, cmds3 ]

        MouseUp mapMouseUpdate ->
            handleMapMouseUpdate mapMouseUpdate model

        MouseOut mapMouseUpdate ->
            handleMapMouseUpdate mapMouseUpdate model

        TrainsReceiveResponse request response ->
            let
                _ =
                    Debug.log "RailViz Trains response" (responseDebugMsg response)

                data =
                    handleRailVizTrainsResponse response model
            in
                { model | fullData = data } ! []

        TrainsReceiveError request msg_ ->
            let
                _ =
                    Debug.log "RailViz Trains error" msg_
            in
                model ! []

        TripsReceiveResponse request response ->
            let
                _ =
                    Debug.log "RailViz Trips response" (responseDebugMsg response)

                data =
                    handleRailVizTrainsResponse response model
            in
                update (Animate model.time) { model | filteredData = Just data }

        TripsReceiveError request msg_ ->
            let
                _ =
                    Debug.log "RailViz Trips error" msg_
            in
                { model | filteredData = Nothing } ! []

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

        SetFilter filterTrips ->
            case filterTrips of
                Just trips ->
                    { model
                        | filteredData = Just (applyFilter trips model.fullData)
                    }
                        ! [ sendTripsRequest
                                model.remoteAddress
                                { trips = trips }
                          ]

                Nothing ->
                    update (Animate model.time) { model | filteredData = Nothing }

        SetTime time ->
            { model
                | systemTime = time
                , time = time + model.timeOffset
            }
                ! []

        SetTimeOffset offset ->
            { model
                | timeOffset = offset
                , time = model.systemTime + offset
            }
                ! [ Debounce.debounceCmd debounceCfg <| RequestUpdate ]


debounceCfg : Debounce.Config Model Msg
debounceCfg =
    Debounce.config
        .debounce
        (\model s -> { model | debounce = s })
        Deb
        500


handleMapMouseUpdate : MapMouseUpdate -> Model -> ( Model, Cmd Msg )
handleMapMouseUpdate mapMouseUpdate model =
    let
        pickId =
            toPickId mapMouseUpdate.color

        mouseX =
            floor mapMouseUpdate.x

        mouseY =
            floor mapMouseUpdate.y

        changed =
            ( model.hoveredPickId, model.mouseX, model.mouseY ) /= ( pickId, mouseX, mouseY )
    in
        if changed then
            let
                model_ =
                    { model
                        | hoveredPickId = pickId
                        , mouseX = floor mapMouseUpdate.x
                        , mouseY = floor mapMouseUpdate.y
                    }
            in
                model_ ! []
        else
            model ! []


fetchTextures : Task Error ( Texture, Texture )
fetchTextures =
    loadTextureWithFilter Nearest "img/railviz/train.png"
        |> Task.andThen
            (\trainTexture ->
                loadTextureWithFilter Nearest "img/railviz/station.png"
                    |> Task.andThen
                        (\stationTexture ->
                            Task.succeed ( trainTexture, stationTexture )
                        )
            )



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


mapId : String
mapId =
    "map"


view : Localization -> Model -> Html Msg
view locale model =
    div [ class "map-container" ]
        [ div [ class "inner-map-container" ]
            [ div [ id mapId ]
                [ Html.canvas
                    [ classList
                        [ "railviz-overlay" => True
                        , "leaflet-zoom-hide" => True
                        , "train-hover" => isJust model.hoveredPickId
                        ]
                    ]
                    []
                ]
            , railVizTooltip model
            , simulationTimeOverlay locale model
            ]
        ]


railVizOverlay : Model -> Html Msg
railVizOverlay model =
    let
        toHtml =
            WebGL.toHtmlWith
                [ Enable Blend
                , Disable DepthTest
                , BlendFunc ( SrcAlpha, OneMinusSrcAlpha )
                ]
                [ classList
                    [ "railviz-overlay" => True
                    , "leaflet-zoom-hide" => True
                    , "train-hover" => isJust model.hoveredPickId
                    ]
                ]
    in
        case model.textures of
            Nothing ->
                toHtml [] []

            Just ( trainTexture, stationTexture ) ->
                let
                    data =
                        getData model

                    trainsBuffer =
                        Trains.mesh model.time data.trains

                    zoom =
                        if isJust model.filteredData then
                            max filteredZoomMin model.mapInfo.zoom
                        else
                            model.mapInfo.zoom

                    persp =
                        perspective model

                    renderables =
                        [ render
                            Trains.vertexShader
                            Trains.fragmentShader
                            trainsBuffer
                            { uTexture = trainTexture
                            , uPerspective = persp
                            , uZoom = zoom
                            }
                        , render
                            Stations.vertexShader
                            Stations.fragmentShader
                            data.stationsDrawable
                            { uTexture = stationTexture
                            , uPerspective = persp
                            , uZoom = zoom
                            }
                        , render
                            Routes.vertexShader
                            Routes.fragmentShader
                            data.routesDrawable
                            { uPerspective = persp
                            , uZoom = zoom
                            , uColor = Routes.lineColor
                            }
                        ]

                    offscreenRenderables =
                        [ render
                            Trains.offscreenVertexShader
                            Trains.offscreenFragmentShader
                            trainsBuffer
                            { uTexture = trainTexture
                            , uPerspective = persp
                            , uZoom = zoom
                            }
                        , render
                            Stations.offscreenVertexShader
                            Stations.offscreenFragmentShader
                            data.stationsDrawable
                            { uTexture = stationTexture
                            , uPerspective = persp
                            , uZoom = zoom
                            }
                        ]
                in
                    toHtml renderables offscreenRenderables


railVizTooltip : Model -> Html Msg
railVizTooltip model =
    case getHoveredTrain model of
        Just train ->
            railVizTrainTooltip model train

        Nothing ->
            case getHoveredStation model of
                Just station ->
                    railVizStationTooltip model station

                Nothing ->
                    div [ class "railviz-tooltip hidden" ] []


railVizTooltipWidth : number
railVizTooltipWidth =
    240


railVizTrainTooltip : Model -> RVTrain -> Html Msg
railVizTrainTooltip model train =
    let
        ttWidth =
            railVizTooltipWidth

        ttHeight =
            55

        margin =
            20

        x =
            (model.mouseX - (ttWidth // 2))
                |> max margin
                |> min (floor model.mapInfo.pixelBounds.width - ttWidth - margin)

        below =
            model.mouseY + margin + ttHeight < floor model.mapInfo.pixelBounds.height

        y =
            if below then
                model.mouseY + margin
            else
                (floor model.mapInfo.pixelBounds.height) - (model.mouseY - margin)

        yAnchor =
            if below then
                "top"
            else
                "bottom"

        trainName =
            String.join " / " train.names

        schedDep =
            Date.fromTime train.scheduledDepartureTime

        schedArr =
            Date.fromTime train.scheduledArrivalTime

        depDelay =
            ceiling ((train.departureTime - train.scheduledDepartureTime) / 60000)

        arrDelay =
            ceiling ((train.arrivalTime - train.scheduledArrivalTime) / 60000)
    in
        div
            [ class "railviz-tooltip train visible"
            , style
                [ yAnchor => (toString y ++ "px")
                , "left" => (toString x ++ "px")
                ]
            ]
            [ div [ class "transport-name" ] [ text trainName ]
            , div [ class "departure" ]
                [ span [ class "station" ] [ text train.departureStation.station.name ]
                , div [ class "time" ]
                    [ span [ class "schedule" ] [ text (formatTime schedDep) ]
                    , delayView depDelay
                    ]
                ]
            , div [ class "arrival" ]
                [ span [ class "station" ] [ text train.arrivalStation.station.name ]
                , div [ class "time" ]
                    [ span [ class "schedule" ] [ text (formatTime schedArr) ]
                    , delayView arrDelay
                    ]
                ]
            ]


railVizStationTooltip : Model -> RVStation -> Html Msg
railVizStationTooltip model station =
    let
        ttWidth =
            railVizTooltipWidth

        ttHeight =
            22

        margin =
            20

        x =
            (model.mouseX - (ttWidth // 2))
                |> max margin
                |> min (floor model.mapInfo.pixelBounds.width - ttWidth - margin)

        below =
            model.mouseY + margin + ttHeight < floor model.mapInfo.pixelBounds.height

        y =
            if below then
                model.mouseY + margin
            else
                (floor model.mapInfo.pixelBounds.height) - (model.mouseY - margin)

        yAnchor =
            if below then
                "top"
            else
                "bottom"

        stationName =
            station.station.name
    in
        div
            [ class "railviz-tooltip station visible"
            , style
                [ yAnchor => (toString y ++ "px")
                , "left" => (toString x ++ "px")
                ]
            ]
            [ div [ class "station-name" ] [ text stationName ] ]


simulationTimeOverlay : Localization -> Model -> Html Msg
simulationTimeOverlay locale model =
    let
        simDate =
            Date.fromTime model.time
    in
        div
            [ class "sim-time-overlay" ]
            [ text (formatDateTimeWithSeconds locale.dateConfig simDate) ]


delayView : Int -> Html Msg
delayView minutes =
    let
        delayType =
            if minutes > 0 then
                "delay pos-delay"
            else
                "delay neg-delay"

        delayText =
            if minutes >= 0 then
                "+" ++ (toString minutes)
            else
                toString minutes
    in
        span [ class delayType ] [ text delayText ]


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



-- REQUESTS


sendTrainRequest : Model -> ( Model, Cmd Msg )
sendTrainRequest model =
    let
        startTime =
            model.time

        intervalLength =
            120 * 1000

        endTime =
            startTime + intervalLength

        nextUpdate =
            Just (model.systemTime + intervalLength - 10)

        bounds =
            model.mapInfo.railVizBounds
    in
        { model | nextUpdate = nextUpdate }
            ! [ sendTrainsRequest model.remoteAddress
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


sendTrainsRequest : String -> RailVizTrainsRequest -> Cmd Msg
sendTrainsRequest remoteAddress request =
    Api.sendRequest
        remoteAddress
        decodeRailVizTrainsResponse
        (TrainsReceiveError request)
        (TrainsReceiveResponse request)
        (Data.RailViz.Request.encodeTrainsRequest request)


sendTripsRequest : String -> RailVizTripsRequest -> Cmd Msg
sendTripsRequest remoteAddress request =
    Api.sendRequest
        remoteAddress
        decodeRailVizTrainsResponse
        (TripsReceiveError request)
        (TripsReceiveResponse request)
        (Data.RailViz.Request.encodeTripsRequest request)


responseDebugMsg : RailVizTrainsResponse -> String
responseDebugMsg response =
    (toString (List.length response.trains))
        ++ " trains, "
        ++ (toString (List.length response.routes))
        ++ " routes, "
        ++ (toString (List.length response.stations))
        ++ " stations"
