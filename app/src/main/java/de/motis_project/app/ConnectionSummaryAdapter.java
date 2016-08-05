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
        // Your holder should contain a member variable
        // for any view that will be set as you render a row
        public TextView nameTextView;
        public Button messageButton;

        // We also create a constructor that accepts the entire item row
        // and does the view lookups to find each subview
        public ViewHolder(View itemView) {
            // Stores the itemView in a public final member variable that can be used
            // to access the context from any ViewHolder instance.
            super(itemView);

            nameTextView = (TextView) itemView.findViewById(R.id.contact_name);
            messageButton = (Button) itemView.findViewById(R.id.message_button);
        }
    }

    public static class Data {
        private String text;

        Data(String text) {
            this.text = text;
        }

        public String getText() {
            return text;
        }

        public static List<Data> createSome(int size) {
            List<Data> data = new ArrayList<>(size);
            for (int i = 0; i < size; i++) {
                data.add(new Data("" + i));
            }
            return data;
        }
    }

    private List<Data> data;

    private Context context;

    public ConnectionSummaryAdapter(Context c, List<Data> d) {
        context = c;
        data = d;
    }

    public Context getContext() {
        return context;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        Context context = parent.getContext();
        LayoutInflater inflater = LayoutInflater.from(context);

        // Inflate the custom layout
        View summaryView = inflater.inflate(R.layout.item_connection, parent, false);

        // Return a new holder instance
        ViewHolder viewHolder = new ViewHolder(summaryView);
        return viewHolder;
    }

    @Override
    public void onBindViewHolder(ViewHolder viewHolder, int position) {
        Data d = data.get(position);

        // Set item views based on your views and data model
        TextView textView = viewHolder.nameTextView;
        textView.setText(d.getText());
        Button button = viewHolder.messageButton;
        button.setText("Message");
    }

    @Override
    public int getItemCount() {
        return data.size();
    }

}
