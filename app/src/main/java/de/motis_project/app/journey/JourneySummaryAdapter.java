package de.motis_project.app.journey;

import android.content.Context;
import android.content.Intent;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import de.motis_project.app.R;
import de.motis_project.app.TimeUtil;
import de.motis_project.app.detail.DetailActivity;
import de.motis_project.app.io.Status;
import de.motis_project.app.io.error.MotisErrorException;
import de.motis_project.app.lib.StickyHeaderAdapter;
import motis.Connection;
import motis.lookup.LookupScheduleInfoResponse;
import rx.Subscription;
import rx.android.schedulers.AndroidSchedulers;
import rx.functions.Action1;
import rx.schedulers.Schedulers;

public class JourneySummaryAdapter
        extends RecyclerView.Adapter<JourneyViewHolder>
        implements StickyHeaderAdapter<JourneyViewHolder> {

    private static class HeaderMapping {
        private final Map<Date, Integer> map = new HashMap<>();

        HeaderMapping(List<Connection> data) {
            int nextId = 0;
            for (Connection con : data) {
                Date date = normalizeDate(con);
                if (!map.containsKey(date)) {
                    map.put(date, nextId);
                    ++nextId;
                }
            }
        }

        public int getHeaderIndex(Connection con) {
            return map.get(normalizeDate(con));
        }

        public static Date normalizeDate(Connection con) {
            Calendar cal = Calendar.getInstance();
            cal.setTime(new Date(con.stops(0).departure().time() * 1000));
            cal.set(Calendar.HOUR_OF_DAY, 0);
            cal.set(Calendar.MINUTE, 0);
            cal.set(Calendar.SECOND, 0);
            cal.set(Calendar.MILLISECOND, 0);
            return cal.getTime();
        }

        public int getLastIndex() {
            return map.size() - 1;
        }
    }

    private static final int VIEW_TYPE_LOADING_SPINNER = 0;
    private static final int VIEW_TYPE_JOURNEY_PREVIEW = 1;
    private static final int VIEW_TYPE_ERROR_BEFORE = 2;
    private static final int VIEW_TYPE_ERROR_AFTER = 3;

    final static int ERROR_TYPE_NO_ERROR = 0;
    final static int ERROR_TYPE_MOTIS_ERROR = 1;
    final static int ERROR_TYPE_OTHER = 2;

    private int errorTypeBefore = ERROR_TYPE_NO_ERROR;
    private int errorTypeAfter = ERROR_TYPE_NO_ERROR;

    private MotisErrorException motisErrorBefore;
    private MotisErrorException motisErrorAfter;

    private int otherErrorMsgBefore;
    private int otherErrorMsgAfter;

    public LookupScheduleInfoResponse scheduleRange;

    private final List<Connection> data;

    private HeaderMapping headerMapping;

    public JourneySummaryAdapter(List<Connection> d) {
        data = d;
        recalculateHeaders();
        getServerScheduleRange();
    }

    @Override
    public int getItemViewType(int position) {
        if (position == 0) {
            return errorTypeBefore != ERROR_TYPE_NO_ERROR ? VIEW_TYPE_ERROR_BEFORE : VIEW_TYPE_LOADING_SPINNER;
        }
        if (position == getItemCount() - 1) {
            return errorTypeAfter != ERROR_TYPE_NO_ERROR ? VIEW_TYPE_ERROR_AFTER : VIEW_TYPE_LOADING_SPINNER;
        }
        return VIEW_TYPE_JOURNEY_PREVIEW;
    }

    @Override
    public JourneyViewHolder onCreateViewHolder(ViewGroup parent,
                                                int viewType) {
        Context context = parent.getContext();
        LayoutInflater inflater = LayoutInflater.from(context);

        switch (viewType) {
            case VIEW_TYPE_JOURNEY_PREVIEW:
                return new JourneySummaryViewHolder(parent, inflater);
            case VIEW_TYPE_LOADING_SPINNER:
                return new JourneyViewHolder(
                        inflater.inflate(
                                R.layout.journey_loading_spinner,
                                parent, false), null);
            case VIEW_TYPE_ERROR_BEFORE:
                if (errorTypeBefore == ERROR_TYPE_MOTIS_ERROR) {
                    return new JourneyErrorViewHolder(parent, inflater, motisErrorBefore, scheduleRange);
                } else {
                    return new JourneyErrorViewHolder(parent, inflater, otherErrorMsgBefore, scheduleRange);
                }
            case VIEW_TYPE_ERROR_AFTER:
                if (errorTypeAfter == ERROR_TYPE_MOTIS_ERROR) {
                    return new JourneyErrorViewHolder(parent, inflater, motisErrorAfter, scheduleRange);
                } else {
                    return new JourneyErrorViewHolder(parent, inflater, otherErrorMsgAfter, scheduleRange);
                }
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
        if (position == 0 || data.size() == 0) {
            return 0;
        }
        if (position == data.size() + 1) {
            return headerMapping.getLastIndex();
        }

        return headerMapping.getHeaderIndex(data.get(position - 1));
    }

    @Override
    public void onBindViewHolder(JourneyViewHolder viewHolder, final int position) {
        final int index = position - 1;
        if (index < 0 || index >= data.size()) {
            return;
        }

        final Connection con = data.get(index);
        JourneySummaryViewHolder jsvh = (JourneySummaryViewHolder) viewHolder;
        jsvh.setConnection(con);
        jsvh.itemView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Status.get().setConnection(con);
                view.getContext().startActivity(new Intent(view.getContext(), DetailActivity.class));
            }
        });
    }

    @Override
    public JourneyViewHolder onCreateHeaderViewHolder(ViewGroup parent) {
        Context context = parent.getContext();
        LayoutInflater inflater = LayoutInflater.from(context);
        View header = inflater.inflate(R.layout.journey_header_item, parent, false);
        return new JourneyViewHolder(header, null);
    }

    @Override
    public void onBindHeaderViewHolder(JourneyViewHolder viewholder, int position) {
        if (data.isEmpty()) {
            System.out.println("NO DATA");
            return;
        }

        final int index = Math.min(data.size() - 1, Math.max(0, position - 1));

        Connection con = data.get(index);

        String depStation = con.stops(0).station().name();
        TextView headerDepText = (TextView) viewholder.itemView.findViewById(R.id.journey_header_departure_text);
        headerDepText.setText(depStation);

        String arrStation = con.stops(con.stopsLength() - 1).station().name();
        TextView headerArrText = (TextView) viewholder.itemView.findViewById(R.id.journey_header_arrival_text);
        headerArrText.setText(arrStation);

        TextView headerDate = (TextView) viewholder.itemView.findViewById(R.id.journey_header_date);
        headerDate.setText(TimeUtil.formatDate(con.stops(0).departure().time()));
    }

    public void recalculateHeaders() {
        headerMapping = new HeaderMapping(data);
    }

    public void setLoadBeforeError(MotisErrorException error) {
        if (error == null) {
            errorTypeBefore = ERROR_TYPE_NO_ERROR;
            return;
        }
        errorTypeBefore = ERROR_TYPE_MOTIS_ERROR;
        motisErrorBefore = error;
        notifyItemChanged(0);
    }

    public void setLoadAfterError(MotisErrorException error) {
        if (error == null) {
            errorTypeAfter = ERROR_TYPE_NO_ERROR;
            return;
        }
        errorTypeAfter = ERROR_TYPE_MOTIS_ERROR;
        motisErrorAfter = error;
        notifyItemChanged(getItemCount() - 1);
    }

    public void setLoadBeforeError(int msg) {
        if (msg == 0) {
            errorTypeBefore = ERROR_TYPE_NO_ERROR;
            return;
        }
        errorTypeBefore = ERROR_TYPE_OTHER;
        otherErrorMsgBefore = msg;
        notifyItemChanged(0);
    }

    public void setLoadAfterError(int msg) {
        if (msg == 0) {
            errorTypeAfter = ERROR_TYPE_NO_ERROR;
            return;
        }
        errorTypeAfter = ERROR_TYPE_OTHER;
        otherErrorMsgAfter = msg;
        notifyItemChanged(getItemCount() - 1);
    }

    public int getErrorStateBefore() {
        return errorTypeBefore;
    }

    public int getErrorStateAfter() {
        return errorTypeAfter;
    }


    public void getServerScheduleRange() {
        Subscription sub1 = Status.get().getServer()
                .scheduleInfo()
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(new Action1<LookupScheduleInfoResponse>() {
                    @Override
                    public void call(LookupScheduleInfoResponse res) {
                        scheduleRange = res;
                        System.out.println("SCHEDULE BEGIN: " + TimeUtil.formatDate(res.begin()));
                        System.out.println("SCHEDULE END:   " + TimeUtil.formatDate(res.end()));
                    }
                }, new Action1<Throwable>() {
                    @Override
                    public void call(Throwable throwable) {
                        throwable.printStackTrace();
                        System.out.println("GET SERVER INFO FAILED");
                    }
                });
    }

}
