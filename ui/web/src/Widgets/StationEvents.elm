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
import Data.Connection.Types exposing (TripId)
import Data.Lookup.Types exposing (..)
import Data.Lookup.Decode exposing (decodeLookupStationEventsResponse)
import Data.Lookup.Request as LookupRequest exposing (encodeStationEventsRequest, initialStationEventsRequest)
import Util.Core exposing ((=>))
import Util.DateFormat exposing (..)
import Util.Api as Api
    exposing
        ( ApiError(..)
        , MotisErrorInfo(..)
        , ModuleErrorInfo(..)
        , LookupErrorInfo(..)
        , MotisErrorDetail
        )
import Localization.Base exposing (..)
import Widgets.LoadingSpinner as LoadingSpinner
import Widgets.Helpers.ConnectionUtil exposing (delayText, zeroDelay, isDelayed)
import Date.Extra.Duration as Duration exposing (DeltaRecord)
import Maybe.Extra


-- MODEL


type alias Model =
    { loading : Bool
    , loadingBefore : Bool
    , loadingAfter : Bool
    , remoteAddress : String
    , stationId : String
    , depEvents : List StationEvent
    , arrEvents : List StationEvent
    , viewType : ViewType
    , errorMessage : Maybe ApiError
    , errorBefore : Maybe ApiError
    , errorAfter : Maybe ApiError
    , request : LookupStationEventsRequest
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


init : String -> String -> Date -> ( Model, Cmd Msg )
init remoteAddress stationId date =
    let
        request =
            initialStationEventsRequest stationId date
    in
        { loading = True
        , loadingBefore = False
        , loadingAfter = False
        , remoteAddress = remoteAddress
        , stationId = stationId
        , depEvents = []
        , arrEvents = []
        , viewType = Departures
        , errorMessage = Nothing
        , errorBefore = Nothing
        , errorAfter = Nothing
        , request = request
        }
            ! [ sendRequest remoteAddress request ]



-- UPDATE


type Msg
    = NoOp
    | ReceiveResponse LookupStationEventsRequest LookupStationEventsResponse
    | ReceiveError LookupStationEventsRequest ApiError
    | SetViewType ViewType


type ExtendIntervalType
    = ExtendBefore
    | ExtendAfter


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        NoOp ->
            model ! []

        ReceiveResponse request response ->
            handleResponse model response ! []

        ReceiveError request msg_ ->
            handleRequestError model msg_ ! []

        SetViewType vt ->
            { model | viewType = vt } ! []


handleResponse : Model -> LookupStationEventsResponse -> Model
handleResponse model response =
    let
        events =
            List.sortBy (.scheduleTime >> Date.toTime) response.events

        depEvents =
            List.filter (\e -> e.eventType == DEP) events

        arrEvents =
            List.filter (\e -> e.eventType == ARR) events
    in
        { model
            | loading = False
            , depEvents = depEvents
            , arrEvents = arrEvents
        }


handleRequestError : Model -> ApiError -> Model
handleRequestError model msg =
    { model
        | loading = False
        , errorMessage = Just msg
        , depEvents = []
        , arrEvents = []
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
            , div [ class "station" ] [ text ("Station " ++ model.stationId) ]
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
    if model.loading then
        div [ class "loading" ] [ LoadingSpinner.view ]
    else if List.isEmpty (getEvents model) then
        case model.errorMessage of
            Just err ->
                errorView "main-error" locale model err

            Nothing ->
                div [ class "no-results" ]
                    [ div [] [ text locale.t.station.noDepartures ] ]
    else
        stationEventsView config locale model


stationEventsView : Config msg -> Localization -> Model -> Html msg
stationEventsView config locale model =
    div [ class "events" ]
        (List.map (eventView config locale) (getEvents model))


eventView : Config msg -> Localization -> StationEvent -> Html msg
eventView (Config { selectTripMsg }) locale event =
    let
        clickAttr =
            List.head event.tripId
                |> Maybe.map (selectTripMsg >> onClick)
                |> Maybe.Extra.maybeToList
    in
        div [ class "pure-g station-event" ]
            [ div [ class "pure-u-4-24 event-time" ]
                [ text (formatTime event.scheduleTime)
                , text " "
                , delay event
                ]
            , div [ class "pure-u-4-24 event-train" ]
                [ span clickAttr [ text event.serviceName ] ]
            , div [ class "pure-u-14-24 event-direction" ]
                [ text event.direction ]
            , div [ class "pure-u-2-24 event-track" ]
                [ text event.track ]
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
        LookupError LookupNotInSchedulePeriod ->
            t.connections.errors.journeyDateNotInSchedule

        _ ->
            t.connections.errors.internalError (toString err)


getEvents : Model -> List StationEvent
getEvents model =
    case model.viewType of
        Departures ->
            model.depEvents

        Arrivals ->
            model.arrEvents


delay : StationEvent -> Html msg
delay event =
    let
        diff =
            Duration.diff event.time event.scheduleTime

        noDelay =
            zeroDelay diff

        posDelay =
            isDelayed diff
    in
        if noDelay then
            text ""
        else
            div
                [ class <|
                    if posDelay then
                        "delay pos-delay"
                    else
                        "delay neg-delay"
                ]
                [ span []
                    [ text <|
                        (if posDelay then
                            "+"
                         else
                            "-"
                        )
                            ++ (delayText diff)
                    ]
                ]



-- API


sendRequest : String -> LookupStationEventsRequest -> Cmd Msg
sendRequest remoteAddress request =
    Api.sendRequest
        remoteAddress
        decodeLookupStationEventsResponse
        (ReceiveError request)
        (ReceiveResponse request)
        (encodeStationEventsRequest request)
