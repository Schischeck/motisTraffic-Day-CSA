module Widgets.Typeahead exposing (Model, Msg, init, subscriptions, update, view)

import Html exposing (..)
import Html.Attributes exposing (..)
import Widgets.Input as Input
import Widgets.ViewUtil exposing (onStopAll)


-- MODEL


type alias Model =
    { suggestions : List String
    , input : String
    , selectedSuggestion : Int
    , visible : Bool
    }



-- UPDATE


type Msg
    = NoOp
    | InputChange
    | EnterSelection
    | SelectionUp
    | SelectionDown
    | Select
    | Hide


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        NoOp ->
            ( model, Cmd.none )

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
{-
   <gb-input value={getValue()}
             label={opts.label}
             icon={opts.icon}
             placeholder={opts.placeholder}
             input-keyup={onKeyUp}>
     <yield/>
   </gb-input>
   <div if={isVisible && proposals.length !== 0} class="paper" onclick={preventDismiss}>
     <ul class="proposals">
       <li each={p, i in proposals}
           class={selected: (i === selectedIndex)}
           onclick={selectProposal}
           onmouseover={setSelectedIndex}>
         {parent.opts.displayProperty ? p[parent.opts.displayProperty] : p}
       </li>
     </ul>
   </div>
-}


view : Model -> Html Msg
view model =
    div []
        [ Input.view [] []
        , div
            [ classList
                [ ( "paper", True )
                , ( "hide", not model.visible )
                ]
            , onStopAll "mousedown" NoOp
            ]
            [ ul [ class "proposals" ] []
            ]
        ]



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
