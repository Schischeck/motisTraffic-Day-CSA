package de.motis_project.app.saved;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteStatement;

import com.google.flatbuffers.FlatBufferBuilder;
import com.squareup.sqlbrite.BriteDatabase;
import com.squareup.sqlbrite.QueryObservable;
import com.squareup.sqlbrite.SqlBrite;

import java.io.Closeable;
import java.nio.ByteBuffer;
import java.util.List;

import motis.Connection;
import rx.Observable;
import rx.schedulers.Schedulers;

public class SavedConnectionsDataSource implements Closeable {
    private static class SavedConnectionsDbHelper extends SQLiteOpenHelper {
        static final int DATABASE_VERSION = 1;
        static final String DATABASE_NAME = "connections.db";
        static final String TABLE = "connections";

        static final String COL_ID = "_id";
        static final String COL_DATA = "data";
        private static final String CREATE_LIST = ""
                + "CREATE TABLE " + TABLE + "("
                + COL_ID + " INTEGER PRIMARY KEY,"
                + COL_DATA + " TEXT NOT NULL"
                + ")";

        SavedConnectionsDbHelper(Context context) {
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

    private final SavedConnectionsDbHelper dbHelper;
    private final SqlBrite sqlBrite;
    private final BriteDatabase db;

    public SavedConnectionsDataSource(Context ctx) {
        sqlBrite = new SqlBrite.Builder()
                .logger(message -> System.out.println("DATABASE message = [" + message + "]"))
                .build();
        dbHelper = new SavedConnectionsDbHelper(ctx);
        db = sqlBrite.wrapDatabaseHelper(dbHelper, Schedulers.io());
        db.setLoggingEnabled(true);
    }

    public void add(FlatBufferBuilder fbb) {
        SQLiteStatement s = dbHelper.getReadableDatabase().compileStatement(
                "INSERT INTO " + SavedConnectionsDbHelper.TABLE
                        + " (" + SavedConnectionsDbHelper.COL_DATA + ") VALUES (?)");
        s.bindBlob(1, fbb.dataBuffer().compact().array());
        db.executeInsert(SavedConnectionsDbHelper.TABLE, s);
    }

    public Observable<List<Connection>> getFavorites() {
        QueryObservable obs = db.createQuery(SavedConnectionsDbHelper.TABLE,
                "SELECT * FROM " + SavedConnectionsDbHelper.TABLE);
        return obs
                .mapToList(c -> Connection.getRootAsConnection(
                        ByteBuffer.wrap(
                                c.getBlob(c.getColumnIndex(SavedConnectionsDbHelper.COL_DATA)))));
    }

    @Override
    public void close() {
        db.close();
    }
}
