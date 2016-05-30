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


view :
    (Msg -> msg)
    -> List (Html.Attribute msg)
    -> List (Html msg)
    -> Model
    -> Html msg
view makeMsg attributes inputWidget model =
    div []
        [ div [ class "label" ] [ text "Date" ]
        , div
            [ classList
                [ ( "gb-input-group", True )
                , ( "gb-input-group-selected", model )
                ]
            ]
            [ div [ class "gb-input-icon" ]
                [ i [ class "icon" ] [ text "\xE878" ] ]
            , input
                (class "gb-input"
                    :: onBlur (makeMsg Blur)
                    :: onFocus (makeMsg Focus)
                    :: attributes
                )
                []
            , div [ class "gb-input-widget" ] inputWidget
            ]
        ]
