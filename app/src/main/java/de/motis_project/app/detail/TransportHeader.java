package de.motis_project.app.detail;

import android.content.Context;
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
import motis.Transport;

public class TransportHeader implements DetailViewHolder {
    private View layout;

    @BindString(R.string.arrival_short)
    String arrivalShort;

    @BindString(R.string.interchange)
    String interchange;

    @BindString(R.string.track_short)
    String trackShort;

    @BindString(R.string.track)
    String track;

    @BindView(R.id.detail_transport_header_transport_name)
    TextView transportName;

    @BindView(R.id.detail_transport_header_interchange)
    TextView interchangeInfo;

    @BindView(R.id.detail_transport_header_track)
    TextView depTrack;

    TransportHeader(Connection con,
                    JourneyUtil.Section prevSection,
                    JourneyUtil.Section section,
                    ViewGroup parent,
                    LayoutInflater inflater) {
        layout = inflater.inflate(R.layout.detail_transport_header, parent, false);
        ButterKnife.bind(this, layout);

        Context context = inflater.getContext();
        long clasz = JourneyUtil.getTransport(con, section).clasz();
        JourneyUtil.tintBackground(context, transportName, clasz);
        JourneyUtil.setIcon(context, transportName, clasz);

        Transport transport = JourneyUtil.getTransport(con, section);
        if (transport != null) {
            transportName.setText(Str.san(transport.name()));
        }

        long arr = con.stops(prevSection.to).arrival().scheduleTime();
        long dep = con.stops(section.from).departure().scheduleTime();
        String durationStr = TimeUtil.formatDuration((dep - arr) / 60);
        String arrTrackName = con.stops(prevSection.to).arrival().track();
        System.out.println("arrTrackName = " + arrTrackName);
        System.out.println("durationStr = " + durationStr);
        if (arrTrackName == null || arrTrackName.isEmpty()) {
            interchangeInfo.setText(String.format(interchange, durationStr));
        } else {
            arrTrackName = arrivalShort + " " +  trackShort + " " + arrTrackName;
            System.out.println(durationStr);
            interchangeInfo.setText(
                    String.format(interchange, arrTrackName + ", " + durationStr));
        }

        String depTrackName = con.stops(section.from).departure().track();
        if (depTrackName == null || depTrackName.isEmpty()) {
            depTrack.setVisibility(View.GONE);
        } else {
            depTrack.setText(String.format(track, depTrackName));
        }
    }

    @Override
    public View getView() {
        return layout;
    }
}
