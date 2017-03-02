module Widgets.Typeahead
    exposing
        ( Model
        , Msg(..)
        , Suggestion(..)
        , init
        , update
        , view
        , getSelectedStation
        , getSelectedAddress
        , getSelectedSuggestion
        )

import Html exposing (Html, div, ul, li, text, i, span)
import Html.Attributes exposing (..)
import Html.Events exposing (onInput, onMouseOver, onFocus, onClick, keyCode, on)
import Html.Lazy exposing (..)
import String
import Dict exposing (..)
import Task
import Json.Decode as Decode
import List.Extra
import Widgets.Input as Input
import Util.View exposing (onStopAll)
import Util.List exposing ((!!))
import Util.Api as Api exposing (ApiError(..))
import Debounce
import Data.Connection.Types exposing (Station)
import Data.StationGuesser.Request as StationGuesser
import Data.StationGuesser.Decode exposing (decodeStationGuesserResponse)
import Data.Address.Types exposing (..)
import Data.Address.Request exposing (encodeAddressRequest)
import Data.Address.Decode exposing (decodeAddressResponse)


-- MODEL


type alias Model =
    { stationSuggestions : List Station
    , addressSuggestions : List Address
    , suggestions : List Suggestion
    , input : String
    , selected : Int
    , visible : Bool
    , inputWidget : Input.Model
    , remoteAddress : String
    , debounce : Debounce.State
    }


type Suggestion
    = StationSuggestion Station
    | AddressSuggestion Address



-- UPDATE


type Msg
    = NoOp
    | StationSuggestionsResponse (List Station)
    | StationSuggestionsError ApiError
    | AddressSuggestionsResponse AddressResponse
    | AddressSuggestionsError ApiError
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
    | ItemSelected


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        NoOp ->
            model ! []

        StationSuggestionsResponse suggestions ->
            updateSuggestions { model | stationSuggestions = suggestions } ! []

        StationSuggestionsError _ ->
            updateSuggestions { model | stationSuggestions = [] } ! []

        AddressSuggestionsResponse response ->
            updateSuggestions
                { model
                    | addressSuggestions = (filterAddressSuggestions response.guesses)
                }
                ! []

        AddressSuggestionsError err ->
            let
                _ =
                    Debug.log "AddressSuggestionsError" err
            in
                updateSuggestions { model | addressSuggestions = [] } ! []

        InputChange str ->
            { model
                | input = str
                , visible = True
            }
                ! [ Debounce.debounceCmd debounceCfg RequestSuggestions ]

        EnterSelection ->
            { model
                | visible = False
                , input = getEntryName model model.selected
            }
                ! [ Debounce.debounceCmd debounceCfg RequestSuggestions
                  , Task.perform identity (Task.succeed ItemSelected)
                  ]

        ClickElement i ->
            { model
                | visible = False
                , selected = 0
                , input = getEntryName model i
            }
                ! [ Debounce.debounceCmd debounceCfg RequestSuggestions
                  , Task.perform identity (Task.succeed ItemSelected)
                  ]

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

                        Input.Click ->
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

        ItemSelected ->
            model ! []


updateSuggestions : Model -> Model
updateSuggestions model =
    let
        stations =
            List.map StationSuggestion model.stationSuggestions

        addresses =
            List.map AddressSuggestion model.addressSuggestions
    in
        { model | suggestions = stations ++ addresses }


getEntryName : Model -> Int -> String
getEntryName { suggestions } idx =
    suggestions
        !! idx
        |> Maybe.map getSuggestionName
        |> Maybe.withDefault ""


getSuggestionName : Suggestion -> String
getSuggestionName suggestion =
    case suggestion of
        StationSuggestion station ->
            station.name

        AddressSuggestion address ->
            getAddressStr address


getAddressStr : Address -> String
getAddressStr address =
    address.name ++ " (" ++ (String.join ", " (List.map .name address.regions)) ++ ")"


getSelectedSuggestion : Model -> Maybe Suggestion
getSelectedSuggestion model =
    let
        input =
            model.input |> String.trim |> String.toLower
    in
        model.suggestions
            |> List.filter (\entry -> String.toLower (getSuggestionName entry) == input)
            |> List.head


getSelectedStation : Model -> Maybe Station
getSelectedStation model =
    case (getSelectedSuggestion model) of
        Just (StationSuggestion station) ->
            Just station

        _ ->
            Nothing


getSelectedAddress : Model -> Maybe Address
getSelectedAddress model =
    case (getSelectedSuggestion model) of
        Just (AddressSuggestion address) ->
            Just address

        _ ->
            Nothing


filterAddressSuggestions : List Address -> List Address
filterAddressSuggestions suggestions =
    suggestions
        |> List.Extra.uniqueBy getAddressStr


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


proposalView : Int -> Int -> Suggestion -> Html Msg
proposalView selected index suggestion =
    let
        name =
            getSuggestionName suggestion

        icon =
            case suggestion of
                StationSuggestion _ ->
                    "place"

                AddressSuggestion _ ->
                    "location_city"
    in
        li
            [ classList [ ( "selected", selected == index ) ]
            , onClick (ClickElement index)
            , onMouseOver (Select index)
            ]
            [ i [ class "icon" ] [ text icon ]
            , span [ title name ] [ text name ]
            ]


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
                (List.indexedMap (proposalView model.selected) model.suggestions)
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
    , stationSuggestions = []
    , addressSuggestions = []
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
    Cmd.batch
        [ requestStationSuggestions remoteAddress input
        , requestAddressSuggestions remoteAddress input
        ]


requestStationSuggestions : String -> String -> Cmd Msg
requestStationSuggestions remoteAddress input =
    Api.sendRequest
        remoteAddress
        decodeStationGuesserResponse
        StationSuggestionsError
        StationSuggestionsResponse
        (StationGuesser.encodeRequest 6 input)


requestAddressSuggestions : String -> String -> Cmd Msg
requestAddressSuggestions remoteAddress input =
    Api.sendRequest
        remoteAddress
        decodeAddressResponse
        AddressSuggestionsError
        AddressSuggestionsResponse
        (encodeAddressRequest input)
