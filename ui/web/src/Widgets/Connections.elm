module Widgets.Connections
    exposing
        ( Model
        , Config(..)
        , Msg(..)
        , SearchAction(..)
        , init
        , update
        , view
        , connectionIdxToListIdx
        , getJourney
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
import Data.Routing.Types exposing (RoutingResponse, SearchDirection(..), Interval)
import Data.Routing.Decode exposing (decodeRoutingResponse)
import Data.Intermodal.Types exposing (IntermodalRoutingRequest)
import Data.Intermodal.Request as IntermodalRoutingRequest
    exposing
        ( IntermodalLocation(..)
        , PretripSearchOptions
        , encodeRequest
        , getInterval
        , setInterval
        , setPretripSearchOptions
        , startToIntermodalLocation
        , destinationToIntermodalLocation
        )
import Data.Journey.Types as Journey exposing (Journey, Train)
import Data.ScheduleInfo.Types as ScheduleInfo exposing (ScheduleInfo)
import Widgets.Helpers.ConnectionUtil exposing (..)
import Widgets.Helpers.ApiErrorUtil exposing (errorText)
import Widgets.JourneyTransportGraph as JourneyTransportGraph
import Widgets.LoadingSpinner as LoadingSpinner
import Widgets.DateHeaders exposing (..)
import Util.Core exposing ((=>))
import Util.DateFormat exposing (..)
import Util.Date exposing (isSameDay, unixTime)
import Util.List exposing ((!!))
import Util.Api as Api exposing (ApiError)
import Localization.Base exposing (..)
import List.Extra exposing (updateAt)


-- MODEL


type alias Model =
    { loading : Bool
    , loadingBefore : Bool
    , loadingAfter : Bool
    , remoteAddress : String
    , journeys : List LabeledJourney
    , journeyTransportGraphs : List JourneyTransportGraph.Model
    , indexOffset : Int
    , errorMessage : Maybe ApiError
    , errorBefore : Maybe ApiError
    , errorAfter : Maybe ApiError
    , scheduleInfo : Maybe ScheduleInfo
    , routingRequest : Maybe IntermodalRoutingRequest
    , newJourneys : List Int
    , allowExtend : Bool
    , labels : List String
    }


type alias LabeledJourney =
    { journey : Journey
    , labels : List String
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
    , journeyTransportGraphs = []
    , indexOffset = 0
    , errorMessage = Nothing
    , errorBefore = Nothing
    , errorAfter = Nothing
    , scheduleInfo = Nothing
    , routingRequest = Nothing
    , newJourneys = []
    , allowExtend = True
    , labels = []
    }


connectionIdxToListIdx : Model -> Int -> Int
connectionIdxToListIdx model connectionIdx =
    connectionIdx - model.indexOffset


getJourney : Model -> Int -> Maybe Journey
getJourney model connectionIdx =
    model.journeys
        !! (connectionIdxToListIdx model connectionIdx)
        |> Maybe.map .journey



-- UPDATE


type Msg
    = NoOp
    | Search SearchAction IntermodalRoutingRequest
    | ExtendSearchInterval ExtendIntervalType
    | ReceiveResponse SearchAction IntermodalRoutingRequest RoutingResponse
    | ReceiveError SearchAction IntermodalRoutingRequest ApiError
    | UpdateScheduleInfo (Maybe ScheduleInfo)
    | ResetNew
    | JTGUpdate Int JourneyTransportGraph.Msg
    | SetRoutingResponses (List ( String, RoutingResponse ))
    | SetError ApiError


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
                            | routingRequest = Just updatedFullRequest
                            , loadingBefore = loadingBefore_
                            , loadingAfter = loadingAfter_
                        }
                            ! [ sendRequest model.remoteAddress action newRequest ]

                Nothing ->
                    model ! []

        ReceiveResponse action request response ->
            if belongsToCurrentSearch model request then
                let
                    model_ =
                        { model | allowExtend = True }
                in
                    (updateModelWithNewResults
                        model_
                        action
                        request
                        [ ( model.remoteAddress, response ) ]
                    )
                        ! []
            else
                model ! []

        ReceiveError action request msg_ ->
            if belongsToCurrentSearch model request then
                (handleRequestError model action msg_) ! []
            else
                model ! []

        SetRoutingResponses responses ->
            let
                placeholderStation =
                    IntermodalStation
                        { id = ""
                        , name = ""
                        , pos = { lat = 0, lng = 0 }
                        }

                request =
                    IntermodalRoutingRequest.initialRequest
                        0
                        placeholderStation
                        placeholderStation
                        []
                        []
                        (Date.fromTime 0)
                        Forward

                model_ =
                    { model | allowExtend = False }
            in
                (updateModelWithNewResults model_ ReplaceResults request responses) ! []

        UpdateScheduleInfo si ->
            { model | scheduleInfo = si } ! []

        ResetNew ->
            { model
                | newJourneys = []
                , journeyTransportGraphs =
                    List.map
                        JourneyTransportGraph.hideTooltips
                        model.journeyTransportGraphs
            }
                ! []

        JTGUpdate idx msg_ ->
            { model
                | journeyTransportGraphs =
                    updateAt (connectionIdxToListIdx model idx)
                        (JourneyTransportGraph.update msg_)
                        model.journeyTransportGraphs
                        |> Maybe.withDefault model.journeyTransportGraphs
            }
                ! []

        SetError err ->
            (handleRequestError model ReplaceResults err) ! []


extendSearchInterval :
    ExtendIntervalType
    -> IntermodalRoutingRequest
    -> ( IntermodalRoutingRequest, IntermodalRoutingRequest )
extendSearchInterval direction base =
    let
        extendBy =
            3600 * 2

        minConnectionCount =
            3

        previousInterval =
            getInterval base

        newIntervalBegin =
            case direction of
                ExtendBefore ->
                    previousInterval.begin - extendBy

                ExtendAfter ->
                    previousInterval.begin

        newIntervalEnd =
            case direction of
                ExtendBefore ->
                    previousInterval.end

                ExtendAfter ->
                    previousInterval.end + extendBy

        newRequest =
            case direction of
                ExtendBefore ->
                    setPretripSearchOptions base
                        { interval =
                            { begin = newIntervalBegin
                            , end = previousInterval.begin - 1
                            }
                        , minConnectionCount = minConnectionCount
                        , extendIntervalEarlier = True
                        , extendIntervalLater = False
                        }

                ExtendAfter ->
                    setPretripSearchOptions base
                        { interval =
                            { begin = previousInterval.end + 1
                            , end = newIntervalEnd
                            }
                        , minConnectionCount = minConnectionCount
                        , extendIntervalEarlier = False
                        , extendIntervalLater = True
                        }

        updatedFullRequest =
            setInterval base
                { begin = newIntervalBegin
                , end = newIntervalEnd
                }
    in
        ( newRequest, updatedFullRequest )


updateModelWithNewResults :
    Model
    -> SearchAction
    -> IntermodalRoutingRequest
    -> List ( String, RoutingResponse )
    -> Model
updateModelWithNewResults model action request responses =
    let
        firstResponse =
            List.head responses
                |> Maybe.map Tuple.second

        updateInterval updateStart updateEnd routingRequest =
            case firstResponse of
                Just response ->
                    let
                        previousInterval =
                            getInterval routingRequest
                    in
                        setInterval routingRequest
                            { begin =
                                if updateStart then
                                    unixTime response.intervalStart
                                else
                                    previousInterval.begin
                            , end =
                                if updateEnd then
                                    unixTime response.intervalEnd
                                else
                                    previousInterval.end
                            }

                Nothing ->
                    routingRequest

        base =
            case action of
                ReplaceResults ->
                    { model
                        | loading = False
                        , errorMessage = Nothing
                        , errorBefore = Nothing
                        , errorAfter = Nothing
                        , routingRequest =
                            Maybe.map (updateInterval True True) model.routingRequest
                    }

                PrependResults ->
                    { model
                        | loadingBefore = False
                        , errorBefore = Nothing
                        , routingRequest =
                            Maybe.map (updateInterval True False) model.routingRequest
                    }

                AppendResults ->
                    { model
                        | loadingAfter = False
                        , errorAfter = Nothing
                        , routingRequest =
                            Maybe.map (updateInterval False True) model.routingRequest
                    }

        journeysToAdd : List LabeledJourney
        journeysToAdd =
            toLabeledJourneys responses

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
                    List.range newIndexOffset (newIndexOffset + (List.length journeysToAdd) - 1)

                AppendResults ->
                    List.range (newIndexOffset + (List.length model.journeys)) (newIndexOffset + (List.length newJourneys) - 1)

        sortedJourneys =
            sortJourneys newJourneys

        journeyTransportGraphs =
            List.map
                (\lj -> JourneyTransportGraph.init transportListViewWidth lj.journey)
                sortedJourneys

        labelsToAdd =
            responses
                |> List.map Tuple.first
                |> List.Extra.unique

        newLabels =
            case action of
                ReplaceResults ->
                    labelsToAdd

                _ ->
                    List.Extra.unique (model.labels ++ labelsToAdd)
    in
        { base
            | journeys = sortedJourneys
            , journeyTransportGraphs = journeyTransportGraphs
            , indexOffset = newIndexOffset
            , newJourneys = newNewJourneys
            , labels = newLabels
        }


toLabeledJourneys : List ( String, RoutingResponse ) -> List LabeledJourney
toLabeledJourneys responses =
    let
        journeys : List ( String, Journey )
        journeys =
            List.concatMap
                (\( label, r ) ->
                    List.map
                        (\c -> ( label, Journey.toJourney c ))
                        r.connections
                )
                responses

        labelJourneys : ( String, Journey ) -> List LabeledJourney -> List LabeledJourney
        labelJourneys ( label, journey ) labeled =
            labeled
                |> List.Extra.findIndex (\lj -> lj.journey == journey)
                |> Maybe.andThen
                    (\idx ->
                        List.Extra.updateAt
                            idx
                            (\lj ->
                                { lj | labels = label :: lj.labels }
                            )
                            labeled
                    )
                |> Maybe.withDefault
                    ({ journey = journey, labels = [ label ] }
                        :: labeled
                    )
    in
        List.foldr labelJourneys [] journeys


sortJourneys : List LabeledJourney -> List LabeledJourney
sortJourneys journeys =
    List.sortBy
        (.journey
            >> .connection
            >> .stops
            >> List.head
            >> Maybe.andThen (.departure >> .schedule_time)
            >> Maybe.map Date.toTime
            >> Maybe.withDefault 0
        )
        journeys


handleRequestError :
    Model
    -> SearchAction
    -> ApiError
    -> Model
handleRequestError model action msg =
    let
        newModel =
            case action of
                ReplaceResults ->
                    { model
                        | loading = False
                        , errorMessage = Just msg
                        , journeys = []
                        , journeyTransportGraphs = []
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


belongsToCurrentSearch : Model -> IntermodalRoutingRequest -> Bool
belongsToCurrentSearch model check =
    case model.routingRequest of
        Just currentRequest ->
            let
                currentInterval =
                    getInterval currentRequest

                checkInterval =
                    getInterval check

                currentStart =
                    startToIntermodalLocation currentRequest.start

                checkStart =
                    startToIntermodalLocation check.start

                currentDest =
                    destinationToIntermodalLocation currentRequest.destination

                checkDest =
                    destinationToIntermodalLocation check.destination
            in
                (currentStart == checkStart)
                    && (currentDest == checkDest)
                    && (currentInterval.begin <= checkInterval.begin)
                    && (currentInterval.end >= checkInterval.end)

        Nothing ->
            False



-- VIEW


view : Config msg -> Localization -> Model -> Html msg
view config locale model =
    if model.loading then
        div [ class "loading" ] [ LoadingSpinner.view ]
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


connectionsView : Config msg -> Localization -> Model -> Html msg
connectionsView config locale model =
    div [ class "connections" ]
        [ extendIntervalButton ExtendBefore config locale model
        , connectionsWithDateHeaders config locale model
        , div [ class "divider footer" ] []
        , extendIntervalButton ExtendAfter config locale model
        ]


connectionsWithDateHeaders : Config msg -> Localization -> Model -> Html msg
connectionsWithDateHeaders config locale model =
    let
        getDate ( idx, labeledJourney, _ ) =
            Connection.departureTime labeledJourney.journey.connection
                |> Maybe.withDefault (Date.fromTime 0)

        renderConnection ( idx, journey, jtg ) =
            ( "connection-" ++ (toString idx)
            , connectionView config locale model.labels idx (List.member idx model.newJourneys) journey jtg
            )

        renderDateHeader date =
            ( "header-" ++ (toString (Date.toTime date)), dateHeader locale date )

        elements =
            List.map3 (\a b c -> ( a, b, c ))
                (List.range
                    model.indexOffset
                    (model.indexOffset + List.length model.journeys - 1)
                )
                model.journeys
                model.journeyTransportGraphs
    in
        Html.Keyed.node "div"
            [ class "connection-list" ]
            (withDateHeaders getDate renderConnection renderDateHeader elements)


connectionView :
    Config msg
    -> Localization
    -> List String
    -> Int
    -> Bool
    -> LabeledJourney
    -> JourneyTransportGraph.Model
    -> Html msg
connectionView (Config { internalMsg, selectMsg }) locale allLabels idx new labeledJourney jtg =
    let
        j =
            labeledJourney.journey

        renderedLabels =
            if List.length allLabels > 1 then
                labelsView allLabels labeledJourney.labels
            else
                text ""
    in
        div
            [ classList
                [ "connection" => True
                , "new" => new
                ]
            , onClick (selectMsg idx)
            ]
            [ renderedLabels
            , div [ class "pure-g" ]
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
                    [ Html.map (\m -> internalMsg (JTGUpdate idx m)) <| JourneyTransportGraph.view locale jtg ]
                ]
            ]


labelsView : List String -> List String -> Html msg
labelsView allLabels journeyLabels =
    let
        labelClass label =
            List.Extra.elemIndex label allLabels
                |> Maybe.withDefault 0
                |> toString

        labelView label =
            div
                [ class ("connection-label with-tooltip label-" ++ (labelClass label))
                , attribute "data-tooltip" label
                ]
                []
    in
        div [ class "labels" ]
            (List.map labelView journeyLabels)


transportListViewWidth : Int
transportListViewWidth =
    335


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


errorView : String -> Localization -> Model -> ApiError -> Html msg
errorView divClass locale model err =
    let
        errorMsg =
            errorText locale err
    in
        div [ class divClass ]
            [ div [] [ text errorMsg ]
            , scheduleRangeView locale model
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
            model.allowExtend
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
              else if model.allowExtend then
                LoadingSpinner.view
              else
                text ""
            ]



-- ROUTING REQUEST


sendRequest : String -> SearchAction -> IntermodalRoutingRequest -> Cmd Msg
sendRequest remoteAddress action request =
    Api.sendRequest
        remoteAddress
        decodeRoutingResponse
        (ReceiveError action request)
        (ReceiveResponse action request)
        (encodeRequest request)
