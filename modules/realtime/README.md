# Realtime Module

## CMake options

* `-DWITH_MYSQL=OFF`: No database support, supports loading messages from text
  files
* `-DWITH_MYSQL=ON`: Enable database support, supports loading messages from
  a MySQL database or text files. Requires mysqlclient and MySQL++.
  
## Command line/config options

All parameters are optional.

* `--realtime.msg_file <filename>`: Read realtime messages from the given
  text file. This option can be used more than once - in this case, realtime
  messages are loaded from the first file, then from the second file and so on.
* `--realtime.manual`: Enable manual mode. In this mode, realtime messages are
  only loaded and processed when a RealtimeForwardTimeRequest is received.
  Otherwise, messages are loaded automatically.
* `--realtime.from <timestamp>`: Timestamp of the first message to load.
  If this parameter is missing, there is no lower bound for messages to load.
  The timestamp can be one of:
    * A unix timestamp
    * A date and time formatted like `YYYYMMDD'T'hhmmss`, e.g. `20150131T235959`
    * A date formatted like `YYYYMMDD` e.g. `20150125`. In this case the time
      is 00:00:00.
* `--realtime.to <timestamp>`: Timestamp of the last message to load.
  If this parameter is missing, there is no upper bound for messages to load.
* `--realtime.debug`: Enable debug output.

## Usage scenarios

### Load messages from text file, manual forwarding

To load all messages from a single text file:

`./motis/webservice/motis-webservice --realtime.msg_file messages-20150815.txt
--realtime.manual`

Then send RealtimeForwardTimeRequest messages to process realtime messages until
a given time.

To load all messages from two text files (order of the files is important):

`./motis/webservice/motis-webservice --realtime.msg_file messages-20150815.txt
--realtime.msg_file messages-20150816.txt --realtime.manual`

### Load messages from text file, automatic forwarding

To automatically load and process all messages from a text file, remove
`--realtime.manual`. This will process all messages during startup.

## Module Messages

For more information about the format and fields of a message, see the .fbs
files in /protocol/realtime.

### RealtimeForwardTimeRequest

To manually forward the time, use `--realtime.manual` and then send a
RealtimeForwardTimeRequest message with the desired unix timestamp.

Example (JSON Syntax):

`{"content_type": "RealtimeForwardTimeRequest", "content":
{"new_time": 1439852400}}`

### RealtimeCurrentTimeRequest

This message can be used to request the timestamp of the last realtime message
that was processed.

Example request:

`{"content_type": "RealtimeCurrentTimeRequest"}`

Example response:

    {
      "content_type": "RealtimeCurrentTimeResponse",
      "content": {
        "last_message_time": 1439675970,
        "stream_end_time": 1439852400
      }
    }
    
In this example, the realtime message stream was forwarded to 1439852400 and the
timestamp of the last realtime message that was processed is 1439675970.
If no messages have been processed so far, the time returned is 0.

### RealtimeTrainInfoRequest

This message requests delay information about a train. It can be used to either
request information for a single event or for all events of a train starting
with the given event.
It will return a RealtimeTrainInfoResponse message with
delay information (scheduled time, real time, delay reason) for these stops.

Example request (single event):

`{"content_type": "RealtimeTrainInfoRequest", "content":
{"first_stop": {"train_nr": 46120, "station_index": 6886, "departure": false,
"real_time": 2744, "route_id": 6819}, "single_event": true}}`

Example response:


    {
      "content_type": "RealtimeTrainInfoResponse",
      "content": {
        "stops": [
          {
            "train_nr": 46120,
            "station_index": 6886,
            "real_time": 2744,
            "scheduled_time": 2739,
            "reason": "Forecast"
          }
        ],
        "route_id": 6819
      }
    }


Example request (all events of the train starting with the given event):

`{"content_type": "RealtimeTrainInfoRequest", "content":
{"first_stop": {"train_nr": 46120, "station_index": 6871, "departure": true,
"real_time": 2741, "route_id": 6819}, "single_event": false}}`

Example response:


    {
      "content_type": "RealtimeTrainInfoResponse",
      "content": {
        "stops": [
          {
            "train_nr": 46120,
            "station_index": 6871,
            "departure": 1,
            "real_time": 2741,
            "scheduled_time": 2736,
            "reason": "Forecast"
          },
          {
            "train_nr": 46120,
            "station_index": 6886,
            "real_time": 2744,
            "scheduled_time": 2739,
            "reason": "Forecast"
          },
          {
            "train_nr": 46120,
            "station_index": 6886,
            "departure": 1,
            "real_time": 2745,
            "scheduled_time": 2740,
            "reason": "Forecast"
          },
          {
            "train_nr": 46120,
            "station_index": 6774,
            "real_time": 2747,
            "scheduled_time": 2742,
            "reason": "Propagation"
          }
        ],
        "route_id": 6819
      }
    }

