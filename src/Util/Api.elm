module Util.Api
    exposing
        ( ApiError(..)
        , MotisErrorInfo
        , sendRequest
        , sendRawRequest
        , sendJsonRequest
        )

import Json.Encode as Encode
import Json.Decode as Decode exposing ((:=))
import Json.Decode.Extra exposing ((|:), withDefault, maybeNull)
import Http as Http
import Task as Task
import Util.Core exposing ((=>))


type alias MotisErrorInfo =
    { errorCode : Int
    , category : String
    , reason : String
    }


type ApiError
    = MotisError MotisErrorInfo
    | HttpError Http.Error


sendRequest :
    String
    -> Decode.Decoder a
    -> (ApiError -> msg)
    -> (a -> msg)
    -> Encode.Value
    -> Cmd msg
sendRequest remoteAddress jsonDecoder onErr onOk value =
    sendJsonRequest
        remoteAddress
        jsonDecoder
        (handleHttpError onErr)
        onOk
        value


handleHttpError : (ApiError -> msg) -> Http.Error -> msg
handleHttpError onErr rawErr =
    case rawErr of
        Http.BadResponse 500 str ->
            case Decode.decodeString decodeErrorResponse str of
                Ok motisError ->
                    onErr (MotisError motisError)

                Err _ ->
                    onErr (HttpError rawErr)

        _ ->
            onErr (HttpError rawErr)


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


decodeErrorResponse : Decode.Decoder MotisErrorInfo
decodeErrorResponse =
    let
        decodeContent : String -> Decode.Decoder MotisErrorInfo
        decodeContent content_type =
            case content_type of
                "MotisError" ->
                    Decode.at [ "content" ] decodeMotisError

                _ ->
                    Decode.fail ("unexpected message type: " ++ content_type)
    in
        ("content_type" := Decode.string) `Decode.andThen` decodeContent


decodeMotisError : Decode.Decoder MotisErrorInfo
decodeMotisError =
    Decode.succeed MotisErrorInfo
        |: ("error_code" := Decode.int)
        |: ("category" := Decode.string)
        |: ("reason" := Decode.string)
