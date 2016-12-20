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
import Widgets.Map.Trains as Trains
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
    , texture = Nothing
    , time = 0
    , systemTime = 0
    , timeOffset = 0
    , remoteAddress = remoteAddress
    , allTrains = []
    , filteredTrains = []
    , filterTrips = Nothing
    , hoveredTrain = Nothing
    , nextUpdate = Nothing
    , debounce = Debounce.init
    , mouseX = 0
    , mouseY = 0
    , zoomOverride = Nothing
    }
        ! [ Task.perform SetTime Time.now
          , mapInit mapId
          ]


filteredZoomOverride : number
filteredZoomOverride =
    13



-- UPDATE


type Msg
    = Animate Time
    | MapLoaded String
    | MapUpdate MapInfo
    | TextureError WebGL.Error
    | TextureLoaded WebGL.Texture
    | MouseMove Port.MapMouseUpdate
    | MouseDown Port.MapMouseUpdate
    | MouseUp Port.MapMouseUpdate
    | MouseOut Port.MapMouseUpdate
    | ReceiveResponse RailVizTrainsRequest RailVizTrainsResponse
    | ReceiveError RailVizTrainsRequest ApiError
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
            in
                { model
                    | systemTime = t
                    , time = simTime
                    , filteredTrains =
                        List.map
                            (Trains.updateCurrentSubSegment simTime)
                            model.filteredTrains
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
            handleMapMouseUpdate mapMouseUpdate model

        MouseDown mapMouseUpdate ->
            let
                ( model_, cmds1 ) =
                    handleMapMouseUpdate mapMouseUpdate model

                selectedTrain =
                    Trains.getHoveredTrain model_

                tripId =
                    selectedTrain
                        |> Maybe.map .trips
                        |> Maybe.andThen List.head

                cmds2 =
                    tripId
                        |> Maybe.map (\tripId -> Navigation.newUrl (toUrl (tripDetailsRoute tripId)))
                        |> Maybe.Extra.maybeToList
            in
                model_ ! (cmds1 :: cmds2)

        MouseUp mapMouseUpdate ->
            handleMapMouseUpdate mapMouseUpdate model

        MouseOut mapMouseUpdate ->
            handleMapMouseUpdate mapMouseUpdate model

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

        SetFilter filterTrips ->
            let
                model_ =
                    { model
                        | filterTrips = filterTrips
                        , filteredTrains = applyFilter filterTrips model.allTrains
                        , zoomOverride = Maybe.map (always filteredZoomOverride) filterTrips
                    }
            in
                update (Animate model.time) model_

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
            Trains.toPickId mapMouseUpdate.color

        mouseX =
            floor mapMouseUpdate.x

        mouseY =
            floor mapMouseUpdate.y

        changed =
            ( model.hoveredTrain, model.mouseX, model.mouseY ) /= ( pickId, mouseX, mouseY )
    in
        if changed then
            let
                model_ =
                    { model
                        | hoveredTrain = pickId
                        , mouseX = floor mapMouseUpdate.x
                        , mouseY = floor mapMouseUpdate.y
                    }
            in
                model_ ! []
        else
            model ! []



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
                [ railVizOverlay model ]
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
                    , "train-hover" => isJust model.hoveredTrain
                    ]
                ]
    in
        case model.texture of
            Nothing ->
                toHtml [] []

            Just tex ->
                let
                    buffer =
                        (Trains.mesh model.time model.filteredTrains)

                    zoom =
                        model.zoomOverride
                            |> Maybe.withDefault model.mapInfo.zoom

                    renderable =
                        render Trains.vertexShader
                            Trains.fragmentShader
                            buffer
                            { texture = tex
                            , perspective = perspective model
                            , zoom = zoom
                            }

                    offscreenRenderable =
                        render Trains.offscreenVertexShader
                            Trains.offscreenFragmentShader
                            buffer
                            { texture = tex
                            , perspective = perspective model
                            , zoom = zoom
                            }
                in
                    toHtml [ renderable ] [ offscreenRenderable ]


railVizTooltip : Model -> Html Msg
railVizTooltip model =
    let
        maybeTrain =
            Trains.getHoveredTrain model
    in
        case maybeTrain of
            Just train ->
                railVizTrainTooltip model train

            Nothing ->
                div [ class "railviz-tooltip hidden" ] []


railVizTrainTooltip : Model -> RVTrain -> Html Msg
railVizTrainTooltip model train =
    let
        ttWidth =
            240

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
            [ class "railviz-tooltip visible"
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
