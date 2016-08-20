package de.motis_project.app.detail;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import butterknife.BindView;
import butterknife.ButterKnife;
import de.motis_project.app.JourneyUtil;
import de.motis_project.app.R;
import de.motis_project.app.Str;
import de.motis_project.app.TimeUtil;
import motis.Connection;
import motis.Stop;
import motis.Transport;

public class TransportDetail {
    View layout;

    @BindView(R.id.detail_transport_dep_station)
    TextView station;

    @BindView(R.id.detail_transport_dep_time)
    TextView time;

    @BindView(R.id.detail_transport_direction_container)
    LinearLayout directionContainer;

    @BindView(R.id.detail_transport_direction)
    TextView direction;

    TransportDetail(Connection con,
                    JourneyUtil.Section section,
                    ViewGroup parent,
                    LayoutInflater inflater) {
        layout = inflater.inflate(R.layout.detail_transport, parent, false);
        ButterKnife.bind(this, layout);

        Stop stop = con.stops(section.from);
        station.setText(stop.station().name());

        time.setText(TimeUtil.formatTime(stop.departure().scheduleTime()));

        String dir = getDirection(con, section);
        if (dir.isEmpty()) {
            directionContainer.setVisibility(View.GONE);
        } else {
            direction.setText(dir);
        }
    }

    private static String getDirection(Connection con, JourneyUtil.Section s) {
        Transport transport = JourneyUtil.getTransport(con, s);
        return (transport == null) ? "" : Str.san(transport.direction());
    }
}
