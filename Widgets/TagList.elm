module Widgets.TagList exposing (Model, Msg, init, subscriptions, update, view)

import Html exposing (..)
import Html.Attributes exposing (..)
import Html.Events exposing (onClick)
import Set exposing (..)
import Mouse
import Widgets.ViewUtil exposing (onStopAll, onStopPropagation)


-- MODEL


type alias Model =
    { tags : Set String
    , selected : Set String
    , visible : Bool
    }



-- UPDATE


type Msg
    = AddTag String
    | RemoveTag String
    | ToggleVisibility
    | Click
    | NoOp


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    ( updateModel msg model, Cmd.none )


updateModel : Msg -> Model -> Model
updateModel msg model =
    case msg of
        NoOp ->
            model

        AddTag t ->
            { model | selected = Set.insert t model.selected, visible = False }

        RemoveTag t ->
            { model | selected = Set.remove t model.selected }

        ToggleVisibility ->
            { model | visible = not model.visible }

        Click ->
            { model | visible = False }



-- VIEW


view : Model -> Html Msg
view model =
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
                [ div [ class "tag outline", onStopPropagation "mousedown" ToggleVisibility ]
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
        div []
            ([ div [ class "label" ] [ text "Label" ] ]
            ++ selectedTags ++ addButton)



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    if model.visible then
        Mouse.downs (\_ -> Click)
    else
        Sub.none



-- INIT


init : ( Model, Cmd Msg )
init =
    ( { tags = Set.fromList [ "\xE531", "\xE536", "\xE52F" ]
      , selected = Set.empty
      , visible = False
      }
    , Cmd.none
    )
