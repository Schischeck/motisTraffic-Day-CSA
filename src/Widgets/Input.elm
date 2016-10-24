module Widgets.Input exposing (..)

import Html exposing (Html, Attribute, div, input, i, text)
import Html.Attributes exposing (..)
import Html.Events exposing (onFocus, onBlur)


-- MODEL


type alias Model =
    Bool


init : Model
init =
    False



-- UPDATE


type Msg
    = Focus
    | Blur


update : Msg -> Model -> Model
update msg model =
    case msg of
        Focus ->
            True

        Blur ->
            False



-- VIEW


addIcon : Maybe String -> List (Html msg) -> List (Html msg)
addIcon icon list =
    case icon of
        Just str ->
            (div [ class "gb-input-icon" ]
                [ i [ class "icon" ] [ text str ] ]
            )
                :: list

        Nothing ->
            list


addInputWidget : Maybe (List (Html msg)) -> List (Html msg) -> List (Html msg)
addInputWidget inputWidget list =
    case inputWidget of
        Just widget ->
            List.append list [ div [ class "gb-input-widget" ] widget ]

        Nothing ->
            list


view :
    (Msg -> msg)
    -> List (Html.Attribute msg)
    -> Maybe (List (Html msg))
    -> Maybe String
    -> Model
    -> Html msg
view makeMsg attributes inputWidget icon model =
    div []
        [ div [ class "label" ] [ text "Date" ]
        , div
            [ classList
                [ ( "gb-input-group", True )
                , ( "gb-input-group-selected", model )
                ]
            ]
            ([ input
                (class "gb-input"
                    :: onBlur (makeMsg Blur)
                    :: onFocus (makeMsg Focus)
                    :: attributes
                )
                []
             ]
                |> addInputWidget inputWidget
                |> addIcon icon
            )
        ]
