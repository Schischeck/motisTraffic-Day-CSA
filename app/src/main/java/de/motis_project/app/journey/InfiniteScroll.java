package de.motis_project.app.journey;

import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;

public class InfiniteScroll extends RecyclerView.OnScrollListener {
    public interface Loader {
        void loadBefore();
        void loadAfter();
    }

    private final LinearLayoutManager layoutManager;
    private final Loader loader;
    private boolean loading = false;

    InfiniteScroll(Loader loader, LinearLayoutManager layoutManager) {
        this.loader = loader;
        this.layoutManager = layoutManager;
    }

    @Override
    public void onScrolled(RecyclerView recyclerView, int dx, int dy) {
        onScrolled();
    }

    private void onScrolled() {
        onScrolled(layoutManager.findFirstVisibleItemPosition());
    }

    private void onScrolled(int first) {
        synchronized (layoutManager) {
            if (loading) {
                return;
            }

            int last = layoutManager.findLastVisibleItemPosition();
            if (last == layoutManager.getItemCount() - 1) {
                loading = true;
                loader.loadAfter();
                return;
            }

            if (first == 0) {
                loading = true;
                loader.loadBefore();
                return;
            }
        }
    }

    public void notifyLoadFinished(int firstVisible) {
        loading = false;
        onScrolled(firstVisible);
    }

    public void notifyLoadFinished() {
        loading = false;
        onScrolled();
    }

    public void notifyLoadStart() {
        loading = true;
    }
}
