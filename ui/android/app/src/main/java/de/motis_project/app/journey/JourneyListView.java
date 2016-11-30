package de.motis_project.app.journey;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.drawable.Drawable;
import android.support.annotation.Nullable;
import android.support.design.widget.AppBarLayout;
import android.support.v4.content.ContextCompat;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.AttributeSet;
import android.view.View;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;

import de.motis_project.app.JourneyUtil;
import de.motis_project.app.R;
import de.motis_project.app.TimeUtil;
import de.motis_project.app.io.Status;
import de.motis_project.app.io.error.MotisErrorException;
import de.motis_project.app.lib.StickyHeaderDecoration;
import de.motis_project.app.query.Query;
import motis.Connection;
import motis.routing.RoutingResponse;
import rx.Subscription;
import rx.android.schedulers.AndroidSchedulers;
import rx.functions.Action1;
import rx.internal.util.SubscriptionList;
import rx.schedulers.Schedulers;

public class JourneyListView
        extends RecyclerView
        implements InfiniteScroll.Loader {

    class SimpleDividerItemDecoration extends RecyclerView.ItemDecoration {
        private final Drawable divider;

        public SimpleDividerItemDecoration(Context context) {
            divider = ContextCompat.getDrawable(context, R.drawable.line_divider);
        }

        @Override
        public void onDrawOver(Canvas c, RecyclerView parent, RecyclerView.State state) {
            int left = parent.getPaddingLeft();
            int right = parent.getWidth() - parent.getPaddingRight();
            for (int i = 0; i < parent.getChildCount(); i++) {
                View child = parent.getChildAt(i);

                RecyclerView.LayoutParams params =
                        (RecyclerView.LayoutParams) child.getLayoutParams();

                int top = child.getBottom() + params.bottomMargin;
                int bottom = top + divider.getIntrinsicHeight();

                divider.setBounds(left, top, right, bottom);
                divider.draw(c);
            }
        }
    }

    private static final int SECOND_IN_MS = 1000;
    private static final int MINUTE_IN_MS = 60 * SECOND_IN_MS;
    private static final int HOUR_IN_MS = 60 * MINUTE_IN_MS;
    private static final int SEARCH_INTERVAL_MS = 2 * HOUR_IN_MS;

    private int STICKY_HEADER_SCROLL_OFFSET;

    public Query query;
    private Date intervalBegin, intervalEnd;

    private View requestPendingView;
    private View queryIncompleteView;
    private ServerErrorView serverErrorView;
    private AppBarLayout appBarLayout;

    boolean serverError = false;
    boolean initialRequestPending = true;

    private SubscriptionList subscriptions = new SubscriptionList();
    private final List<Connection> data = new ArrayList<>();
    private final LinearLayoutManager layoutManager = new LinearLayoutManager(getContext());
    private final JourneySummaryAdapter adapter = new JourneySummaryAdapter(data);
    private final InfiniteScroll infiniteScroll = new InfiniteScroll(this, layoutManager, adapter);
    private final StickyHeaderDecoration stickyHeaderDecorator = new StickyHeaderDecoration(adapter);

    public JourneyListView(Context context) {
        super(context);
        init();
    }

    public JourneyListView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public JourneyListView(Context context, @Nullable AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init();
    }

    private void init() {
        setAdapter(adapter);
        addOnScrollListener(infiniteScroll);
        addItemDecoration(new SimpleDividerItemDecoration(getContext()));
        addItemDecoration(stickyHeaderDecorator);
        setLayoutManager(layoutManager);

        Resources r = getResources();
        STICKY_HEADER_SCROLL_OFFSET = r.getDimensionPixelOffset(R.dimen.journey_list_floating_header_height);
    }

    public void notifyQueryChanged() {
        appBarLayout.setExpanded(true, true);

        subscriptions.unsubscribe();
        subscriptions = new SubscriptionList();

        serverError = false;
        initialRequestPending = true;
        data.clear();
        adapter.setLoadAfterError(null);
        adapter.setLoadBeforeError(null);
        adapter.recalculateHeaders();
        stickyHeaderDecorator.clearCache();
        adapter.notifyDataSetChanged();

        intervalBegin = query.getTime();
        intervalEnd = new Date(intervalBegin.getTime() + SEARCH_INTERVAL_MS);
        final Date searchIntervalBegin = new Date(intervalBegin.getTime());
        final Date searchIntervalEnd = new Date(intervalEnd.getTime());
        infiniteScroll.setLoading();
        updateVisibility();
        route(searchIntervalBegin, searchIntervalEnd, true, true, 5, resObj -> {
            RoutingResponse res = (RoutingResponse) resObj;
            logResponse(res, searchIntervalBegin, searchIntervalEnd, "INITIAL");

            intervalBegin = new Date(res.intervalBegin() * 1000);
            intervalEnd = new Date(res.intervalEnd() * 1000);
            initialRequestPending = false;

            if (res.connectionsLength() == 0) {
                serverError = true;
                serverErrorView.setEmptyResponse();
            }

            data.clear();
            for (int i = 0; i < res.connectionsLength(); i++) {
                data.add(res.connections(i));
            }
            sortConnections(data);
            adapter.recalculateHeaders();
            stickyHeaderDecorator.clearCache();
            adapter.notifyDataSetChanged();
            layoutManager.scrollToPositionWithOffset(1, STICKY_HEADER_SCROLL_OFFSET);
            infiniteScroll.notifyLoadFinished();
            updateVisibility();
        }, t -> {
            initialRequestPending = false;
            infiniteScroll.notifyLoadFinished();
            serverError = true;
            if (t instanceof MotisErrorException) {
                serverErrorView.setErrorCode((MotisErrorException) t);
            }
            updateVisibility();
        });
    }

    public void route(Date searchIntervalBegin, Date searchIntervalEnd,
                      boolean extendIntervalEarlier,
                      boolean extendIntervalLater,
                      int min_connection_count,
                      Action1 action, Action1<Throwable> errorAction) {
        Subscription sub = Status.get().getServer()
                .route(query.getFromId(), query.getToId(),
                       query.isArrival(),
                       searchIntervalBegin, searchIntervalEnd,
                       extendIntervalEarlier, extendIntervalLater,
                       min_connection_count)
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(action, errorAction);
        subscriptions.add(sub);
    }

    public void notifyDestroy() {
        subscriptions.clear();
    }

    @Override
    public void loadBefore() {
        if (adapter.getErrorStateBefore() != JourneySummaryAdapter.ERROR_TYPE_NO_ERROR) {
            return;
        }

        final Date searchIntervalBegin = new Date(intervalBegin.getTime() - SEARCH_INTERVAL_MS);
        final Date searchIntervalEnd = new Date(intervalBegin.getTime() - MINUTE_IN_MS);

        route(searchIntervalBegin, searchIntervalEnd, true, false, 3, resObj -> {
            RoutingResponse res = (RoutingResponse) resObj;
            logResponse(res, searchIntervalBegin, searchIntervalEnd, "LOAD_BEFORE");

            List<Connection> newData = new ArrayList<>(res.connectionsLength());
            for (int i = 0; i < res.connectionsLength(); ++i) {
                newData.add(res.connections(i));
            }

            sortConnections(newData);

            intervalBegin = searchIntervalBegin;
            data.addAll(0, newData);

            adapter.recalculateHeaders();
            stickyHeaderDecorator.clearCache();
            adapter.notifyItemRangeInserted(1, newData.size());
            if (layoutManager.findFirstVisibleItemPosition() == 0) {
                layoutManager.scrollToPosition(newData.size() + 1);
            }

            infiniteScroll.notifyLoadBeforeFinished(newData.size());
            if (res.connectionsLength() == 0) {
                infiniteScroll.onScrolled();
            }
            updateVisibility();
        }, t -> {
            infiniteScroll.notifyLoadBeforeFinished();
            if (t instanceof MotisErrorException) {
                adapter.setLoadBeforeError((MotisErrorException) t);
            }
            updateVisibility();
        });
    }

    @Override
    public void loadAfter() {
        if (adapter.getErrorStateAfter() != JourneySummaryAdapter.ERROR_TYPE_NO_ERROR) {
            return;
        }

        final Date searchIntervalBegin = new Date(intervalEnd.getTime() + MINUTE_IN_MS);
        final Date searchIntervalEnd = new Date(intervalEnd.getTime() + SEARCH_INTERVAL_MS);

        route(searchIntervalBegin, searchIntervalEnd, false, true, 0, resObj -> {
            RoutingResponse res = (RoutingResponse) resObj;
            logResponse(res, searchIntervalBegin, searchIntervalEnd, "LOAD_AFTER");

            List<Connection> newData = new ArrayList<>(res.connectionsLength());
            for (int i = 0; i < res.connectionsLength(); ++i) {
                newData.add(res.connections(i));
            }

            sortConnections(newData);

            intervalEnd = searchIntervalEnd;
            int oldDisplayItemCount = adapter.getItemCount();
            data.addAll(newData);
            adapter.notifyItemRangeInserted(oldDisplayItemCount - 1, newData.size());
            adapter.recalculateHeaders();
            stickyHeaderDecorator.clearCache();
            infiniteScroll.notifyLoadAfterFinished();
            if (res.connectionsLength() == 0) {
                infiniteScroll.onScrolled();
            }
            updateVisibility();
        }, t -> {
            infiniteScroll.notifyLoadAfterFinished();
            if (t instanceof MotisErrorException) {
                adapter.setLoadAfterError((MotisErrorException) t);
            }
            updateVisibility();
        });
    }

    private void sortConnections(List<Connection> data) {
        Collections.sort(data, (a, b) -> {
            long depA = a.stops(0).departure().scheduleTime();
            long depB = b.stops(0).departure().scheduleTime();
            return Long.compare(depA, depB);
        });
    }

    public void setRequestPendingView(View v) {
        this.requestPendingView = v;
    }

    public void setQueryIncompleteView(View v) {
        this.queryIncompleteView = v;
    }

    public void setServerErrorView(ServerErrorView v) {
        this.serverErrorView = v;
    }

    public void setAppBarLayout(AppBarLayout appBarLayout) {
        this.appBarLayout = appBarLayout;
    }

    private void logResponse(RoutingResponse res, Date intervalBegin, Date intervalEnd,
                             String type) {
        System.out.println(new StringBuilder().append(type).append("  ").append("Routing from ")
                                   .append(TimeUtil.formatDate(intervalBegin)).append(", ")
                                   .append(TimeUtil.formatTime(intervalBegin)).append(" until ")
                                   .append(TimeUtil.formatDate(intervalEnd)).append(", ")
                                   .append(TimeUtil.formatTime(intervalEnd)));
        for (int i = 0; i < res.connectionsLength(); i++) {
            Connection con = res.connections(i);
            Date depTime = new Date(con.stops(0).departure().scheduleTime() * 1000);
            Date arrTime =
                    new Date(con.stops(con.stopsLength() - 1).arrival().scheduleTime() * 1000);
            int interchangeCount = JourneyUtil.getSections(con).size() - 1;
            long travelTime = con.stops(con.stopsLength() - 1).arrival().scheduleTime() -
                    con.stops(0).departure().scheduleTime();

            StringBuilder sb = new StringBuilder();
            sb.append("start: ").append(depTime).append("  ");
            sb.append("end: ").append(arrTime).append("  ");
            sb.append("Duration: ").append(TimeUtil.formatDuration(travelTime / 60))
                    .append("  ");
            sb.append("Interchanges: ").append(interchangeCount).append("  ");
            System.out.println(sb);
        }
    }

    private void updateVisibility() {
        if (!query.isComplete()) {
            this.setVisibility(View.GONE);
            requestPendingView.setVisibility(View.GONE);
            queryIncompleteView.setVisibility(View.VISIBLE);
            serverErrorView.setVisibility(View.GONE);
            return;
        }

        if (initialRequestPending) {
            this.setVisibility(View.GONE);
            requestPendingView.setVisibility(View.VISIBLE);
            queryIncompleteView.setVisibility(View.GONE);
            serverErrorView.setVisibility(View.GONE);
            return;
        }

        if (serverError) {
            this.setVisibility(View.GONE);
            requestPendingView.setVisibility(View.GONE);
            queryIncompleteView.setVisibility(View.GONE);
            serverErrorView.setVisibility(View.VISIBLE);
            return;
        }

        this.setVisibility(View.VISIBLE);
        requestPendingView.setVisibility(View.GONE);
        queryIncompleteView.setVisibility(View.GONE);
        serverErrorView.setVisibility(View.GONE);
    }
}
