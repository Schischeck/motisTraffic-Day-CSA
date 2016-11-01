module Widgets.Typeahead exposing (Model, Msg, init, update, view)

import Html exposing (Html, div, ul, li, text)
import Html.Attributes exposing (..)
import Html.Events exposing (onInput, onMouseOver, onFocus, onClick, keyCode, on)
import Html.Lazy exposing (..)
import String
import Dict exposing (..)
import Json.Encode as Encode
import Json.Decode as Decode
import Widgets.Input as Input
import Util.View exposing (onStopAll)
import Util.Core exposing ((=>))
import Util.Api as Api


-- MODEL


type alias Model =
    { suggestions : List String
    , input : String
    , selected : Int
    , visible : Bool
    , inputWidget : Input.Model
    , remoteAddress : String
    }



-- UPDATE


type Msg
    = NoOp
    | ReceiveSuggestions (List String)
    | InputChange String
    | EnterSelection
    | ClickElement Int
    | SelectionUp
    | SelectionDown
    | Select Int
    | Hide
    | InputUpdate Input.Msg


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    ( updateModel msg model, command msg model )


updateModel : Msg -> Model -> Model
updateModel msg model =
    case msg of
        NoOp ->
            model

        ReceiveSuggestions suggestions ->
            { model | suggestions = suggestions }

        InputChange str ->
            { model | input = str }

        EnterSelection ->
            { model
                | visible = False
                , input = Maybe.withDefault "" (nthElement model.selected model.suggestions)
            }

        ClickElement i ->
            { model
                | visible = False
                , selected = 0
                , input = Maybe.withDefault "" (nthElement i model.suggestions)
            }

        SelectionUp ->
            { model | selected = (model.selected - 1) % List.length model.suggestions }

        SelectionDown ->
            { model | selected = (model.selected + 1) % List.length model.suggestions }

        Select index ->
            { model | selected = index }

        Hide ->
            { model | visible = False, selected = 0 }

        InputUpdate msg' ->
            let
                updated =
                    case msg' of
                        Input.Focus ->
                            { model | visible = True }

                        Input.Blur ->
                            { model | visible = False, selected = 0 }
            in
                { updated | inputWidget = Input.update msg' model.inputWidget }


command : Msg -> Model -> Cmd Msg
command msg model =
    case msg of
        InputChange str ->
            if String.length str > 2 then
                requestSuggestions model.remoteAddress str
            else
                Cmd.none

        _ ->
            Cmd.none



-- VIEW


up : Int
up =
    38


down : Int
down =
    40


enter : Int
enter =
    13


escape : Int
escape =
    27


onKey : Msg -> Dict Int Msg -> Html.Attribute Msg
onKey fail msgs =
    let
        tagger code =
            Dict.get code msgs |> Maybe.withDefault fail
    in
        on "keydown" (Decode.map tagger keyCode)


proposalView : Int -> Int -> String -> Html Msg
proposalView selected index str =
    li
        [ classList [ ( "selected", selected == index ) ]
        , onClick (ClickElement index)
        , onMouseOver (Select index)
        ]
        [ text str ]


typeaheadView : String -> Maybe String -> Model -> Html Msg
typeaheadView label icon model =
    div []
        [ Input.view InputUpdate
            [ value model.input
            , onInput InputChange
            , onKey NoOp
                (Dict.fromList
                    [ ( down, SelectionDown )
                    , ( up, SelectionUp )
                    , ( enter, EnterSelection )
                    , ( escape, Hide )
                    ]
                )
            ]
            label
            Nothing
            icon
            model.inputWidget
        , div
            [ classList
                [ ( "paper", True )
                , ( "hide", not model.visible || (List.length model.suggestions) == 0 )
                ]
            , onStopAll "mousedown" NoOp
            ]
            [ ul [ class "proposals" ]
                (List.indexedMap (proposalView model.selected) model.suggestions)
            ]
        ]


view : String -> Maybe String -> Model -> Html Msg
view label icon model =
    lazy3 typeaheadView label icon model



-- SUBSCRIPTIONS
{- no subs atm -}
-- INIT


init : String -> String -> Model
init remoteAddress initialValue =
    { suggestions = []
    , input = initialValue
    , selected = 0
    , visible = False
    , inputWidget = Input.init
    , remoteAddress = remoteAddress
    }



-- REMOTE SUGGESTIONS


generateRequest : String -> Encode.Value
generateRequest input =
    Encode.object
        [ "destination"
            => Encode.object
                [ "type" => Encode.string "Module"
                , "target" => Encode.string "/guesser"
                ]
        , "content_type" => Encode.string "StationGuesserRequest"
        , "content"
            => Encode.object
                [ "input" => Encode.string input
                , "guess_count" => Encode.int 6
                ]
        ]


requestSuggestions : String -> String -> Cmd Msg
requestSuggestions remoteAddress input =
    Api.sendRequest
        remoteAddress
        suggestionsDecoder
        (\_ -> NoOp)
        ReceiveSuggestions
        (generateRequest input)


suggestionDecoder : Decode.Decoder String
suggestionDecoder =
    Decode.at [ "name" ] Decode.string


suggestionsDecoder : Decode.Decoder (List String)
suggestionsDecoder =
    Decode.at [ "content", "guesses" ] (Decode.list suggestionDecoder)



-- UTIL


nthElement : Int -> List a -> Maybe a
nthElement index l =
    List.drop index l
        |> List.head
