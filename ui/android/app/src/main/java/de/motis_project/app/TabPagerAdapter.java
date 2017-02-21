package de.motis_project.app;

import android.content.Context;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;

import de.motis_project.app.favorites.FavoritesFragment;
import de.motis_project.app.query.QueryFragment;
import de.motis_project.app.saved.SavedConnectionsFragment;

public class TabPagerAdapter extends FragmentPagerAdapter {
    private String search = "";
    private String connections = "";
    private String favorites = "";

    public TabPagerAdapter(FragmentManager fm, Context context) {
        super(fm);
        search = context.getString(R.string.search);
        connections = context.getString(R.string.connections);
        favorites = context.getString(R.string.favorites);
    }

    @Override
    public Fragment getItem(int position) {
        switch (position) {
            case 0:
                return new FavoritesFragment();
            case 1:
                return new QueryFragment();
            case 2:
                return new SavedConnectionsFragment();
            default:
                return null;
        }
    }

    @Override
    public int getCount() {
        return 3;
    }

    @Override
    public CharSequence getPageTitle(int position) {
        switch (position) {
            case 0:
                return favorites;
            case 1:
                return search;
            case 2:
                return connections;
            default:
                return "";
        }
    }
}
