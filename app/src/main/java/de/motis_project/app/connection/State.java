package de.motis_project.app.connection;

import java.io.IOException;

public class State {
    private static final String SERVER_URL = "ws://192.168.1.106:8080";

    private static State SINGLETON;
    private Server server;

    private State() {
        server = new Server(SERVER_URL);
        try {
            server.connect();
        } catch (IOException e) {
            System.out.println("fatal error connecting to " + SERVER_URL);
            e.printStackTrace();
        }
    }

    public static synchronized State get() {
        if (SINGLETON == null) {
            SINGLETON = new State();
        }
        return SINGLETON;
    }

    public Server getServer() {
        return server;
    }
}
