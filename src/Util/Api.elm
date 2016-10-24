module Util.Api exposing (..)

import Json.Encode as Encode
import Http as Http
import Task as Task
import Util.Core exposing ((=>))


sendRequest :
    String
    -> Encode.Value
    -> (Http.RawError -> msg)
    -> (Http.Response -> msg)
    -> Cmd msg
sendRequest remoteAddress value onErr onOk =
    Http.send Http.defaultSettings
        { verb = "POST"
        , headers = [ "Content-Type" => "application/json" ]
        , url = remoteAddress
        , body = value |> Encode.encode 0 |> Http.string
        }
        |> Task.perform onErr onOk
