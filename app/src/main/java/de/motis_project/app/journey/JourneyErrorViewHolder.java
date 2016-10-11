package de.motis_project.app.journey;

import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import java.util.HashSet;
import java.util.Set;

import butterknife.BindString;
import butterknife.BindView;
import butterknife.OnClick;
import de.motis_project.app.R;

public class JourneyErrorViewHolder extends JourneyViewHolder {

    public static final SparseArray<String> messages = new SparseArray<>();

    public static final Set<Integer> retryableCodes = new HashSet<>();

    static {
        messages.put(4, "Zeitraum nicht im Fahrplan");
    }

    @BindView(R.id.journey_loading_error_message)
    TextView messageView;

    @BindView(R.id.journey_loading_error_retry)
    Button retryButton;

    @OnClick(R.id.journey_loading_error_retry)
    void onClick() {
    }

    @BindString(R.string.server_error)
    String default_message;

    public JourneyErrorViewHolder(ViewGroup parent, LayoutInflater inflater, int errorCode) {
        super(inflater.inflate(R.layout.journey_loading_error, parent, false), inflater);

        if (retryableCodes.contains(errorCode)) {
            retryButton.setVisibility(View.VISIBLE);
        }
        messageView.setText(messages.get(errorCode, default_message));
    }
}
