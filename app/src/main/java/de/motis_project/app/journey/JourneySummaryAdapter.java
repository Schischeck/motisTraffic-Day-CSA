package de.motis_project.app.journey;

import android.content.Context;
import android.content.Intent;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import java.util.List;

import de.motis_project.app.detail.JourneyDetail;
import de.motis_project.app.R;
import de.motis_project.app.io.Status;
import de.motis_project.app.lib.StickyHeaderAdapter;
import motis.Connection;

public class JourneySummaryAdapter
        extends RecyclerView.Adapter<JourneyViewHolder>
        implements StickyHeaderAdapter<JourneyViewHolder> {
    private final int VIEW_TYPE_LOADING_SPINNER = 0;
    private final int VIEW_TYPE_JOURNEY_PREVIEW = 1;

    private final List<Connection> data;

    public JourneySummaryAdapter(List<Connection> d) {
        data = d;
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
        return 0;
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
        return new JourneyViewHolder(false,
                                     inflater.inflate(
                                             R.layout.journey_header_item,
                                             parent, false), inflater);
    }

    @Override
    public void onBindHeaderViewHolder(JourneyViewHolder viewholder,
                                       int position) {
    }
}
