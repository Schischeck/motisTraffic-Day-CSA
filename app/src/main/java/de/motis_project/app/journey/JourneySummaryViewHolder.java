package de.motis_project.app.journey;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.List;

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

        transports.removeAllViews();
        addTransportViews(JourneyUtil.getTransports(con));
    }

    void addTransportViews(List<JourneyUtil.DisplayTransport> transports) {
        ViewMode viewMode = getViewMode(transports);
        for (int i = 0; i < transports.size(); i++) {
            JourneyUtil.DisplayTransport t = transports.get(i);
            TextView view = (TextView) inflater.inflate(R.layout.journey_item_transport_train,
                                                        this.transports, false);

            Context context = inflater.getContext();
            JourneyUtil.tintBackground(context, view, t.clasz);
            JourneyUtil.setIcon(context, view, t.clasz);

            if (viewMode != ViewMode.OFF) {
                view.setText(viewMode == ViewMode.SHORT ? t.shortName : t.longName);
            }
            this.transports.addView(view);

            if (i != transports.size() - 1) {
                this.transports.addView(inflater.inflate(
                        R.layout.journey_item_transport_separator,
                        this.transports,
                        false));
            }
        }
    }

    static private ViewMode getViewMode(List<JourneyUtil.DisplayTransport> transports) {
        final int MAX_SIZE = 25;
        if (getTextLengthSum(transports, ViewMode.LONG) + transports.size() <= MAX_SIZE) {
            return ViewMode.LONG;
        } else if (getTextLengthSum(transports, ViewMode.SHORT) + transports.size() <= MAX_SIZE) {
            return ViewMode.SHORT;
        } else {
            return ViewMode.OFF;
        }
    }

    static private int getTextLengthSum(List<JourneyUtil.DisplayTransport> transports, ViewMode mode) {
        StringBuffer buf = new StringBuffer();
        for (JourneyUtil.DisplayTransport t : transports) {
            buf.append(mode == ViewMode.LONG ? t.longName : t.shortName);
        }
        return buf.toString().length();
    }
}