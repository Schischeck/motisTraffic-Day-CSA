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
import Util.Core exposing ((=>))
import Json.Encode as Encode
import Json.Decode as Decode
import Json.Decode.Pipeline as JDP exposing (decode, required, optional, hardcoded, requiredAt)


-- MODEL


type alias Model =
    { tags : List Tag
    , selected : List Tag
    , visible : Bool
    , ignoreNextToggle : Bool
    }


type Tag
    = WalkTag TagOptions
    | BikeTag TagOptions
    | CarTag TagOptions


type alias TagOptions =
    { maxDuration : Int }


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


encodeTag : Tag -> Encode.Value
encodeTag tag =
    case tag of
        WalkTag o ->
            Encode.object
                [ "type" => Encode.string "Walk"
                , "max_duration" => Encode.int o.maxDuration
                ]

        BikeTag o ->
            Encode.object
                [ "type" => Encode.string "Bike"
                , "max_duration" => Encode.int o.maxDuration
                ]

        CarTag o ->
            Encode.object
                [ "type" => Encode.string "Car"
                , "max_duration" => Encode.int o.maxDuration
                ]


decodeTag : Decode.Decoder Tag
decodeTag =
    let
        parseTag : String -> Decode.Decoder Tag
        parseTag type_ =
            case type_ of
                "Walk" ->
                    decodeOptions
                        |> Decode.map WalkTag

                "Bike" ->
                    decodeOptions
                        |> Decode.map BikeTag

                "Car" ->
                    decodeOptions
                        |> Decode.map CarTag

                _ ->
                    Decode.fail "unknown tag type"

        decodeOptions : Decode.Decoder TagOptions
        decodeOptions =
            decode TagOptions
                |> JDP.required "max_duration" Decode.int
    in
        (Decode.field "type" Decode.string) |> Decode.andThen parseTag


saveSelections : Model -> String
saveSelections model =
    model.selected
        |> List.map encodeTag
        |> Encode.list
        |> Encode.encode 0


restoreSelections : String -> List Tag
restoreSelections str =
    Decode.decodeString (Decode.list decodeTag) str
        |> Result.withDefault []
