package de.motis_project.app.journey;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.TextView;

import butterknife.BindString;
import butterknife.ButterKnife;
import de.motis_project.app.R;

public class ServerErrorView extends TextView {

    @BindString(R.string.server_error)
    String default_message;

    @BindString(R.string.empty_response)
    String emptyResponseMessage;

    public ServerErrorView(Context context) {
        super(context);
        ButterKnife.bind(this);
    }

    public ServerErrorView(Context context, AttributeSet attrs) {
        super(context, attrs);
        ButterKnife.bind(this);
    }

    public void setErrorCode(int ec) {
        String msg = JourneyErrorViewHolder.messages.get(ec, default_message);
        setText(msg);
    }

    public void setEmptyResponse() {
        setText(emptyResponseMessage);
    }
}
