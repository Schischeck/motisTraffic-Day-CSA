package de.motis_project.app.detail;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
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

public class StopOver implements DetailViewHolder {
    private View layout;

    @BindView(R.id.detail_stopover_stop_name)
    TextView stopName;

    @BindView(R.id.detail_stopover_delay)
    TextView delay;

    @BindView(R.id.detail_stopover_stop_time)
    TextView stopTime;

    @BindView(R.id.detail_stopover_vertline)
    View line;

    @BindView(R.id.detail_stopover_bullet)
    View bullet;

    @BindColor(R.color.delayed)
    int colorRed;

    @BindColor(R.color.ontime)
    int colorGreen;

    StopOver(Connection con, JourneyUtil.Section section, Stop stop, ViewGroup parent, LayoutInflater inflater) {
        layout = inflater.inflate(R.layout.detail_stopover, parent, false);
        ButterKnife.bind(this, layout);

        Context context = inflater.getContext();
        long clasz = JourneyUtil.getTransport(con, section).clasz();
        JourneyUtil.setBackgroundColor(context, line, clasz);
        JourneyUtil.tintBackground(context, bullet, clasz);

        EventInfo ev = stop.departure();

        stopName.setText(Str.san(stop.station().name()));
        stopTime.setText(TimeUtil.formatTime(ev.scheduleTime()));

        delay.setText(TimeUtil.delayString(stop.departure()));
        delay.setTextColor(TimeUtil.delay(stop.departure()) ? colorRed : colorGreen);
    }

    @Override
    public View getView() {
        return layout;
    }
}
