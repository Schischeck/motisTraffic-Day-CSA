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
        int last = layoutManager.findLastVisibleItemPosition();
        if (last == layoutManager.getItemCount() - 1) {
            loader.loadAfter();
            return;
        }

        if (first == 0) {
            loader.loadBefore();
            return;
        }
    }

    public void notifyLoadFinished(int firstVisible) {
        onScrolled(firstVisible);
    }

    public void notifyLoadFinished() {
        onScrolled();
    }
}
