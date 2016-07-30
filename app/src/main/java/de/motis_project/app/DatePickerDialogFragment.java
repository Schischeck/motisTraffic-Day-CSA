package de.motis_project.app;

import android.app.DatePickerDialog;
import android.app.Dialog;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;

import java.util.Calendar;

public class DatePickerDialogFragment extends DialogFragment {

    DatePickerDialog.OnDateSetListener listener;

    QueryFragment.DateSelected state;

    public static DatePickerDialogFragment newInstance(DatePickerDialog.OnDateSetListener l, QueryFragment.DateSelected state) {
        DatePickerDialogFragment fragment = new DatePickerDialogFragment();
        fragment.state = state;
        fragment.listener = l;
        return fragment;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        int year = state.c.get(Calendar.YEAR);
        int month = state.c.get(Calendar.MONTH);
        int day = state.c.get(Calendar.DAY_OF_MONTH);
        return new DatePickerDialog(getActivity(), listener, year, month, day);
    }
}