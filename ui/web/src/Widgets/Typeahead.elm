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
        , getSuggestionName
        , getShortSuggestionName
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
import Maybe.Extra
import Widgets.Input as Input
import Util.View exposing (onStopAll)
import Util.List exposing ((!!), last)
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
    , hoverIndex : Int
    , selectedSuggestion : Maybe Suggestion
    , visible : Bool
    , inputWidget : Input.Model
    , remoteAddress : String
    , debounce : Debounce.State
    }


type Suggestion
    = StationSuggestion Station
    | AddressSuggestion Address


init : String -> String -> ( Model, Cmd Msg )
init remoteAddress initialValue =
    { suggestions = []
    , stationSuggestions = []
    , addressSuggestions = []
    , input = initialValue
    , hoverIndex = 0
    , selectedSuggestion = Nothing
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
                , selectedSuggestion = getSuggestionByName model str
            }
                ! [ Debounce.debounceCmd debounceCfg RequestSuggestions ]

        EnterSelection ->
            { model
                | visible = False
                , input = getEntryName model model.hoverIndex
                , selectedSuggestion = model.suggestions !! model.hoverIndex
            }
                ! [ Debounce.debounceCmd debounceCfg RequestSuggestions
                  , Task.perform identity (Task.succeed ItemSelected)
                  ]

        ClickElement i ->
            { model
                | visible = False
                , hoverIndex = 0
                , input = getEntryName model i
                , selectedSuggestion = model.suggestions !! i
            }
                ! [ Debounce.debounceCmd debounceCfg RequestSuggestions
                  , Task.perform identity (Task.succeed ItemSelected)
                  ]

        SelectionUp ->
            { model | hoverIndex = (model.hoverIndex - 1) % List.length model.suggestions } ! []

        SelectionDown ->
            { model | hoverIndex = (model.hoverIndex + 1) % List.length model.suggestions } ! []

        Select index ->
            { model | hoverIndex = index } ! []

        Hide ->
            { model | visible = False, hoverIndex = 0 } ! []

        InputUpdate msg_ ->
            let
                updated =
                    case msg_ of
                        Input.Focus ->
                            { model | visible = True }

                        Input.Click ->
                            { model | visible = True }

                        Input.Blur ->
                            { model | visible = False, hoverIndex = 0 }
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

        model1 =
            { model | suggestions = stations ++ addresses }

        model2 =
            case model1.selectedSuggestion of
                Nothing ->
                    { model1
                        | selectedSuggestion = getSuggestionByName model1 model1.input
                    }

                Just _ ->
                    model1
    in
        model2


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


getShortSuggestionName : Suggestion -> String
getShortSuggestionName suggestion =
    case suggestion of
        StationSuggestion station ->
            station.name

        AddressSuggestion address ->
            getShortAddressStr address


getAddressStr : Address -> String
getAddressStr address =
    let
        region =
            getRegionStr address
    in
        if String.isEmpty region then
            address.name
        else
            address.name ++ ", " ++ region


getShortAddressStr : Address -> String
getShortAddressStr address =
    case getCity address of
        Just city ->
            address.name ++ ", " ++ city

        Nothing ->
            address.name


getRegionStr : Address -> String
getRegionStr address =
    let
        city =
            getCity address

        country =
            getCountry address
    in
        [ city, country ]
            |> Maybe.Extra.values
            |> String.join ", "


getCity : Address -> Maybe String
getCity address =
    address.regions
        |> List.filter (\a -> a.adminLevel <= 8)
        |> List.head
        |> Maybe.map .name


getCountry : Address -> Maybe String
getCountry address =
    address.regions
        |> List.filter (\a -> a.adminLevel == 2)
        |> List.head
        |> Maybe.map .name


getSelectedSuggestion : Model -> Maybe Suggestion
getSelectedSuggestion model =
    model.selectedSuggestion


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


getSuggestionByName : Model -> String -> Maybe Suggestion
getSuggestionByName model rawInput =
    let
        input =
            rawInput |> String.trim |> String.toLower

        checkEntry entry =
            (String.toLower (getSuggestionName entry)) == input
    in
        model.suggestions
            |> List.filter checkEntry
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


proposalView : Int -> Int -> Suggestion -> Html Msg
proposalView hoverIndex index suggestion =
    let
        fullName =
            getSuggestionName suggestion

        content =
            case suggestion of
                StationSuggestion station ->
                    stationView station

                AddressSuggestion address ->
                    addressView address
    in
        li
            [ classList [ ( "selected", hoverIndex == index ) ]
            , onClick (ClickElement index)
            , onMouseOver (Select index)
            , title fullName
            ]
            content


stationView : Station -> List (Html Msg)
stationView station =
    let
        name =
            station.name
    in
        [ i [ class "icon" ] [ text "train" ]
        , span [ class "station" ] [ text name ]
        ]


addressView : Address -> List (Html Msg)
addressView address =
    let
        name =
            address.name

        region =
            getRegionStr address
    in
        [ i [ class "icon" ] [ text "place" ]
        , span [ class "address-name" ] [ text name ]
        , span [ class "address-region" ] [ text region ]
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
                (List.indexedMap (proposalView model.hoverIndex) model.suggestions)
            ]
        ]


view : Int -> String -> Maybe String -> Model -> Html Msg
view tabIndex label icon model =
    lazy2 typeaheadView ( tabIndex, label, icon ) model



-- SUBSCRIPTIONS
{- no subs atm -}
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
