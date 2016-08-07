package de.motis_project.app.journey;

import android.content.Context;
import android.os.AsyncTask;
import android.support.annotation.Nullable;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.AttributeSet;

import java.util.List;

public class JourneyListView extends RecyclerView {
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

        addOnScrollListener(new InfiniteScroll(layoutManager) {
            @Override
            void loadBefore() {
                new AsyncTask<Void, Void, Void>() {
                    @Override
                    protected Void doInBackground(Void... voids) {
                        try {
                            Thread.sleep(1000);
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
                new AsyncTask<Void, Void, Void>() {
                    @Override
                    protected Void doInBackground(Void... voids) {
                        try {
                            Thread.sleep(1000);
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
