package de.motis_project.app.detail;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Window;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.text.SimpleDateFormat;
import java.util.Date;

import butterknife.BindString;
import butterknife.BindView;
import butterknife.ButterKnife;
import de.motis_project.app.JourneyUtil;
import de.motis_project.app.R;
import de.motis_project.app.TimeUtil;
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

    @BindView(R.id.detail_journey_details)
    LinearLayout journeyDetails;

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

        LayoutInflater inflater = getLayoutInflater();
        journeyDetails.addView(
                new FirstTransportHeader(con, journeyDetails, inflater).layout, 0);
        journeyDetails.addView(
                new TransportDetail(
                        con, JourneyUtil.getSections(con).get(0), journeyDetails,
                        inflater).layout, 1);
        journeyDetails.addView(
                new TransportStops(
                        con, JourneyUtil.getSections(con).get(0), journeyDetails,
                        inflater).layout, 2);
        journeyDetails.addView(
                new TransportTargetStation(
                        con, JourneyUtil.getSections(con).get(0), journeyDetails,
                        inflater).layout, 3);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
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
        numberOfTransfers.setText(String.format(transferPlural, transferCount));
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
