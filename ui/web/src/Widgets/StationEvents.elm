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
import Widgets.DateHeaders exposing (..)
import Date.Extra.Duration as Duration exposing (DeltaRecord)
import Maybe.Extra exposing (isJust)


-- MODEL


type alias Model =
    { loading : Bool
    , loadingBefore : Bool
    , loadingAfter : Bool
    , remoteAddress : String
    , stationId : String
    , stationName : String
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


init : String -> String -> String -> Date -> ( Model, Cmd Msg )
init remoteAddress stationId stationName date =
    let
        request =
            initialStationEventsRequest stationId date
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
        }
            ! [ sendRequest remoteAddress ReplaceResults request ]



-- UPDATE


type Msg
    = NoOp
    | ReceiveResponse SearchAction LookupStationEventsRequest LookupStationEventsResponse
    | ReceiveError SearchAction LookupStationEventsRequest ApiError
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
                ( newRequest, updatedFullRequest ) =
                    extendInterval direction model.request

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
                    | request = updatedFullRequest
                    , loadingBefore = loadingBefore_
                    , loadingAfter = loadingAfter_
                }
                    ! [ sendRequest model.remoteAddress action newRequest ]


handleResponse : Model -> SearchAction -> LookupStationEventsResponse -> Model
handleResponse model action response =
    let
        events =
            List.sortWith compareEvents response.events

        depEvents =
            List.filter (\e -> e.eventType == DEP) events

        arrEvents =
            List.filter (\e -> e.eventType == ARR) events

        base =
            case action of
                ReplaceResults ->
                    { model
                        | loading = False
                        , errorMessage = Nothing
                        , errorBefore = Nothing
                        , errorAfter = Nothing
                    }

                PrependResults ->
                    { model
                        | loadingBefore = False
                        , errorBefore = Nothing
                    }

                AppendResults ->
                    { model
                        | loadingAfter = False
                        , errorAfter = Nothing
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
        { model
            | loading = False
            , loadingBefore = False
            , loadingAfter = False
            , depEvents = newDepEvents
            , arrEvents = newArrEvents
        }


compareEvents : StationEvent -> StationEvent -> Basics.Order
compareEvents a b =
    let
        getDate =
            .scheduleTime >> Date.toTime

        byDate =
            compare (getDate a) (getDate b)
    in
        if byDate == EQ then
            compare a.serviceName b.serviceName
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
    -> LookupStationEventsRequest
    -> ( LookupStationEventsRequest, LookupStationEventsRequest )
extendInterval direction base =
    let
        extendBy =
            3600 * 2

        newIntervalStart =
            case direction of
                ExtendBefore ->
                    base.intervalStart - extendBy

                ExtendAfter ->
                    base.intervalStart

        newIntervalEnd =
            case direction of
                ExtendBefore ->
                    base.intervalEnd

                ExtendAfter ->
                    base.intervalEnd + extendBy

        newRequest =
            case direction of
                ExtendBefore ->
                    { base
                        | intervalStart = newIntervalStart
                        , intervalEnd = base.intervalStart - 1
                    }

                ExtendAfter ->
                    { base
                        | intervalStart = base.intervalEnd + 1
                        , intervalEnd = newIntervalEnd
                    }
    in
        ( newRequest
        , { base
            | intervalStart = newIntervalStart
            , intervalEnd = newIntervalEnd
          }
        )



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
            else if List.isEmpty (getEvents model) then
                case model.errorMessage of
                    Just err ->
                        errorView "main-error" locale model err

                    Nothing ->
                        div [ class "no-results" ]
                            [ div [] [ text locale.t.station.noDepartures ] ]
            else
                stationEventsView config locale model
    in
        div [ class "events" ] [ content ]


stationEventsView : Config msg -> Localization -> Model -> Html msg
stationEventsView config locale model =
    div []
        [ extendIntervalButton ExtendBefore config locale model
        , div [ class "event-list" ]
            (withDateHeaders
                .scheduleTime
                (eventView config locale)
                (dateHeader locale)
                (getEvents model)
            )
        , div [ class "divider footer" ] []
        , extendIntervalButton ExtendAfter config locale model
        ]


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


extendIntervalButton :
    ExtendIntervalType
    -> Config msg
    -> Localization
    -> Model
    -> Html msg
extendIntervalButton direction (Config { internalMsg }) locale model =
    let
        enabled =
            case direction of
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
              else
                LoadingSpinner.view
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


sendRequest : String -> SearchAction -> LookupStationEventsRequest -> Cmd Msg
sendRequest remoteAddress action request =
    Api.sendRequest
        remoteAddress
        decodeLookupStationEventsResponse
        (ReceiveError action request)
        (ReceiveResponse action request)
        (encodeStationEventsRequest request)
