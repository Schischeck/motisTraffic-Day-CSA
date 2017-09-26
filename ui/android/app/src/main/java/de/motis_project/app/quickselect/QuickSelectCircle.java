package de.motis_project.app.quickselect;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.support.design.widget.Snackbar;
import android.support.v4.content.ContextCompat;
import android.text.TextPaint;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Stream;

import de.motis_project.app.R;
import de.motis_project.app.io.Status;
import motis.Station;
import rx.Observable;
import rx.Subscriber;
import rx.android.schedulers.AndroidSchedulers;
import rx.functions.Action1;
import rx.schedulers.Schedulers;

public class QuickSelectCircle extends View implements View.OnTouchListener {
    static final TextPaint tPaint = new TextPaint();
    final static Paint dbgPaint = new Paint();
    final Paint accentPaint = new Paint();
    final Paint highlightPaint = new Paint();
    private List<SelectableItem> icons = new ArrayList<>();
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

    public QuickSelectCircle(Context c) {
        super(c);
        init(c);
    }

    public QuickSelectCircle(Context c, AttributeSet attribs) {
        super(c, attribs);
        buildIcons();
        init(c);
    }

    private static float iconTextSize(Canvas c) {
        return c.getWidth() / 10;
    }

    private void init(Context c) {
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

        buildIcons();
    }

    void buildIcons() {
        Paint myPosBgPaint = new Paint(highlightPaint);
        Paint myPosHlPaint = new Paint(accentPaint);
        myPosHlPaint.setStyle(Paint.Style.STROKE);

        Status.get()
                .getQuickSelectDb()
                .getAll()
                .startWith(new ArrayList<QuickSelectDataSource.Location>())
                .map(locations -> {
                    List<SelectableItem> newicons = new ArrayList<>();
                    newicons.add(
                            new DrawableSymbol(myPosBgPaint, myPosHlPaint, getResources().getDrawable(R.drawable.ic_my_location_black_24dp)));
                    for (QuickSelectDataSource.Location e : locations) {
                        newicons.add(new EmojiCircle(e.symbol, tPaint, this.highlightPaint));
                    }
                    return newicons;
                })
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(new Subscriber<List<SelectableItem>>() {
                    @Override
                    public void onCompleted() {
                    }

                    @Override
                    public void onError(Throwable e) {
                    }

                    @Override
                    public void onNext(List<SelectableItem> selectableItems) {
                        icons = selectableItems;
                        invalidate();
                    }
                });
    }

    public int isEmojiPos(float x, float y) {
        for (int i = 0; i < icons.size(); i++) {
            if (icons.get(i).getBounds().contains((int) x, (int) y)) {
                return i;
            }
        }
        return -1;
    }

    public int isEmojiPos(MotionEvent.PointerCoords pos) {
        return isEmojiPos(pos.x, pos.y);
    }

    protected final void onDraw(Canvas c) {
        int width = c.getWidth();
        accentPaint.setStrokeWidth(0.03f * (width * 2));
        highlightPaint.setStrokeWidth(0.005f * (width * 2));
        drawCircleSymbols(c);

        // connecting line and circle highlights
        int startIconIdx = isEmojiPos(touchStartPos);
        int nowIconIndex = isEmojiPos(touchNowPos);
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

    private void drawCircleSymbols(Canvas c) {
        float cx = c.getWidth() / 2.0f;
        float cy = c.getHeight() / 2.0f;
        float itemCount = icons.size() - 1;
        float dim = c.getWidth() / 3.0f;
        tPaint.setTextSize(iconTextSize(c));
        for (int i = 0; i < icons.size(); ++i) {
            SelectableItem symbol = icons.get(i);
            float x, y, size;
            if (i == 0) {
                x = c.getWidth() / 2.0f;
                y = c.getHeight() / 2.0f;
                size = iconTextSize(c) * 0.8f;
            } else {
                x = (float) (cx + dim * Math.cos(Math.PI * ((i - 1) * 2) / itemCount));
                y = (float) (cy + dim * Math.sin(Math.PI * ((i - 1) * 2) / itemCount));
                size = iconTextSize(c);
            }
            symbol.draw(c, x, y, size);
        }
    }

    public final boolean onTouch(View v, MotionEvent motionEvent) {
        return onTouchEvent(motionEvent);
    }

    public boolean onTouchEvent(MotionEvent me) {
        me.getPointerCoords(0, touchNowPos);
        if (isEmojiPos(touchNowPos) >= 0) {
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
                handleGuesture();
                break;
        }
        return true;
    }

    private void handleGuesture() {
        int down_idx = isEmojiPos(touchStartPos);
        int up_idx = isEmojiPos(touchNowPos);
        if (down_idx != -1 && up_idx != -1) {
            Toast.makeText(getContext(), "from=" + down_idx + " to=" + up_idx, Toast.LENGTH_SHORT).show();
        }
    }
}