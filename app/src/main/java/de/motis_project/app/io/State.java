package de.motis_project.app.io;

import android.os.Handler;

import java.io.IOException;

public class State {
    private static final String SERVER_URL = "ws://ws.motis-project.de";

    private static State SINGLETON;
    private Server server;

    private State(Handler handler) {
        server = new Server(SERVER_URL, handler);
    }

    public static void init(Handler handler) {
        SINGLETON = new State(handler);

        try {
            SINGLETON.getServer().connect();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static synchronized State get() {
        return SINGLETON;
    }

    public Server getServer() {
        return server;
    }
}
