module Util.Api
    exposing
        ( ApiError(..)
        , MotisErrorInfo(..)
        , ModuleErrorInfo(..)
        , RoutingErrorInfo(..)
        , LookupErrorInfo(..)
        , AccessErrorInfo(..)
        , OsrmErrorInfo(..)
        , MotisErrorDetail
        , sendRequest
        , decodeErrorResponse
        )

import Json.Encode as Encode
import Json.Decode as Decode exposing (nullable)
import Json.Decode.Pipeline exposing (decode, required, optional, hardcoded)
import Http


type ApiError
    = MotisError MotisErrorInfo
    | TimeoutError
    | NetworkError
    | HttpError Int
    | DecodeError String


type MotisErrorInfo
    = ModuleError ModuleErrorInfo
    | RoutingError RoutingErrorInfo
    | LookupError LookupErrorInfo
    | AccessError AccessErrorInfo
    | OsrmError OsrmErrorInfo
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


type LookupErrorInfo
    = LookupNotInSchedulePeriod
    | LookupStationNotFound
    | LookupRouteNotFound
    | LookupRouteEdgeNotFound
    | UnknownLookupError MotisErrorDetail


type AccessErrorInfo
    = AccessStationNotFound
    | AccessServiceNotFound
    | AccessTimestampNotInSchedule
    | UnknownAccessError MotisErrorDetail


type OsrmErrorInfo
    = OsrmProfileNotAvailable
    | OsrmNoRoutingResponse
    | UnknownOsrmError MotisErrorDetail


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
                    onErr (handleHttpError httpErr)
    in
        Http.send toMsg request


handleHttpResponse :
    Decode.Decoder a
    -> Http.Response String
    -> Result String (ApiResult a)
handleHttpResponse jsonDecoder response =
    case Decode.decodeString jsonDecoder response.body of
        Ok value ->
            Ok (ApiSuccess value)

        Err msg ->
            Ok (ApiFailure (DecodeError msg))


handleHttpError : Http.Error -> ApiError
handleHttpError rawError =
    case rawError of
        Http.Timeout ->
            TimeoutError

        Http.NetworkError ->
            NetworkError

        Http.BadPayload err _ ->
            DecodeError err

        Http.BadStatus res ->
            case Decode.decodeString decodeErrorResponse res.body of
                Ok value ->
                    MotisError value

                Err msg ->
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

        "motis::lookup" ->
            LookupError (mapLookupError err)

        "motis::access" ->
            AccessError (mapAccessError err)

        "motis::osrm" ->
            OsrmError (mapOsrmError err)

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


mapLookupError : MotisErrorDetail -> LookupErrorInfo
mapLookupError err =
    case err.errorCode of
        2 ->
            LookupNotInSchedulePeriod

        3 ->
            LookupStationNotFound

        4 ->
            LookupRouteNotFound

        5 ->
            LookupRouteEdgeNotFound

        _ ->
            UnknownLookupError err


mapAccessError : MotisErrorDetail -> AccessErrorInfo
mapAccessError err =
    case err.errorCode of
        2 ->
            AccessStationNotFound

        3 ->
            AccessServiceNotFound

        4 ->
            AccessTimestampNotInSchedule

        _ ->
            UnknownAccessError err


mapOsrmError : MotisErrorDetail -> OsrmErrorInfo
mapOsrmError err =
    case err.errorCode of
        1 ->
            OsrmProfileNotAvailable

        2 ->
            OsrmNoRoutingResponse

        _ ->
            UnknownOsrmError err
