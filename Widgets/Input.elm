module Widgets.Input exposing (view)

import Html exposing (Html, Attribute, div, input, i, text)
import Html.Attributes exposing (..)


-- VIEW

view : List (Html.Attribute msg) -> Html msg
view attr =
    div []
        [ div [ class "label" ] [ text "Date" ]
        , div [ class "gb-input-group" ]
            [ div [ class "gb-input-icon" ]
                [ i [ class "icon" ] [ text "\xE878" ] ]
            , input (class "gb-input" :: attr) []
            , div [ class "gb-input-widget" ] []
            ]
        ]
