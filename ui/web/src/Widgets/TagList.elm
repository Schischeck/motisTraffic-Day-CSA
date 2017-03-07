module Widgets.TagList
    exposing
        ( Model
        , Tag(..)
        , Msg
        , init
        , subscriptions
        , update
        , view
        , getSelectedTags
        )

import Html exposing (..)
import Html.Attributes exposing (..)
import Html.Events exposing (onClick)
import Html.Lazy exposing (..)
import Mouse
import Util.View exposing (onStopAll, onStopPropagation)


-- MODEL


type alias Model =
    { tags : List Tag
    , selected : List Tag
    , visible : Bool
    , ignoreNextToggle : Bool
    }


type Tag
    = WalkTag { maxDuration : Int }
    | BikeTag { maxDuration : Int }
    | CarTag { maxDuration : Int }


init : Model
init =
    { tags =
        [ WalkTag { maxDuration = 900 }
        , BikeTag { maxDuration = 900 }
        ]
    , selected = []
    , visible = False
    , ignoreNextToggle = False
    }


getSelectedTags : Model -> List Tag
getSelectedTags model =
    model.selected



-- UPDATE


type Msg
    = AddTag Tag
    | RemoveTag Tag
    | ToggleVisibility
    | Click
    | NoOp


update : Msg -> Model -> Model
update msg model =
    case msg of
        NoOp ->
            model

        AddTag t ->
            { model
                | selected = model.selected ++ [ t ]
                , visible = False
                , ignoreNextToggle = True
            }

        RemoveTag t ->
            { model | selected = List.filter (\s -> s /= t) model.selected }

        ToggleVisibility ->
            if model.ignoreNextToggle then
                { model | ignoreNextToggle = False }
            else
                { model | visible = not model.visible }

        Click ->
            { model | visible = False }



-- VIEW


tagListView : String -> Model -> Html Msg
tagListView label model =
    let
        availableTags =
            List.filter (\t -> not (List.member t model.selected)) model.tags
                |> List.map
                    (\t ->
                        a [ class "tag", onClick (AddTag t) ]
                            [ i [ class "icon" ] [ text (tagIcon t) ] ]
                    )
                |> div
                    [ classList
                        [ ( "add", True )
                        , ( "paper", True )
                        , ( "hide", not model.visible )
                        ]
                    , onStopAll "mousedown" NoOp
                    ]

        addButton =
            if (List.length model.selected == List.length model.tags) then
                []
            else
                [ div [ class "tag outline", onClick ToggleVisibility ]
                    ([ i [ class "icon" ] [ text "add" ] ]
                        ++ [ availableTags ]
                    )
                ]

        selectedTags =
            model.selected
                |> List.map
                    (\t ->
                        a [ class "tag" ]
                            [ i [ class "icon" ] [ text (tagIcon t) ]
                            , i [ class "remove icon", onClick (RemoveTag t) ] [ text "cancel" ]
                            ]
                    )
    in
        div [ class "clear" ]
            ([ div [ class "label" ] [ text label ] ]
                ++ selectedTags
                ++ addButton
            )


tagIcon : Tag -> String
tagIcon tag =
    case tag of
        WalkTag _ ->
            "directions_walk"

        BikeTag _ ->
            "directions_bike"

        CarTag _ ->
            "directions_car"


view : String -> Model -> Html Msg
view label model =
    lazy2 tagListView label model



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    if model.visible then
        Mouse.downs (\_ -> Click)
    else
        Sub.none
