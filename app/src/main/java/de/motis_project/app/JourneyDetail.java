package de.motis_project.app;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.Window;

public class JourneyDetail extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_ACTION_BAR);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.journey_detail);
        setSupportActionBar((Toolbar) findViewById(R.id.journey_detail_toolbar));
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        setTitle("So. 14.08.2016");
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.journey_detail_toolbar, menu);
        return super.onCreateOptionsMenu(menu);
    }
}
