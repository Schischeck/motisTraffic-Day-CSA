package de.motis_project.app.query.guesser;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.support.annotation.NonNull;
import android.support.v4.content.ContextCompat;
import android.support.v4.graphics.drawable.DrawableCompat;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.List;

import de.motis_project.app.R;

public class GuesserListAdapter extends ArrayAdapter<LocationGuess> {
    private final LayoutInflater inflater;

    public GuesserListAdapter(Context context) {
        super(context, R.layout.query_guesser_list_item);
        inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    public void setContent(List<LocationGuess> suggestions) {
        clear();
        addAll(suggestions);
    }

    @Override
    public int getItemViewType(int position) {
        return getItem(position).type;
    }

    @NonNull
    @Override
    public View getView(int position, View view, ViewGroup parent) {
        if (view == null) {
            view = inflater.inflate(R.layout.query_guesser_list_item, parent, false);
        }

        LocationGuess item = getItem(position);

        TextView tv = (TextView) view.findViewById(R.id.guess_text);
        tv.setText(item.name);

        int symbol;
        switch (item.type) {
            case LocationGuess.ADDRESS:
                symbol = R.drawable.ic_place_black_24dp;
                break;
            case LocationGuess.FAVORITE:
                symbol = R.drawable.ic_favorite_black_24dp;
                break;
            case LocationGuess.STATION:
                symbol = R.drawable.tram;
                break;
            default:
                symbol = R.drawable.ic_place_black_24dp;
        }
        ImageView icon = (ImageView) view.findViewById(R.id.guess_icon);
        Drawable drawable = ContextCompat.getDrawable(getContext(), symbol);
        DrawableCompat.setTint(drawable, ContextCompat.getColor(getContext(), R.color.md_black));
        icon.setImageDrawable(drawable);

        return view;
    }

    @Override
    public int getViewTypeCount() {
        return 3;
    }
}
