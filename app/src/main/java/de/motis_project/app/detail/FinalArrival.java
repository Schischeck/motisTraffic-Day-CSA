package de.motis_project.app.detail;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import butterknife.BindString;
import butterknife.BindView;
import butterknife.ButterKnife;
import de.motis_project.app.JourneyUtil;
import de.motis_project.app.R;
import de.motis_project.app.Str;
import de.motis_project.app.TimeUtil;
import motis.Connection;
import motis.EventInfo;
import motis.Stop;

public class FinalArrival implements DetailViewHolder {
    private View layout;

    @BindString(R.string.track)
    String track;

    @BindView(R.id.detail_final_arrival_time)
    TextView arrivalTime;

    @BindView(R.id.detail_final_arrival_station)
    TextView arrivalStation;

    @BindView(R.id.detail_final_arrival_track)
    TextView arrivalTrack;

    FinalArrival(Connection con,
                 JourneyUtil.Section section,
                 ViewGroup parent,
                 LayoutInflater inflater) {
        layout = inflater.inflate(R.layout.detail_final_arrival, parent, false);
        ButterKnife.bind(this, layout);

        Stop stop = con.stops(section.to);
        EventInfo arr = stop.arrival();
        arrivalTime.setText(TimeUtil.formatTime(arr.scheduleTime()));
        arrivalStation.setText(stop.station().name());

        String arrTrackStr = Str.san(arr.track());
        if (arrTrackStr.isEmpty()) {
            arrivalTrack.setVisibility(View.GONE);
        } else {
            arrivalTrack.setText(String.format(track, arrTrackStr));
        }
        // TODO(felix) arrival time delay
    }

    @Override
    public View getView() {
        return layout;
    }
}
