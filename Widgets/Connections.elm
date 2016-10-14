module Widgets.Connections exposing (Model, Msg(..), init, update, view)

import Html exposing (Html, div, ul, li, text)
import Html.Attributes exposing (..)
import Html.Events exposing (onInput, onMouseOver, onFocus, onClick, keyCode, on)
import Html.Lazy exposing (lazy)
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


-- MODEL


type alias Model =
    { loading : Bool
    , remoteAddress : String
    , connections : List Connection
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
                , connections =
                    case connectionsFromJson json of
                        Just c ->
                            c

                        Nothing ->
                            []
            }


command : Msg -> Model -> Cmd Msg
command msg model =
    case msg of
        Search req ->
            sendRequest model.remoteAddress req

        _ ->
            Cmd.none



-- VIEW


twoDigits : Int -> String
twoDigits =
    toString >> String.pad 2 '0'


formatTime : Date -> String
formatTime d =
    (Date.hour d |> twoDigits) ++ ":" ++ (Date.minute d |> twoDigits)


last : List a -> Maybe a
last =
    List.foldl (Just >> always) Nothing


departureTime : Connection -> Maybe Date
departureTime connection =
    (List.head connection.stops
        |> Maybe.map .departure
    )
        `Maybe.andThen` .schedule_time


arrivalTime : Connection -> Maybe Date
arrivalTime connection =
    (last connection.stops
        |> Maybe.map .arrival
    )
        `Maybe.andThen` .schedule_time


duration : Connection -> Maybe DeltaRecord
duration connection =
    Maybe.map2 Duration.diff (arrivalTime connection) (departureTime connection)


durationText : DeltaRecord -> String
durationText delta =
    (twoDigits delta.hour) ++ ":" ++ (twoDigits delta.minute)


interchanges : Connection -> Int
interchanges connection =
    List.length <| List.filter .interchange connection.stops


transportCategories : Connection -> Set String
transportCategories connection =
    let
        category : Move -> Maybe String
        category move =
            case move of
                Transport t ->
                    Just t.category_name

                Walk _ ->
                    Nothing
    in
        List.filterMap category connection.transports |> Set.fromList


connectionsView : Model -> Html Msg
connectionsView model =
    div [ class "connections" ]
        ([ div [ class "pure-g header" ]
            [ div [ class "pure-u-8-24" ]
                [ text "Station" ]
            , div [ class "pure-u-5-24" ]
                [ text "Time" ]
            , div [ class "pure-u-4-24" ]
                [ text "Duration" ]
            , div [ class "pure-u-3-24" ]
                [ text "Interch." ]
            , div [ class "pure-u-4-24" ]
                [ text "Categories" ]
            ]
         ]
            ++ (List.map connectionView model.connections)
        )


allStopsView : Connection -> Html Msg
allStopsView c =
    let
        stopView : Stop -> Html Msg
        stopView stop =
            li [] [ text stop.station.name ]
    in
        div [ class "all-stops" ]
            [ ul [] (List.map stopView c.stops) ]


connectionView : Connection -> Html Msg
connectionView c =
    div [ class "connection" ]
        [ div [ class "pure-g" ]
            [ div [ class "pure-u-8-24" ]
                [ div [] [ text (Maybe.map (.station >> .name) (List.head c.stops) |> Maybe.withDefault "?") ]
                , div [] [ text (Maybe.map (.station >> .name) (last c.stops) |> Maybe.withDefault "?") ]
                ]
            , div [ class "pure-u-5-24" ]
                [ div [] [ text (Maybe.map formatTime (departureTime c) |> Maybe.withDefault "?") ]
                , div [] [ text (Maybe.map formatTime (arrivalTime c) |> Maybe.withDefault "?") ]
                ]
            , div [ class "pure-u-4-24" ]
                [ div [] [ text (Maybe.map durationText (duration c) |> Maybe.withDefault "?") ] ]
            , div [ class "pure-u-3-24" ]
                [ div [] [ text <| toString <| interchanges c ] ]
            , div [ class "pure-u-4-24" ]
                [ div [] [ text <| String.join ", " (transportCategories c |> Set.toList) ] ]
            ]
        , allStopsView c
        ]


view : Model -> Html Msg
view model =
    if model.loading then
        text "Loading..."
    else if List.isEmpty model.connections then
        text "No connections found."
    else
        lazy connectionsView model



-- SUBSCRIPTIONS
{- no subs atm -}
-- INIT


init : String -> Model
init remoteAddress =
    { loading = False
    , remoteAddress = remoteAddress
    , connections = []
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


connectionsFromJson : String -> Maybe (List Connection)
connectionsFromJson json =
    let
        response =
            Decode.decodeString decodeRoutingResponse json

        _ =
            case response of
                Ok _ ->
                    Debug.log "connectionsFromJson" "OK"

                Err e ->
                    Debug.log "connectionsFromJson" e
    in
        response |> Result.toMaybe


(=>) : a -> b -> ( a, b )
(=>) =
    (,)
