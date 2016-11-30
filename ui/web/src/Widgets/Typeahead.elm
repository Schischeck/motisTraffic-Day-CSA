module Widgets.Typeahead
    exposing
        ( Model
        , Msg
        , init
        , update
        , view
        , getSelectedStation
        )

import Html exposing (Html, div, ul, li, text)
import Html.Attributes exposing (..)
import Html.Events exposing (onInput, onMouseOver, onFocus, onClick, keyCode, on)
import Html.Lazy exposing (..)
import String
import Dict exposing (..)
import Json.Decode as Decode
import Widgets.Input as Input
import Util.View exposing (onStopAll)
import Util.List exposing ((!!))
import Util.Api as Api
import Debounce
import Data.Connection.Types exposing (Station)
import Data.StationGuesser.Request exposing (encodeRequest)
import Data.StationGuesser.Decode exposing (decodeStationGuesserResponse)


-- MODEL


type alias Model =
    { suggestions : List Station
    , input : String
    , selected : Int
    , visible : Bool
    , inputWidget : Input.Model
    , remoteAddress : String
    , debounce : Debounce.State
    }



-- UPDATE


type Msg
    = NoOp
    | ReceiveSuggestions (List Station)
    | InputChange String
    | EnterSelection
    | ClickElement Int
    | SelectionUp
    | SelectionDown
    | Select Int
    | Hide
    | InputUpdate Input.Msg
    | Deb (Debounce.Msg Msg)
    | RequestSuggestions


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        NoOp ->
            model ! []

        ReceiveSuggestions suggestions ->
            { model | suggestions = suggestions } ! []

        InputChange str ->
            { model | input = str } ! [ Debounce.debounceCmd debounceCfg RequestSuggestions ]

        EnterSelection ->
            { model
                | visible = False
                , input = getStationName model model.selected
            }
                ! [ Debounce.debounceCmd debounceCfg RequestSuggestions ]

        ClickElement i ->
            { model
                | visible = False
                , selected = 0
                , input = getStationName model i
            }
                ! [ Debounce.debounceCmd debounceCfg RequestSuggestions ]

        SelectionUp ->
            { model | selected = (model.selected - 1) % List.length model.suggestions } ! []

        SelectionDown ->
            { model | selected = (model.selected + 1) % List.length model.suggestions } ! []

        Select index ->
            { model | selected = index } ! []

        Hide ->
            { model | visible = False, selected = 0 } ! []

        InputUpdate msg_ ->
            let
                updated =
                    case msg_ of
                        Input.Focus ->
                            { model | visible = True }

                        Input.Blur ->
                            { model | visible = False, selected = 0 }
            in
                { updated | inputWidget = Input.update msg_ model.inputWidget } ! []

        Deb a ->
            Debounce.update debounceCfg a model

        RequestSuggestions ->
            model
                ! [ if String.length model.input > 2 then
                        requestSuggestions model.remoteAddress model.input
                    else
                        Cmd.none
                  ]


getStationName : Model -> Int -> String
getStationName { suggestions } idx =
    suggestions
        !! idx
        |> Maybe.map .name
        |> Maybe.withDefault ""


getSelectedStation : Model -> Maybe Station
getSelectedStation model =
    let
        input =
            model.input |> String.trim |> String.toLower
    in
        model.suggestions
            |> List.filter (\station -> String.toLower station.name == input)
            |> List.head


debounceCfg : Debounce.Config Model Msg
debounceCfg =
    Debounce.config
        .debounce
        (\model s -> { model | debounce = s })
        Deb
        100



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


typeaheadView : ( Int, String, Maybe String ) -> Model -> Html Msg
typeaheadView ( tabIndex, label, icon ) model =
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
            , tabindex tabIndex
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
                (List.indexedMap (proposalView model.selected) (List.map .name model.suggestions))
            ]
        ]


view : Int -> String -> Maybe String -> Model -> Html Msg
view tabIndex label icon model =
    lazy2 typeaheadView ( tabIndex, label, icon ) model



-- SUBSCRIPTIONS
{- no subs atm -}
-- INIT


init : String -> String -> ( Model, Cmd Msg )
init remoteAddress initialValue =
    { suggestions = []
    , input = initialValue
    , selected = 0
    , visible = False
    , inputWidget = Input.init
    , remoteAddress = remoteAddress
    , debounce = Debounce.init
    }
        ! (if String.isEmpty initialValue then
            []
           else
            [ requestSuggestions remoteAddress initialValue ]
          )



-- REMOTE SUGGESTIONS


requestSuggestions : String -> String -> Cmd Msg
requestSuggestions remoteAddress input =
    Api.sendRequest
        remoteAddress
        decodeStationGuesserResponse
        (\_ -> NoOp)
        ReceiveSuggestions
        (encodeRequest 6 input)
