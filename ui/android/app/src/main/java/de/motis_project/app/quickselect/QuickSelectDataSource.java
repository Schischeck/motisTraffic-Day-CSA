package de.motis_project.app.quickselect;

import android.content.ContentValues;
import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import com.squareup.sqlbrite.BriteDatabase;
import com.squareup.sqlbrite.QueryObservable;
import com.squareup.sqlbrite.SqlBrite;

import java.util.List;

import de.motis_project.app.query.guesser.FavoritesDataSource;
import rx.Observable;
import rx.schedulers.Schedulers;

public class QuickSelectDataSource {
    public static class Location {
        public final String name;
        public final String symbol;
        public final String station;
        public final double lat;
        public final double lng;

        public Location(String name, String symbol, String station, double lat, double lng) {
            this.name = name;
            this.symbol = symbol;
            this.station = station;
            this.lat = lat;
            this.lng = lng;
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

        public static final String CREATE_SQL = ""
                + "CREATE TABLE " + TABLE + "("
                + COL_NAME + " TEXT NOT NULL, "
                + COL_STATION + " TEXT, "
                + COL_LAT + " DOUBLE NOT NULL, "
                + COL_LNG + " DOUBLE NOT NULL, "
                + COL_SYMBOL + " TEXT NOT NULL"
                + ")";

        Table(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL(CREATE_SQL);
            db.execSQL(FavoritesDataSource.Table.CREATE_SQL);
        }

        @Override
        public void onUpgrade(SQLiteDatabase sqLiteDatabase, int i, int i1) {
        }
    }

    private static final String SQL_GET = "" +
            "SELECT * FROM " + QuickSelectDataSource.Table.TABLE + " LIMIT 6;";

    private final SqlBrite sqlBrite;
    private final BriteDatabase db;

    public QuickSelectDataSource(Context ctx) {
        sqlBrite = new SqlBrite.Builder()
                .logger(message -> System.out.println("DATABASE message = [" + message + "]"))
                .build();
        db = sqlBrite.wrapDatabaseHelper(new Table(ctx), Schedulers.io());
        db.setLoggingEnabled(true);
    }

    public void addStation(String name, String station, double lat, double lng, String symbol) {
        ContentValues entry = new ContentValues();
        entry.put(Table.COL_NAME, name);
        entry.put(Table.COL_STATION, station);
        entry.put(Table.COL_LAT, lat);
        entry.put(Table.COL_LNG, lng);
        entry.put(Table.COL_SYMBOL, symbol);
        db.insert(Table.TABLE, entry);
    }

    public void addLocation(String name, double lat, double lng, String symbol) {
        ContentValues entry = new ContentValues();
        entry.put(Table.COL_NAME, name);
        entry.putNull(Table.COL_STATION);
        entry.put(Table.COL_LAT, lat);
        entry.put(Table.COL_LNG, lng);
        entry.put(Table.COL_SYMBOL, symbol);

        db.insert(Table.TABLE, entry);
    }

    public Observable<List<Location>> getAll() {
        QueryObservable obs = db.createQuery(QuickSelectDataSource.Table.TABLE, SQL_GET);
        return obs.mapToList(c -> {
            String name = c.getString(c.getColumnIndex(Table.COL_NAME));
            String symbol = c.getString(c.getColumnIndex(Table.COL_SYMBOL));
            String stationId = c.getString(c.getColumnIndex(Table.COL_STATION));
            double lat = c.getDouble(c.getColumnIndex(Table.COL_LAT));
            double lng = c.getDouble(c.getColumnIndex(Table.COL_LNG));

            return new Location(name, symbol, stationId, lat, lng);
        });
    }

    public void clearTable() {
        db.executeAndTrigger(Table.TABLE, "DELETE FROM " + Table.TABLE + ";");
    }
}
