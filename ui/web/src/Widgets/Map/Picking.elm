module Widgets.Map.Picking exposing (..)

import Widgets.Map.RailVizModel exposing (..)
import Math.Vector3 as Vector3 exposing (Vec3, vec3)
import Bitwise
import Util.List exposing ((!!))


toPickColor : Int -> Vec3
toPickColor pickId =
    let
        tf x =
            (toFloat x) / 255.0

        r =
            tf (Bitwise.and pickId 255)

        g =
            tf (Bitwise.and (Bitwise.shiftRightZfBy 8 pickId) 255)

        b =
            tf (Bitwise.and (Bitwise.shiftRightZfBy 16 pickId) 255)
    in
        vec3 r g b


toPickId : Maybe ( Int, Int, Int, Int ) -> Maybe Int
toPickId color =
    case color of
        Just ( r, g, b, a ) ->
            if a == 0 then
                Nothing
            else
                Just (r + (Bitwise.shiftLeftBy 8 g) + (Bitwise.shiftLeftBy 16 b))

        Nothing ->
            Nothing


stationPickIdBase : Int
stationPickIdBase =
    100000


getHoveredTrain : Model -> Maybe RVTrain
getHoveredTrain model =
    model.hoveredPickId
        |> Maybe.andThen (\pickId -> (getTrains model) !! pickId)


getHoveredStation : Model -> Maybe RVStation
getHoveredStation model =
    model.hoveredPickId
        |> Maybe.andThen (\pickId -> (getStations model) !! (pickId - stationPickIdBase))
