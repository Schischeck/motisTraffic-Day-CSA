package de.motis_project.app.query.guesser;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.support.v4.util.Pair;
import android.widget.EditText;
import android.widget.ListView;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.OnItemClick;
import butterknife.OnTextChanged;
import de.motis_project.app.R;
import de.motis_project.app.io.Status;
import rx.Observable;
import rx.Subscriber;
import rx.android.schedulers.AndroidSchedulers;
import rx.schedulers.Schedulers;

public class GuesserActivity extends FragmentActivity {
    public static final String RESULT_NAME = "result_name";
    public static final String RESULT_ID = "result_id";
    public static final String QUERY = "route";

    private GuesserListAdapter adapter;

    Observable observable;

    FavoritesDataSource favDataSource;

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
        adapter.clear();
    }

    @OnItemClick(R.id.suggestionslist)
    void onSuggestionSelected(int pos) {
        Intent i = new Intent();
        StationGuess selected = adapter.getItem(pos);
        String eva = selected.eva;
        String stationName = selected.name;
        i.putExtra(RESULT_NAME,
                stationName);
        i.putExtra(RESULT_ID, eva);
        setResult(Activity.RESULT_OK, i);

        favDataSource.addOrIncrement(eva, stationName);

        finish();
    }

    @OnTextChanged(R.id.searchInput)
    void getSuggestions(CharSequence inputText) {
        Observable<List<StationGuess>> guesses;
        if (inputText.length() < 3) {
            guesses = Observable.empty();
        } else {
            guesses = Status.get().getServer()
                    .guess(inputText.toString())
                    .subscribeOn(Schedulers.io())
                    .observeOn(AndroidSchedulers.mainThread())
                    .map(res -> {
                        final int totalGuesses = res.guessesLength();
                        List<StationGuess> guessList = new ArrayList<>(totalGuesses);
                        for (int i = 0; i < totalGuesses; i++) {
                            final String name = res.guesses(i).name();
                            final String id = res.guesses(i).id();
                            guessList.add(new StationGuess(id, name, i));
                        }
                        return guessList;
                    });
        }

        guesses = guesses.startWith(new ArrayList<StationGuess>());
        Observable<List<StationGuess>> favorites = favDataSource.getFavorites(inputText)
                .startWith(new ArrayList<StationGuess>());

        Observable<Pair<List<StationGuess>, List<StationGuess>>> o =
                Observable.combineLatest(guesses, favorites, (g, f) -> {
                    List<StationGuess> guessList = new ArrayList<>(g.size() + f.size());
                    g.removeAll(f);
                    Collections.sort(f, (lhs, rhs) -> Integer.compare(rhs.count, lhs.count));
                    Collections.sort(g, (lhs, rhs) -> Integer.compare(lhs.count, rhs.count));
                    guessList.addAll(f);
                    guessList.addAll(g);
                    return new Pair<>(f, g);
                });

        o.subscribe(new Subscriber<Pair<List<StationGuess>, List<StationGuess>>>() {
            @Override
            public void onCompleted() {
            }

            @Override
            public void onError(Throwable e) {
            }

            @Override
            public void onNext(Pair<List<StationGuess>, List<StationGuess>> guesses) {
                adapter.setContent(guesses.first, guesses.second);
            }
        });

        if (this.observable != null) {
            this.observable.unsubscribeOn(AndroidSchedulers.mainThread());
        }
        this.observable = o;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.query_guesser_activity);
        ButterKnife.bind(this);

        adapter = new GuesserListAdapter(this);
        suggestions.setAdapter(adapter);

        favDataSource = new FavoritesDataSource(this);

        String query = getIntent().getStringExtra(QUERY);
        if (query != null) {
            searchInput.setText(query);
            searchInput.setSelection(query.length());
            getSuggestions(query);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        favDataSource.closeDb();
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