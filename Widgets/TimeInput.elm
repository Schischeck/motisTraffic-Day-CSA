module Widgets.TimeInput exposing (Model, Msg, update, view, init)

import Task
import String
import Html exposing (..)
import Date exposing (Date)
import Html.Events exposing (onInput)
import Html.Attributes exposing (value)
import Html.Lazy exposing (lazy)
import Widgets.Input as Input
import Widgets.StringSplitUtil exposing (..)
import Date.Extra.Create exposing (dateFromFields)


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
    | InputUpdate Input.Msg


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    ( updateModel msg model, Cmd.none )


updateModel : Msg -> Model -> Model
updateModel msg model =
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


timeInputView : Model -> Html Msg
timeInputView model =
    Input.view InputUpdate [ onInput TimeInput, value model.inputStr ] [] model.inputWidget


view : Model -> Html Msg
view model =
    lazy timeInputView model
