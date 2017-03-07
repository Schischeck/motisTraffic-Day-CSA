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
        , saveSelections
        )

import Html exposing (..)
import Html.Attributes exposing (..)
import Html.Events exposing (onClick)
import Html.Lazy exposing (..)
import Mouse
import Util.View exposing (onStopAll, onStopPropagation)
import Util.List exposing ((!!))


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


defaultMaxDuration : Int
defaultMaxDuration =
    900


init : Maybe String -> Model
init storedSelections =
    { tags =
        [ WalkTag { maxDuration = defaultMaxDuration }
        , BikeTag { maxDuration = defaultMaxDuration }
        ]
    , selected =
        storedSelections
            |> Maybe.map restoreSelections
            |> Maybe.withDefault []
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



-- LOCAL STORAGE


saveSelections : Model -> String
saveSelections model =
    let
        tagToString tag =
            case tag of
                WalkTag o ->
                    "w," ++ toString o.maxDuration

                BikeTag o ->
                    "b," ++ toString o.maxDuration

                CarTag o ->
                    "c," ++ toString o.maxDuration
    in
        model.selected
            |> List.map tagToString
            |> String.join ";"


restoreSelections : String -> List Tag
restoreSelections str =
    let
        parseTag s =
            let
                parts =
                    String.split "," s

                t =
                    List.head parts

                args =
                    List.drop 1 parts

                getInt idx default =
                    args
                        !! idx
                        |> Maybe.map String.toInt
                        |> Maybe.map (Result.withDefault default)
                        |> Maybe.withDefault default
            in
                case t of
                    Just "w" ->
                        Just <|
                            WalkTag { maxDuration = getInt 0 defaultMaxDuration }

                    Just "b" ->
                        Just <|
                            BikeTag { maxDuration = getInt 0 defaultMaxDuration }

                    Just "c" ->
                        Just <|
                            CarTag { maxDuration = getInt 0 defaultMaxDuration }

                    _ ->
                        Nothing
    in
        str
            |> String.split ";"
            |> List.filterMap parseTag
