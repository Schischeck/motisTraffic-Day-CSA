module Widgets.Helpers.ApiErrorUtil exposing (..)

import Localization.Base exposing (..)
import Util.Api as Api exposing (..)


errorText : Localization -> ApiError -> String
errorText locale err =
    case err of
        MotisError err_ ->
            motisErrorMsg locale err_

        TimeoutError ->
            locale.t.errors.timeout

        NetworkError ->
            locale.t.errors.network

        HttpError status ->
            locale.t.errors.http status

        DecodeError msg ->
            locale.t.errors.decode msg


motisErrorMsg : Localization -> MotisErrorInfo -> String
motisErrorMsg { t } err =
    case err of
        RoutingError JourneyDateNotInSchedule ->
            t.errors.journeyDateNotInSchedule

        AccessError AccessTimestampNotInSchedule ->
            t.errors.journeyDateNotInSchedule

        _ ->
            t.errors.internalError (toString err)
