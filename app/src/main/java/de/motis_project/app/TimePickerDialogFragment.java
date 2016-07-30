package de.motis_project.app;

import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.provider.MediaStore;
import android.support.v4.app.DialogFragment;
import android.support.v7.app.AlertDialog;
import android.text.format.DateFormat;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.TimePicker;

import java.util.Calendar;
import java.util.Date;

public class TimePickerDialogFragment extends DialogFragment {

    public interface TimePickerDialogFragmentSetListener {
        void onTimeSet(TimePicker tp, QueryFragment.DateSelected state);
    }

    static TimePickerDialogFragmentSetListener listener;

    QueryFragment.DateSelected state;

    boolean isArrival = false;

    public static TimePickerDialogFragment newInstance(TimePickerDialogFragmentSetListener l, QueryFragment.DateSelected state) {
        TimePickerDialogFragment fragment = new TimePickerDialogFragment();
        fragment.state = state;
        fragment.listener = l;
        return fragment;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        LayoutInflater inflater = getActivity().getLayoutInflater();
        View v = inflater.inflate(R.layout.dialog_time_picker, null);

        final TimePicker tp = (TimePicker) v.findViewById(R.id.timePicker);
        tp.setIs24HourView(new DateFormat().is24HourFormat(getContext()));
        tp.setOnTimeChangedListener(new TimePicker.OnTimeChangedListener() {
            @Override
            public void onTimeChanged(TimePicker timePicker, int h, int m) {
                state.setTime(h, m);
            }
        });

        final RadioButton departureBtn = (RadioButton) v.findViewById(R.id.depature_btn);
        final RadioButton arrivalBtn = (RadioButton) v.findViewById(R.id.arrival_btn);

        if (state.isArrival) {
            arrivalBtn.setChecked(true);
            departureBtn.setChecked(false);
        } else {
            arrivalBtn.setChecked(false);
            departureBtn.setChecked(true);
        }

        tp.setCurrentHour(state.c.get(Calendar.HOUR_OF_DAY));
        tp.setCurrentMinute(state.c.get(Calendar.MINUTE));

        arrivalBtn.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                state.setArrival(b);
            }
        });

        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setView(v)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        listener.onTimeSet(tp, state);
                    }
                })
                .setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                    }
                });

        final Dialog dialog = builder.create();
        dialog.getWindow().requestFeature(Window.FEATURE_NO_TITLE);

        return dialog;
    }

}