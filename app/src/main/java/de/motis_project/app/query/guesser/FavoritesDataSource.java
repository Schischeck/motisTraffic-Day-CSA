package de.motis_project.app.query.guesser;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import com.squareup.sqlbrite.BriteDatabase;
import com.squareup.sqlbrite.QueryObservable;
import com.squareup.sqlbrite.SqlBrite;

import java.util.List;

import rx.Observable;
import rx.android.schedulers.AndroidSchedulers;
import rx.schedulers.Schedulers;

public class FavoritesDataSource {
    private class FavoritesDbHelper extends SQLiteOpenHelper {
        static final int DATABASE_VERSION = 1;
        static final String DATABASE_NAME = "stations.db";

        static final String TABLE = "favorites";
        static final String COL_STATION_ID = "_id";
        static final String COL_STATION_NAME = "name";
        static final String COL_SELECTED_COUNT = "count";

        private static final String CREATE_LIST = ""
                + "CREATE TABLE " + TABLE + "("
                + COL_STATION_ID + " TEXT NOT NULL PRIMARY KEY,"
                + COL_STATION_NAME + " TEXT NOT NULL,"
                + COL_SELECTED_COUNT + " INTEGER NOT NULL DEFAULT 0"
                + ")";

        FavoritesDbHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL(CREATE_LIST);
        }

        @Override
        public void onUpgrade(SQLiteDatabase sqLiteDatabase, int i, int i1) {
        }
    }

    private static final String SQL_GET_TOP = "" +
            "SELECT * FROM " + FavoritesDbHelper.TABLE +
            " WHERE " + FavoritesDbHelper.COL_STATION_NAME + " LIKE '%%%s%%'" +
            " ORDER BY " + FavoritesDbHelper.COL_SELECTED_COUNT +
            " DESC LIMIT 5";

    private final SqlBrite sqlBrite;
    private final BriteDatabase db;

    public FavoritesDataSource(Context ctx) {
        sqlBrite = new SqlBrite.Builder().logger(
                message -> System.out.println("DATABASE message = [" + message + "]")).build();
        db = sqlBrite.wrapDatabaseHelper(new FavoritesDbHelper(ctx), Schedulers.io());
        db.setLoggingEnabled(true);
    }

    public void addOrIncrement(String eva, String stationName) {
        try (BriteDatabase.Transaction t = db.newTransaction()) {
            db.execute(
                    "INSERT OR IGNORE INTO " + FavoritesDbHelper.TABLE +
                            " VALUES ('" + eva + "', '" + stationName + "', 0)");
            db.execute(
                    "UPDATE " + FavoritesDbHelper.TABLE +
                            " SET " + FavoritesDbHelper.COL_SELECTED_COUNT + " = " +
                            FavoritesDbHelper.COL_SELECTED_COUNT + " + 1 " +
                            " WHERE " + FavoritesDbHelper.COL_STATION_ID + " = " + eva);
            t.markSuccessful();
        }
    }

    public Observable<List<StationGuess>> getFavorites(CharSequence queryString) {
        String query = String.format(SQL_GET_TOP, queryString);
        QueryObservable obs = db.createQuery(FavoritesDbHelper.TABLE, query);
        return obs.mapToList(c -> {
            String eva = c.getString(c.getColumnIndex(FavoritesDbHelper.COL_STATION_ID));
            String name = c.getString(c.getColumnIndex(FavoritesDbHelper.COL_STATION_NAME));
            int count = c.getInt(c.getColumnIndex(FavoritesDbHelper.COL_SELECTED_COUNT));
            return new StationGuess(eva, name, count);
        }).subscribeOn(Schedulers.io()).observeOn(AndroidSchedulers.mainThread());
    }

    public void closeDb() {
        db.close();
    }
}
