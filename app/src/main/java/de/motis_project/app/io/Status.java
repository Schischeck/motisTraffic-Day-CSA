package de.motis_project.app.io;

import android.os.Handler;

import java.io.IOException;

public class Status {
    private static final String SERVER_URL = "ws://ws.motis-project.de";

    private static Status SINGLETON;
    private MotisServer server;

    private Status(Handler handler) {
        server = new MotisServer(SERVER_URL, handler);
    }

    public static void init(Handler handler) {
        SINGLETON = new Status(handler);

        try {
            SINGLETON.getServer().connect();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static synchronized Status get() {
        return SINGLETON;
    }

    public MotisServer getServer() {
        return server;
    }
}
