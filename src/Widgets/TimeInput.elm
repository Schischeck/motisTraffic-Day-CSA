module Widgets.TimeInput exposing (Model, Msg, update, view, init)

import Task
import String
import Html exposing (..)
import Date exposing (Date)
import Html.Events exposing (onInput, onClick)
import Html.Attributes exposing (value, class)
import Html.Lazy exposing (..)
import Widgets.Input as Input
import Widgets.Button as Button
import Util.StringSplit exposing (..)
import Date.Extra.Create exposing (dateFromFields)
import Date.Extra.Duration as Duration


-- MODEL


type alias Model =
    { date : Date
    , inputStr : String
    , inputWidget : Input.Model
    }


init : ( Model, Cmd Msg )
init =
    ( { date = dateFromFields 0 Date.Jan 0 0 0 0 0
      , inputStr = formatDate (dateFromFields 0 Date.Jan 0 0 0 0 0)
      , inputWidget = Input.init
      }
    , getCurrentDate
    )


getCurrentDate : Cmd Msg
getCurrentDate =
    Task.perform NoOp InitDate Date.now



-- UPDATE


type Msg
    = TimeInput String
    | InitDate Date
    | NoOp String
    | PrevHour
    | NextHour
    | InputUpdate Input.Msg


update : Msg -> Model -> Model
update msg model =
    case msg of
        TimeInput s ->
            case parseInput s of
                Nothing ->
                    { model | inputStr = s }

                Just date ->
                    { model | date = date, inputStr = s }

        InitDate d ->
            { model | date = d, inputStr = formatDate d }

        NoOp s ->
            model

        PrevHour ->
            let
                newDate =
                    Duration.add Duration.Hour -1 model.date
            in
                { model | date = newDate, inputStr = formatDate newDate }

        NextHour ->
            let
                newDate =
                    Duration.add Duration.Hour 1 model.date
            in
                { model | date = newDate, inputStr = formatDate newDate }

        InputUpdate msg' ->
            { model | inputWidget = Input.update msg' model.inputWidget }


parseInput : String -> Maybe Date
parseInput str =
    let
        hourStr =
            intNthToken 0 ":" str

        minuteStr =
            intNthToken 1 ":" str
    in
        Maybe.map2 toDate hourStr minuteStr


toDate : Int -> Int -> Date
toDate h m =
    dateFromFields 0 Date.Jan 0 h m 0 0


formatDate : Date -> String
formatDate d =
    (toString (Date.hour d) |> String.padLeft 2 '0')
        ++ ":"
        ++ (toString (Date.minute d) |> String.padLeft 2 '0')



-- VIEW


hourButtons : Html Msg
hourButtons =
    div [ class "hour-buttons" ]
        [ div [] [ Button.view [ onClick PrevHour ] [ i [ class "icon" ] [ text "chevron_left" ] ] ]
        , div [] [ Button.view [ onClick NextHour ] [ i [ class "icon" ] [ text "chevron_right" ] ] ]
        ]


timeInputView : String -> Model -> Html Msg
timeInputView label model =
    Input.view InputUpdate
        [ onInput TimeInput, value model.inputStr ]
        label
        (Just [ hourButtons ])
        (Just "schedule")
        model.inputWidget


view : String -> Model -> Html Msg
view label model =
    lazy2 timeInputView label model
