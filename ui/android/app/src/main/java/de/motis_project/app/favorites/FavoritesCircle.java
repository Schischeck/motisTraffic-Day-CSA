package de.motis_project.app.favorites;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.support.v4.content.ContextCompat;
import android.support.v4.graphics.drawable.DrawableCompat;
import android.support.v4.util.Pair;
import android.text.TextPaint;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import de.motis_project.app.R;

public class FavoritesCircle extends View implements View.OnTouchListener {
    private static final TextPaint tPaint = new TextPaint();
    private final Paint accentPaint = new Paint();
    private final Paint dbgPaint = new Paint();
    private final Paint highlightPaint = new Paint();
    private List<Pair<Float, Float>> iconCoords = new ArrayList<>();
    private List<String> icons = new ArrayList<>();
    private final Paint shadowPaint = new Paint();
    private boolean touchDown = false;
    private MotionEvent.PointerCoords touchNowPos = new MotionEvent.PointerCoords();
    private MotionEvent.PointerCoords touchStartPos = new MotionEvent.PointerCoords();

    static {
        tPaint.setTextSize(64);
        tPaint.setAntiAlias(true);
        tPaint.setTextAlign(Paint.Align.CENTER);
    }

    public FavoritesCircle(Context c) {
        super(c);
        init(c);
    }

    public FavoritesCircle(Context c, AttributeSet attribs) {
        super(c, attribs);
        buildIcons();
        init(c);
    }

    private void drawHighlight(Canvas c, int iconIdx) {
        Pair<Float, Float> pos = this.iconCoords.get(iconIdx);
        c.translate(pos.first, pos.second);
        c.drawCircle(0, 0, iconTextSize(c), this.highlightPaint);
        c.translate(-pos.first, -pos.second);
    }

    private void drawCircleSymbols(Canvas c) {
        float cx = c.getWidth() / 2.0f;
        float cy = c.getHeight() / 2.0f;
        float icon_count = this.icons.size();
        float dim = c.getWidth() / 3.0f;
        tPaint.setTextSize(iconTextSize(c));
        this.iconCoords.clear();
        int i = 0;
        for (String symbol : icons) {
            Rect bounds = new Rect();
            tPaint.getTextBounds(symbol, 0, symbol.length(), bounds);
            float x = (float) (cx + dim * Math.cos(Math.PI * (i * 2) / icon_count));
            float y = (float) (cy + dim * Math.sin(Math.PI * (i * 2) / icon_count));
            this.iconCoords.add(new Pair<>(x, y + bounds.exactCenterY()));
            c.translate(x, y);
            c.drawCircle(0, bounds.exactCenterY(), iconTextSize(c), this.shadowPaint);
            c.drawText(symbol, 0, symbol.length(), 0, 0, tPaint);
            c.translate(-x, -y);
            i++;
        }
    }

    private static float iconTextSize(Canvas c) {
        return c.getWidth() / 10;
    }

    private void init(Context c) {
        buildIcons();
        this.accentPaint.setColor(ContextCompat.getColor(c, R.color.colorAccent));
        this.accentPaint.setAntiAlias(true);
        this.accentPaint.setStrokeCap(Paint.Cap.ROUND);
        this.highlightPaint.setColor(ContextCompat.getColor(c, R.color.colorPrimary));
        this.highlightPaint.setAntiAlias(true);
        this.highlightPaint.setStyle(Paint.Style.STROKE);
        this.shadowPaint.setShadowLayer(15.0F, 0.0F, 5.0F, 1426063360);
        this.shadowPaint.setColor(-1);
        this.shadowPaint.setAntiAlias(true);
        setLayerType(1, this.highlightPaint);
        this.dbgPaint.setColor(Color.RED);
        this.dbgPaint.setAntiAlias(false);
        this.dbgPaint.setStyle(Paint.Style.STROKE);
        this.dbgPaint.setStrokeWidth(10.0F);
        setOnTouchListener(this);
    }

    void buildIcons() {
        String[] symbols = {
                new String(Character.toChars(127980)),
                new String(Character.toChars(127979)),
                new String(Character.toChars(127946)),
                new String(Character.toChars(127972)),
                new String(Character.toChars(9917)),
                new String(Character.toChars(127828))};
        this.icons = Arrays.asList(symbols);
    }

    public int isIconPos(float x, float y) {
        for (int i = 0; i < this.iconCoords.size(); i++) {
            Pair<Float, Float> iconPos = iconCoords.get(i);
            float dx = iconPos.first - x;
            float dy = iconPos.second - y;
            if (Math.sqrt(dx * dx + dy * dy) < tPaint.getTextSize()) {
                return i;
            }
        }
        return -1;
    }

    public int isIconPos(MotionEvent.PointerCoords pos) {
        return isIconPos(pos.x, pos.y);
    }

    protected final void onDraw(Canvas c) {
        c.drawARGB(0, 225, 225, 255);
        int width = c.getWidth();
        this.accentPaint.setStrokeWidth(0.03f * (width * 2));
        this.highlightPaint.setStrokeWidth(0.005f * (width * 2));
        drawMyPosSymbol(c);
        drawCircleSymbols(c);

        // connecting line and circle highlights
        int startIconIdx = isIconPos(this.touchStartPos);
        int nowIconIndex = isIconPos(this.touchNowPos);
        if (touchDown && startIconIdx >= 0 && nowIconIndex >= 0) {
            drawHighlight(c, startIconIdx);
            drawHighlight(c, nowIconIndex);
            Pair<Float, Float> lineBegin = this.iconCoords.get(startIconIdx);
            c.drawLine(lineBegin.first, lineBegin.second, touchNowPos.x, touchNowPos.y, this.accentPaint);
        } else if (touchDown && startIconIdx >= 0 /* && nowIconIdx < 0) */) {
            drawHighlight(c, startIconIdx);
            Pair<Float, Float> lineBegin = this.iconCoords.get(startIconIdx);
            c.drawLine(lineBegin.first, lineBegin.second, touchNowPos.x, touchNowPos.y, this.accentPaint);
        }
    }

    private void drawMyPosSymbol(Canvas canvas) {
        this.highlightPaint.setStyle(Paint.Style.FILL);
        float iconRadius = 0.66f * iconTextSize(canvas);
        float iconSize = iconRadius / 2.0f;
        canvas.drawCircle(canvas.getWidth() / 2.0f, canvas.getHeight() / 2.0f, iconRadius, this.shadowPaint);
        canvas.drawCircle(canvas.getWidth() / 2.0f, canvas.getHeight() / 2.0f, iconRadius, this.highlightPaint);
        Drawable myPosDrawable = getResources().getDrawable(R.drawable.ic_delete_black_24dp);
        DrawableCompat.setTint(DrawableCompat.wrap(myPosDrawable), Color.WHITE);
        myPosDrawable.setBounds((int) (canvas.getWidth() / 2.0f - iconSize),
                (int) (canvas.getHeight() / 2.0f - iconSize),
                (int) (iconSize + canvas.getWidth() / 2.0f),
                (int) (iconSize + canvas.getHeight() / 2.0f));
        myPosDrawable.draw(canvas);
        this.highlightPaint.setStyle(Paint.Style.STROKE);
    }

    public final boolean onTouch(View v, MotionEvent motionEvent) {
        return onTouchEvent(motionEvent);
    }

    public boolean onTouchEvent(MotionEvent me) {
        me.getPointerCoords(0, this.touchNowPos);
        if (isIconPos(this.touchNowPos) >= 0) {
            getParent().requestDisallowInterceptTouchEvent(true);
        }
        invalidate();
        me.getPointerCoords(0, this.touchNowPos);
        switch (me.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                touchDown = true;
                me.getPointerCoords(0, this.touchStartPos);
                break;
            case MotionEvent.ACTION_UP:
                touchDown = false;
                break;
        }
        return true;
    }
}