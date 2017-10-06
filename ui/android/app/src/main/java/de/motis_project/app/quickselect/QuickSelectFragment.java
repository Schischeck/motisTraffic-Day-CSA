package de.motis_project.app.quickselect;

import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import de.motis_project.app.R;
import de.motis_project.app.io.Status;

public class QuickSelectFragment extends Fragment {
    public QuickSelectFragment() {
    }

    @BindView(R.id.add_place_fab)
    FloatingActionButton addPlaceFAB;

    @BindView(R.id.quickselect)
    QuickSelectCircle quickselect;

    @OnClick(R.id.add_place_fab)
    public void setAddPlace(View view) {
        Status.get().getQuickSelectDb().updateSymbolOrInsert("", "", 0.0, 0.0, "\uD83C\uDFED");
    }

    @OnClick(R.id.add_person_fab)
    public void setAddPerson(View view) {
        Status.get().getQuickSelectDb().clearTable();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View layout = inflater.inflate(R.layout.quickselect_fragment, container, false);
        ButterKnife.bind(this, layout);


        return layout;
    }

}
