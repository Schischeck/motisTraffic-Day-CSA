package de.motis_project.app.journey;

import android.content.Context;
import android.content.Intent;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import de.motis_project.app.R;
import de.motis_project.app.TimeUtil;
import de.motis_project.app.detail.JourneyDetail;
import de.motis_project.app.io.Status;
import de.motis_project.app.lib.StickyHeaderAdapter;
import motis.Connection;

public class JourneySummaryAdapter
        extends RecyclerView.Adapter<JourneyViewHolder>
        implements StickyHeaderAdapter<JourneyViewHolder> {
    private static class HeaderMapping {
        private final Map<Date, Integer> map = new HashMap<>();

        HeaderMapping(List<Connection> data) {
            int nextId = 0;
            for (Connection con : data) {
                Date date = normalizeDate(con);
                if (!map.containsKey(date)) {
                    map.put(date, nextId);
                    ++nextId;
                }
            }
        }

        public int getHeaderIndex(Connection con) {
            return map.get(normalizeDate(con));
        }

        public static Date normalizeDate(Connection con) {
            Calendar cal = Calendar.getInstance();
            cal.setTime(new Date(con.stops(0).departure().time() * 1000));
            cal.set(Calendar.HOUR_OF_DAY, 0);
            cal.set(Calendar.MINUTE, 0);
            cal.set(Calendar.SECOND, 0);
            cal.set(Calendar.MILLISECOND, 0);
            return cal.getTime();
        }

        public int getLastIndex() {
            return map.size() - 1;
        }
    }

    private final int VIEW_TYPE_LOADING_SPINNER = 0;
    private final int VIEW_TYPE_JOURNEY_PREVIEW = 1;

    private final List<Connection> data;

    private HeaderMapping headerMapping;

    public JourneySummaryAdapter(List<Connection> d) {
        data = d;
        recalculateHeaders();
    }

    @Override
    public int getItemViewType(int position) {
        if (position == 0 || position == getItemCount() - 1) {
            return VIEW_TYPE_LOADING_SPINNER;
        } else {
            return VIEW_TYPE_JOURNEY_PREVIEW;
        }
    }

    @Override
    public JourneyViewHolder onCreateViewHolder(ViewGroup parent,
                                                int viewType) {
        Context context = parent.getContext();
        LayoutInflater inflater = LayoutInflater.from(context);

        switch (viewType) {
            case VIEW_TYPE_JOURNEY_PREVIEW:
                return new JourneyViewHolder(
                        true,
                        inflater.inflate(
                                R.layout.journey_list_item,
                                parent, false), inflater);
            case VIEW_TYPE_LOADING_SPINNER:
                return new JourneyViewHolder(
                        false,
                        inflater.inflate(
                                R.layout.journey_loading_spinner,
                                parent, false), inflater);
            default:
                throw new RuntimeException("unknown view type");
        }
    }

    @Override
    public int getItemCount() {
        return data.size() + 2;
    }

    @Override
    public long getHeaderId(int position) {
        if (position == 0 || data.size() == 0) {
            return 0;
        }
        if (position == data.size() + 1) {
            return headerMapping.getLastIndex();
        }

        return headerMapping.getHeaderIndex(data.get(position - 1));
    }

    @Override
    public void onBindViewHolder(JourneyViewHolder viewHolder, final int position) {
        final int index = position - 1;
        if (index < 0 || index >= data.size()) {
            return;
        }

        final Connection con = data.get(index);
        viewHolder.setConnection(con);
        viewHolder.itemView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Status.get().setConnection(con);
                view.getContext().startActivity(new Intent(view.getContext(), JourneyDetail.class));
            }
        });
    }

    @Override
    public JourneyViewHolder onCreateHeaderViewHolder(ViewGroup parent) {
        Context context = parent.getContext();
        LayoutInflater inflater = LayoutInflater.from(context);
        View header = inflater.inflate(R.layout.journey_header_item, parent, false);
        return new JourneyViewHolder(false, header, inflater);
    }

    @Override
    public void onBindHeaderViewHolder(JourneyViewHolder viewholder, int position) {
        if (data.isEmpty()) {
            System.out.println("NO DATA");
            return;
        }

        final int index = Math.min(data.size() - 1, Math.max(0, position - 1));

        Connection con = data.get(index);

        String depStation = con.stops(0).station().name();
        TextView headerDepText = (TextView) viewholder.itemView.findViewById(R.id.journey_header_departure_text);
        headerDepText.setText(depStation);

        String arrStation = con.stops(con.stopsLength() - 1).station().name();
        TextView headerArrText = (TextView) viewholder.itemView.findViewById(R.id.journey_header_arrival_text);
        headerArrText.setText(arrStation);

        TextView headerDate = (TextView) viewholder.itemView.findViewById(R.id.journey_header_date);
        headerDate.setText(TimeUtil.formatDate(con.stops(0).departure().time()));
    }

    public void recalculateHeaders() {
        headerMapping = new HeaderMapping(data);
    }

}
