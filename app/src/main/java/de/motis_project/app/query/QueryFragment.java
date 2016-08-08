package de.motis_project.app.query;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.DatePicker;
import android.widget.EditText;
import android.widget.TextView;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import butterknife.BindString;
import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.OnTouch;
import de.motis_project.app.R;
import de.motis_project.app.journey.JourneyListView;

public class QueryFragment extends Fragment
        implements android.app.DatePickerDialog.OnDateSetListener, TimePickerDialogFragment.ChangeListener {
    public static final int SELECT_START_LOCATION = 1;
    public static final int SELECT_DEST_LOCATION = 2;

    private Context context;
    private Query query;

    @BindString(R.string.arrival_short)
    String arrivalStr;

    @BindString(R.string.departure_short)
    String departureStr;

    @BindView(R.id.start_input)
    EditText fromInput;

    @BindView(R.id.dest_input)
    EditText toInput;

    @BindView(R.id.date_text)
    TextView dateText;

    @BindView(R.id.time_text)
    TextView timeText;

    @BindView(R.id.connection_list)
    JourneyListView journeyListView;

    public QueryFragment() {
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.query_fragment, container, false);
        ButterKnife.bind(this, view);

        query = new Query(
                savedInstanceState,
                getContext().getSharedPreferences("query", Context.MODE_PRIVATE));

        Date d = query.getTime();
        updateTimeDisplay(query.isArrival(), d);
        updateDateDisplay(d);

        fromInput.setText(query.getFromName());
        toInput.setText(query.getToName());

        journeyListView.scrollToPosition(1);

        return view;
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        query.updateBundle(outState);
    }

    @OnTouch(R.id.start_input)
    boolean openSearchStart(View v, MotionEvent e) {
        openGuesser(v, e, SELECT_START_LOCATION, fromInput.getText().toString());
        return true;
    }

    @OnTouch(R.id.dest_input)
    boolean openSearchDest(View v, MotionEvent e) {
        openGuesser(v, e, SELECT_DEST_LOCATION, toInput.getText().toString());
        return true;
    }

    @OnClick(R.id.date_select)
    void showDatePickerDialog() {
        DialogFragment dialogFragment = DatePickerDialogFragment.newInstance(
                query.getYear(), query.getMonth(), query.getDay());
        dialogFragment.setTargetFragment(this, 0);
        dialogFragment.show(getActivity().getSupportFragmentManager(), "datePicker");
    }

    @OnClick(R.id.time_select)
    void showTimePickerDialog() {
        DialogFragment dialogFragment = TimePickerDialogFragment.newInstance(
                query.isArrival(), query.getHour(), query.getMinute());
        dialogFragment.setTargetFragment(this, 0);
        dialogFragment.show(getActivity().getSupportFragmentManager(), "timePicker");
    }

    @OnClick(R.id.switch_stations_btn)
    void swapStations() {
        query.swapStations();

        String from = fromInput.getText().toString();
        fromInput.setText(toInput.getText().toString());
        toInput.setText(from);
    }

    private void openGuesser(@Nullable View v, MotionEvent e, int requestCode, String query) {
        if (e.getAction() != MotionEvent.ACTION_UP || context == null) {
            return;
        }

        Intent i = new Intent(context, GuesserActivity.class);
        i.putExtra(GuesserActivity.QUERY, query);
        startActivityForResult(i, requestCode);
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        this.context = context;
    }

    @Override
    public void onDetach() {
        super.onDetach();
        context = null;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (data == null || data.getExtras() == null ||
                data.getExtras().getString(GuesserActivity.RESULT_NAME) == null ||
                data.getExtras().getString(GuesserActivity.RESULT_ID) == null ||
                resultCode != Activity.RESULT_OK) {
            return;
        }

        String name = data.getExtras().getString(GuesserActivity.RESULT_NAME);
        String id = data.getExtras().getString(GuesserActivity.RESULT_ID);
        switch (requestCode) {
            case SELECT_START_LOCATION:
                query.setFrom(id, name);
                fromInput.setText(name);
                break;
            case SELECT_DEST_LOCATION:
                query.setTo(id, name);
                toInput.setText(name);
                break;
        }
    }

    @Override
    public void onDateSet(@Nullable DatePicker view, int year, int month, int day) {
        query.setDate(year, month, day);

        Calendar cal = Calendar.getInstance();
        Query.setDate(cal, year, month, day);
        updateDateDisplay(cal.getTime());
    }

    @Override
    public void onTimeSet(boolean isArrival, int hour, int minute) {
        query.setTime(isArrival, hour, minute);

        Calendar cal = Calendar.getInstance();
        Query.setTime(cal, hour, minute);
        updateTimeDisplay(isArrival, cal.getTime());
    }

    private void updateTimeDisplay(boolean isArrival, Date time) {
        String formattedTime = SimpleDateFormat
                .getTimeInstance(java.text.DateFormat.SHORT)
                .format(time);
        String formattedArrival = (isArrival ? arrivalStr : departureStr);
        timeText.setText(formattedArrival + " " + formattedTime);
    }

    private void updateDateDisplay(Date date) {
        String formattedDate = SimpleDateFormat
                .getDateInstance(java.text.DateFormat.SHORT)
                .format(date);
        dateText.setText(formattedDate);
    }
}