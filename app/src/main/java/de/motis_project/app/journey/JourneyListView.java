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

import java.util.List;

import de.motis_project.app.R;

public class JourneyListView extends RecyclerView {

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
        final List<JourneySummaryAdapter.Data> data = JourneySummaryAdapter.Data.createSome(50);
        final JourneySummaryAdapter adapter = new JourneySummaryAdapter(data);
        final LinearLayoutManager layoutManager = new CustomLinearLayoutManager(getContext());
        setAdapter(adapter);
        setLayoutManager(layoutManager);
        addItemDecoration(new SimpleDividerItemDecoration(getContext()));

        addOnScrollListener(new InfiniteScroll(layoutManager) {
            @Override
            void loadBefore() {
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
                        notifyLoadFinished();
                        data.addAll(0, newData);
                        // lie about inserted position to scroll to position 1
                        adapter.notifyItemRangeInserted(0, newData.size());
                    }
                }.execute();
            }

            @Override
            void loadAfter() {
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
                        notifyLoadFinished();
                        int oldDisplayItemCount = adapter.getItemCount();
                        data.addAll(newData);
                        adapter.notifyItemRangeInserted(oldDisplayItemCount - 1, newData.size());
                    }
                }.execute();
            }
        });
    }
}
