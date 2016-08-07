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
        private Drawable divider;

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

                RecyclerView.LayoutParams params = (RecyclerView.LayoutParams) child.getLayoutParams();

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
        final LinearLayoutManager layoutManager = new LinearLayoutManager(getContext());
        setAdapter(adapter);
        setLayoutManager(layoutManager);
        addItemDecoration(new SimpleDividerItemDecoration(getContext()));

        addOnScrollListener(new InfiniteScroll(layoutManager) {
            @Override
            void loadBefore() {
                System.out.println("JourneyListView.loadBefore");

                adapter.setLoadingBefore(true);

                new AsyncTask<Void, Void, Void>() {
                    @Override
                    protected Void doInBackground(Void... voids) {
                        try {
                            Thread.sleep(2000);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        data.addAll(0, JourneySummaryAdapter.Data.createSome(20));
                        return null;
                    }

                    @Override
                    protected void onPostExecute(Void aVoid) {
                        notifyLoadFinished();
                        adapter.notifyItemRangeInserted(0, 20);
                    }
                }.execute();
            }

            @Override
            void loadAfter() {
                adapter.setLoadingAfter(true);

                new AsyncTask<Void, Void, Void>() {
                    @Override
                    protected Void doInBackground(Void... voids) {

                        try {
                            Thread.sleep(2000);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        data.addAll(JourneySummaryAdapter.Data.createSome(20));
                        return null;
                    }

                    @Override
                    protected void onPostExecute(Void aVoid) {
                        notifyLoadFinished();
                        adapter.notifyItemRangeInserted(data.size() - 20, 20);
                    }
                }.execute();
            }
        });
    }
}
