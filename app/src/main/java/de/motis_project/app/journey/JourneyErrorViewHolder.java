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
import de.motis_project.app.TimeUtil;
import de.motis_project.app.io.error.MotisErrorException;
import motis.lookup.LookupScheduleInfoResponse;

public class JourneyErrorViewHolder extends JourneyViewHolder {

    private static final SparseArray<String> messages = new SparseArray<>();

    //TODO(Simon): more messages
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
    String defaultMessage;

    @BindString(R.string.schedule_range)
    String scheduleRangeTemplate;

    public JourneyErrorViewHolder(ViewGroup parent, LayoutInflater inflater, MotisErrorException mee, LookupScheduleInfoResponse scheduleInfo) {
        super(inflater.inflate(R.layout.journey_loading_error, parent, false), inflater);
        messageView.setText(buildMessage(mee, scheduleInfo));
    }

    public JourneyErrorViewHolder(ViewGroup parent, LayoutInflater inflater, int msgId, LookupScheduleInfoResponse scheduleInfo) {
        super(inflater.inflate(R.layout.journey_loading_error, parent, false), inflater);
        messageView.setText(inflater.getContext().getText(msgId));
    }

    private String buildMessage(MotisErrorException mee, LookupScheduleInfoResponse scheduleInfo) {
        return buildMessage(mee, defaultMessage, scheduleInfo, scheduleRangeTemplate);
    }

    public static String buildMessage(MotisErrorException mee,
                                      String defaultMessage,
                                      LookupScheduleInfoResponse scheduleInfo,
                                      String scheduleRangeTemplate) {
        String msg = messages.get(mee.code, defaultMessage);
        if (mee.code == 4 && scheduleInfo != null) {
            msg += "\n" + String.format(scheduleRangeTemplate, TimeUtil.formatDate(scheduleInfo.begin()),
                    TimeUtil.formatDate(scheduleInfo.end()));
        }
        return msg;
    }
}
