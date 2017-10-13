package de.motis_project.app.quickselect;

import android.content.Context;
import android.database.DatabaseUtils;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.support.annotation.Nullable;

import com.squareup.sqlbrite.BriteDatabase;
import com.squareup.sqlbrite.SqlBrite;

import java.util.List;
import java.util.Locale;

import rx.Observable;
import rx.schedulers.Schedulers;

public class QuickSelectDataSource {
    public static class Location {
        public final String name;
        public final String symbol;
        public final String station;
        public final double lat;
        public final double lng;
        public final int count;

        public Location(String name, String symbol, String station, double lat, double lng, int count) {
            this.name = name;
            this.symbol = symbol;
            this.station = station;
            this.lat = lat;
            this.lng = lng;
            this.count = count;
        }
    }

    public static class Table extends SQLiteOpenHelper {
        static final int DATABASE_VERSION = 1;
        static final String DATABASE_NAME = "stations.db";

        static final String TABLE = "quickselect";
        static final String COL_NAME = "name";
        static final String COL_STATION = "station";
        static final String COL_LAT = "lat";
        static final String COL_LNG = "lng";
        static final String COL_SYMBOL = "symbol";
        static final String COL_COUNT = "count";

        public static final String CREATE_SQL = ""
                + "CREATE TABLE " + TABLE + "("
                + COL_NAME + " TEXT NOT NULL PRIMARY KEY, "
                + COL_STATION + " TEXT, "
                + COL_LAT + " DOUBLE NOT NULL, "
                + COL_LNG + " DOUBLE NOT NULL, "
                + COL_SYMBOL + " TEXT, "
                + COL_COUNT + " INTEGER NOT NULL DEFAULT 0"
                + ")";

        Table(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL(CREATE_SQL);
        }

        @Override
        public void onUpgrade(SQLiteDatabase sqLiteDatabase, int i, int i1) {
        }
    }

    private final SqlBrite sqlBrite;
    private final BriteDatabase db;

    public QuickSelectDataSource(Context ctx) {
        sqlBrite = new SqlBrite.Builder()
                .logger(message -> System.out.println("DATABASE message = [" + message + "]"))
                .build();
        db = sqlBrite.wrapDatabaseHelper(new Table(ctx), Schedulers.io());
        db.setLoggingEnabled(true);
    }

    public void addOrIncrement(String name, @Nullable String station, double lat, double lng, @Nullable String symbol) {
        name = DatabaseUtils.sqlEscapeString(name);
        station = station != null ? DatabaseUtils.sqlEscapeString(station) : "NULL";
        symbol = symbol != null ? DatabaseUtils.sqlEscapeString(symbol) : "NULL";
        try (BriteDatabase.Transaction t = db.newTransaction()) {
            db.execute(String.format(Locale.US,
                    "INSERT OR IGNORE INTO %s VALUES(%s, %s, %f, %f, %s, 0)",
                    Table.TABLE, name, station, lat, lng, symbol));
            db.executeAndTrigger(Table.TABLE, String.format(Locale.US,
                    "UPDATE %s SET %s = %s + 1 WHERE %s = %s",
                    Table.TABLE, Table.COL_COUNT, Table.COL_COUNT, Table.COL_NAME, name));
            t.markSuccessful();
        }
    }

    public void updateSymbolOrInsert(String name, @Nullable String station, double lat, double lng, @Nullable String symbol) {
        name = DatabaseUtils.sqlEscapeString(name);
        station = station != null ? DatabaseUtils.sqlEscapeString(station) : "NULL";
        symbol = symbol != null ? DatabaseUtils.sqlEscapeString(symbol) : "NULL";
        try (BriteDatabase.Transaction t = db.newTransaction()) {
            db.executeAndTrigger(Table.TABLE, String.format(Locale.US,
                    "INSERT OR REPLACE INTO %s VALUES(%s, %s, %f, %f, %s, " +
                            "(SELECT %s FROM %s WHERE %s = %s))",
                    Table.TABLE, name, station, lat, lng, symbol, Table.COL_COUNT, Table.TABLE, Table.COL_NAME, name));
            t.markSuccessful();
        }
    }

    public Observable<List<Location>> getFavorites(String in) {
        in = DatabaseUtils.sqlEscapeString("%" + in + "%");

        String query = String.format("SELECT * FROM %s" +
                        " WHERE %s LIKE %s" +
                        " AND %s > 0" +
                        " ORDER BY %s DESC" +
                        " LIMIT 5",
                Table.TABLE,
                Table.COL_NAME, in,
                Table.COL_COUNT,
                Table.COL_COUNT);
        return db.createQuery(QuickSelectDataSource.Table.TABLE,
                query)
                .mapToList(c -> {
                    String name = c.getString(c.getColumnIndex(Table.COL_NAME));
                    String symbol = c.getString(c.getColumnIndex(Table.COL_SYMBOL));
                    String stationId = c.getString(c.getColumnIndex(Table.COL_STATION));
                    double lat = c.getDouble(c.getColumnIndex(Table.COL_LAT));
                    double lng = c.getDouble(c.getColumnIndex(Table.COL_LNG));
                    int count = c.getInt(c.getColumnIndex(Table.COL_COUNT));

                    return new Location(name, symbol, stationId, lat, lng, count);
                });
    }

    public Observable<List<Location>> getQuickSelect() {
        return db.createQuery(QuickSelectDataSource.Table.TABLE,
                String.format("SELECT * FROM %s" +
                                " WHERE %s IS NOT NULL" +
                                " LIMIT 5",
                        Table.TABLE,
                        Table.COL_SYMBOL))
                .mapToList(c -> {
                    String name = c.getString(c.getColumnIndex(Table.COL_NAME));
                    String symbol = c.getString(c.getColumnIndex(Table.COL_SYMBOL));
                    String stationId = c.getString(c.getColumnIndex(Table.COL_STATION));
                    double lat = c.getDouble(c.getColumnIndex(Table.COL_LAT));
                    double lng = c.getDouble(c.getColumnIndex(Table.COL_LNG));
                    int count = c.getInt(c.getColumnIndex(Table.COL_COUNT));

                    return new Location(name, symbol, stationId, lat, lng, count);
                });
    }

    public void clearTable() {
        db.executeAndTrigger(Table.TABLE, "DELETE FROM " + Table.TABLE + ";");
    }
}
