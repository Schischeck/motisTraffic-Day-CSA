package de.motis_project.app.quickselect;


import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.support.annotation.NonNull;
import android.support.v4.graphics.drawable.DrawableCompat;

public class DrawableSymbol implements SelectableItem {
    private final Paint shadowPaint = new Paint();
    private final Paint highlightPaint;
    private final Drawable drawable;
    private final Paint bgPaint;

    DrawableSymbol(Paint bgPaint, Paint highlightPaint, Drawable drawable) {
        shadowPaint.setShadowLayer(15.0f, 0.0f, 5.0f, Color.argb(100, 0, 0, 0));
        shadowPaint.setColor(Color.WHITE);
        shadowPaint.setAntiAlias(true);
        this.highlightPaint = highlightPaint;
        this.bgPaint = bgPaint;
        this.drawable = drawable;
    }

    @Override
    public void drawHighlight(Canvas canvas) {
        Rect bounds = getBounds();
        float radius = bounds.width();
        highlightPaint.setStrokeWidth(radius / 8);
        canvas.drawCircle(bounds.exactCenterX(), bounds.exactCenterY(), radius, highlightPaint);
    }

    @Override
    public void draw(@NonNull Canvas canvas, float x, float y, float iconRadius) {
        bgPaint.setStyle(Paint.Style.FILL);
        float iconSize = iconRadius / 2.0f;
        canvas.drawCircle(x, y, iconRadius, shadowPaint);
        canvas.drawCircle(x, y, iconRadius, bgPaint);
        DrawableCompat.setTint(DrawableCompat.wrap(drawable), Color.WHITE);
        drawable.setBounds((int) (x - iconSize),
                (int) (y - iconSize),
                (int) (iconSize + x),
                (int) (iconSize + y));
        drawable.draw(canvas);
        bgPaint.setStyle(Paint.Style.STROKE);
    }

    @Override
    public Rect getBounds() {
        return drawable.getBounds();
    }
}
