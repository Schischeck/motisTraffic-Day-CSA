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
import Json.Decode as Decode exposing (nullable)
import Json.Decode.Pipeline exposing (decode, required, optional, hardcoded)
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


type ApiResult a
    = ApiSuccess a
    | ApiFailure ApiError


sendRequest :
    String
    -> Decode.Decoder a
    -> (ApiError -> msg)
    -> (a -> msg)
    -> Encode.Value
    -> Cmd msg
sendRequest remoteAddress jsonDecoder onErr onOk value =
    let
        request =
            Http.request
                { method = "POST"
                , headers = []
                , url = remoteAddress
                , body = value |> Http.jsonBody
                , expect =
                    Http.expectStringResponse (handleHttpResponse jsonDecoder)
                , timeout = Nothing
                , withCredentials = False
                }

        toMsg result =
            case result of
                Ok apiResult ->
                    case apiResult of
                        ApiSuccess x ->
                            onOk x

                        ApiFailure x ->
                            onErr x

                Err httpErr ->
                    onErr (promoteError httpErr)
    in
        Http.send toMsg request


handleHttpResponse :
    Decode.Decoder a
    -> Http.Response String
    -> Result String (ApiResult a)
handleHttpResponse jsonDecoder response =
    if response.status.code >= 200 && response.status.code < 300 then
        case Decode.decodeString jsonDecoder response.body of
            Ok value ->
                Ok (ApiSuccess value)

            Err msg ->
                Ok (ApiFailure (DecodeError msg))
    else if response.status.code == 500 then
        case Decode.decodeString decodeErrorResponse response.body of
            Ok value ->
                Ok (ApiFailure (MotisError value))

            Err msg ->
                Ok (ApiFailure (DecodeError msg))
    else
        Ok (ApiFailure (HttpError response.status.code))


promoteError : Http.Error -> ApiError
promoteError rawError =
    case rawError of
        Http.Timeout ->
            TimeoutError

        Http.NetworkError ->
            NetworkError

        Http.BadPayload err _ ->
            DecodeError err

        Http.BadStatus res ->
            HttpError res.status.code

        Http.BadUrl _ ->
            HttpError 0


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
        (Decode.field "content_type" Decode.string)
            |> Decode.andThen decodeContent


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
