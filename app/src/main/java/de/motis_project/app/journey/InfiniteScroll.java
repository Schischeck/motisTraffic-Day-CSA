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

    private boolean loadingBefore = false;
    private boolean loadingAfter = false;

    InfiniteScroll(Loader loader, LinearLayoutManager layoutManager) {
        this.loader = loader;
        this.layoutManager = layoutManager;
    }

    @Override
    public void onScrolled(RecyclerView recyclerView, int dx, int dy) {
        onScrolled();
    }

    public void onScrolled() {
        onScrolled(layoutManager.findFirstVisibleItemPosition());
    }

    private void onScrolled(int first) {
        synchronized (layoutManager) {
            if (!loadingAfter) {
                int last = layoutManager.findLastVisibleItemPosition();
                if (last != RecyclerView.NO_POSITION && last == layoutManager.getItemCount() - 1) {
                    loadingAfter = true;
                    loader.loadAfter();
                    return;
                }
            }
            if (!loadingBefore) {
                if (first == 0) {
                    loadingBefore = true;
                    loader.loadBefore();
                    return;
                }
            }
        }
    }

    public void notifyLoadBeforeFinished(int firstVisible) {
        onScrolled(firstVisible);
        loadingBefore = false;
    }

    public void notifyLoadBeforeFinished() {
        onScrolled();
        loadingBefore = false;
    }

    public void notifyLoadAfterFinished(int firstVisible) {
        onScrolled(firstVisible);
        loadingAfter = false;
    }

    public void notifyLoadAfterFinished() {
        onScrolled();
        loadingAfter = false;
    }

    public void notifyLoadFinished() {
        notifyLoadAfterFinished();
        notifyLoadBeforeFinished();
    }

    public void setLoading() {
        loadingAfter = true;
        loadingBefore = true;
    }
}
