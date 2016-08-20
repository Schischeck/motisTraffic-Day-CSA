package de.motis_project.app.journey;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.drawable.Drawable;
import android.support.annotation.Nullable;
import android.support.v4.content.ContextCompat;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.AttributeSet;
import android.view.View;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.List;
import java.util.concurrent.TimeUnit;

import de.motis_project.app.JourneyUtil;
import de.motis_project.app.R;
import de.motis_project.app.TimeUtil;
import de.motis_project.app.io.Status;
import de.motis_project.app.lib.StickyHeaderDecoration;
import de.motis_project.app.query.Query;
import motis.Connection;
import motis.routing.RoutingResponse;
import rx.Observable;
import rx.Subscription;
import rx.android.schedulers.AndroidSchedulers;
import rx.functions.Action1;
import rx.functions.Func1;
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

    public Query query;
    private Date intervalBegin, intervalEnd;

    private final SubscriptionList subscriptions = new SubscriptionList();
    private final List<Connection> data = new ArrayList<>();
    private final LinearLayoutManager layoutManager = new LinearLayoutManager(getContext());
    private final InfiniteScroll infiniteScroll = new InfiniteScroll(this, layoutManager);
    private final JourneySummaryAdapter adapter = new JourneySummaryAdapter(data);
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
    }

    public void notifyQueryChanged() {
        intervalBegin = query.getTime();
        intervalEnd = new Date(intervalBegin.getTime() + SEARCH_INTERVAL_MS);

        data.clear();
        adapter.recalculateHeaders();
        adapter.notifyDataSetChanged();

        final Date searchIntervalBegin = new Date(intervalBegin.getTime());
        final Date searchIntervalEnd = new Date(intervalEnd.getTime());
        infiniteScroll.notifyLoadStart();
        route(searchIntervalBegin, searchIntervalEnd, new Action1<RoutingResponse>() {
            @Override
            public void call(RoutingResponse res) {
                infiniteScroll.notifyLoadFinished();
                logResponse(res, searchIntervalBegin, searchIntervalEnd, "INITIAL");

                data.clear();
                for (int i = 0; i < res.connectionsLength(); i++) {
                    data.add(res.connections(i));
                }
                sortConnections(data);
                adapter.recalculateHeaders();
                stickyHeaderDecorator.clearHeaderCache();
                adapter.notifyDataSetChanged();
            }
        });
    }

    public void route(Date searchIntervalBegin, Date searchIntervalEnd,
                      Action1 action) {
        Subscription sub = Status.get().getServer()
                .route(query.getFromId(), query.getToId(),
                       query.isArrival(),
                       searchIntervalBegin, searchIntervalEnd)
                .retryWhen(
                        new Func1<Observable<? extends Throwable>, Observable<?>>() {
                            @Override
                            public Observable<?> call(Observable<? extends Throwable> attempts) {
                                return attempts.flatMap(new Func1<Throwable, Observable<?>>() {
                                    @Override
                                    public Observable<?> call(Throwable throwable) {
                                        return Observable.timer(1, TimeUnit.SECONDS);
                                    }
                                });
                            }
                        })
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(action, new Action1<Throwable>() {
                    @Override
                    public void call(Throwable throwable) {
                        throwable.printStackTrace();
                        System.out.println("RETRY?!");
                    }
                });
        subscriptions.add(sub);
    }

    public void notifyDestroy() {
        subscriptions.clear();
    }

    @Override
    public void loadBefore() {
        final Date searchIntervalBegin = new Date(intervalBegin.getTime() - SEARCH_INTERVAL_MS);
        final Date searchIntervalEnd = new Date(intervalBegin.getTime() - MINUTE_IN_MS);

        route(searchIntervalBegin, searchIntervalEnd,
              new Action1<RoutingResponse>() {
                  @Override
                  public void call(RoutingResponse res) {
                      logResponse(res, searchIntervalBegin, searchIntervalEnd, "LOAD_BEFORE");

                      List<Connection> newData = new ArrayList<>(res.connectionsLength());
                      for (int i = 0; i < res.connectionsLength(); ++i) {
                          newData.add(res.connections(i));
                      }

                      sortConnections(newData);

                      intervalBegin = searchIntervalBegin;
                      data.addAll(0, newData);

                      adapter.recalculateHeaders();
                      stickyHeaderDecorator.clearHeaderCache();
                      adapter.notifyItemRangeInserted(1, newData.size());
                      if (layoutManager.findFirstVisibleItemPosition() == 0) {
                          layoutManager.scrollToPosition(newData.size() + 1);
                      }

                      infiniteScroll.notifyLoadFinished(newData.size());
                  }
              });
    }

    @Override
    public void loadAfter() {
        final Date searchIntervalBegin = new Date(intervalEnd.getTime() + MINUTE_IN_MS);
        final Date searchIntervalEnd = new Date(intervalEnd.getTime() + SEARCH_INTERVAL_MS);

        route(searchIntervalBegin, searchIntervalEnd,
              new Action1<RoutingResponse>() {
                  @Override
                  public void call(RoutingResponse res) {
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
                      stickyHeaderDecorator.clearHeaderCache();

                      infiniteScroll.notifyLoadFinished();
                  }
              });
    }

    private void sortConnections(List<Connection> data) {
        Collections.sort(data, new Comparator<Connection>() {
            @Override
            public int compare(Connection a, Connection b) {
                long depA = a.stops(0).departure().scheduleTime();
                long depB = b.stops(0).departure().scheduleTime();
                return Long.compare(depA, depB);
            }
        });
    }

    private void logResponse(RoutingResponse res, Date intervalBegin, Date intervalEnd,
                             String type) {
        System.out.println(new StringBuilder().append(type).append("  ").append("Routing from ")
                                   .append(TimeUtil.formatDate(intervalBegin)).append(", ")
                                   .append(TimeUtil.formatTime(intervalBegin)).append(" until ")
                                   .append(TimeUtil.formatDate(intervalEnd)).append(", ")
                                   .append(TimeUtil.formatTime(intervalEnd))

        );
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
            sb.append("Duration: ").append(TimeUtil.getDurationString(travelTime / 60))
                    .append("  ");
            sb.append("Interchanges: ").append(interchangeCount).append("  ");
            System.out.println(sb);
        }
    }
}
