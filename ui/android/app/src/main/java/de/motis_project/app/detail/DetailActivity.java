package de.motis_project.app.detail;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Window;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashSet;

import butterknife.BindString;
import butterknife.BindView;
import butterknife.ButterKnife;
import de.motis_project.app.JourneyUtil;
import de.motis_project.app.R;
import de.motis_project.app.TimeUtil;
import de.motis_project.app.io.Status;
import de.motis_project.app.journey.CopyConnection;
import de.motis_project.app.saved.SavedConnectionsDataSource;
import motis.Connection;

public class DetailActivity extends AppCompatActivity {
    private Connection con;

    private HashSet<JourneyUtil.Section> expandedSections = new HashSet<>();

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
        create();
    }

    void create() {
        TransportBuilder.setConnection(getLayoutInflater(), journeyDetails, con, expandedSections);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        create();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                return true;
            case R.id.detail_save_connection:
                System.out.println("DetailActivity.onOptionsItemSelected --> SAVE CONNECTION");
                if (con != null) {
                    try {
                        CopyConnection.copyConnection(con);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                } else {
                    System.out.println("CON == NULL!");
                }
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
        travelDuration.setText(TimeUtil.formatDuration(minutes));

        int transferCount = getNumberOfTransfers(con);
        String transferPlural = (transferCount == 1) ? transfer : transfers;
        numberOfTransfers.setText(String.format(transferPlural, transferCount));
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.journey_detail_toolbar, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        MenuItem actionViewItem = menu.findItem(R.id.detail_save_connection);
        actionViewItem.setOnMenuItemClickListener(e -> {
            try (SavedConnectionsDataSource db = new SavedConnectionsDataSource(this)) {
                db.add(CopyConnection.copyConnection(con));
            }
            Toast.makeText(this, "saved", Toast.LENGTH_SHORT).show();
            return true;
        });
        return super.onPrepareOptionsMenu(menu);
    }

    private static int getNumberOfTransfers(Connection con) {
        int transfers = 0;
        for (int i = 0; i < con.stopsLength(); i++) {
            if (con.stops(i).exit()) {
                transfers++;
            }
        }
        return transfers - 1;
    }

    public void expandSection(JourneyUtil.Section section) {
        expandedSections.add(section);
        create();
    }

    public void contractSection(JourneyUtil.Section section) {
        expandedSections.remove(section);
        create();
    }
}
