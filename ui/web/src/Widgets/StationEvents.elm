module Widgets.StationEvents
    exposing
        ( Model
        , Config(..)
        , Msg(..)
        , init
        , update
        , view
        )

import Html exposing (Html, div, ul, li, text, span, i, a, input, label)
import Html.Attributes exposing (..)
import Html.Events exposing (onInput, onMouseOver, onFocus, onClick, keyCode, on)
import Html.Lazy exposing (..)
import Date exposing (Date)
import Time exposing (Time)
import Data.Connection.Types exposing (TripId)
import Data.RailViz.Types exposing (..)
import Data.RailViz.Decode exposing (decodeRailVizStationResponse)
import Data.RailViz.Request as StationRequest exposing (encodeStationRequest, initialStationRequest)
import Util.Core exposing ((=>))
import Util.DateFormat exposing (..)
import Util.List exposing (last)
import Util.Api as Api
    exposing
        ( ApiError(..)
        , MotisErrorInfo(..)
        , ModuleErrorInfo(..)
        , AccessErrorInfo(..)
        , MotisErrorDetail
        )
import Localization.Base exposing (..)
import Widgets.LoadingSpinner as LoadingSpinner
import Widgets.Helpers.ConnectionUtil exposing (delay)
import Widgets.DateHeaders exposing (..)
import Maybe.Extra exposing (isJust)
import Random exposing (maxInt)


-- MODEL


type alias Model =
    { loading : Bool
    , loadingBefore : Bool
    , loadingAfter : Bool
    , remoteAddress : String
    , stationId : String
    , stationName : String
    , depEvents : List RailVizEvent
    , arrEvents : List RailVizEvent
    , viewType : ViewType
    , errorMessage : Maybe ApiError
    , errorBefore : Maybe ApiError
    , errorAfter : Maybe ApiError
    , request : RailVizStationRequest
    , minTime : Int
    , maxTime : Int
    , allowBefore : Bool
    , allowAfter : Bool
    }


type ViewType
    = Departures
    | Arrivals


type Config msg
    = Config
        { internalMsg : Msg -> msg
        , selectTripMsg : TripId -> msg
        , goBackMsg : msg
        }


init : String -> String -> String -> Date -> ( Model, Cmd Msg )
init remoteAddress stationId stationName date =
    let
        request =
            initialStationRequest stationId date
    in
        { loading = True
        , loadingBefore = False
        , loadingAfter = False
        , remoteAddress = remoteAddress
        , stationId = stationId
        , stationName = stationName
        , depEvents = []
        , arrEvents = []
        , viewType = Departures
        , errorMessage = Nothing
        , errorBefore = Nothing
        , errorAfter = Nothing
        , request = request
        , minTime = maxInt
        , maxTime = 0
        , allowBefore = True
        , allowAfter = True
        }
            ! [ sendRequest remoteAddress ReplaceResults request ]



-- UPDATE


type Msg
    = NoOp
    | ReceiveResponse SearchAction RailVizStationRequest RailVizStationResponse
    | ReceiveError SearchAction RailVizStationRequest ApiError
    | SetViewType ViewType
    | ExtendInterval ExtendIntervalType


type ExtendIntervalType
    = ExtendBefore
    | ExtendAfter


type SearchAction
    = ReplaceResults
    | PrependResults
    | AppendResults


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        NoOp ->
            model ! []

        ReceiveResponse action request response ->
            handleResponse model action response ! []

        ReceiveError action request msg_ ->
            handleRequestError model action msg_ ! []

        SetViewType vt ->
            { model | viewType = vt } ! []

        ExtendInterval direction ->
            let
                newRequest =
                    extendInterval direction model

                action =
                    case direction of
                        ExtendBefore ->
                            PrependResults

                        ExtendAfter ->
                            AppendResults

                loadingBefore_ =
                    case direction of
                        ExtendBefore ->
                            True

                        ExtendAfter ->
                            model.loadingBefore

                loadingAfter_ =
                    case direction of
                        ExtendBefore ->
                            model.loadingAfter

                        ExtendAfter ->
                            True
            in
                { model
                    | loadingBefore = loadingBefore_
                    , loadingAfter = loadingAfter_
                }
                    ! [ sendRequest model.remoteAddress action newRequest ]


handleResponse : Model -> SearchAction -> RailVizStationResponse -> Model
handleResponse model action response =
    let
        events =
            List.sortWith compareEvents response.events

        depEvents =
            List.filter (\e -> e.eventType == DEP) events

        arrEvents =
            List.filter (\e -> e.eventType == ARR) events

        newMinTime =
            events
                |> List.head
                |> Maybe.map getScheduleTime
                |> Maybe.map (\t -> floor (t / 1000))
                |> Maybe.map (Basics.min model.minTime)
                |> Maybe.withDefault model.minTime

        newMaxTime =
            events
                |> last
                |> Maybe.map getScheduleTime
                |> Maybe.map (\t -> floor (t / 1000))
                |> Maybe.map (Basics.max model.maxTime)
                |> Maybe.withDefault model.maxTime

        base =
            case action of
                ReplaceResults ->
                    { model
                        | loading = False
                        , errorMessage = Nothing
                        , errorBefore = Nothing
                        , errorAfter = Nothing
                        , allowBefore = True
                        , allowAfter = True
                    }

                PrependResults ->
                    { model
                        | loadingBefore = False
                        , errorBefore = Nothing
                        , allowBefore = not (List.isEmpty events)
                    }

                AppendResults ->
                    { model
                        | loadingAfter = False
                        , errorAfter = Nothing
                        , allowAfter = not (List.isEmpty events)
                    }

        ( newDepEvents, newArrEvents ) =
            case action of
                ReplaceResults ->
                    ( depEvents, arrEvents )

                PrependResults ->
                    ( depEvents ++ model.depEvents
                    , arrEvents ++ model.arrEvents
                    )

                AppendResults ->
                    ( model.depEvents ++ depEvents
                    , model.arrEvents ++ arrEvents
                    )
    in
        { base
            | depEvents = newDepEvents
            , arrEvents = newArrEvents
            , minTime = newMinTime
            , maxTime = newMaxTime
        }


compareEvents : RailVizEvent -> RailVizEvent -> Basics.Order
compareEvents a b =
    let
        byDate =
            compare (getScheduleTime a) (getScheduleTime b)
    in
        if byDate == EQ then
            compare (getServiceName a) (getServiceName b)
        else
            byDate


handleRequestError : Model -> SearchAction -> ApiError -> Model
handleRequestError model action msg =
    let
        newModel =
            case action of
                ReplaceResults ->
                    { model
                        | loading = False
                        , errorMessage = Just msg
                        , depEvents = []
                        , arrEvents = []
                    }

                PrependResults ->
                    { model
                        | loadingBefore = False
                        , errorBefore = Just msg
                    }

                AppendResults ->
                    { model
                        | loadingAfter = False
                        , errorAfter = Just msg
                    }
    in
        newModel


extendInterval :
    ExtendIntervalType
    -> Model
    -> RailVizStationRequest
extendInterval direction model =
    let
        base =
            model.request
    in
        case direction of
            ExtendBefore ->
                { base
                    | time = model.minTime
                    , direction = EARLIER
                }

            ExtendAfter ->
                { base
                    | time = model.maxTime
                    , direction = LATER
                }



-- VIEW


view : Config msg -> Localization -> Model -> Html msg
view config locale model =
    div [ class "station-events" ]
        [ lazy3 headerView config locale model
        , lazy3 contentView config locale model
        ]


headerView : Config msg -> Localization -> Model -> Html msg
headerView config locale model =
    let
        (Config { goBackMsg }) =
            config
    in
        div [ class "header" ]
            [ div [ onClick goBackMsg, class "back" ]
                [ i [ class "icon" ] [ text "arrow_back" ] ]
            , div [ class "station" ] [ text model.stationName ]
            , eventTypePicker config locale model
            ]


eventTypePicker : Config msg -> Localization -> Model -> Html msg
eventTypePicker (Config { internalMsg }) locale model =
    div [ class "event-type-picker" ]
        [ div []
            [ input
                [ type_ "radio"
                , id "station-departures"
                , name "station-event-types"
                , checked (model.viewType == Departures)
                , onClick (internalMsg (SetViewType Departures))
                ]
                []
            , label [ for "station-departures" ] [ text locale.t.search.departure ]
            ]
        , div []
            [ input
                [ type_ "radio"
                , id "station-arrivals"
                , name "station-event-types"
                , checked (model.viewType == Arrivals)
                , onClick (internalMsg (SetViewType Arrivals))
                ]
                []
            , label [ for "station-arrivals" ] [ text locale.t.search.arrival ]
            ]
        ]


contentView : Config msg -> Localization -> Model -> Html msg
contentView config locale model =
    let
        content =
            if model.loading then
                div [ class "loading" ] [ LoadingSpinner.view ]
            else
                case model.errorMessage of
                    Just err ->
                        errorView "main-error" locale model err

                    Nothing ->
                        stationEventsView config locale model
    in
        div [ class "events" ] [ content ]


stationEventsView : Config msg -> Localization -> Model -> Html msg
stationEventsView config locale model =
    let
        events =
            getEvents model

        content =
            if List.isEmpty events then
                div [ class "no-results" ]
                    [ div [ class "divider" ] []
                    , div [ class "msg" ]
                        [ case model.viewType of
                            Departures ->
                                text locale.t.station.noDepartures

                            Arrivals ->
                                text locale.t.station.noArrivals
                        ]
                    ]
            else
                div [ class "event-list" ]
                    (withDateHeaders
                        getScheduleDate
                        (eventView config locale)
                        (dateHeader locale)
                        (getEvents model)
                    )
    in
        div []
            [ extendIntervalButton ExtendBefore config locale model
            , content
            , div [ class "divider footer" ] []
            , extendIntervalButton ExtendAfter config locale model
            ]


eventView : Config msg -> Localization -> RailVizEvent -> Html msg
eventView (Config { selectTripMsg }) locale event =
    let
        clickAttr =
            List.head event.trips
                |> Maybe.map (.id >> selectTripMsg >> onClick)
                |> Maybe.Extra.maybeToList

        scheduleTime =
            getScheduleDate event

        transport =
            List.head event.trips
                |> Maybe.map .transport

        serviceName =
            getServiceName event

        direction =
            transport
                |> Maybe.map .direction
                |> Maybe.withDefault "?"
    in
        div [ class "pure-g station-event" ]
            [ div [ class "pure-u-4-24 event-time" ]
                [ text (formatTime scheduleTime)
                , text " "
                , delay event.event
                ]
            , div [ class "pure-u-4-24 event-train" ]
                [ span clickAttr [ text serviceName ] ]
            , div [ class "pure-u-14-24 event-direction" ]
                [ text direction ]
            , div [ class "pure-u-2-24 event-track" ]
                [ text event.event.track ]
            ]


extendIntervalButton :
    ExtendIntervalType
    -> Config msg
    -> Localization
    -> Model
    -> Html msg
extendIntervalButton direction (Config { internalMsg }) locale model =
    let
        allowed =
            case direction of
                ExtendBefore ->
                    model.allowBefore

                ExtendAfter ->
                    model.allowAfter

        enabled =
            allowed
                && case direction of
                    ExtendBefore ->
                        not model.loadingBefore

                    ExtendAfter ->
                        not model.loadingAfter

        divClass =
            case direction of
                ExtendBefore ->
                    "search-before"

                ExtendAfter ->
                    "search-after"

        title =
            case direction of
                ExtendBefore ->
                    locale.t.connections.extendBefore

                ExtendAfter ->
                    locale.t.connections.extendAfter

        clickHandler =
            if enabled then
                internalMsg <| ExtendInterval direction
            else
                internalMsg NoOp

        err =
            case direction of
                ExtendBefore ->
                    model.errorBefore

                ExtendAfter ->
                    model.errorAfter
    in
        div
            [ classList
                [ "extend-search-interval" => True
                , divClass => True
                , "disabled" => not enabled
                , "error" => (enabled && (isJust err))
                ]
            ]
            [ if enabled then
                case err of
                    Nothing ->
                        a
                            [ onClick clickHandler ]
                            [ text title ]

                    Just error ->
                        errorView "error" locale model error
              else if allowed then
                LoadingSpinner.view
              else
                text ""
            ]


errorView : String -> Localization -> Model -> ApiError -> Html msg
errorView divClass locale model err =
    let
        errorMsg =
            case err of
                MotisError err_ ->
                    motisErrorMsg locale err_

                TimeoutError ->
                    locale.t.connections.errors.timeout

                NetworkError ->
                    locale.t.connections.errors.network

                HttpError status ->
                    locale.t.connections.errors.http status

                DecodeError msg ->
                    locale.t.connections.errors.decode msg
    in
        div [ class divClass ]
            [ div [] [ text errorMsg ]
            ]


motisErrorMsg : Localization -> MotisErrorInfo -> String
motisErrorMsg { t } err =
    case err of
        AccessError AccessTimestampNotInSchedule ->
            t.connections.errors.journeyDateNotInSchedule

        _ ->
            t.connections.errors.internalError (toString err)


getEvents : Model -> List RailVizEvent
getEvents model =
    case model.viewType of
        Departures ->
            model.depEvents

        Arrivals ->
            model.arrEvents


getScheduleTime : RailVizEvent -> Time
getScheduleTime event =
    event.event.schedule_time
        |> Maybe.map Date.toTime
        |> Maybe.withDefault 0


getScheduleDate : RailVizEvent -> Date
getScheduleDate event =
    event.event.schedule_time
        |> Maybe.withDefault (Date.fromTime 0)


getServiceName : RailVizEvent -> String
getServiceName =
    .trips
        >> List.head
        >> Maybe.map .transport
        >> Maybe.map .name
        >> Maybe.withDefault "?"



-- API


sendRequest : String -> SearchAction -> RailVizStationRequest -> Cmd Msg
sendRequest remoteAddress action request =
    Api.sendRequest
        remoteAddress
        decodeRailVizStationResponse
        (ReceiveError action request)
        (ReceiveResponse action request)
        (encodeStationRequest request)
