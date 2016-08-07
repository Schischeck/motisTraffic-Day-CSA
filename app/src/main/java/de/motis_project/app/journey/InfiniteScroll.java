package de.motis_project.app.journey;

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
        System.out.println("########### !!!!!!!!!!! InfiniteScroll.onScrolled");

        synchronized (layoutManager) {
            if (loading) {
                return;
            }

            int first = layoutManager.findFirstVisibleItemPosition();
            System.out.println("first = " + first);
            System.out.println("first completely = " + layoutManager.findFirstCompletelyVisibleItemPosition());
            System.out.println("top first = " +
                    layoutManager.findViewByPosition(layoutManager.findFirstCompletelyVisibleItemPosition()).getTop());
            if (first == 0) {
                loading = true;
                loadBefore();
                return;
            }

            int last = layoutManager.findLastVisibleItemPosition();
            if (last == layoutManager.getItemCount() - 1) {
                loading = true;
                loadAfter();
                return;
            }
        }
    }

    void notifyLoadFinished() {
        loading = false;
    }

    abstract void loadBefore();

    abstract void loadAfter();
}
