module Widgets.Connections exposing (Model, Config(..), Msg(..), init, update, view)

import Html exposing (Html, div, ul, li, text, span, i)
import Html.Attributes exposing (..)
import Html.Events exposing (onInput, onMouseOver, onFocus, onClick, keyCode, on)
import Html.Lazy exposing (lazy)
import Svg
import Svg.Attributes exposing (xlinkHref)
import String
import Set exposing (Set)
import Json.Encode as Encode
import Json.Decode as Decode exposing ((:=))
import Widgets.ViewUtil exposing (onStopAll)
import Http as Http
import Task as Task
import Date exposing (Date)
import Date.Extra.Duration as Duration exposing (DeltaRecord)
import Debug
import Widgets.Data.Connection exposing (..)
import Widgets.ConnectionUtil exposing (..)
import Widgets.ConnectionDetails as ConnectionDetails


-- MODEL


type alias Model =
    { loading : Bool
    , remoteAddress : String
    , journeys : List Journey
    }


type Config msg
    = Config
        { internalMsg : Msg -> msg
        , selectMsg : Journey -> msg
        }



-- UPDATE


type Msg
    = NoOp
    | Search Request
    | ReceiveResponse String


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

        ReceiveResponse json ->
            { model
                | loading = False
                , journeys = Maybe.withDefault [] (journeysFromJson json)
            }


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
                [ text "Time" ]
            , div [ class "pure-u-4-24" ]
                [ text "Duration" ]
            , div [ class "pure-u-15-24" ]
                [ text "Trains" ]
            ]
         ]
            ++ (List.map (connectionView config) model.journeys)
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


connectionView : Config msg -> Journey -> Html msg
connectionView (Config { internalMsg, selectMsg }) j =
    div [ class "connection", onClick (selectMsg j) ]
        [ div [ class "pure-g" ]
            [ div [ class "pure-u-5-24 connection-times" ]
                [ div [ class "connection-departure" ]
                    [ text (Maybe.map formatTime (departureTime j.connection) |> Maybe.withDefault "?")
                    , text " "
                    , Maybe.map delay (departureEvent j.connection) |> Maybe.withDefault (text "")
                    ]
                , div [ class "connection-arrival" ]
                    [ text (Maybe.map formatTime (arrivalTime j.connection) |> Maybe.withDefault "?")
                    , text " "
                    , Maybe.map delay (arrivalEvent j.connection) |> Maybe.withDefault (text "")
                    ]
                ]
            , div [ class "pure-u-4-24 connection-duration" ]
                [ div [] [ text (Maybe.map durationText (duration j.connection) |> Maybe.withDefault "?") ] ]
            , div [ class "pure-u-15-24 connection-trains" ]
                [ trainsView j ]
            ]
        ]


view : Config msg -> Model -> Html msg
view config model =
    if model.loading then
        text "Loading..."
    else if List.isEmpty model.journeys then
        text "No connections found."
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
    }



-- ROUTING REQUEST / RESPONSE


unixTime : Date -> Int
unixTime d =
    (floor (Date.toTime d)) // 1000


generateRequest : Request -> Encode.Value
generateRequest request =
    let
        selectedTime =
            unixTime request.date

        startTime =
            selectedTime - 3600

        endTime =
            selectedTime + 3600

        _ =
            Debug.log "generateRequest"
                ("selectedTime="
                    ++ (toString selectedTime)
                    ++ ", startTime="
                    ++ (toString startTime)
                    ++ ", endTime="
                    ++ (toString endTime)
                )
    in
        Encode.object
            [ "destination"
                => Encode.object
                    [ "type" => Encode.string "Module"
                    , "target" => Encode.string "/routing"
                    ]
            , "content_type" => Encode.string "RoutingRequest"
            , "content"
                => Encode.object
                    [ "start_type" => Encode.string "PretripStart"
                    , "start"
                        => Encode.object
                            [ "station"
                                => Encode.object
                                    [ "name" => Encode.string request.from
                                    , "id" => Encode.string ""
                                    ]
                            , "interval"
                                => Encode.object
                                    [ "begin" => Encode.int startTime
                                    , "end" => Encode.int endTime
                                    ]
                            ]
                    , "destination"
                        => Encode.object
                            [ "name" => Encode.string request.to
                            , "id" => Encode.string ""
                            ]
                    , "search_type" => Encode.string "Default"
                    , "search_dir" => Encode.string "Forward"
                    , "via" => Encode.list []
                    , "additional_edges" => Encode.list []
                    ]
            ]


sendRequest : String -> Request -> Cmd Msg
sendRequest remoteAddress request =
    Http.send Http.defaultSettings
        { verb = "POST"
        , headers = [ "Content-Type" => "application/json" ]
        , url = remoteAddress
        , body = generateRequest request |> Encode.encode 0 |> Http.string
        }
        |> Task.perform (\_ -> NoOp) responseReader


responseReader : Http.Response -> Msg
responseReader res =
    case res.value of
        Http.Text t ->
            ReceiveResponse t

        _ ->
            NoOp


journeysFromJson : String -> Maybe (List Journey)
journeysFromJson json =
    let
        response =
            Decode.decodeString decodeRoutingResponse json

        toJourney : Connection -> Journey
        toJourney connection =
            { connection = connection
            , trains = groupTrains connection
            }

        _ =
            case response of
                Err e ->
                    Debug.log "journeysFromJson" e

                _ ->
                    ""
    in
        case response of
            Ok connections ->
                Just <| List.map toJourney connections

            Err _ ->
                Nothing
