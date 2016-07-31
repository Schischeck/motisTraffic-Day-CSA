package de.motis_project.app;

import java.io.IOException;

import de.motis_project.app.connection.State;

public class Application extends android.app.Application {
    @Override
    public void onCreate() {
        super.onCreate();
        try {
            State.get().getServer().connect();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
