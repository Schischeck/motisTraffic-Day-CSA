package de.motis_project.app.journey;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import de.motis_project.app.R;

public class JourneySummaryAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {
    private final int VIEW_TYPE_LOADING_SPINNER = 0;
    private final int VIEW_TYPE_JOURNEY_PREVIEW = 1;

    private final static Random rand = new Random();

    private final static int[] itemLayouts = {
            R.layout.journey_item,
            R.layout.journey_item_journey2,
            R.layout.journey_item_journey3,
            R.layout.journey_item_journey4,
            R.layout.journey_item_journey5
    };

    public static class JourneyViewHolder extends RecyclerView.ViewHolder {
        public JourneyViewHolder(View itemView) {
            super(itemView);
        }
    }

    public static class Data {
        public final String text;

        Data(String text) {
            this.text = text;
        }

        public static List<Data> createSome(int size) {
            List<Data> data = new ArrayList<>(size);
            for (int i = 0; i < size; i++) {
                data.add(new Data("" + i));
            }
            return data;
        }
    }

    private final List<Data> data;

    public JourneySummaryAdapter(List<Data> d) {
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
    public JourneyViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        Context context = parent.getContext();
        LayoutInflater inflater = LayoutInflater.from(context);

        switch (viewType) {
            case VIEW_TYPE_JOURNEY_PREVIEW:
                return new JourneyViewHolder(
                        inflater.inflate(itemLayouts[rand.nextInt(itemLayouts.length)], parent, false));
            case VIEW_TYPE_LOADING_SPINNER:
                return new JourneyViewHolder(
                        inflater.inflate(R.layout.journey_loading_spinner, parent, false));
            default:
                throw new RuntimeException("unknown view type");
        }
    }

    @Override
    public void onBindViewHolder(RecyclerView.ViewHolder viewHolder, int position) {
        if (getItemViewType(position) == VIEW_TYPE_JOURNEY_PREVIEW) {
            JourneyViewHolder journey = (JourneyViewHolder) viewHolder;
        }
    }

    @Override
    public int getItemCount() {
        return data.size() + 2;
    }
}
