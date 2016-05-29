-- MODEL

type alias Model =
  { suggestions : List String
  , input : String
  , selectedSuggestion : Int
  , visible : Bool
  }


-- UPDATE

type Msg = InputChange
         | Select Int
         | SelectionUp
         | SelectionDown
         

update : Msg -> Model -> (Model, Cmd Msg)
update msg model =
  case msg of
    InputChange -> model
    

-- VIEW

view : Model -> Html Msg
view model =
  [text "TODO"]


-- SUBSCRIPTIONS

subscriptions : Model -> Sub Msg
subscriptions model =
  Sub.none


-- INIT

init : (Model, Cmd Msg)
init =
  { suggestions = []
  , input = ""
  , selectedSuggestion : 
  }