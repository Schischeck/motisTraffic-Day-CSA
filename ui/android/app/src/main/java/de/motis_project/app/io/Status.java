package de.motis_project.app.io;

import android.content.Context;
import android.os.Handler;

import java.io.IOException;

import de.motis_project.app.quickselect.QuickSelectDataSource;
import de.motis_project.app.saved.SavedConnectionsDataSource;
import motis.Connection;

public class Status {
    private static final String SERVER_URL = "ws://ws.motis-project.de";

    private static Status SINGLETON;
    private final MotisServer server;
    private SavedConnectionsDataSource savedConnectionsDb;
    private QuickSelectDataSource quickSelectDb;
    private Connection connection;

    private Status(Context ctx, Handler handler) {
        savedConnectionsDb = new SavedConnectionsDataSource(ctx);
        quickSelectDb = new QuickSelectDataSource(ctx);
        server = new MotisServer(SERVER_URL, handler);
    }

    public static void init(Context ctx, Handler handler) {
        SINGLETON = new Status(ctx, handler);

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

    public Connection getConnection() { return connection; }

    public void setConnection(Connection connection) { this.connection = connection; }

    public SavedConnectionsDataSource getSavedConnectionsDb() {
        return savedConnectionsDb;
    }

    public QuickSelectDataSource getQuickSelectDb() { return quickSelectDb; }
}
