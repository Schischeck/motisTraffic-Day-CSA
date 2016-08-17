package de.motis_project.app;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.Window;

import java.text.SimpleDateFormat;
import java.util.Date;

import de.motis_project.app.io.Status;
import motis.Connection;

public class JourneyDetail extends AppCompatActivity {
    private Connection conn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_ACTION_BAR);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.journey_detail);
        setSupportActionBar((Toolbar) findViewById(R.id.journey_detail_toolbar));
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        conn = Status.get().getConnection();

        String formattedDate = SimpleDateFormat
                .getDateInstance(java.text.DateFormat.SHORT)
                .format(new Date(conn.stops(0).departure().scheduleTime() * 1000));
        setTitle(formattedDate);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.journey_detail_toolbar, menu);
        return super.onCreateOptionsMenu(menu);
    }
}
