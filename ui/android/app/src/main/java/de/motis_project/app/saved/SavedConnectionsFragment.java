package de.motis_project.app.saved;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import java.util.List;

import de.motis_project.app.JourneyUtil;
import de.motis_project.app.R;
import de.motis_project.app.TimeUtil;
import de.motis_project.app.detail.DetailActivity;
import de.motis_project.app.io.Status;
import de.motis_project.app.journey.TransportViewCreator;
import motis.Connection;
import motis.Stop;
import rx.Subscriber;
import rx.Subscription;

public class SavedConnectionsFragment extends Fragment {
    static private class ConnectionAdapter extends BaseAdapter {
        private final LayoutInflater inflater;
        private final List<Connection> connections;

        public ConnectionAdapter(Context context, List<Connection> connections) {
            this.connections = connections;
            inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return connections.size();
        }

        @Override
        public Connection getItem(int position) {
            return connections.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @NonNull
        @Override
        public View getView(int position, View view, ViewGroup parent) {
            if (view == null) {
                view = inflater.inflate(R.layout.saved_connection, parent, false);
            }

            Connection con = getItem(position);
            Stop dep = con.stops(0);
            Stop arr = con.stops(con.stopsLength() - 1);

            ((TextView) view.findViewById(R.id.saved_connection_from))
                    .setText(dep.station().name());

            ((TextView) view.findViewById(R.id.saved_connection_to))
                    .setText(arr.station().name());

            ((TextView) view.findViewById(R.id.saved_connection_dep_time))
                    .setText(TimeUtil.formatTime(dep.departure().time()));

            ((TextView) view.findViewById(R.id.saved_connection_arr_time))
                    .setText(TimeUtil.formatTime(arr.arrival().time()));

            ((TextView) view.findViewById(R.id.saved_connection_duration))
                    .setText(TimeUtil.formatDuration(
                            (arr.arrival().time() - dep.departure().time()) / 60));

            ((TextView) view.findViewById(R.id.saved_connection_date))
                    .setText(TimeUtil.formatDate(dep.departure().time()));

            TransportViewCreator.addTransportViews(
                    JourneyUtil.getTransports(con), inflater,
                    (LinearLayout) view.findViewById(R.id.saved_connection_transports));

            return view;
        }
    }

    private Context context;

    private Subscription favs;

    public SavedConnectionsFragment() {
    }

    public static SavedConnectionsFragment newInstance() {
        SavedConnectionsFragment fragment = new SavedConnectionsFragment();
        Bundle args = new Bundle();
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        favs.unsubscribe();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View layout = inflater.inflate(R.layout.saved_overview, container, false);
        ListView listView = (ListView) layout.findViewById(R.id.saved_connection_list);
        listView.setOnItemClickListener((adapterView, view, pos, id) -> {
            Status.get().setConnection(((ConnectionAdapter) listView.getAdapter()).getItem(pos));
            view.getContext().startActivity(new Intent(view.getContext(), DetailActivity.class));
        });
        favs = Status.get().getSavedConnectionsDb().getSavedConnections()
                .subscribe(new Subscriber<List<Connection>>() {
                    @Override
                    public void onCompleted() {
                    }

                    @Override
                    public void onError(Throwable e) {
                    }

                    @Override
                    public void onNext(List<Connection> connections) {
                        final ConnectionAdapter adapter = new ConnectionAdapter(context, connections);
                        getActivity().runOnUiThread(() -> listView.setAdapter(adapter));
                    }
                });
        return layout;
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        this.context = context;
    }

    @Override
    public void onDetach() {
        super.onDetach();
        context = null;
    }
}
