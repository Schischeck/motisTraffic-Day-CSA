package de.motis_project.app.quickselect;

import android.graphics.Canvas;
import android.graphics.Rect;
import android.support.annotation.NonNull;

interface SelectableItem {
    void drawHighlight(Canvas canvas);

    void draw(@NonNull Canvas canvas, float x, float y, float radius);

    Rect getBounds();
}
