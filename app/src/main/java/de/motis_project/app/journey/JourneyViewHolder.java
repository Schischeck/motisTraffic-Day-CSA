package de.motis_project.app.journey;

import android.support.annotation.Nullable;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import de.motis_project.app.R;
import motis.Connection;
import motis.EventInfo;
import motis.Move;
import motis.MoveWrapper;
import motis.Transport;

public class JourneyViewHolder extends RecyclerView.ViewHolder {
    static private class Section {
        public Section(int from, int to) {
            this.from = from;
            this.to = to;
        }

        int from, to;
    }

    static private class DisplayTransport {
        final long clasz;
        final String longName;
        final String shortName;

        public DisplayTransport(Transport t) {
            longName = t.name();
            shortName = getShortName(t);
            clasz = t.clasz();
        }

        private static String getShortName(Transport t) {
            if (t.name().length() < 7) {
                return t.name();
            } else if (t.lineId().isEmpty() && t.trainNr() == 0) {
                return t.name();
            } else if (t.lineId().isEmpty()) {
                return Long.toString(t.trainNr());
            } else {
                return t.lineId();
            }
        }
    }

    private final LayoutInflater inflater;

    static StringBuffer durationBuf = new StringBuffer();

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

    final boolean bound;

    public JourneyViewHolder(boolean bind, View view, LayoutInflater inflater) {
        super(view);
        this.bound = bind;
        this.inflater = inflater;
        if (bind) {
            ButterKnife.bind(this, view);
        }
    }

    void setConnection(Connection con) {
        if (!bound) {
            return;
        }

        EventInfo dep = con.stops(0).departure();
        EventInfo arr = con.stops(con.stopsLength() - 1).arrival();

        long minutes = (arr.scheduleTime() - dep.scheduleTime()) / 60;
        duration.setText(getDurationString(minutes));

        depSchedTime.setText(formatTime(dep.scheduleTime()));
        depTime.setText(formatTime(dep.time()));

        arrSchedTime.setText(formatTime(arr.scheduleTime()));
        arrTime.setText(formatTime(arr.time()));

        transports.removeAllViews();
        addTransportViews(getTransports(con));
    }

    void addTransportViews(List<DisplayTransport> transports) {
        for (int i = 0; i < transports.size(); ++i) {
            DisplayTransport t = transports.get(i);

            TextView view = (TextView) inflater.inflate(
                    t.clasz > 3
                        ? R.layout.journey_item_transport_bus
                        : R.layout.journey_item_transport_train,
                    this.transports, false);
            if (transports.size() < 6) {
                view.setText(transports.size() > 3 ? t.shortName : t.longName);
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

    static List<DisplayTransport> getTransports(Connection con) {
        int lastSectionEnd = 0;
        List<Section> sections = new ArrayList<>();
        for (int i = 0; i < con.stopsLength(); i++) {
            if (con.stops(i).interchange() || i == con.stopsLength() - 1) {
                sections.add(new Section(lastSectionEnd, i));
                lastSectionEnd = i;
            }
        }

        List<DisplayTransport> displayTransports = new ArrayList<>();
        for (Section s : sections) {
            Transport t = getTransportForSection(con, s);
            if (t != null) {
                displayTransports.add(new DisplayTransport(t));
            }
        }

        return displayTransports;
    }

    @Nullable
    static Transport getTransportForSection(Connection c, Section s) {
        for (int i = 0; i < c.transportsLength(); ++i) {
            MoveWrapper m = c.transports(i);
            if (m.moveType() == Move.Transport) {
                Transport t = new Transport();
                m.move(t);

                if (t.range().from() <= s.to && t.range().to() > s.from) {
                    return t;
                }
            }
        }
        return null;
    }

    static String formatTime(long timestamp) {
        return SimpleDateFormat
                .getTimeInstance(java.text.DateFormat.SHORT)
                .format(new Date(timestamp * 1000));
    }

    static String getDurationString(long minutes) {
        durationBuf.setLength(0);

        long displayMinutes = minutes % 60;
        long displayHours = minutes / 60;

        if (displayHours != 0) {
            durationBuf.append(displayHours).append("h ");
        }
        durationBuf.append(displayMinutes).append("min");

        return durationBuf.toString();
    }
}
