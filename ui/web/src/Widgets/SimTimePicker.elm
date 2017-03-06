module Widgets.SimTimePicker
    exposing
        ( Msg(SetLocale, Show, Hide, Toggle, SetSimulationTime)
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
    }


init : Localization -> ( Model, Cmd Msg )
init locale =
    let
        ( dateModel, dateCmd ) =
            Calendar.init locale.dateConfig

        ( timeModel, timeCmd ) =
            TimeInput.init
    in
        { dateInput = dateModel
        , timeInput = timeModel
        , debounce = Debounce.init
        , visible = False
        , pickedTime = 0
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
    | Show Date
    | Hide
    | Toggle Date
    | SetSimulationTime


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

        Show currentDate ->
            let
                model1 =
                    { model | pickedTime = Date.toTime currentDate }

                ( model2, cmds1 ) =
                    update (DateUpdate (Calendar.InitDate True currentDate)) model1

                ( model3, cmds2 ) =
                    update (TimeUpdate (TimeInput.InitDate True currentDate)) model2
            in
                { model3 | visible = True } ! [ cmds1, cmds2 ]

        Hide ->
            { model | visible = False } ! []

        Toggle currentDate ->
            if model.visible then
                update Hide model
            else
                update (Show currentDate) model

        SetSimulationTime ->
            -- handled in Main
            model ! []


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
        if model.visible && changed then
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
        [ div [ class "date" ]
            [ Html.map DateUpdate <|
                Calendar.view 20 locale.t.search.date model.dateInput
            ]
        , div [ class "time" ]
            [ Html.map TimeUpdate <|
                TimeInput.view 21 locale.t.search.time model.timeInput
            ]
        , div [ class "close", onClick Hide ]
            [ i [ class "icon" ] [ text "close" ] ]
        ]
