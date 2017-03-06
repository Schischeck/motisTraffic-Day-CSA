module Widgets.SimTimePicker
    exposing
        ( Msg(SetLocale, Show, Hide, Toggle, SetSimulationTime, DisableSimMode)
        , Model
        , init
        , view
        , update
        , getSelectedSimTime
        )

import Html exposing (Html, Attribute, div, text, span, input, label, i, a)
import Html.Attributes exposing (..)
import Html.Events exposing (onClick)
import Time exposing (Time)
import Date exposing (Date)
import Html exposing (Html)
import Task
import Util.Core exposing ((=>))
import Util.Date exposing (combineDateTime)
import Localization.Base exposing (..)
import Widgets.TimeInput as TimeInput
import Widgets.Calendar as Calendar
import Debounce


-- MODEL


type alias Model =
    { dateInput : Calendar.Model
    , timeInput : TimeInput.Model
    , debounce : Debounce.State
    , visible : Bool
    , pickedTime : Time
    , simModeActive : Bool
    }


init : Localization -> ( Model, Cmd Msg )
init locale =
    let
        ( dateModel, dateCmd ) =
            Calendar.init locale.dateConfig

        ( timeModel, timeCmd ) =
            TimeInput.init True
    in
        { dateInput = dateModel
        , timeInput = timeModel
        , debounce = Debounce.init
        , visible = False
        , pickedTime = 0
        , simModeActive = False
        }
            ! [ Cmd.map DateUpdate dateCmd
              , Cmd.map TimeUpdate timeCmd
              ]



-- UPDATE


type Msg
    = SetLocale Localization
    | DateUpdate Calendar.Msg
    | TimeUpdate TimeInput.Msg
    | Deb (Debounce.Msg Msg)
    | Show Date Bool
    | Hide
    | Toggle Date Bool
    | SetSimulationTime
    | ToggleSimMode
    | DisableSimMode


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        SetLocale locale ->
            { model
                | dateInput = Calendar.update (Calendar.SetDateConfig locale.dateConfig) model.dateInput
            }
                ! []

        DateUpdate msg_ ->
            handleTimeUpdate ({ model | dateInput = Calendar.update msg_ model.dateInput } ! [])

        TimeUpdate msg_ ->
            handleTimeUpdate ({ model | timeInput = TimeInput.update msg_ model.timeInput } ! [])

        Deb a ->
            Debounce.update debounceCfg a model

        Show currentDate simModeActive ->
            let
                model1 =
                    { model
                        | pickedTime = Date.toTime currentDate
                        , simModeActive = simModeActive
                    }

                ( model2, cmds1 ) =
                    update (DateUpdate (Calendar.InitDate True currentDate)) model1

                ( model3, cmds2 ) =
                    update (TimeUpdate (TimeInput.InitDate True currentDate)) model2
            in
                { model3 | visible = True } ! [ cmds1, cmds2 ]

        Hide ->
            { model | visible = False } ! []

        Toggle currentDate simModeActive ->
            if model.visible then
                update Hide model
            else
                update (Show currentDate simModeActive) model

        SetSimulationTime ->
            -- handled in Main
            model ! []

        DisableSimMode ->
            -- handled in Main
            model ! []

        ToggleSimMode ->
            let
                active =
                    not model.simModeActive

                model_ =
                    { model | simModeActive = active }

                cmd =
                    if active then
                        Debounce.debounceCmd debounceCfg <| SetSimulationTime
                    else
                        Task.perform identity (Task.succeed DisableSimMode)
            in
                handleTimeUpdate (model_ ! [ cmd ])


handleTimeUpdate : ( Model, Cmd Msg ) -> ( Model, Cmd Msg )
handleTimeUpdate ( model, cmd ) =
    let
        newTime =
            Date.toTime (combineDateTime model.dateInput.date model.timeInput.date)

        changed =
            newTime /= model.pickedTime

        model_ =
            { model | pickedTime = newTime }
    in
        if model.visible && model.simModeActive && changed then
            model_ ! [ cmd, Debounce.debounceCmd debounceCfg <| SetSimulationTime ]
        else
            model_ ! [ cmd ]


debounceCfg : Debounce.Config Model Msg
debounceCfg =
    Debounce.config
        .debounce
        (\model s -> { model | debounce = s })
        Deb
        1000


getSelectedSimTime : Model -> Time
getSelectedSimTime model =
    model.pickedTime



-- VIEW


view : Localization -> Model -> Html Msg
view locale model =
    div
        [ classList
            [ "sim-time-picker-overlay" => True
            , "hidden" => not model.visible
            ]
        ]
        [ div [ class "title" ]
            [ input
                [ type_ "checkbox"
                , id "sim-mode-checkbox"
                , name "sim-mode-checkbox"
                , checked model.simModeActive
                , onClick ToggleSimMode
                ]
                []
            , label [ for "sim-mode-checkbox" ] [ text locale.t.simTime.simMode ]
            ]
        , div
            [ classList
                [ "date" => True
                , "disabled" => not model.simModeActive
                ]
            ]
            [ Html.map DateUpdate <|
                Calendar.view 20 locale.t.search.date model.dateInput
            ]
        , div
            [ classList
                [ "time" => True
                , "disabled" => not model.simModeActive
                ]
            ]
            [ Html.map TimeUpdate <|
                TimeInput.view 21 locale.t.search.time model.timeInput
            ]
        , div [ class "close", onClick Hide ]
            [ i [ class "icon" ] [ text "close" ] ]
        ]
