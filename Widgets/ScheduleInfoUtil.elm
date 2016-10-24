module Widgets.ScheduleInfoUtil exposing (..)

import Date exposing (Date)
import Date.Extra.Duration as Duration exposing (DeltaRecord)
import Http as Http
import Task as Task
import Widgets.Data.ScheduleInfo as SI exposing (ScheduleInfo)


type alias Model =
    Maybe ScheduleInfo
