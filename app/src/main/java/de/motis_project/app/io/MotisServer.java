package de.motis_project.app.io;

import android.os.Handler;

import java.util.Date;

public class MotisServer extends Server {
    private int nextMsgId = 0;

    public MotisServer(String url, Handler handler) {
        super(url, handler);
    }

    public int guess(String input) {
        int id = ++nextMsgId;
        send(MessageBuilder.guess(id, input));
        return id;
    }

    public int route(String fromId, String toId, boolean isArrival, Date time) {
        int id = ++nextMsgId;
        send(MessageBuilder.route(id, fromId, toId, isArrival, time));
        return id;
    }
}
