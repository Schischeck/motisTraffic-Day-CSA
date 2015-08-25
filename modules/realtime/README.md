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

### RealtimeForwardTimeRequest

To manually forward the time, use `--realtime.manual` and then send a
RealtimeForwardTimeRequest message with the desired unix timestamp.

Example (JSON Syntax):

`{"content_type": "RealtimeForwardTimeRequest", "content":
{"new_time": 1439852400}}`

### RealtimeTrainInfoRequest

This message contains information about the first stop of a train from the
current graph. It will return a RealtimeTrainInfoResponse message with
delay information (scheduled time, real time, delay reason) for all stops of
the train.

Example request:

`{"content_type": "RealtimeTrainInfoRequest", "content":
{"first_stop": {"train_nr": 46120, "station_index": 6871, "departure": true,
"real_time": 2741}}}`

Example response:


    {
      "content_type": "RealtimeTrainInfoResponse",
      "content": {
        "stops": [
          {
            "train_nr": 46120,
            "station_index": 6871,
            "real_time": 2741,
            "scheduled_time": 2736,
            "reason": "Forecast"
          },
          {
            "train_nr": 46120,
            "station_index": 6886,
            "departure": 0,
            "real_time": 2744,
            "scheduled_time": 2739,
            "reason": "Forecast"
          },
          {
            "train_nr": 46120,
            "station_index": 6886,
            "real_time": 2745,
            "scheduled_time": 2740,
            "reason": "Forecast"
          },
          {
            "train_nr": 46120,
            "station_index": 6774,
            "departure": 0,
            "real_time": 2747,
            "scheduled_time": 2742,
            "reason": "Propagation"
          }
        ]
      }
    }

