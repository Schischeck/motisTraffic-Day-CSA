package de.motis_project.app.detail;

import android.graphics.drawable.Drawable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import butterknife.BindDrawable;
import butterknife.BindString;
import butterknife.BindView;
import butterknife.ButterKnife;
import de.motis_project.app.JourneyUtil;
import de.motis_project.app.R;
import de.motis_project.app.TimeUtil;
import motis.Connection;

public class TransportStops {
    View layout;

    @BindString(R.string.detail_transport_stops_summary)
    String summaryTemplate;

    @BindString(R.string.stop)
    String stop;

    @BindString(R.string.stops)
    String stops;

    @BindView(R.id.detail_transport_stops_upper)
    ImageView upper;

    @BindView(R.id.detail_transport_stops_lower)
    ImageView lower;

    @BindView(R.id.detail_transport_stops_summary)
    TextView summary;

    @BindDrawable(R.drawable.ic_expand_less_black_24dp)
    Drawable less;

    @BindDrawable(R.drawable.ic_expand_more_black_24dp)
    Drawable more;

    TransportStops(Connection con,
                   JourneyUtil.Section section,
                   ViewGroup parent,
                   LayoutInflater inflater) {
        layout = inflater.inflate(R.layout.detail_transport_stops, parent, false);
        ButterKnife.bind(this, layout);

        setExpanded(false);

        long dep = con.stops(section.from).departure().scheduleTime();
        long arr = con.stops(section.to).arrival().scheduleTime();
        long durationMinutes = (arr - dep) / 60;
        int numStops = section.to - section.from;
        summary.setText(
                String.format(summaryTemplate,
                              section.to - section.from,
                              numStops == 1 ? stop : stops,
                              TimeUtil.getDurationString(durationMinutes)));
    }

    void setExpanded(boolean expaneded) {
        if (expaneded) {
            upper.setImageDrawable(more);
            lower.setImageDrawable(less);
        } else {
            upper.setImageDrawable(less);
            lower.setImageDrawable(more);
        }
    }
}
