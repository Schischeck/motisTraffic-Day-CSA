package de.motis_project.app.journey;

import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import butterknife.BindColor;
import butterknife.BindView;
import de.motis_project.app.JourneyUtil;
import de.motis_project.app.R;
import de.motis_project.app.TimeUtil;
import motis.Connection;
import motis.EventInfo;
import motis.TimestampReason;

public class JourneySummaryViewHolder extends JourneyViewHolder {
    enum ViewMode {
        LONG,
        SHORT,
        OFF
    }

    @BindView(R.id.dep_sched_time)
    TextView depSchedTime;

    @BindView(R.id.dep_time)
    TextView depTime;

    @BindView(R.id.arr_sched_time)
    TextView arrSchedTime;

    @BindView(R.id.arr_time)
    TextView arrTime;

    @BindView(R.id.duration)
    TextView duration;

    @BindView(R.id.transports)
    LinearLayout transports;

    @BindColor(R.color.delayed)
    int colorRed;

    @BindColor(R.color.ontime)
    int colorGreen;

    public JourneySummaryViewHolder(ViewGroup parent, LayoutInflater inflater) {
        super(inflater.inflate(R.layout.journey_list_item, parent, false), inflater);
    }


    void setConnection(Connection con) {
        EventInfo dep = con.stops(0).departure();
        EventInfo arr = con.stops(con.stopsLength() - 1).arrival();

        long minutes = (arr.scheduleTime() - dep.scheduleTime()) / 60;
        duration.setText(TimeUtil.formatDuration(minutes));

        depSchedTime.setText(TimeUtil.formatTime(dep.scheduleTime()));
        depTime.setText(TimeUtil.formatTime(dep.time()));

        arrSchedTime.setText(TimeUtil.formatTime(arr.scheduleTime()));
        arrTime.setText(TimeUtil.formatTime(arr.time()));

        if (dep.reason() != TimestampReason.SCHEDULE) {
            depTime.setTextColor(TimeUtil.delay(dep) ? colorRed : colorGreen);
        }
        if (arr.reason() != TimestampReason.SCHEDULE) {
            arrTime.setTextColor(TimeUtil.delay(arr) ? colorRed : colorGreen);
        }

        TransportViewCreator.addTransportViews(JourneyUtil.getTransports(con), inflater, transports);
    }
}