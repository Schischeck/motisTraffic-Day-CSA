package de.motis_project.app;

import android.os.AsyncTask;
import android.util.Log;

import com.google.flatbuffers.FlatBufferBuilder;

import motis.Message;

public abstract class FetchDataTask extends AsyncTask<FlatBufferBuilder, Void, Message> {
    static boolean executed = false;

    @Override
    protected Message doInBackground(FlatBufferBuilder... builders) {
        Server s = Server.instance();
        FlatBufferBuilder b = builders[0];
        try {
            return s.request(b);
        } catch (Exception e) {
            return null;
        }
    }
}
