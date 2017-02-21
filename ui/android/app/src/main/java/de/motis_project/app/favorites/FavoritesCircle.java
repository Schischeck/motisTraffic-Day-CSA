package de.motis_project.app.favorites;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class FavoritesCircle extends View {
    private List<LinearLayout> icons = new ArrayList<>();

    public FavoritesCircle(Context context, AttributeSet attributes) {
        super(context, attributes);
        buildIcons(context);
    }

    public FavoritesCircle(Context context) {
        super(context);
        buildIcons(context);
    }

    void buildIcons(Context context) {
        icons = Arrays.asList(buildIcon(context, 0x1F498),
                              buildIcon(context, 0x1F3EB),
                              buildIcon(context, 0x1F3CA),
                              buildIcon(context, 0x1F3E4),
                              buildIcon(context, 0x26BD),
                              buildIcon(context, 0x1F354));
    }

    private static LinearLayout buildIcon(Context context, int symbol) {
        LinearLayout layout = new LinearLayout(context);
        layout.setGravity(Gravity.CENTER_VERTICAL | Gravity.CENTER_HORIZONTAL);
        final TextView tv = new TextView(context);
        tv.setText(new String(Character.toChars(symbol)));
        layout.addView(tv);
        layout.measure(1, 1);
        layout.layout(0, 0, 1, 1);
        System.out.println(layout.getWidth() + ", " + layout.getHeight());
        return layout;
    }

    @Override
    protected final void onDraw(final Canvas canvas) {
        float cx = canvas.getWidth() / 2.0f;
        float cy = canvas.getHeight()  / 2.0f;
        float n = icons.size();
        float r = canvas.getWidth() / 3.0f;

        int i = 0;
        for (LinearLayout icon : icons) {
            float x = (float) (cx + r * Math.cos(2 * i * Math.PI / n));
            float y = (float) (cy + r * Math.sin(2 * i * Math.PI / n));
            canvas.translate(x, y);
            canvas.scale(3, 3);
            icon.draw(canvas);
            canvas.scale(1.0f / 3.0f, 1.0f / 3.0f);
            canvas.translate(-x, -y);
            ++i;
        }
    }
}
