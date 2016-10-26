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
import Html.Keyed as Keyed
import String
import Date exposing (Date)
import Data.Connection.Types as Connection exposing (Connection, Stop)
import Data.Connection.Decode
import Data.Journey.Types as Journey exposing (Journey, Train)
import Data.ScheduleInfo.Types as ScheduleInfo exposing (ScheduleInfo)
import Data.Routing.Request exposing (RoutingRequest, encodeRequest)
import Widgets.Helpers.ConnectionUtil exposing (..)
import Util.Core exposing ((=>))
import Util.DateFormat exposing (..)
import Util.Api as Api
    exposing
        ( ApiError(..)
        , MotisErrorInfo(..)
        , ModuleErrorInfo(..)
        , RoutingErrorInfo(..)
        , MotisErrorDetail
        )


-- MODEL


type alias Model =
    { loading : Bool
    , loadingBefore : Bool
    , loadingAfter : Bool
    , remoteAddress : String
    , journeys : List Journey
    , indexOffset : Int
    , errorMessage : Maybe String
    , scheduleInfo : Maybe ScheduleInfo
    , routingRequest : Maybe RoutingRequest
    }


type Config msg
    = Config
        { internalMsg : Msg -> msg
        , selectMsg : Int -> msg
        }



-- UPDATE


type Msg
    = NoOp
    | Search SearchAction RoutingRequest
    | ExtendSearchInterval ExtendIntervalType
    | ReceiveResponse SearchAction RoutingRequest (List Connection)
    | ReceiveError SearchAction RoutingRequest ApiError
    | UpdateScheduleInfo (Maybe ScheduleInfo)


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

        ReceiveResponse action request connections ->
            if belongsToCurrentSearch model request then
                (updateModelWithNewResults model action request connections) ! []
            else
                model ! []

        ReceiveError action request msg' ->
            if belongsToCurrentSearch model request then
                (handleRequestError model action request msg') ! []
            else
                model ! []

        UpdateScheduleInfo si ->
            { model | scheduleInfo = si } ! []


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
                    }

                ExtendAfter ->
                    { base
                        | intervalStart = base.intervalEnd
                        , intervalEnd = newIntervalEnd
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
    -> List Connection
    -> Model
updateModelWithNewResults model action request connections =
    let
        base =
            case action of
                ReplaceResults ->
                    { model
                        | loading = False
                        , errorMessage = Nothing
                    }

                PrependResults ->
                    { model
                        | loadingBefore = False
                    }

                AppendResults ->
                    { model
                        | loadingAfter = False
                    }

        journeysToAdd : List Journey
        journeysToAdd =
            List.map Journey.toJourney connections

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
    in
        { base
            | journeys = sortJourneys newJourneys
            , indexOffset = newIndexOffset
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
        errorMsg =
            case msg of
                MotisError err ->
                    motisErrorMsg err

                TimeoutError ->
                    "Request timeout"

                NetworkError ->
                    "Network error"

                HttpError status ->
                    "HTTP error " ++ (toString status)

                DecodeError msg ->
                    "Invalid response: " ++ msg

        newModel =
            case action of
                ReplaceResults ->
                    { model
                        | loading = False
                        , errorMessage = Just errorMsg
                        , journeys = []
                    }

                PrependResults ->
                    { model
                        | loadingBefore = False
                    }

                AppendResults ->
                    { model
                        | loadingAfter = False
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


connectionsView : Config msg -> Model -> Html msg
connectionsView config model =
    div [ class "connections" ]
        [ extendIntervalButton ExtendBefore config model
        , div [ class "pure-g header" ]
            [ div [ class "pure-u-6-24" ]
                [ text "Zeit" ]
            , div [ class "pure-u-4-24" ]
                [ text "Dauer" ]
            , div [ class "pure-u-14-24" ]
                [ text "Verkehrsmittel" ]
            ]
        , Keyed.node "div"
            [ class "connection-list" ]
            (List.map2
                (keyedConnectionView config)
                [model.indexOffset..(model.indexOffset + List.length model.journeys - 1)]
                model.journeys
            )
        , extendIntervalButton ExtendAfter config model
        ]


trainsView : TransportViewMode -> Journey -> Html msg
trainsView viewMode j =
    let
        trainBoxes =
            List.map (trainView viewMode) j.trains

        prefix =
            case j.leadingWalk of
                Just _ ->
                    [ walkBox ]

                Nothing ->
                    []

        suffix =
            case j.trailingWalk of
                Just _ ->
                    [ walkBox ]

                Nothing ->
                    []

        transportBoxes =
            prefix ++ trainBoxes ++ suffix

        content =
            case viewMode of
                IconOnlyNoSep ->
                    transportBoxes

                _ ->
                    List.intersperse (i [ class "icon train-sep" ] [ text "keyboard_arrow_right" ]) <|
                        transportBoxes
    in
        div [ class "train-list" ] content


trainView : TransportViewMode -> Train -> Html msg
trainView viewMode train =
    let
        transport =
            List.head train.transports
    in
        case transport of
            Just t ->
                trainBox viewMode t

            Nothing ->
                div [ class "train-box train-class-0" ] [ text "???" ]


keyedConnectionView : Config msg -> Int -> Journey -> ( String, Html msg )
keyedConnectionView config idx j =
    ( toString idx, connectionView config idx j )


connectionView : Config msg -> Int -> Journey -> Html msg
connectionView (Config { internalMsg, selectMsg }) idx j =
    div [ class "connection", onClick (selectMsg idx) ]
        [ div [ class "pure-g" ]
            [ div [ class "pure-u-6-24 connection-times" ]
                [ div [ class "connection-departure" ]
                    [ text (Maybe.map (formatShortDateTime deDateConfig) (Connection.departureTime j.connection) |> Maybe.withDefault "?")
                    , text " "
                    , Maybe.map delay (Connection.departureEvent j.connection) |> Maybe.withDefault (text "")
                    ]
                , div [ class "connection-arrival" ]
                    [ text (Maybe.map (formatShortDateTime deDateConfig) (Connection.arrivalTime j.connection) |> Maybe.withDefault "?")
                    , text " "
                    , Maybe.map delay (Connection.arrivalEvent j.connection) |> Maybe.withDefault (text "")
                    ]
                ]
            , div [ class "pure-u-4-24 connection-duration" ]
                [ div [] [ text (Maybe.map durationText (Connection.duration j.connection) |> Maybe.withDefault "?") ] ]
            , div [ class "pure-u-14-24 connection-trains" ]
                [ trainsView (pickTransportViewMode transportListViewWidth j) j ]
            ]
        ]


transportListViewWidth : Int
transportListViewWidth =
    360


scheduleRangeView : Model -> Html msg
scheduleRangeView { scheduleInfo } =
    case scheduleInfo of
        Just si ->
            div [ class "schedule-range" ]
                [ text <|
                    "Auskunft von "
                        ++ (formatDateTime deDateConfig si.begin)
                        ++ " bis "
                        ++ (formatDateTime deDateConfig si.end)
                        ++ " möglich"
                ]

        Nothing ->
            text ""


view : Config msg -> Model -> Html msg
view config model =
    if model.loading then
        div [ class "loader" ] [ text "Verbindungen suchen..." ]
    else if List.isEmpty model.journeys then
        case model.errorMessage of
            Just err ->
                div [ class "error" ]
                    [ div [] [ text err ]
                    , scheduleRangeView model
                    ]

            Nothing ->
                div [ class "no-results" ]
                    [ div [] [ text "Keine Verbindungen gefunden" ]
                    , scheduleRangeView model
                    ]
    else
        lazy2 connectionsView config model


motisErrorMsg : MotisErrorInfo -> String
motisErrorMsg err =
    case err of
        RoutingError JourneyDateNotInSchedule ->
            "Zeitraum nicht im Fahrplan"

        _ ->
            "Interner Fehler (" ++ (toString err) ++ ")"


extendIntervalButton : ExtendIntervalType -> Config msg -> Model -> Html msg
extendIntervalButton direction (Config { internalMsg }) model =
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
                    "Frühere Verbindungen suchen"

                ExtendAfter ->
                    "Spätere Verbindungen suchen"

        clickHandler =
            if enabled then
                internalMsg <| ExtendSearchInterval direction
            else
                internalMsg NoOp
    in
        div
            [ classList
                [ "extend-search-interval" => True
                , divClass => True
                , "disabled" => not enabled
                ]
            ]
            [ a
                [ class "gb-button gb-button-small gb-button-PRIMARY_COLOR disable-select"
                , onClick clickHandler
                ]
                [ text title ]
            ]


pickTransportViewMode : Int -> Journey -> TransportViewMode
pickTransportViewMode maxTotalWidth { trains, leadingWalk, trailingWalk } =
    let
        countMaybe mb =
            case mb of
                Just _ ->
                    1

                Nothing ->
                    0

        walkCount =
            (countMaybe leadingWalk) + (countMaybe trailingWalk)

        separators =
            16 * ((List.length trains) - 1 + walkCount)

        iconOnlyBase =
            32

        iconTextBase =
            40

        avgCharWidth =
            9

        transportName f train =
            case List.head train.transports of
                Just t ->
                    f t

                Nothing ->
                    "???"

        boxWidth viewMode t =
            case viewMode of
                LongName ->
                    iconTextBase
                        + avgCharWidth
                        * String.length (transportName longTransportName t)

                ShortName ->
                    iconTextBase
                        + avgCharWidth
                        * String.length (transportName shortTransportName t)

                IconOnly ->
                    iconOnlyBase

                IconOnlyNoSep ->
                    iconOnlyBase

        longNameWidth =
            separators
                + (List.sum <| List.map (boxWidth LongName) trains)
                + (walkCount * iconOnlyBase)

        shortNameWidth =
            separators
                + (List.sum <| List.map (boxWidth ShortName) trains)
                + (walkCount * iconOnlyBase)

        iconOnlyWidth =
            separators
                + (List.sum <| List.map (boxWidth IconOnly) trains)
                + (walkCount * iconOnlyBase)
    in
        if longNameWidth <= maxTotalWidth then
            LongName
        else if shortNameWidth <= maxTotalWidth then
            ShortName
        else if iconOnlyWidth <= maxTotalWidth then
            IconOnly
        else
            IconOnlyNoSep



-- SUBSCRIPTIONS
{- no subs atm -}
-- INIT


init : String -> Model
init remoteAddress =
    { loading = False
    , loadingBefore = False
    , loadingAfter = False
    , remoteAddress = remoteAddress
    , journeys = []
    , indexOffset = 0
    , errorMessage = Nothing
    , scheduleInfo = Nothing
    , routingRequest = Nothing
    }



-- ROUTING REQUEST / RESPONSE


sendRequest : String -> SearchAction -> RoutingRequest -> Cmd Msg
sendRequest remoteAddress action request =
    Api.sendRequest
        remoteAddress
        Data.Connection.Decode.decodeRoutingResponse
        (ReceiveError action request)
        (ReceiveResponse action request)
        (encodeRequest request)
