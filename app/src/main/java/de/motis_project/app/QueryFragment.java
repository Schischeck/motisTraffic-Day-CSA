package de.motis_project.app;

import android.app.Activity;
import android.app.DatePickerDialog;
import android.app.Dialog;
import android.app.TimePickerDialog;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
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
import android.widget.TimePicker;

import java.sql.Time;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import butterknife.BindString;
import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.OnTouch;


public class QueryFragment extends Fragment implements DatePickerDialog.OnDateSetListener, TimePickerDialogFragment.TimePickerDialogFragmentSetListener {
    public static final int SELECT_START_LOCATION = 1;
    public static final int SELECT_DEST_LOCATION = 2;

    static class DateSelected {
        Calendar c;
        boolean isArrival;

        DateSelected() {
            c = Calendar.getInstance();
            this.isArrival = false;
        }

        void setTime(int hour, int minute) {
            c.set(Calendar.HOUR_OF_DAY, hour);
            c.set(Calendar.MINUTE, minute);
        }

        void setDate(int dayOfMonth, int month, int year) {
            c.set(Calendar.DAY_OF_MONTH, dayOfMonth);
            c.set(Calendar.MONTH, month);
            c.set(Calendar.YEAR, year);
        }

        void setArrival(boolean isArrival) {
            this.isArrival = isArrival;
        }
    }

    private OnFragmentInteractionListener mListener;

    DateSelected dateSelected;

    @BindString(R.string.arrival_short)
    String arrivalStr;

    @BindString(R.string.departure_short)
    String departureStr;

    @BindView(R.id.start_input)
    EditText startInput;

    @BindView(R.id.dest_input)
    EditText destInput;

    @BindView(R.id.date_text)
    TextView dateText;

    @BindView(R.id.time_text)
    TextView timeText;

    @OnTouch(R.id.start_input)
    boolean openSearchStart(View v, MotionEvent e) {
        openSearch(v, e, QueryFragment.SELECT_START_LOCATION, startInput.getText().toString());
        return true;
    }

    @OnTouch(R.id.dest_input)
    boolean openSearchDest(View v, MotionEvent e) {
        openSearch(v, e, QueryFragment.SELECT_DEST_LOCATION, destInput.getText().toString());
        return true;
    }

    @OnClick(R.id.date_select)
    void showDatePickerDialog(View v) {
        DialogFragment dialogFragment = DatePickerDialogFragment.newInstance(this, dateSelected);
        dialogFragment.show(getActivity().getSupportFragmentManager(), "datePicker");
    }

    @OnClick(R.id.time_select)
    void showTimePickerDialog(View v) {
        DialogFragment dialogFragment = TimePickerDialogFragment.newInstance(this, dateSelected);
        dialogFragment.show(getActivity().getSupportFragmentManager(), "timePicker");
    }

    void openSearch(@Nullable View v, MotionEvent e, int requestCode, String query) {
        if (e.getAction() == MotionEvent.ACTION_UP && mListener != null) {
            Intent i = new Intent((Context) mListener, SearchActivity.class);
            i.putExtra("query", query);
            startActivityForResult(i, requestCode);
        }
    }

    public QueryFragment() {
        // Required empty public constructor
    }

    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @param param1 Parameter 1.
     * @param param2 Parameter 2.
     * @return A new instance of fragment QueryFragment.
     */
    // TODO: Rename and change types and number of parameters
    public static QueryFragment newInstance(String param1, String param2) {
        QueryFragment fragment = new QueryFragment();
        fragment.setArguments(new Bundle());
        return fragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View view = inflater.inflate(R.layout.fragment_query, container, false);
        ButterKnife.bind(this, view);

        dateSelected = new DateSelected();
        dateText.setText(getDateDisplayString(dateSelected));
        timeText.setText(getTimeDisplayString(dateSelected));

        return view;
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        if (context instanceof OnFragmentInteractionListener) {
            mListener = (OnFragmentInteractionListener) context;
        } else {
            throw new RuntimeException(context.toString()
                    + " must implement OnFragmentInteractionListener");
        }
    }

    @Override
    public void onDetach() {
        super.onDetach();
        mListener = null;
    }

    /**
     * This interface must be implemented by activities that contain this
     * fragment to allow an interaction in this fragment to be communicated
     * to the activity and potentially other fragments contained in that
     * activity.
     * <p/>
     * See the Android Training lesson <a href=
     * "http://developer.android.com/training/basics/fragments/communicating.html"
     * >Communicating with Other Fragments</a> for more information.
     */
    public interface OnFragmentInteractionListener {
        // TODO: Update argument type and name
        void onFragmentInteraction(Uri uri);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (data == null || data.getExtras() == null || data.getExtras().getString("result") == null || resultCode != Activity.RESULT_OK) {
            return;
        }
        Bundle extras = data.getExtras();
        String result = extras.getString("result");
        switch (requestCode) {
            case SELECT_START_LOCATION:
                startInput.setText(result);
                break;
            case SELECT_DEST_LOCATION:
                destInput.setText(result);
                break;
        }
    }

    @Override
    public void onDateSet(@Nullable DatePicker view, int year, int month, int day) {
        dateSelected.setDate(day, month, year);
        dateText.setText(getDateDisplayString(dateSelected));
    }

    @Override
    public void onTimeSet(@Nullable TimePicker timePicker, DateSelected state) {
        dateSelected = state;
        timeText.setText(getTimeDisplayString(dateSelected));
    }

    String getTimeDisplayString(DateSelected state) {
        String prefix = (state.isArrival ? arrivalStr : departureStr) + " ";
        return prefix + SimpleDateFormat.getTimeInstance(java.text.DateFormat.SHORT).format(state.c.getTime());
    }

    String getDateDisplayString(DateSelected state) {
        return SimpleDateFormat.getDateInstance(java.text.DateFormat.SHORT).format(state.c.getTime());
    }
}
