module Widgets.Connections
    exposing
        ( Model
        , Config(..)
        , Msg(..)
        , SearchAction(..)
        , init
        , update
        , view
        )

import Html exposing (Html, div, ul, li, text, span, i, a)
import Html.Attributes exposing (..)
import Html.Events exposing (onInput, onMouseOver, onFocus, onClick, keyCode, on)
import Html.Lazy exposing (..)
import Html.Keyed
import Date exposing (Date)
import Date.Extra.Duration as Duration exposing (Duration(..))
import Maybe.Extra exposing (isJust)
import Data.Connection.Types as Connection exposing (Connection, Stop)
import Data.Routing.Types exposing (RoutingRequest, RoutingResponse)
import Data.Routing.Decode exposing (decodeRoutingResponse)
import Data.Routing.Request exposing (encodeRequest)
import Data.Journey.Types as Journey exposing (Journey, Train)
import Data.ScheduleInfo.Types as ScheduleInfo exposing (ScheduleInfo)
import Widgets.Helpers.ConnectionUtil exposing (..)
import Widgets.JourneyTransportGraph as JourneyTransportGraph
import Util.Core exposing ((=>))
import Util.DateFormat exposing (..)
import Util.Date exposing (isSameDay, unixTime)
import Util.Api as Api
    exposing
        ( ApiError(..)
        , MotisErrorInfo(..)
        , ModuleErrorInfo(..)
        , RoutingErrorInfo(..)
        , MotisErrorDetail
        )
import Localization.Base exposing (..)


-- MODEL


type alias Model =
    { loading : Bool
    , loadingBefore : Bool
    , loadingAfter : Bool
    , remoteAddress : String
    , journeys : List Journey
    , indexOffset : Int
    , errorMessage : Maybe ApiError
    , errorBefore : Maybe ApiError
    , errorAfter : Maybe ApiError
    , scheduleInfo : Maybe ScheduleInfo
    , routingRequest : Maybe RoutingRequest
    , newJourneys : List Int
    }


type Config msg
    = Config
        { internalMsg : Msg -> msg
        , selectMsg : Int -> msg
        }


init : String -> Model
init remoteAddress =
    { loading = False
    , loadingBefore = False
    , loadingAfter = False
    , remoteAddress = remoteAddress
    , journeys = []
    , indexOffset = 0
    , errorMessage = Nothing
    , errorBefore = Nothing
    , errorAfter = Nothing
    , scheduleInfo = Nothing
    , routingRequest = Nothing
    , newJourneys = []
    }



-- UPDATE


type Msg
    = NoOp
    | Search SearchAction RoutingRequest
    | ExtendSearchInterval ExtendIntervalType
    | ReceiveResponse SearchAction RoutingRequest RoutingResponse
    | ReceiveError SearchAction RoutingRequest ApiError
    | UpdateScheduleInfo (Maybe ScheduleInfo)
    | ResetNew


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

        Search action req ->
            { model
                | loading = True
                , loadingBefore = False
                , loadingAfter = False
                , routingRequest = Just req
            }
                ! [ sendRequest model.remoteAddress ReplaceResults req ]

        ExtendSearchInterval direction ->
            case model.routingRequest of
                Just baseRequest ->
                    let
                        ( newRequest, updatedFullRequest ) =
                            extendSearchInterval direction baseRequest

                        action =
                            case direction of
                                ExtendBefore ->
                                    PrependResults

                                ExtendAfter ->
                                    AppendResults

                        loadingBefore' =
                            case direction of
                                ExtendBefore ->
                                    True

                                ExtendAfter ->
                                    model.loadingBefore

                        loadingAfter' =
                            case direction of
                                ExtendBefore ->
                                    model.loadingAfter

                                ExtendAfter ->
                                    True
                    in
                        { model
                            | routingRequest = Just updatedFullRequest
                            , loadingBefore = loadingBefore'
                            , loadingAfter = loadingAfter'
                        }
                            ! [ sendRequest model.remoteAddress action newRequest ]

                Nothing ->
                    model ! []

        ReceiveResponse action request response ->
            if belongsToCurrentSearch model request then
                (updateModelWithNewResults model action request response) ! []
            else
                model ! []

        ReceiveError action request msg' ->
            if belongsToCurrentSearch model request then
                (handleRequestError model action request msg') ! []
            else
                model ! []

        UpdateScheduleInfo si ->
            { model | scheduleInfo = si } ! []

        ResetNew ->
            { model | newJourneys = [] } ! []


extendSearchInterval :
    ExtendIntervalType
    -> RoutingRequest
    -> ( RoutingRequest, RoutingRequest )
extendSearchInterval direction base =
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
                        , intervalEnd = base.intervalStart
                        , minConnectionCount = 0
                    }

                ExtendAfter ->
                    { base
                        | intervalStart = base.intervalEnd
                        , intervalEnd = newIntervalEnd
                        , minConnectionCount = 0
                    }
    in
        ( newRequest
        , { base
            | intervalStart = newIntervalStart
            , intervalEnd = newIntervalEnd
          }
        )


updateModelWithNewResults :
    Model
    -> SearchAction
    -> RoutingRequest
    -> RoutingResponse
    -> Model
updateModelWithNewResults model action request response =
    let
        connections =
            response.connections

        updateInterval routingRequest =
            { routingRequest
                | intervalStart = unixTime response.intervalStart
                , intervalEnd = unixTime response.intervalEnd
            }

        base =
            case action of
                ReplaceResults ->
                    { model
                        | loading = False
                        , errorMessage = Nothing
                        , errorBefore = Nothing
                        , errorAfter = Nothing
                        , routingRequest =
                            Maybe.map updateInterval model.routingRequest
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

        journeysToAdd : List Journey
        journeysToAdd =
            connections
                |> List.map Journey.toJourney

        newJourneys =
            case action of
                ReplaceResults ->
                    journeysToAdd

                PrependResults ->
                    journeysToAdd ++ model.journeys

                AppendResults ->
                    model.journeys ++ journeysToAdd

        newIndexOffset =
            case action of
                ReplaceResults ->
                    0

                PrependResults ->
                    model.indexOffset - (List.length journeysToAdd)

                AppendResults ->
                    model.indexOffset

        newNewJourneys =
            case action of
                ReplaceResults ->
                    []

                PrependResults ->
                    [newIndexOffset..(newIndexOffset + (List.length journeysToAdd) - 1)]

                AppendResults ->
                    [newIndexOffset + (List.length model.journeys)..newIndexOffset + (List.length newJourneys) - 1]
    in
        { base
            | journeys = sortJourneys newJourneys
            , indexOffset = newIndexOffset
            , newJourneys = newNewJourneys
        }


sortJourneys : List Journey -> List Journey
sortJourneys journeys =
    List.sortBy
        (.connection
            >> .stops
            >> List.head
            >> (\m -> Maybe.andThen m (.departure >> .schedule_time))
            >> Maybe.map Date.toTime
            >> Maybe.withDefault 0
        )
        journeys


handleRequestError :
    Model
    -> SearchAction
    -> RoutingRequest
    -> ApiError
    -> Model
handleRequestError model action request msg =
    let
        newModel =
            case action of
                ReplaceResults ->
                    { model
                        | loading = False
                        , errorMessage = Just msg
                        , journeys = []
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


belongsToCurrentSearch : Model -> RoutingRequest -> Bool
belongsToCurrentSearch model check =
    case model.routingRequest of
        Just currentRequest ->
            (currentRequest.from == check.from)
                && (currentRequest.to == check.to)
                && (currentRequest.intervalStart <= check.intervalStart)
                && (currentRequest.intervalEnd >= check.intervalEnd)

        Nothing ->
            False



-- VIEW


connectionsView : Config msg -> Localization -> Model -> Html msg
connectionsView config locale model =
    div [ class "connections" ]
        [ extendIntervalButton ExtendBefore config locale model
        , connectionsWithDateHeaders config locale model
        , extendIntervalButton ExtendAfter config locale model
        ]


connectionsWithDateHeaders : Config msg -> Localization -> Model -> Html msg
connectionsWithDateHeaders config locale model =
    let
        getDate ( idx, journey ) =
            Connection.departureTime journey.connection |> Maybe.withDefault (Date.fromTime 0)

        renderConnection ( idx, journey ) =
            ( "connection-" ++ (toString idx)
            , connectionView config locale idx (List.member idx model.newJourneys) journey
            )

        renderDateHeader date =
            ( "header-" ++ (toString (Date.toTime date)), dateHeader locale date )

        elements =
            List.map2 (,)
                [model.indexOffset..(model.indexOffset + List.length model.journeys - 1)]
                model.journeys
    in
        Html.Keyed.node "div"
            [ class "connection-list" ]
            (withDateHeaders getDate renderConnection renderDateHeader elements)


connectionView :
    Config msg
    -> Localization
    -> Int
    -> Bool
    -> Journey
    -> Html msg
connectionView (Config { internalMsg, selectMsg }) locale idx new j =
    div
        [ classList
            [ "connection" => True
            , "new" => new
            ]
        , onClick (selectMsg idx)
        ]
        [ div [ class "pure-g" ]
            [ div [ class "pure-u-4-24 connection-times" ]
                [ div [ class "connection-departure" ]
                    [ text (Maybe.map formatTime (Connection.departureTime j.connection) |> Maybe.withDefault "?")
                    , text " "
                    , Maybe.map delay (Connection.departureEvent j.connection) |> Maybe.withDefault (text "")
                    ]
                , div [ class "connection-arrival" ]
                    [ text (Maybe.map formatTime (Connection.arrivalTime j.connection) |> Maybe.withDefault "?")
                    , text " "
                    , Maybe.map delay (Connection.arrivalEvent j.connection) |> Maybe.withDefault (text "")
                    ]
                ]
            , div [ class "pure-u-4-24 connection-duration" ]
                [ div [] [ text (Maybe.map durationText (Connection.duration j.connection) |> Maybe.withDefault "?") ] ]
            , div [ class "pure-u-16-24 connection-trains" ]
                [ JourneyTransportGraph.view transportListViewWidth j ]
            ]
        ]


dateHeader : Localization -> Date -> Html msg
dateHeader { dateConfig } date =
    div [ class "date-header" ] [ span [] [ text <| formatDate dateConfig date ] ]


withDateHeaders :
    (a -> Date)
    -> (a -> ( String, Html msg ))
    -> (Date -> ( String, Html msg ))
    -> List a
    -> List ( String, Html msg )
withDateHeaders getDate renderElement renderDateHeader elements =
    let
        f element ( lastDate, result ) =
            let
                currentDate =
                    getDate element

                base =
                    if not (isSameDay currentDate lastDate) then
                        result ++ [ renderDateHeader currentDate ]
                    else
                        result
            in
                ( currentDate, base ++ [ renderElement element ] )

        ( _, result ) =
            List.foldl f ( Date.fromTime 0, [] ) elements
    in
        result


transportListViewWidth : Int
transportListViewWidth =
    390


scheduleRangeView : Localization -> Model -> Html msg
scheduleRangeView { t } { scheduleInfo } =
    case scheduleInfo of
        Just si ->
            let
                begin =
                    si.begin

                end =
                    Duration.add Hour -12 si.end
            in
                div [ class "schedule-range" ]
                    [ text <| t.connections.scheduleRange begin end ]

        Nothing ->
            text ""


view : Config msg -> Localization -> Model -> Html msg
view config locale model =
    if model.loading then
        div [ class "loading" ] [ loadingSpinner ]
    else if List.isEmpty model.journeys then
        case model.errorMessage of
            Just err ->
                errorView "main-error" locale model err

            Nothing ->
                div [ class "no-results" ]
                    [ if isJust model.routingRequest then
                        div [] [ text locale.t.connections.noResults ]
                      else
                        text ""
                    , scheduleRangeView locale model
                    ]
    else
        lazy3 connectionsView config locale model


loadingSpinner : Html msg
loadingSpinner =
    div [ class "spinner" ]
        [ div [ class "bounce1" ] []
        , div [ class "bounce2" ] []
        , div [ class "bounce3" ] []
        ]


errorView : String -> Localization -> Model -> ApiError -> Html msg
errorView divClass locale model err =
    let
        errorMsg =
            case err of
                MotisError err' ->
                    motisErrorMsg locale err'

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
            , scheduleRangeView locale model
            ]


motisErrorMsg : Localization -> MotisErrorInfo -> String
motisErrorMsg { t } err =
    case err of
        RoutingError JourneyDateNotInSchedule ->
            t.connections.errors.journeyDateNotInSchedule

        _ ->
            t.connections.errors.internalError (toString err)


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
                internalMsg <| ExtendSearchInterval direction
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
                loadingSpinner
            ]



-- ROUTING REQUEST


sendRequest : String -> SearchAction -> RoutingRequest -> Cmd Msg
sendRequest remoteAddress action request =
    Api.sendRequest
        remoteAddress
        decodeRoutingResponse
        (ReceiveError action request)
        (ReceiveResponse action request)
        (encodeRequest request)
