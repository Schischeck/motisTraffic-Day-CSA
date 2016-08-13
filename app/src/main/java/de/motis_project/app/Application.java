package de.motis_project.app;

import android.os.Handler;
import android.os.Looper;

import de.motis_project.app.io.Status;

public class Application extends android.app.Application {
    @Override
    public void onCreate() {
        super.onCreate();
        Status.init(new Handler(Looper.getMainLooper()));
    }
}
