package de.motis_project.app2.io;

import android.content.Context;
import android.os.Handler;

import java.io.IOException;

import de.motis_project.app2.query.guesser.FavoritesDataSource;
import de.motis_project.app2.saved.SavedConnectionsDataSource;
import motis.Connection;

public class Status {
    private static final String SERVER_URL = "ws://demo.motis-project.de";

    private static Status SINGLETON;
    private final MotisServer server;
    private SavedConnectionsDataSource savedConnectionsDb;
    private FavoritesDataSource favoritesDb;
    private Connection connection;

    private Status(Context ctx, Handler handler) {
        savedConnectionsDb = new SavedConnectionsDataSource(ctx);
        favoritesDb = new FavoritesDataSource(ctx);
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

    public FavoritesDataSource getFavoritesDb() {
        return favoritesDb;
    }
}
