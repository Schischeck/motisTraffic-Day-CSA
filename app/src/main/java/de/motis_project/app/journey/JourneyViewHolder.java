package de.motis_project.app.journey;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.support.v4.content.ContextCompat;
import android.support.v4.graphics.drawable.DrawableCompat;
import android.support.v4.util.LongSparseArray;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import de.motis_project.app.JourneyUtil;
import de.motis_project.app.R;
import de.motis_project.app.TimeUtil;
import motis.Connection;
import motis.EventInfo;

public class JourneyViewHolder extends RecyclerView.ViewHolder {
    private final LayoutInflater inflater;

    @BindView(R.id.dep_sched_time)
    TextView depSchedTime;

    @BindView(R.id.dep_time)
    TextView depTime;

    @BindView(R.id.arr_sched_time)
    TextView arrSchedTime;

    @BindView(R.id.arr_time)
    TextView arrTime;

    @BindView(R.id.duration)
    TextView duration;

    @BindView(R.id.transports)
    LinearLayout transports;

    final boolean bound;

    private static final LongSparseArray<Integer> colors = new LongSparseArray<>();

    static {
        colors.put(0, R.color.colorPrimary);
        colors.put(1, R.color.colorPrimaryDark);
        colors.put(2, R.color.colorPrimaryLight);
        colors.put(3, R.color.teal_500);
    }

    private static final LongSparseArray<Integer> icons = new LongSparseArray<>();

    static {
        icons.put(0, R.drawable.ic_directions_railway_black_24dp);
        icons.put(1, R.drawable.ic_directions_railway_black_24dp);
        icons.put(2, R.drawable.ic_directions_railway_black_24dp);
        icons.put(3, R.drawable.ic_directions_railway_black_24dp);
    }

    public JourneyViewHolder(boolean bind, View view, LayoutInflater inflater) {
        super(view);
        this.bound = bind;
        this.inflater = inflater;
        if (bind) {
            ButterKnife.bind(this, view);
        }
    }

    void setConnection(Connection con) {
        if (!bound) {
            return;
        }

        EventInfo dep = con.stops(0).departure();
        EventInfo arr = con.stops(con.stopsLength() - 1).arrival();

        long minutes = (arr.scheduleTime() - dep.scheduleTime()) / 60;
        duration.setText(TimeUtil.formatDuration(minutes));

        depSchedTime.setText(TimeUtil.formatTime(dep.scheduleTime()));
        depTime.setText(TimeUtil.formatTime(dep.time()));

        arrSchedTime.setText(TimeUtil.formatTime(arr.scheduleTime()));
        arrTime.setText(TimeUtil.formatTime(arr.time()));

        transports.removeAllViews();
        addTransportViews(JourneyUtil.getTransports(con));
    }

    void addTransportViews(List<JourneyUtil.DisplayTransport> transports) {
        for (int i = 0; i < transports.size(); i++) {
            JourneyUtil.DisplayTransport t = transports.get(i);

            TextView view = (TextView) inflater.inflate(R.layout.journey_item_transport_train,
                    this.transports, false);

            Context context = inflater.getContext();

            Drawable bg = DrawableCompat.wrap(view.getBackground());
            DrawableCompat.setTint(bg.mutate(), getColor(context, t.clasz));
            view.setBackground(bg);

            view.setCompoundDrawablesWithIntrinsicBounds(getIcon(context, t.clasz), 0, 0, 0);

            if (transports.size() < 6) {
                view.setText(transports.size() > 3 ? t.shortName : t.longName);
            }
            this.transports.addView(view);

            if (i != transports.size() - 1) {
                this.transports.addView(inflater.inflate(
                        R.layout.journey_item_transport_separator,
                        this.transports,
                        false));
            }
        }
    }

    static int getColor(Context c, long clasz) {
        int id = colors.get(clasz, R.color.colorAccent);
        return ContextCompat.getColor(c, id);
    }

    static int getIcon(Context c, long clasz) {
        return icons.get(clasz, R.drawable.ic_directions_bus_black_24dp);
    }
}