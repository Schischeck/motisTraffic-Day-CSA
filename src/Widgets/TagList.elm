module Widgets.TagList exposing (Model, Msg, init, subscriptions, update, view)

import Html exposing (..)
import Html.Attributes exposing (..)
import Html.Events exposing (onClick)
import Html.Lazy exposing (lazy)
import Set exposing (..)
import Mouse
import Util.View exposing (onStopAll, onStopPropagation)


-- MODEL


type alias Model =
    { tags : Set String
    , selected : Set String
    , visible : Bool
    , ignoreNextToggle : Bool
    }



-- UPDATE


type Msg
    = AddTag String
    | RemoveTag String
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
                | selected = Set.insert t model.selected
                , visible = False
                , ignoreNextToggle = True
            }

        RemoveTag t ->
            { model | selected = Set.remove t model.selected }

        ToggleVisibility ->
            if model.ignoreNextToggle then
                { model | ignoreNextToggle = False }
            else
                { model | visible = not model.visible }

        Click ->
            { model | visible = False }



-- VIEW


tagListView : Model -> Html Msg
tagListView model =
    let
        availableTags =
            Set.toList (Set.diff model.tags model.selected)
                |> List.map
                    (\s ->
                        a [ class "tag", onClick (AddTag s) ]
                            [ i [ class "icon" ] [ text s ] ]
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
            if (Set.size model.selected == Set.size model.tags) then
                []
            else
                [ div [ class "tag outline", onClick ToggleVisibility ]
                    ([ i [ class "icon" ] [ text "\xE145" ] ]
                        ++ [ availableTags ]
                    )
                ]

        selectedTags =
            Set.toList model.selected
                |> List.map
                    (\s ->
                        a [ class "tag" ]
                            [ i [ class "icon" ] [ text s ]
                            , i [ class "remove icon", onClick (RemoveTag s) ] [ text "\xE5C9" ]
                            ]
                    )
    in
        div [ class "clear" ]
            ([ div [ class "label" ] [ text "Label" ] ]
                ++ selectedTags
                ++ addButton
            )


view : Model -> Html Msg
view model =
    lazy tagListView model



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    if model.visible then
        Mouse.downs (\_ -> Click)
    else
        Sub.none



-- INIT


init : Model
init =
    { tags = Set.fromList [ "\xE531", "\xE536", "\xE52F" ]
    , selected = Set.empty
    , visible = False
    , ignoreNextToggle = False
    }
