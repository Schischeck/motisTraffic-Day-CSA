module Util.Api
    exposing
        ( ApiError(..)
        , MotisErrorInfo
        , sendRequest
        )

import Json.Encode as Encode
import Json.Decode as Decode exposing ((:=))
import Json.Decode.Extra exposing ((|:), withDefault, maybeNull)
import Http
import Task exposing (Task, andThen, mapError, succeed, fail)
import Util.Core exposing ((=>))


type alias MotisErrorInfo =
    { errorCode : Int
    , category : String
    , reason : String
    }


type ApiError
    = MotisError MotisErrorInfo
    | TimeoutError
    | NetworkError
    | HttpError Int
    | DecodeError String



{--
MotisError:
category            errorCode   reason
motis::module       0           module: no error
motis::module       1           module: unable to parse message
motis::module       2           module: malformed message
motis::module       3           module: target not found
motis::module       4           module: unkown error
motis::module       5           module: unexpected message type
motis::routing      0           routing: no error
motis::routing      1           routing: station could not be guessed
motis::routing      2           routing: requested search type not supported
motis::routing      3           routing: path length not supported
motis::routing      4           routing: journey date not in schedule
motis::routing      5           routing: event not found
motis::routing      6           routing: edge type not supported
motis::routing      7           routing: too many start labels (route edge not sorted?)
--}


sendRequest :
    String
    -> Decode.Decoder a
    -> (ApiError -> msg)
    -> (a -> msg)
    -> Encode.Value
    -> Cmd msg
sendRequest remoteAddress jsonDecoder onErr onOk value =
    Http.send Http.defaultSettings
        { verb = "POST"
        , headers = [ "Content-Type" => "application/json" ]
        , url = remoteAddress
        , body = value |> Encode.encode 0 |> Http.string
        }
        |> handleResponse jsonDecoder
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


handleResponse :
    Decode.Decoder a
    -> Task Http.RawError Http.Response
    -> Task ApiError a
handleResponse decoder response =
    let
        decode str =
            case Decode.decodeString decoder str of
                Ok v ->
                    succeed v

                Err msg ->
                    fail (DecodeError msg)
    in
        mapError promoteError response
            `andThen` handleHttpResponse decode


handleHttpResponse :
    (String -> Task ApiError a)
    -> Http.Response
    -> Task ApiError a
handleHttpResponse handle response =
    if response.status >= 200 && response.status < 300 then
        case response.value of
            Http.Text str ->
                handle str

            _ ->
                fail (DecodeError "Response body is a blob, expecting a string.")
    else if response.status == 500 then
        case response.value of
            Http.Text str ->
                case Decode.decodeString decodeErrorResponse str of
                    Ok err ->
                        fail (MotisError err)

                    Err _ ->
                        fail (HttpError response.status)

            _ ->
                fail (HttpError response.status)
    else
        fail (HttpError response.status)


promoteError : Http.RawError -> ApiError
promoteError rawError =
    case rawError of
        Http.RawTimeout ->
            TimeoutError

        Http.RawNetworkError ->
            NetworkError
