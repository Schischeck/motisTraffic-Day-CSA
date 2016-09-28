package de.motis_project.app.detail;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import java.util.List;

import butterknife.BindString;
import butterknife.BindView;
import butterknife.ButterKnife;
import de.motis_project.app.JourneyUtil;
import de.motis_project.app.R;
import de.motis_project.app.Str;
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

        Context context = inflater.getContext();
        long clasz = JourneyUtil.getTransport(con, JourneyUtil.getSections(con).get(0)).clasz();
        JourneyUtil.tintBackground(context, transportName, clasz);
        JourneyUtil.setIcon(context, transportName, clasz);

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

        return Str.san(transport.name());
    }

    private static String getTrackName(Connection con) {
        if (con.stopsLength() == 0) {
            return "";
        } else {
            return Str.san(con.stops(0).departure().track());
        }
    }
}
