package de.motis_project.app;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import java.util.List;

import butterknife.BindString;
import butterknife.BindView;
import butterknife.ButterKnife;
import motis.Connection;
import motis.Transport;

public class FirstTransportHeader {
    View layout;

    @BindString(R.string.track)
    String track;

    @BindView(R.id.detail_first_transport_name)
    TextView transportName;

    @BindView(R.id.detail_first_transport_track)
    TextView depTrack;

    FirstTransportHeader(Connection con, ViewGroup parent, LayoutInflater inflater) {
        layout = inflater.inflate(
                R.layout.detail_first_transport_header, parent, false);
        ButterKnife.bind(this, layout);

        transportName.setText(getTransportName(con));

        String trackName = getTrackName(con);
        if (trackName == null || trackName.isEmpty()) {
            depTrack.setVisibility(View.GONE);
        } else {
            depTrack.setText(String.format(track, trackName));
        }
    }

    private static String getTransportName(Connection con) {
        List<JourneyUtil.Section> sections = JourneyUtil.getSections(con);
        if (sections.isEmpty()) {
            return "";
        }

        Transport transport = JourneyUtil.getTransport(con, sections.get(0));
        if (transport == null) {
            return "";
        }

        return san(transport.name());
    }

    private static String getTrackName(Connection con) {
        if (con.stopsLength() == 0) {
            return "";
        } else {
            return san(con.stops(0).departure().track());
        }
    }

    private static String san(String s) {
        return s == null ? "" : s;
    }
}
