module Widgets.Connections exposing (Model, Msg(..), init, update, view)

import Html exposing (Html, div, ul, li, text, span)
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


-- MODEL


type alias Model =
    { loading : Bool
    , remoteAddress : String
    , connections : List Connection
    }


type alias Train =
    { stops : List Stop
    , transports : List TransportInfo
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
    Basics.max 0 ((List.length <| List.filter .enter connection.stops) - 1)


transportsForRange : Connection -> Int -> Int -> List TransportInfo
transportsForRange connection from to =
    let
        checkMove : Move -> Maybe TransportInfo
        checkMove move =
            case move of
                Transport transport ->
                    if transport.range.from < to && transport.range.to > from then
                        Just transport
                    else
                        Nothing

                Walk _ ->
                    Nothing
    in
        List.filterMap checkMove connection.transports


groupTrains : Connection -> List Train
groupTrains connection =
    let
        indexedStops : List ( Int, Stop )
        indexedStops =
            List.indexedMap (,) connection.stops

        add_stop : List Train -> Stop -> List Train
        add_stop trains stop =
            case List.head trains of
                Just train ->
                    { train | stops = stop :: train.stops }
                        :: (List.tail trains |> Maybe.withDefault [])

                Nothing ->
                    -- should not happen
                    Debug.log "groupTrains: add_stop empty list" []

        finish_train : List Train -> Int -> Int -> List Train
        finish_train trains start_idx end_idx =
            case List.head trains of
                Just train ->
                    { train | transports = transportsForRange connection start_idx end_idx }
                        :: (List.tail trains |> Maybe.withDefault [])

                Nothing ->
                    -- should not happen
                    Debug.log "groupTrains: finish_train empty list" []

        group : ( Int, Stop ) -> ( List Train, Bool, Int ) -> ( List Train, Bool, Int )
        group ( idx, stop ) ( trains, in_train, end_idx ) =
            let
                ( trains', in_train', end_idx' ) =
                    if stop.enter then
                        ( finish_train (add_stop trains stop) idx end_idx, False, -1 )
                    else
                        ( trains, in_train, end_idx )
            in
                if stop.leave then
                    ( add_stop ((Train [] []) :: trains') stop, True, idx )
                else if in_train then
                    ( add_stop trains' stop, in_train', end_idx' )
                else
                    ( trains', in_train', end_idx' )

        ( trains, _, _ ) =
            List.foldr group ( [], False, -1 ) indexedStops
    in
        trains


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


useLineId : Int -> Bool
useLineId class =
    class == 5 || class == 6


transportName : TransportInfo -> String
transportName transport =
    if useLineId transport.class then
        transport.line_id
    else
        transport.name


connectionsView : Model -> Html Msg
connectionsView model =
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
            ++ (List.map connectionView model.connections)
        )


trainsView : Connection -> Html Msg
trainsView c =
    let
        trains =
            groupTrains c
    in
        div [ class "train-list" ] <| List.map trainView trains


trainIcon : Int -> String
trainIcon class =
    case class of
        0 ->
            "train"

        1 ->
            "train"

        2 ->
            "train"

        3 ->
            "train"

        4 ->
            "train"

        5 ->
            "sbahn"

        6 ->
            "ubahn"

        7 ->
            "tram"

        _ ->
            "bus"


trainView : Train -> Html Msg
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


connectionView : Connection -> Html Msg
connectionView c =
    div [ class "connection" ]
        [ div [ class "pure-g" ]
            [ div [ class "pure-u-5-24" ]
                [ div [] [ text (Maybe.map formatTime (departureTime c) |> Maybe.withDefault "?") ]
                , div [] [ text (Maybe.map formatTime (arrivalTime c) |> Maybe.withDefault "?") ]
                ]
            , div [ class "pure-u-4-24" ]
                [ div [] [ text (Maybe.map durationText (duration c) |> Maybe.withDefault "?") ] ]
            , div [ class "pure-u-15-24" ]
                [ trainsView c ]
            ]
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
