module Widgets.Connections exposing (Model, Config(..), Msg(..), init, update, view)

import Html exposing (Html, div, ul, li, text, span, i)
import Html.Attributes exposing (..)
import Html.Events exposing (onInput, onMouseOver, onFocus, onClick, keyCode, on)
import Html.Lazy exposing (lazy)
import Svg
import Svg.Attributes exposing (xlinkHref)
import Json.Decode as Decode exposing ((:=))
import Http
import Date exposing (Date)
import Data.Connection.Types as Connection exposing (Connection, Stop)
import Data.Connection.Decode
import Data.Journey.Types as Journey exposing (Journey, Train)
import Data.ScheduleInfo.Types as ScheduleInfo exposing (ScheduleInfo)
import Data.Routing.Request exposing (RoutingRequest, encodeRequest)
import Widgets.ConnectionUtil exposing (..)
import Util.DateFormat exposing (..)
import Util.Api as Api exposing (ApiError(..))


-- MODEL


type alias Model =
    { loading : Bool
    , remoteAddress : String
    , journeys : List Journey
    , errorMessage : Maybe String
    , scheduleInfo : Maybe ScheduleInfo
    }


type Config msg
    = Config
        { internalMsg : Msg -> msg
        , selectMsg : Int -> msg
        }



-- UPDATE


type Msg
    = NoOp
    | Search Request
    | ReceiveResponse (List Connection)
    | ReceiveError ApiError
    | UpdateScheduleInfo (Maybe ScheduleInfo)


type alias Request =
    { from : String
    , to : String
    , date : Date
    }


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    ( updateModel msg model, command msg model )


updateModel : Msg -> Model -> Model
updateModel msg model =
    case msg of
        NoOp ->
            model

        Search _ ->
            { model | loading = True }

        ReceiveResponse connections ->
            { model
                | loading = False
                , errorMessage = Nothing
                , journeys = List.map Journey.toJourney connections
            }

        ReceiveError msg' ->
            let
                errorMsg =
                    case msg' of
                        MotisError { errorCode, category, reason } ->
                            "Motis error "
                                ++ category
                                ++ " error code "
                                ++ (toString errorCode)
                                ++ ": "
                                ++ reason

                        TimeoutError ->
                            "Request timeout"

                        NetworkError ->
                            "Network error"

                        HttpError status ->
                            "HTTP error " ++ (toString status)

                        DecodeError msg ->
                            "Invalid response: " ++ msg
            in
                { model
                    | loading = False
                    , errorMessage = Just errorMsg
                    , journeys = []
                }

        UpdateScheduleInfo si ->
            { model | scheduleInfo = si }


command : Msg -> Model -> Cmd Msg
command msg model =
    case msg of
        Search req ->
            sendRequest model.remoteAddress req

        _ ->
            Cmd.none



-- VIEW


connectionsView : Config msg -> Model -> Html msg
connectionsView config model =
    div [ class "connections" ]
        ([ div [ class "pure-g header" ]
            [ div [ class "pure-u-5-24" ]
                [ text "Zeit" ]
            , div [ class "pure-u-4-24" ]
                [ text "Dauer" ]
            , div [ class "pure-u-15-24" ]
                [ text "Verkehrsmittel" ]
            ]
         ]
            ++ (List.indexedMap (connectionView config) model.journeys)
        )


trainsView : Journey -> Html msg
trainsView j =
    div [ class "train-list" ] <|
        List.intersperse (i [ class "icon train-sep" ] [ text "keyboard_arrow_right" ]) <|
            List.map trainView j.trains


trainView : Train -> Html msg
trainView train =
    let
        transport =
            List.head train.transports
    in
        case transport of
            Just t ->
                div [ class <| "train-box train-class-" ++ (toString t.class) ]
                    [ Svg.svg
                        [ Svg.Attributes.class "train-icon" ]
                        [ Svg.use [ xlinkHref <| "icons.svg#" ++ (trainIcon t.class) ] [] ]
                    , text <| transportName t
                    ]

            Nothing ->
                div [ class "train-box train-class-0" ] [ text "???" ]


connectionView : Config msg -> Int -> Journey -> Html msg
connectionView (Config { internalMsg, selectMsg }) idx j =
    div [ class "connection", onClick (selectMsg idx) ]
        [ div [ class "pure-g" ]
            [ div [ class "pure-u-5-24 connection-times" ]
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
            , div [ class "pure-u-15-24 connection-trains" ]
                [ trainsView j ]
            ]
        ]


scheduleRangeView : Model -> Html msg
scheduleRangeView { scheduleInfo } =
    case scheduleInfo of
        Just si ->
            div [] [ text <| "Auskunft von " ++ (toString si.begin) ++ " bis " ++ (toString si.end) ++ " möglich" ]

        Nothing ->
            text ""


view : Config msg -> Model -> Html msg
view config model =
    if model.loading then
        div [ class "loader" ] [ text "Loading..." ]
    else if List.isEmpty model.journeys then
        case model.errorMessage of
            Just err ->
                div [ class "error" ] [ text err ]

            Nothing ->
                div [ class "no-results" ]
                    [ div [] [ text "Keine Verbindungen gefunden." ]
                    , scheduleRangeView model
                    ]
    else
        lazy (connectionsView config) model



-- SUBSCRIPTIONS
{- no subs atm -}
-- INIT


init : String -> Model
init remoteAddress =
    { loading = False
    , remoteAddress = remoteAddress
    , journeys = []
    , errorMessage = Nothing
    , scheduleInfo = Nothing
    }



-- ROUTING REQUEST / RESPONSE


sendRequest : String -> Request -> Cmd Msg
sendRequest remoteAddress request =
    Api.sendRequest
        remoteAddress
        Data.Connection.Decode.decodeRoutingResponse
        ReceiveError
        ReceiveResponse
        (encodeRequest request)
