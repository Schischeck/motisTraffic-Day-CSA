package de.motis_project.app.query;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.OnItemClick;
import butterknife.OnTextChanged;
import de.motis_project.app.R;
import de.motis_project.app.io.Status;
import de.motis_project.app.io.db.FavoritesDataSource;
import motis.guesser.StationGuesserResponse;
import rx.Observable;
import rx.Subscriber;
import rx.android.schedulers.AndroidSchedulers;
import rx.schedulers.Schedulers;

public class GuesserActivity extends FragmentActivity {
    public static final String RESULT_NAME = "result_name";
    public static final String RESULT_ID = "result_id";
    public static final String QUERY = "route";

    private List<String> resultIds = new ArrayList<String>();

    Observable observable;

    FavoritesDataSource favorites = new FavoritesDataSource(this);

    @BindView(R.id.suggestionslist)
    ListView suggestions;

    @BindView(R.id.searchInput)
    EditText searchInput;

    @OnClick(R.id.backButton)
    void closeActivity() {
        setResult(Activity.RESULT_CANCELED, null);
        finish();
    }

    @OnClick(R.id.clearButton)
    void clearInput() {
        searchInput.setText("");
        setResults(new ArrayList<String>(), new ArrayList<String>());
    }

    @OnItemClick(R.id.suggestionslist)
    void onSuggestionSelected(int pos) {
        Intent i = new Intent();
        String eva = resultIds.get(pos).toString();
        String stationName = suggestions.getAdapter().getItem(pos).toString();
        i.putExtra(RESULT_NAME,
                stationName);
        i.putExtra(RESULT_ID, eva);
        setResult(Activity.RESULT_OK, i);

        favorites.addOrIncrement(eva, stationName);

        finish();
    }

    @OnTextChanged(R.id.searchInput)
    void getSuggestions(CharSequence inputText) {
        if (inputText.length() < 3) {
            return;
        }

        Observable o = Status.get().getServer()
                .guess(inputText.toString())
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread());
        o.subscribe(new Subscriber<StationGuesserResponse>() {
            @Override
            public void onCompleted() {
            }

            @Override
            public void onError(Throwable e) {
            }

            @Override
            public void onNext(StationGuesserResponse res) {
                final ArrayList<String>
                        names = new ArrayList<>(res.guessesLength()),
                        ids = new ArrayList<>(res.guessesLength());
                for (int i = 0; i < res.guessesLength(); i++) {
                    names.add(res.guesses(i).name());
                    ids.add(res.guesses(i).id());
                }
                setResults(names, ids);
            }
        });

        if (observable != null) {
            observable.unsubscribeOn(AndroidSchedulers.mainThread());
        }
        observable = o;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.query_guesser_activity);
        ButterKnife.bind(this);
        setResults(new ArrayList<String>(), new ArrayList<String>());

        String query = getIntent().getStringExtra(QUERY);
        if (query != null) {
            searchInput.setText(query);
            searchInput.setSelection(query.length());
        }

        favorites.getFavorites().subscribe(new Subscriber<List<FavoritesDataSource.FavoriteStation>>() {
            @Override
            public void onCompleted() {
            }

            @Override
            public void onError(Throwable e) {
            }

            @Override
            public void onNext(List<FavoritesDataSource.FavoriteStation> favs) {

            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        favorites.closeDb();
    }

    public void setResults(ArrayList<String> names, ArrayList<String> ids) {
        ArrayAdapter<String> adapter =
                new ArrayAdapter<String>(GuesserActivity.this,
                        R.layout.query_guesser_list_item,
                        R.id.guess_text, names);
        suggestions.setAdapter(adapter);
        resultIds = ids;
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onStop() {
        super.onStop();
    }
}
