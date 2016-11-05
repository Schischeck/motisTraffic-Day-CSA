package de.motis_project.app.query;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class FavoritesDbHelper extends SQLiteOpenHelper {
    public static final int DATABASE_VERSION = 1;
    public static final String DATABASE_NAME = "stations.db";

    public static final String TABLE = "favorites";
    public static final String COL_EVA = "_id";
    public static final String COL_STATION_NAME = "name";
    public static final String COL_SELECTED_COUNT = "count";

    private static final String CREATE_LIST = ""
            + "CREATE TABLE " + TABLE + "("
            + COL_EVA + " TEXT NOT NULL PRIMARY KEY," // eva id
            + COL_STATION_NAME + " TEXT NOT NULL,"
            + COL_SELECTED_COUNT + " INTEGER NOT NULL DEFAULT 0"
            + ")";

    public FavoritesDbHelper(Context context) {
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
