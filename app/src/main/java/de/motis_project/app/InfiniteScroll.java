package de.motis_project.app;

import android.os.Looper;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;

public abstract class InfiniteScroll extends RecyclerView.OnScrollListener {
    private final LinearLayoutManager layoutManager;
    private boolean loading = false;

    InfiniteScroll(LinearLayoutManager layoutManager) {
        this.layoutManager = layoutManager;
    }

    @Override
    public void onScrolled(RecyclerView recyclerView, int dx, int dy) {
        System.out.println("MAIN THREAD: " + (Looper.myLooper() == Looper.getMainLooper()));

        if (loading) {
            return;
        }

        int first = layoutManager.findFirstVisibleItemPosition();
        if (first == 0) {
            loading = true;
            loadBefore();
        }

        int last = layoutManager.findLastVisibleItemPosition();
        if (last == layoutManager.getItemCount() - 1) {
            loading = true;
            loadAfter();
        }
    }

    void notifyLoadFinished() {
        loading = false;
    }

    abstract void loadBefore();
    abstract void loadAfter();
}
