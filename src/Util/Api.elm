module Util.Api exposing (..)

import Json.Encode as Encode
import Json.Decode as Decode
import Http as Http
import Task as Task
import Util.Core exposing ((=>))


sendRawRequest :
    String
    -> (Http.RawError -> msg)
    -> (Http.Response -> msg)
    -> Encode.Value
    -> Cmd msg
sendRawRequest remoteAddress onErr onOk value =
    Http.send Http.defaultSettings
        { verb = "POST"
        , headers = [ "Content-Type" => "application/json" ]
        , url = remoteAddress
        , body = value |> Encode.encode 0 |> Http.string
        }
        |> Task.perform onErr onOk


sendJsonRequest :
    String
    -> Decode.Decoder a
    -> (Http.Error -> msg)
    -> (a -> msg)
    -> Encode.Value
    -> Cmd msg
sendJsonRequest remoteAddress jsonDecoder onErr onOk value =
    Http.send Http.defaultSettings
        { verb = "POST"
        , headers = [ "Content-Type" => "application/json" ]
        , url = remoteAddress
        , body = value |> Encode.encode 0 |> Http.string
        }
        |> Http.fromJson jsonDecoder
        |> Task.perform onErr onOk
