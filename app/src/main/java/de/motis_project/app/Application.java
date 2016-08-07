package de.motis_project.app;

import android.os.Handler;
import android.os.Looper;

import de.motis_project.app.io.State;

public class Application extends android.app.Application {
    @Override
    public void onCreate() {
        super.onCreate();
        State.init(new Handler(Looper.getMainLooper()));
    }
}
