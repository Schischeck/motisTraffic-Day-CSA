package de.motis_project.app.io;

import com.google.flatbuffers.FlatBufferBuilder;

import org.iq80.snappy.Snappy;

import java.nio.ByteBuffer;
import java.util.Date;

import motis.Destination;
import motis.DestinationType;
import motis.Interval;
import motis.Message;
import motis.MotisError;
import motis.MsgContent;
import motis.guesser.StationGuesserRequest;
import motis.routing.InputStation;
import motis.routing.PretripStart;
import motis.routing.RoutingRequest;
import motis.routing.SearchDir;
import motis.routing.SearchType;
import motis.routing.Start;

public class MessageBuilder {
    public static Message error(int ssid, int code, String category,
                                String reason) {
        FlatBufferBuilder b = new FlatBufferBuilder();
        int error = MotisError
                .createMotisError(b, code, b.createString(category),
                                  b.createString(reason));
        b.finish(Message.createMessage(b, 0, MsgContent.MotisError, error,
                                       ssid));
        return Message.getRootAsMessage(b.dataBuffer());
    }

    public static byte[] guess(int ssid, String input) {
        FlatBufferBuilder b = new FlatBufferBuilder();
        int guesserRequestOffset = StationGuesserRequest
                .createStationGuesserRequest(b, 10, b.createString(input));
        int destination = Destination.createDestination(
                b, DestinationType.Module, b.createString("/guesser"));
        b.finish(Message.createMessage(
                b, destination, MsgContent.StationGuesserRequest,
                guesserRequestOffset, ssid));
        return Snappy.compress(b.sizedByteArray());
    }

    public static byte[] route(
            int ssid,
            String fromId, String toId,
            boolean isArrival,
            Date intervalBegin, Date intervalEnd) {
        FlatBufferBuilder b = new FlatBufferBuilder();

        String startStationId = isArrival ? toId : fromId;
        String targetStationId = isArrival ? fromId : toId;

        int start = createPreTripStart(
                b, intervalBegin, intervalEnd, startStationId);
        int routingRequest = RoutingRequest.createRoutingRequest(
                b, Start.PretripStart, start,
                InputStation.createInputStation(
                        b, b.createString(targetStationId), b.createString("")),
                SearchType.Default,
                isArrival ? SearchDir.Backward : SearchDir.Forward,
                RoutingRequest.createViaVector(b, new int[]{}),
                RoutingRequest.createAdditionalEdgesVector(b, new int[]{}));
        b.finish(Message.createMessage(
                b, Destination.createDestination(
                        b, DestinationType.Module, b.createString("/routing")),
                MsgContent.RoutingRequest, routingRequest, ssid));

        return Snappy.compress(b.sizedByteArray());
    }

    static private int createPreTripStart(
            FlatBufferBuilder b,
            Date intervalBegin, Date intervalEnd,
            String startStationId) {
        int station = InputStation.createInputStation(
                b, b.createString(startStationId), b.createString(""));

        PretripStart.startPretripStart(b);
        PretripStart.addStation(b, station);
        PretripStart.addInterval(
                b, Interval.createInterval(
                        b, intervalBegin.getTime() / 1000,
                        intervalEnd.getTime() / 1000));
        return PretripStart.endPretripStart(b);
    }

    public static Message decode(byte[] buf) {
        final byte[] uncompressed = Snappy.uncompress(buf, 0, buf.length);
        return Message.getRootAsMessage(ByteBuffer.wrap(uncompressed));
    }
}