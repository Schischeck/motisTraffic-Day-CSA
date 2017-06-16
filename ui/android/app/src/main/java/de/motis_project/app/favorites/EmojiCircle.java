package de.motis_project.app.favorites;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.support.annotation.IntRange;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.TextPaint;

public class EmojiCircle extends Drawable {
    private final Paint shadowPaint = new Paint();
    private final Paint highlightPaint;
    private final TextPaint tPaint;
    private final String symbol;

    public EmojiCircle(String symbol, TextPaint tPaint, Paint highlightPaint) {
        shadowPaint.setShadowLayer(15.0f, 0.0f, 5.0f, Color.argb(100, 0, 0, 0));
        shadowPaint.setColor(Color.WHITE);
        shadowPaint.setAntiAlias(true);
        this.tPaint = tPaint;
        this.highlightPaint = highlightPaint;
        this.symbol = symbol;
    }

    public void setBounds(float x, float y, float radius) {
        int r = Math.round(radius);
        setBounds(new Rect((int) (x - r), (int) (y - r), (int) (x + r), (int) (y + r)));
    }

    @Override
    public void draw(@NonNull Canvas canvas) {
        Rect outBounds = getBounds();
        Rect textBounds = new Rect();
        tPaint.getTextBounds(symbol, 0, symbol.length(), textBounds);

        canvas.translate(outBounds.exactCenterX(), outBounds.exactCenterY());
        canvas.drawCircle(0, 0, outBounds.width() / 2, shadowPaint);
        canvas.drawText(symbol, 0, symbol.length(),
                0, -textBounds.exactCenterY(), tPaint);
        canvas.translate(-outBounds.exactCenterX(), -outBounds.exactCenterY());
    }

    public void drawHighlight(Canvas canvas) {
        Rect bounds = getBounds();
        float radius = bounds.width() / 2f;
        canvas.drawCircle(bounds.exactCenterX(), bounds.exactCenterY(), radius, highlightPaint);
    }

    public void draw(@NonNull Canvas canvas, float x, float y, float radius) {
        setBounds(x, y, radius);
        draw(canvas);
    }

    @Override
    public void setAlpha(@IntRange(from = 0, to = 255) int alpha) {
        throw new UnsupportedOperationException();
    }

    @Override
    public void setColorFilter(@Nullable ColorFilter colorFilter) {
        throw new UnsupportedOperationException();
    }

    @Override
    public int getOpacity() {
        return PixelFormat.TRANSLUCENT;
    }
}
