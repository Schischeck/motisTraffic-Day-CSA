/*
 * Copyright 2014 Eduardo Barrenechea
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Source: https://github.com/edubarr/header-decor
// 2016-08-07: modified to not draw a margin
// 2016-10-15: removed cache, consider layout margin of items for header placement

package de.motis_project.app.lib;

import android.graphics.Canvas;
import android.graphics.Rect;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.view.ViewGroup;

public class StickyHeaderDecoration extends RecyclerView.ItemDecoration {

    private StickyHeaderAdapter mAdapter;

    public StickyHeaderDecoration(StickyHeaderAdapter adapter) {
        mAdapter = adapter;
    }

    @Override
    public void getItemOffsets(
            Rect outRect, View view, RecyclerView parent, RecyclerView.State state) {
        int position = parent.getChildAdapterPosition(view);

        int headerHeight = 0;
        if (position != RecyclerView.NO_POSITION && hasHeader(position)) {
            View header = getHeader(parent, position).itemView;
            headerHeight = header.getHeight();
        }

        outRect.set(0, headerHeight, 0, 0);
    }

    private boolean hasHeader(int position) {
        if (position == 0) {
            return true;
        }

        int previous = position - 1;
        return mAdapter.getHeaderId(position) != mAdapter.getHeaderId(previous);
    }

    private RecyclerView.ViewHolder getHeader(RecyclerView parent, int position) {
        final RecyclerView.ViewHolder holder = mAdapter.onCreateHeaderViewHolder(parent);
        final View header = holder.itemView;

        mAdapter.onBindHeaderViewHolder(holder, position);

        int widthSpec = View.MeasureSpec.makeMeasureSpec(
                parent.getWidth(), View.MeasureSpec.EXACTLY);
        int heightSpec = View.MeasureSpec.makeMeasureSpec(
                parent.getHeight(), View.MeasureSpec.UNSPECIFIED);

        int childWidth = ViewGroup.getChildMeasureSpec(
                widthSpec, 0, header.getLayoutParams().width);
        int childHeight = ViewGroup.getChildMeasureSpec(
                heightSpec, 0, header.getLayoutParams().height);

        header.measure(childWidth, childHeight);
        header.layout(0, 0, header.getMeasuredWidth(), header.getMeasuredHeight());

        return holder;
    }

    @Override
    public void onDrawOver(Canvas c, RecyclerView parent, RecyclerView.State state) {
        final int count = parent.getChildCount();

        for (int layoutPos = 0; layoutPos < count; layoutPos++) {
            final View child = parent.getLayoutManager().getChildAt(layoutPos);
            final int adapterPos = parent.getChildAdapterPosition(child);
            if (adapterPos != RecyclerView.NO_POSITION && (layoutPos == 0 || hasHeader(adapterPos))) {
                View header = getHeader(parent, adapterPos).itemView;
                c.save();
                c.translate(0, getHeaderTop(parent, child, header, adapterPos, layoutPos));
                header.draw(c);
                c.restore();
            }
        }
    }

    private int getHeaderTop(
            RecyclerView parent, View child, View header, int adapterPos, int layoutPos) {
        RecyclerView.LayoutParams params =
                (RecyclerView.LayoutParams) child.getLayoutParams();
        int headerOffset = params.topMargin + header.getHeight();
        int top = ((int) child.getY()) - headerOffset;

        if (layoutPos == 0) {
            final int count = parent.getChildCount();
            final long currentId = mAdapter.getHeaderId(adapterPos);

            for (int i = 1; i < count; i++) {
                View childHere = parent.getLayoutManager().getChildAt(i);
                int adapterPosHere = parent.getChildAdapterPosition(childHere);
                if (adapterPosHere != RecyclerView.NO_POSITION) {
                    long nextId = mAdapter.getHeaderId(adapterPosHere);
                    if (nextId != currentId) {
                        final View next = childHere;
                        final int offset = ((int) next.getY()) -
                                (headerOffset + getHeader(parent, adapterPosHere).itemView.getHeight());
                        if (offset < 0) {
                            return offset;
                        } else {
                            break;
                        }
                    }
                }
            }

            top = Math.max(0, top);
        }

        return top;
    }
}
