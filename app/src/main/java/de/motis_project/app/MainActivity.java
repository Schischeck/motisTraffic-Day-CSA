package de.motis_project.app;

import android.os.Bundle;
import android.support.annotation.StringRes;
import android.support.design.widget.TabLayout;
import android.support.v4.app.FragmentActivity;
import android.support.v4.view.ViewPager;
import android.widget.Toast;

import butterknife.BindView;
import butterknife.ButterKnife;

public class MainActivity extends FragmentActivity {
    void showToast(@StringRes final int text) {
        Toast toast = Toast.makeText(getApplicationContext(), getResources().getResourceName(text), Toast.LENGTH_SHORT);
        toast.show();
    }

    @BindView(R.id.viewPager)
    ViewPager viewPager;

    @BindView(R.id.tabLayout)
    TabLayout tabLayout;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);

        viewPager.setAdapter(new TabPagerAdapter(getSupportFragmentManager()));
        tabLayout.setupWithViewPager(viewPager);
    }

}
