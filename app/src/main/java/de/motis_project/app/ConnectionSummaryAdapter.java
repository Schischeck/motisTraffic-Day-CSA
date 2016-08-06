package de.motis_project.app;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

public class ConnectionSummaryAdapter
        extends RecyclerView.Adapter<ConnectionSummaryAdapter.ViewHolder> {
    public static class ViewHolder extends RecyclerView.ViewHolder {
        public TextView nameTextView;
        public Button messageButton;

        public ViewHolder(View itemView) {
            super(itemView);
            nameTextView = (TextView) itemView.findViewById(R.id.contact_name);
            messageButton = (Button) itemView.findViewById(R.id.message_button);
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

    public ConnectionSummaryAdapter(List<Data> d) {
        data = d;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        Context context = parent.getContext();
        LayoutInflater inflater = LayoutInflater.from(context);
        View summaryView = inflater.inflate(R.layout.item_connection, parent, false);
        return new ViewHolder(summaryView);
    }

    @Override
    public void onBindViewHolder(ViewHolder viewHolder, int position) {
        Data d = data.get(position);
        viewHolder.nameTextView.setText(d.text);
        viewHolder.messageButton.setText("Message");
    }

    @Override
    public int getItemCount() {
        return data.size();
    }
}
