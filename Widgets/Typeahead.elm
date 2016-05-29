module Widgets.Typeahead exposing (Model, Msg, init, subscriptions, update, view)

import Html exposing (..)


-- MODEL


type alias Model =
    { suggestions : List String
    , input : String
    , selectedSuggestion : Int
    , visible : Bool
    }



-- UPDATE


type Msg
    = InputChange
    | EnterSelection
    | SelectionUp
    | SelectionDown
    | Select
    | Hide


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        InputChange ->
            ( model, Cmd.none )

        EnterSelection ->
            ( model, Cmd.none )

        SelectionUp ->
            ( model, Cmd.none )

        SelectionDown ->
            ( model, Cmd.none )
            
        Select ->
            ( model, Cmd.none )
            
        Hide ->
            ( model, Cmd.none )
            



-- VIEW


view : Model -> Html Msg
view model =
    div [] [ text "TODO" ]



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.none



-- INIT


init : ( Model, Cmd Msg )
init =
    ( { suggestions = []
      , input = ""
      , selectedSuggestion = 0
      , visible = False
      }
    , Cmd.none
    )
