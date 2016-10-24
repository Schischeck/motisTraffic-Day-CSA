module Data.ScheduleInfo.Request exposing (request)

import Json.Encode as Encode
import Util.Core exposing ((=>))


request : String
request =
    Encode.object
        [ "destination"
            => Encode.object
                [ "type" => Encode.string "Module"
                , "target" => Encode.string "/lookup/schedule_info"
                ]
        , "content_type" => Encode.string "MotisNoMessage"
        , "content"
            => Encode.object
                []
        ]
        |> Encode.encode 0
