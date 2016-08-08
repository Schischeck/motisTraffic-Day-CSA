package de.motis_project.app.journey;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.support.annotation.Nullable;
import android.support.v4.content.ContextCompat;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.AttributeSet;
import android.view.View;

import java.util.Date;
import java.util.List;

import de.motis_project.app.R;
import de.motis_project.app.io.Server;
import de.motis_project.app.lib.StickyHeaderDecoration;
import motis.Connection;
import motis.Message;
import motis.MsgContent;
import motis.routing.RoutingResponse;

public class JourneyListView extends RecyclerView implements Server.Listener, InfiniteScroll.Loader {
    class SimpleDividerItemDecoration extends RecyclerView.ItemDecoration {
        private final Drawable divider;

        public SimpleDividerItemDecoration(Context context) {
            divider = ContextCompat.getDrawable(context, R.drawable.line_divider);
        }

        @Override
        public void onDrawOver(Canvas c, RecyclerView parent, RecyclerView.State state) {
            int left = parent.getPaddingLeft();
            int right = parent.getWidth() - parent.getPaddingRight();

            int childCount = parent.getChildCount();
            for (int i = 0; i < childCount; i++) {
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

    private final List<JourneySummaryAdapter.Data> data = JourneySummaryAdapter.Data.createSome(50);
    private final JourneySummaryAdapter adapter = new JourneySummaryAdapter(data);
    private final LinearLayoutManager layoutManager = new CustomLinearLayoutManager(getContext());
    private final InfiniteScroll infiniteScroll = new InfiniteScroll(this, layoutManager);

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
        addItemDecoration(new StickyHeaderDecoration(adapter));
        setLayoutManager(layoutManager);
    }

    @Override
    public void loadBefore() {
        new AsyncTask<Void, Void, List<JourneySummaryAdapter.Data>>() {
            @Override
            protected List<JourneySummaryAdapter.Data> doInBackground(Void... voids) {
                try {
                    Thread.sleep(2000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                return JourneySummaryAdapter.Data.createSome(20);
            }

            @Override
            protected void onPostExecute(List<JourneySummaryAdapter.Data> newData) {
                infiniteScroll.notifyLoadFinished();
                data.addAll(0, newData);
                // lie about inserted position to scroll to position 1
                adapter.notifyItemRangeInserted(0, newData.size());
            }
        }.execute();
    }

    @Override
    public void loadAfter() {
        new AsyncTask<Void, Void, List<JourneySummaryAdapter.Data>>() {
            @Override
            protected List<JourneySummaryAdapter.Data> doInBackground(Void... voids) {
                try {
                    Thread.sleep(2000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                return JourneySummaryAdapter.Data.createSome(20);
            }

            @Override
            protected void onPostExecute(List<JourneySummaryAdapter.Data> newData) {
                infiniteScroll.notifyLoadFinished();
                int oldDisplayItemCount = adapter.getItemCount();
                data.addAll(newData);
                adapter.notifyItemRangeInserted(oldDisplayItemCount - 1, newData.size());
            }
        }.execute();
    }

    @Override
    public void onMessage(Message m) {
        if (m.contentType() != MsgContent.RoutingResponse) {
            return;
        }

        RoutingResponse res = new RoutingResponse();
        res = (RoutingResponse) m.content(res);

        for (int i = 0; i < res.connectionsLength(); i++) {
            Connection con = res.connections(i);

            long depUnixTime = con.stops(0).departure().scheduleTime();
            long arrUnixTime = con.stops(con.stopsLength() - 1).arrival().scheduleTime();
            long duration = arrUnixTime - depUnixTime;

            Date departure = new Date(depUnixTime * 1000);
            Date arrival = new Date(arrUnixTime * 1000);

            System.out.println("Connection " + (i + 1) + "/" + res.connectionsLength()+ ": "
                                       + "duration=" + (duration / 60) + "min, "
                                       + "transfers=" + countTransfers(con));
            System.out.println("departure: " + departure);
            System.out.println("arrival: " + arrival);
        }
    }

    int countTransfers(Connection con) {
        int transfers = 0;
        for (int i = 0; i < con.stopsLength(); i++) {
            transfers += con.stops(i).interchange() ? 1 : 0;
        }
        return transfers;
    }

    @Override
    public void onConnect() {
    }

    @Override
    public void onDisconnect() {
    }
}
