package de.motis_project.app;

import android.content.Context;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;

import de.motis_project.app.query.QueryFragment;

public class TabPagerAdapter extends FragmentPagerAdapter {
    private String search = "";

    public TabPagerAdapter(FragmentManager fm, Context context) {
        super(fm);
        search = context.getString(R.string.search);
    }

    @Override
    public Fragment getItem(int position) {
        switch (position) {
            case 0:
                return new QueryFragment();
            default:
                return new Fragment();
        }
    }

    @Override
    public int getCount() {
        return 2;
    }

    @Override
    public CharSequence getPageTitle(int position) {
        switch (position % 2) {
            case 0:
                return search;
            default:
                return "";
        }
    }
}
