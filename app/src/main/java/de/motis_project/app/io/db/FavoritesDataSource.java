package de.motis_project.app.io.db;

import android.content.Context;

import com.squareup.sqlbrite.BriteDatabase;
import com.squareup.sqlbrite.QueryObservable;
import com.squareup.sqlbrite.SqlBrite;

import java.util.List;

import rx.Observable;
import rx.android.schedulers.AndroidSchedulers;
import rx.schedulers.Schedulers;

public class FavoritesDataSource {
    public static class FavoriteStation {
        final String eva;
        final String name;
        final int count;

        public FavoriteStation(String eva, String name, int count) {
            this.eva = eva;
            this.name = name;
            this.count = count;
        }
    }

    private static final String sqlGetTop = "SELECT * FROM " + FavoritesDbHelper.TABLE +
            " ORDER BY " + FavoritesDbHelper.COL_SELECTED_COUNT + " DESC LIMIT 5";

    private final SqlBrite sqlBrite;
    private final BriteDatabase db;
    private final QueryObservable favoritesObs;

    public FavoritesDataSource(Context ctx) {
        sqlBrite = new SqlBrite.Builder().logger(message ->
                System.out.println("DATABASE message = [" + message + "]")
        ).build();
        db = sqlBrite.wrapDatabaseHelper(new FavoritesDbHelper(ctx), Schedulers.io());
        db.setLoggingEnabled(true);
        favoritesObs = db.createQuery(FavoritesDbHelper.TABLE, sqlGetTop);
    }

    public void addOrIncrement(String eva, String stationName) {
        try (BriteDatabase.Transaction t = db.newTransaction()) {
            db.execute("INSERT OR IGNORE INTO " + FavoritesDbHelper.TABLE +
                    " VALUES (\"" + eva + "\", \"" + stationName + "\", " + "0" + ")");
            db.execute("UPDATE " + FavoritesDbHelper.TABLE +
                    " SET " + FavoritesDbHelper.COL_SELECTED_COUNT + " = " + FavoritesDbHelper.COL_SELECTED_COUNT + " + 1 " +
                    " WHERE " + FavoritesDbHelper.COL_EVA + " = " + eva);

            t.markSuccessful();
        }
    }

    public Observable<List<FavoriteStation>> getFavorites() {
        return favoritesObs.mapToList(c -> {
            String eva = c.getString(c.getColumnIndex(FavoritesDbHelper.COL_EVA));
            String name = c.getString(c.getColumnIndex(FavoritesDbHelper.COL_STATION_NAME));
            int count = c.getInt(c.getColumnIndex(FavoritesDbHelper.COL_SELECTED_COUNT));
            return new FavoriteStation(eva, name, count);
        }).subscribeOn(Schedulers.io()).observeOn(AndroidSchedulers.mainThread());
    }

    public void closeDb() {
        db.close();
    }
}
