package de.motis_project.app.favorites;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.support.v4.content.ContextCompat;
import android.support.v4.graphics.drawable.DrawableCompat;
import android.text.TextPaint;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import de.motis_project.app.R;

public class FavoritesCircle extends View implements View.OnTouchListener {
    static final TextPaint tPaint = new TextPaint();
    final static Paint dbgPaint = new Paint();
    final Paint accentPaint = new Paint();
    final Paint highlightPaint = new Paint();
    private List<EmojiCircle> icons = new ArrayList<>();
    private final Paint shadowPaint = new Paint();
    private boolean touchDown = false;
    private MotionEvent.PointerCoords touchNowPos = new MotionEvent.PointerCoords();
    private MotionEvent.PointerCoords touchStartPos = new MotionEvent.PointerCoords();

    static {
        tPaint.setAntiAlias(true);
        tPaint.setTextAlign(Paint.Align.CENTER);

        dbgPaint.setColor(Color.RED);
        dbgPaint.setAntiAlias(false);
        dbgPaint.setStyle(Paint.Style.STROKE);
        dbgPaint.setStrokeWidth(10.0f);
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

    private void drawCircleSymbols(Canvas c) {
        float cx = c.getWidth() / 2.0f;
        float cy = c.getHeight() / 2.0f;
        float itemCount = icons.size();
        float dim = c.getWidth() / 3.0f;
        tPaint.setTextSize(iconTextSize(c));
        int i = 0;
        for (EmojiCircle symbol : icons) {
            float x = (float) (cx + dim * Math.cos(Math.PI * (i * 2) / itemCount));
            float y = (float) (cy + dim * Math.sin(Math.PI * (i * 2) / itemCount));
            symbol.draw(c, x, y, iconTextSize(c));
            i++;
        }
    }

    private static float iconTextSize(Canvas c) {
        return c.getWidth() / 10;
    }

    private void init(Context c) {
        buildIcons();

        accentPaint.setColor(ContextCompat.getColor(c, R.color.colorAccent));
        accentPaint.setAntiAlias(true);
        accentPaint.setStrokeCap(Paint.Cap.ROUND);

        highlightPaint.setColor(ContextCompat.getColor(c, R.color.colorPrimary));
        highlightPaint.setAntiAlias(true);
        highlightPaint.setStyle(Paint.Style.STROKE);

        shadowPaint.setShadowLayer(15.0f, 0.0f, 5.0f, 0x55000000);
        shadowPaint.setColor(Color.WHITE);
        shadowPaint.setAntiAlias(true);
        setLayerType(LAYER_TYPE_SOFTWARE, shadowPaint);
        setOnTouchListener(this);
    }

    void buildIcons() {
        EmojiCircle[] symbols = {
                new EmojiCircle(new String(Character.toChars(0x1F498)), tPaint, highlightPaint),
                new EmojiCircle(new String(Character.toChars(0x1F3EB)), tPaint, highlightPaint),
                new EmojiCircle(new String(Character.toChars(0x1F3CA)), tPaint, highlightPaint),
                new EmojiCircle(new String(Character.toChars(0x1F3E4)), tPaint, highlightPaint),
                new EmojiCircle(new String(Character.toChars(0x26BD)), tPaint, highlightPaint),
                new EmojiCircle(new String(Character.toChars(0x1F354)), tPaint, highlightPaint)};
        icons = Arrays.asList(symbols);
    }

    public int isIconPos(float x, float y) {
        for (int i = 0; i < icons.size(); i++) {
            if (icons.get(i).getBounds().contains((int) x, (int) y)) {
                return i;
            }
        }
        return -1;
    }

    public int isIconPos(MotionEvent.PointerCoords pos) {
        return isIconPos(pos.x, pos.y);
    }

    protected final void onDraw(Canvas c) {
        int width = c.getWidth();
        accentPaint.setStrokeWidth(0.03f * (width * 2));
        highlightPaint.setStrokeWidth(0.005f * (width * 2));
        drawCircleSymbols(c);
        drawMyPosSymbol(c);

        // connecting line and circle highlights
        int startIconIdx = isIconPos(touchStartPos);
        int nowIconIndex = isIconPos(touchNowPos);
        if (touchDown && startIconIdx >= 0 && nowIconIndex >= 0) {
            icons.get(startIconIdx).drawHighlight(c);
            icons.get(nowIconIndex).drawHighlight(c);
            Rect lineBegin = icons.get(startIconIdx).getBounds();
            c.drawLine(lineBegin.exactCenterX(), lineBegin.exactCenterY(),
                    touchNowPos.x, touchNowPos.y, accentPaint);
        } else if (touchDown && startIconIdx >= 0 /* && nowIconIdx < 0) */) {
            icons.get(startIconIdx).drawHighlight(c);
            Rect lineBegin = icons.get(startIconIdx).getBounds();
            c.drawLine(lineBegin.exactCenterX(), lineBegin.exactCenterY(), touchNowPos.x, touchNowPos.y, accentPaint);
        }
    }

    private void drawMyPosSymbol(Canvas canvas) {
        highlightPaint.setStyle(Paint.Style.FILL);
        float iconRadius = 0.66f * iconTextSize(canvas);
        float iconSize = iconRadius / 2.0f;
        float cx = canvas.getWidth() / 2.0f;
        float cy = canvas.getHeight() / 2.0f;
        canvas.drawCircle(cx, cy, iconRadius, shadowPaint);
        canvas.drawCircle(cx, cy, iconRadius, highlightPaint);
        Drawable myPosDrawable = getResources().getDrawable(R.drawable.ic_my_location_black_24dp);
        DrawableCompat.setTint(DrawableCompat.wrap(myPosDrawable), Color.WHITE);
        myPosDrawable.setBounds((int) (cx - iconSize),
                (int) (cy - iconSize),
                (int) (iconSize + cx),
                (int) (iconSize + cy));
        myPosDrawable.draw(canvas);
        highlightPaint.setStyle(Paint.Style.STROKE);
    }

    public final boolean onTouch(View v, MotionEvent motionEvent) {
        return onTouchEvent(motionEvent);
    }

    public boolean onTouchEvent(MotionEvent me) {
        me.getPointerCoords(0, touchNowPos);
        if (isIconPos(touchNowPos) >= 0) {
            getParent().requestDisallowInterceptTouchEvent(true);
        }
        invalidate();
        me.getPointerCoords(0, touchNowPos);
        switch (me.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                touchDown = true;
                me.getPointerCoords(0, touchStartPos);
                break;
            case MotionEvent.ACTION_UP:
                touchDown = false;
                break;
        }
        return true;
    }
}