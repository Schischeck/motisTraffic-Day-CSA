package de.motis_project.app.query.guesser;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v4.content.ContextCompat;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.List;

import de.motis_project.app.R;

public class GuesserListAdapter extends ArrayAdapter<StationGuess> {
    private final LayoutInflater inflater;
    private int favoriteCount;

    private static final int FAVORITE_ITEM = 0;
    private static final int SUGGESTED_ITEM = 1;

    public GuesserListAdapter(Context context) {
        super(context, R.layout.query_guesser_list_item);
        inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    public void setContent(List<StationGuess> favorites, List<StationGuess> suggestions) {
        clear();
        addAll(favorites);
        addAll(suggestions);
        favoriteCount = favorites.size();
    }

    @Override
    public int getItemViewType(int position) {
        return position < favoriteCount ? FAVORITE_ITEM : SUGGESTED_ITEM;
    }

    @NonNull
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if (convertView == null) {
            convertView = inflater.inflate(R.layout.query_guesser_list_item, parent, false);
        }

        int viewType = getItemViewType(position);
        TextView tv = (TextView) convertView.findViewById(R.id.guess_text);
        ImageView icon = (ImageView) convertView.findViewById(R.id.guess_icon);

        tv.setText(getItem(position).name);
        if (viewType == FAVORITE_ITEM) {
            icon.setImageDrawable(ContextCompat.getDrawable(getContext(), R.drawable.ic_favorite_black_24dp));
        } else {
            icon.setImageDrawable(ContextCompat.getDrawable(getContext(), R.drawable.ic_place_black_24dp));
        }
        return convertView;
    }

    @Override
    public int getViewTypeCount() {
        return 2;
    }
}
