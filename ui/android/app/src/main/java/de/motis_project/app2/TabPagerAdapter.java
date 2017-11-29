package de.motis_project.app2;

import android.content.Context;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;

import de.motis_project.app2.query.QueryFragment;
import de.motis_project.app2.saved.SavedConnectionsFragment;

public class TabPagerAdapter extends FragmentPagerAdapter {
    private String search = "";
    private String connections = "";

    public TabPagerAdapter(FragmentManager fm, Context context) {
        super(fm);
        search = context.getString(R.string.search);
        connections = context.getString(R.string.connections);
    }

    @Override
    public Fragment getItem(int position) {
        switch (position) {
            case 0:
                return new QueryFragment();
            case 1:
                return new SavedConnectionsFragment();
            default:
                return null;
        }
    }

    @Override
    public int getCount() {
        return 2;
    }

    @Override
    public CharSequence getPageTitle(int position) {
        switch (position) {
            case 0:
                return search;
            case 1:
                return connections;
            default:
                return "";
        }
    }
}
