package de.motis_project.app.io;

import java.nio.ByteBuffer;

import motis.Destination;
import motis.DestinationType;
import motis.Message;
import motis.MotisError;
import motis.MsgContent;
import motis.guesser.StationGuesserRequest;

import org.iq80.snappy.Snappy;

import com.google.flatbuffers.FlatBufferBuilder;

public class MessageBuilder {
    public static Message error(int ssid, int code, String category, String reason) {
        FlatBufferBuilder b = new FlatBufferBuilder();
        int error = MotisError.createMotisError(b, code,b.createString(category), b.createString(reason));
        b.finish(Message.createMessage(b, 0, MsgContent.MotisError, error, ssid));
        return Message.getRootAsMessage(b.dataBuffer());
    }

    public static byte[] guess(int ssid, String input) {
        FlatBufferBuilder b = new FlatBufferBuilder();
        int guesserRequestOffset = StationGuesserRequest
                .createStationGuesserRequest(b, 10, b.createString(input));
        int destination = Destination.createDestination(b,
                DestinationType.Module, b.createString("/guesser"));
        b.finish(Message.createMessage(b, destination,
                MsgContent.StationGuesserRequest, guesserRequestOffset, ssid));
        return Snappy.compress(b.sizedByteArray());
    }

    public static Message decode(byte[] buf) {
        final byte[] uncompressed = Snappy.uncompress(buf, 0, buf.length);
        return Message.getRootAsMessage(ByteBuffer.wrap(uncompressed));
    }
}