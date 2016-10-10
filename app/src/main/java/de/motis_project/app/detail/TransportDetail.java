package de.motis_project.app.detail;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;
import de.motis_project.app.JourneyUtil;
import de.motis_project.app.R;
import de.motis_project.app.Str;
import de.motis_project.app.TimeUtil;
import motis.Connection;
import motis.EventInfo;
import motis.Stop;
import motis.TimestampReason;
import motis.Transport;

public class TransportDetail implements DetailViewHolder {
    private View layout;

    @BindView(R.id.detail_transport_dep_station)
    TextView station;

    @BindView(R.id.detail_transport_dep_time)
    TextView time;

    @BindView(R.id.detail_transpot_dep_delay)
    TextView delay;

    @BindView(R.id.detail_transport_direction_container)
    LinearLayout directionContainer;

    @BindView(R.id.detail_transport_direction)
    TextView direction;

    @BindView(R.id.detail_transport_vertline)
    View line;

    @BindColor(R.color.delayed)
    int colorRed;

    @BindColor(R.color.ontime)
    int colorGreen;

    TransportDetail(Connection con,
                    JourneyUtil.Section section,
                    ViewGroup parent,
                    LayoutInflater inflater) {
        layout = inflater.inflate(R.layout.detail_transport, parent, false);
        ButterKnife.bind(this, layout);

        Stop stop = con.stops(section.from);
        station.setText(stop.station().name());

        time.setText(TimeUtil.formatTime(stop.departure().scheduleTime()));

        long clasz = JourneyUtil.getTransport(con, section).clasz();
        JourneyUtil.setBackgroundColor(inflater.getContext(), line, clasz);

        String dir = getDirection(con, section);
        if (dir.isEmpty()) {
            directionContainer.setVisibility(View.GONE);
        } else {
            direction.setText(dir);
        }

        EventInfo dep = stop.departure();
        delay.setText(TimeUtil.delayString(dep));
        delay.setTextColor(TimeUtil.delay(dep) ? colorRed : colorGreen);
    }

    private static String getDirection(Connection con, JourneyUtil.Section s) {
        Transport transport = JourneyUtil.getTransport(con, s);
        return (transport == null) ? "" : Str.san(transport.direction());
    }

    @Override
    public View getView() {
        return layout;
    }
}
