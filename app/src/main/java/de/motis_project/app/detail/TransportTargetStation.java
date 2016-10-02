package de.motis_project.app.detail;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import butterknife.BindView;
import butterknife.ButterKnife;
import de.motis_project.app.JourneyUtil;
import de.motis_project.app.R;
import de.motis_project.app.TimeUtil;
import motis.Connection;
import motis.Stop;

public class TransportTargetStation implements DetailViewHolder {
    private View layout;

    @BindView(R.id.detail_transport_target_station_arr_time)
    TextView arrivalTime;

    @BindView(R.id.detail_transport_target_station_delay)
    TextView arrivalDelay;

    @BindView(R.id.detail_transport_target_station)
    TextView targetStation;

    @BindView(R.id.detail_transport_target_station_vertline)
    View line;

    TransportTargetStation(Connection con,
                           JourneyUtil.Section section,
                           ViewGroup parent,
                           LayoutInflater inflater) {
        layout = inflater.inflate(R.layout.detail_transport_target_stop, parent, false);
        ButterKnife.bind(this, layout);

        long clasz = JourneyUtil.getTransport(con, section).clasz();
        JourneyUtil.tintBackground(inflater.getContext(), line, clasz);

        Stop stop = con.stops(section.to);

        arrivalTime.setText(TimeUtil.formatTime(stop.arrival().scheduleTime()));
        // TODO(felix) arrivalDelay.setText(...);
        targetStation.setText(stop.station().name());
    }

    @Override
    public View getView() {
        return layout;
    }
}
