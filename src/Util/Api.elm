module Util.Api
    exposing
        ( ApiError(..)
        , MotisErrorInfo(..)
        , ModuleErrorInfo(..)
        , RoutingErrorInfo(..)
        , MotisErrorDetail
        , sendRequest
        )

import Json.Encode as Encode
import Json.Decode as Decode exposing ((:=))
import Json.Decode.Pipeline exposing (decode, required, optional, hardcoded, nullable)
import Http
import Task exposing (Task, andThen, mapError, succeed, fail)
import Util.Core exposing ((=>))


type ApiError
    = MotisError MotisErrorInfo
    | TimeoutError
    | NetworkError
    | HttpError Int
    | DecodeError String


type MotisErrorInfo
    = ModuleError ModuleErrorInfo
    | RoutingError RoutingErrorInfo
    | UnknownMotisError MotisErrorDetail


type ModuleErrorInfo
    = UnableToParseMsg
    | MalformedMsg
    | TargetNotFound
    | UnexpectedMessageType
    | UnknownModuleError MotisErrorDetail


type RoutingErrorInfo
    = NoGuessForStation
    | SearchTypeNotSupported
    | PathLengthNotSupported
    | JourneyDateNotInSchedule
    | EventNotFound
    | EdgeTypeNotSupported
    | TooManyStartLabels
    | UnknownRoutingError MotisErrorDetail


type alias MotisErrorDetail =
    { errorCode : Int
    , category : String
    , reason : String
    }


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
            |> andThen (handleHttpResponse decode)


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
        ("content_type" := Decode.string) |> Decode.andThen decodeContent


decodeMotisErrorDetail : Decode.Decoder MotisErrorDetail
decodeMotisErrorDetail =
    decode MotisErrorDetail
        |> required "error_code" Decode.int
        |> required "category" Decode.string
        |> required "reason" Decode.string


decodeMotisError : Decode.Decoder MotisErrorInfo
decodeMotisError =
    decodeMotisErrorDetail |> Decode.andThen (Decode.succeed << mapMotisError)


mapMotisError : MotisErrorDetail -> MotisErrorInfo
mapMotisError err =
    case err.category of
        "motis::module" ->
            ModuleError (mapModuleError err)

        "motis::routing" ->
            RoutingError (mapRoutingError err)

        _ ->
            UnknownMotisError err


mapModuleError : MotisErrorDetail -> ModuleErrorInfo
mapModuleError err =
    case err.errorCode of
        1 ->
            UnableToParseMsg

        2 ->
            MalformedMsg

        3 ->
            TargetNotFound

        4 ->
            UnknownModuleError err

        5 ->
            UnexpectedMessageType

        _ ->
            UnknownModuleError err


mapRoutingError : MotisErrorDetail -> RoutingErrorInfo
mapRoutingError err =
    case err.errorCode of
        1 ->
            NoGuessForStation

        2 ->
            SearchTypeNotSupported

        3 ->
            PathLengthNotSupported

        4 ->
            JourneyDateNotInSchedule

        5 ->
            EventNotFound

        6 ->
            EdgeTypeNotSupported

        7 ->
            TooManyStartLabels

        _ ->
            UnknownRoutingError err
