package de.motis_project.app.journey;

import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import butterknife.BindString;
import butterknife.BindView;
import butterknife.OnClick;
import de.motis_project.app.R;
import de.motis_project.app.TimeUtil;
import de.motis_project.app.io.error.MotisErrorException;
import motis.lookup.LookupScheduleInfoResponse;

public class JourneyErrorViewHolder extends JourneyViewHolder {

    @BindView(R.id.journey_loading_error_message)
    TextView messageView;

    @BindView(R.id.journey_loading_error_retry)
    Button retryButton;

    @OnClick(R.id.journey_loading_error_retry)
    void onClick() {
    }

    @BindString(R.string.schedule_range)
    String scheduleRangeTemplate;

    @BindString(R.string.routing_error)
    String routingErrorMessage;

    public JourneyErrorViewHolder(ViewGroup parent, LayoutInflater inflater, MotisErrorException mee, LookupScheduleInfoResponse scheduleInfo) {
        super(inflater.inflate(R.layout.journey_loading_error, parent, false), inflater);
        messageView.setText(buildMessage(mee, scheduleInfo));
    }

    public JourneyErrorViewHolder(ViewGroup parent, LayoutInflater inflater, int msgId, LookupScheduleInfoResponse scheduleInfo) {
        super(inflater.inflate(R.layout.journey_loading_error, parent, false), inflater);
        messageView.setText(inflater.getContext().getText(msgId));
    }

    private static String messageFromMotisError(MotisErrorException mee) {
        return mee.category + ": " + mee.reason;
    }

    private String buildMessage(MotisErrorException mee, LookupScheduleInfoResponse scheduleInfo) {
        return buildMessage(mee, scheduleInfo, scheduleRangeTemplate, routingErrorMessage);
    }

    public static String buildMessage(MotisErrorException mee,
                                      LookupScheduleInfoResponse scheduleInfo,
                                      String scheduleRangeTemplate, String routingErrorMessage) {
        if (mee.category.equals("motis::routing") && mee.code == 4) {
            String scheduleInfoStr = String.format(scheduleRangeTemplate,
                    TimeUtil.formatDate(scheduleInfo.begin()),
                    TimeUtil.formatDate(scheduleInfo.end()));
            return routingErrorMessage + "\n" + scheduleInfoStr;
        } else {
            return messageFromMotisError(mee);
        }
    }
}
