package de.motis_project.app2.detail;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;
import de.motis_project.app2.JourneyUtil;
import de.motis_project.app2.R;
import de.motis_project.app2.TimeUtil;
import motis.Connection;
import motis.EventInfo;
import motis.Stop;

public class TransportTargetStation implements DetailViewHolder {
    private View layout;

    @BindView(R.id.detail_transport_target_station_arr_time) TextView arrivalTime;
    @BindView(R.id.detail_transport_target_station_delay) TextView arrivalDelay;
    @BindView(R.id.detail_transport_target_station) TextView targetStation;
    @BindView(R.id.detail_transport_target_station_vertline) View line;

    @BindColor(R.color.delayed) int colorRed;
    @BindColor(R.color.ontime) int colorGreen;

    TransportTargetStation(Connection con,
                           JourneyUtil.Section section,
                           ViewGroup parent,
                           LayoutInflater inflater) {
        layout = inflater.inflate(R.layout.detail_transport_target_stop, parent, false);
        ButterKnife.bind(this, layout);

        long clasz = JourneyUtil.getTransport(con, section).clasz();
        JourneyUtil.setBackgroundColor(inflater.getContext(), line, clasz);

        Stop stop = con.stops(section.to);

        arrivalTime.setText(TimeUtil.formatTime(stop.arrival().scheduleTime()));
        targetStation.setText(stop.station().name());

        EventInfo arr = stop.arrival();
        arrivalDelay.setText(TimeUtil.delayString(arr));
        arrivalDelay.setTextColor(TimeUtil.delay(arr) ? colorRed : colorGreen);
        arrivalDelay.setVisibility(View.VISIBLE);
    }

    @Override
    public View getView() {
        return layout;
    }
}
