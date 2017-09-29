package de.motis_project.app.query.guesser;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.widget.EditText;
import android.widget.ListView;

import com.jakewharton.rxbinding.widget.RxTextView;
import com.jakewharton.rxbinding.widget.TextViewTextChangeEvent;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.OnItemClick;
import de.motis_project.app.R;
import de.motis_project.app.io.Status;
import rx.Observable;
import rx.Subscriber;
import rx.Subscription;

public class GuesserActivity extends FragmentActivity {
    public static final String RESULT_NAME = "result_name";
    public static final String RESULT_ID = "result_id";
    public static final String QUERY = "route";

    private GuesserListAdapter adapter;
    private Subscription subscription;

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
        LocationGuess selected = adapter.getItem(pos);

        Intent i = new Intent();
        i.putExtra(RESULT_NAME, selected.name);
        i.putExtra(RESULT_ID, selected.stationId);
        setResult(Activity.RESULT_OK, i);

        if (selected.type == LocationGuess.STATION) {
            Status.get().getFavoritesDb().addOrIncrement(selected.stationId, selected.name);
        }

        finish();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.query_guesser_activity);
        ButterKnife.bind(this);

        adapter = new GuesserListAdapter(this);
        suggestions.setAdapter(adapter);

        String query = getIntent().getStringExtra(QUERY);
        if (query != null) {
            subscription = setupSubscription(query);
            searchInput.setText(query);
            searchInput.setSelection(query.length());
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        subscription.unsubscribe();
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onStop() {
        super.onStop();
    }

    private Subscription setupSubscription(String init) {
        Observable<List<LocationGuess>> favoriteGuesses =
                RxTextView.textChangeEvents(searchInput)
                        .map(TextViewTextChangeEvent::text)
                        .map(CharSequence::toString)
                        .startWith(init)
                        .map(String::toLowerCase)
                        .flatMap(in -> Status.get().getFavoritesDb().getFavorites(in));

        Observable<List<LocationGuess>> stationGuesses =
                RxTextView.textChangeEvents(searchInput)
                        .map(TextViewTextChangeEvent::text)
                        .map(CharSequence::toString)
                        .startWith(init)
                        .map(String::toLowerCase)
                        .filter(in -> in.length() >= 3)
                        .flatMap(in -> Status.get().getServer().guessStation(in))
                        .map(res -> {
                            List<LocationGuess> guesses = new ArrayList<>(res.guessesLength());
                            for (int i = 0; i < res.guessesLength(); i++) {
                                guesses.add(new LocationGuess(
                                        res.guesses(i).name(),
                                        res.guesses(i).pos().lat(),
                                        res.guesses(i).pos().lng(),
                                        -i - 1,
                                        LocationGuess.STATION,
                                        res.guesses(i).id()));
                            }
                            return guesses;
                        })
                        .startWith(new ArrayList<LocationGuess>());

        Observable<List<LocationGuess>> addressGuesses =
                RxTextView.textChangeEvents(searchInput)
                        .map(TextViewTextChangeEvent::text)
                        .map(CharSequence::toString)
                        .startWith(init)
                        .map(String::toLowerCase)
                        .filter(in -> in.length() >= 3)
                        .flatMap(in -> Status.get().getServer().guessAddress(in))
                        .map(res -> {
                            List<LocationGuess> guesses = new ArrayList<>(res.guessesLength());
                            for (int i = 0; i < res.guessesLength(); i++) {
                                guesses.add(new LocationGuess(
                                        res.guesses(i).name(),
                                        res.guesses(i).pos().lat(),
                                        res.guesses(i).pos().lng(),
                                        -i - 1,
                                        LocationGuess.ADDRESS));
                            }
                            return guesses;
                        })
                        .startWith(new ArrayList<LocationGuess>());

        return Observable
                .combineLatest(favoriteGuesses, stationGuesses, addressGuesses, (f, s, a) -> {
                    final List<LocationGuess> guesses = new ArrayList<>();
                    s.removeAll(f);
                    guesses.addAll(f);
                    guesses.addAll(a);
                    guesses.addAll(s);
                    Collections.sort(guesses);
                    return guesses;
                })
                .subscribe(new Subscriber<List<LocationGuess>>() {
                    @Override
                    public void onCompleted() {
                    }

                    @Override
                    public void onError(Throwable e) {
                    }

                    @Override
                    public void onNext(List<LocationGuess> guesses) {
                        runOnUiThread(() -> adapter.setContent(guesses));
                    }
                });
    }
}