package de.motis_project.app.query;

import android.content.SharedPreferences;

import java.util.Calendar;
import java.util.Date;

public class Query {
    private static final String IS_ARRIVAL = "IS_ARRIVAL";
    private static final String YEAR = "QUERY_YEAR";
    private static final String MONTH = "QUERY_MONTH";
    private static final String DAY = "QUERY_DAY";
    private static final String HOUR = "QUERY_HOUR";
    private static final String MINUTE = "QUERY_MINUTE";
    private static final String FROM = "QUERY_FROM";
    private static final String TO = "QUERY_TO";

    private final SharedPreferences q;
    private final Calendar cal;

    public Query(SharedPreferences q) {
        this.q = q;
        this.cal = Calendar.getInstance();
    }

    public boolean isArrival() { return q.getBoolean(IS_ARRIVAL, false); }

    public int getYear() { return q.getInt(YEAR, cal.get(Calendar.YEAR)); }

    public int getMonth() { return q.getInt(MONTH, cal.get(Calendar.MONTH)); }

    public int getDay() { return q.getInt(DAY, cal.get(Calendar.DAY_OF_MONTH)); }

    public int getHour() { return q.getInt(HOUR, cal.get(Calendar.HOUR_OF_DAY)); }

    public int getMinute() { return q.getInt(MINUTE, cal.get(Calendar.MINUTE)); }

    public String getFrom() { return q.getString(FROM, ""); }

    public String getTo() { return q.getString(TO, ""); }

    public Date getTime() {
        Calendar cal = Calendar.getInstance();
        setDate(cal, getYear(), getMonth(), getDay());
        setTime(cal, getHour(), getMinute());
        return cal.getTime();
    }

    public static void setDate(Calendar cal, int year, int month, int day) {
        cal.set(Calendar.YEAR, year);
        cal.set(Calendar.MONTH, month);
        cal.set(Calendar.DAY_OF_MONTH, day);
    }

    public static void setTime(Calendar cal, int hour, int minute) {
        cal.set(Calendar.HOUR_OF_DAY, hour);
        cal.set(Calendar.MINUTE, minute);
    }

    public void setDate(int year, int month, int day) {
        SharedPreferences.Editor edit = q.edit();
        edit.putInt(YEAR, year);
        edit.putInt(MONTH, month);
        edit.putInt(DAY, day);
        edit.apply();
    }

    public void setTime(boolean isArrival, int hour, int minute) {
        SharedPreferences.Editor edit = q.edit();
        edit.putBoolean(IS_ARRIVAL, isArrival);
        edit.putInt(HOUR, hour);
        edit.putInt(MINUTE, minute);
        edit.apply();
    }

    public void setFrom(String from) {
        SharedPreferences.Editor edit = q.edit();
        edit.putString(FROM, from);
        edit.apply();
    }

    public void setTo(String to) {
        SharedPreferences.Editor edit = q.edit();
        edit.putString(TO, to);
        edit.apply();
    }

    public void swapStations() {
        SharedPreferences.Editor edit = q.edit();
        edit.putString(FROM, getTo());
        edit.putString(TO, getFrom());
        edit.apply();
    }
}