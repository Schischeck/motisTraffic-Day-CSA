package de.motis_project.app.journey;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.TextView;

import butterknife.BindString;
import butterknife.ButterKnife;
import de.motis_project.app.R;
import de.motis_project.app.io.error.MotisErrorException;
import motis.lookup.LookupScheduleInfoResponse;

public class ServerErrorView extends TextView {

    @BindString(R.string.server_error)
    String default_message;

    @BindString(R.string.empty_response)
    String emptyResponseMessage;

    @BindString(R.string.schedule_range)
    String scheduleRangeTemplate;

    public ServerErrorView(Context context) {
        super(context);
        ButterKnife.bind(this);
    }

    public ServerErrorView(Context context, AttributeSet attrs) {
        super(context, attrs);
        ButterKnife.bind(this);
    }

    public void setErrorCode(MotisErrorException mee, LookupScheduleInfoResponse scheduleInfo) {
        setText(JourneyErrorViewHolder
                .buildMessage(mee, default_message, scheduleInfo, scheduleRangeTemplate));
    }

    public void setEmptyResponse() {
        setText(emptyResponseMessage);
    }
}
