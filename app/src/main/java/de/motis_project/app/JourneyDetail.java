package de.motis_project.app;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.Window;
import android.widget.TextView;

import java.text.SimpleDateFormat;
import java.util.Date;

import butterknife.BindString;
import butterknife.BindView;
import butterknife.ButterKnife;
import de.motis_project.app.io.Status;
import motis.Connection;

public class JourneyDetail extends AppCompatActivity {
    private Connection con;

    @BindString(R.string.transfer)
    String transfer;

    @BindString(R.string.transfers)
    String transfers;

    @BindView(R.id.detail_dep_station)
    TextView depStation;

    @BindView(R.id.detail_arr_station)
    TextView arrStation;

    @BindView(R.id.detail_dep_schedule_time)
    TextView depSchedTime;

    @BindView(R.id.detail_arr_schedule_time)
    TextView arrSchedTime;

    @BindView(R.id.detail_travel_duration)
    TextView travelDuration;

    @BindView(R.id.detail_number_of_transfers)
    TextView numberOfTransfers;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_ACTION_BAR);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.detail);
        setSupportActionBar((Toolbar) findViewById(R.id.journey_detail_toolbar));
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        con = Status.get().getConnection();

        String formattedDate = SimpleDateFormat
                .getDateInstance(java.text.DateFormat.SHORT)
                .format(new Date(con.stops(0).departure().scheduleTime() * 1000));
        setTitle(formattedDate);

        ButterKnife.bind(this);
        initHeader();
    }

    void initHeader() {
        depStation.setText(con.stops(0).station().name());
        arrStation.setText(con.stops(con.stopsLength() - 1).station().name());

        long depTime = con.stops(0).departure().scheduleTime();
        long arrTime = con.stops(con.stopsLength() - 1).arrival().scheduleTime();
        depSchedTime.setText(TimeUtil.formatTime(depTime));
        arrSchedTime.setText(TimeUtil.formatTime(arrTime));

        long minutes = (arrTime - depTime) / 60;
        travelDuration.setText(TimeUtil.getDurationString(minutes));

        int transferCount = getNumberOfTransfers(con);
        String transferPlural = (transferCount == 1) ? transfer : transfers;
        numberOfTransfers.setText(Integer.toString(transferCount) + " " + transferPlural);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.journey_detail_toolbar, menu);
        return super.onCreateOptionsMenu(menu);
    }

    private static int getNumberOfTransfers(Connection con) {
        int transfers = 0;
        for (int i = 0; i < con.stopsLength(); i++) {
            if (con.stops(i).interchange()) {
                transfers++;
            }
        }
        return transfers;
    }
}
