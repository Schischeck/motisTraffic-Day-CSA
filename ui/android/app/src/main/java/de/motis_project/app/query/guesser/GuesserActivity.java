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
import java.util.Comparator;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.OnItemClick;
import de.motis_project.app.R;
import de.motis_project.app.io.Status;
import de.motis_project.app.quickselect.QuickSelectDataSource;
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
        GuesserListItem selected = adapter.getItem(pos);

        Intent i = new Intent();
        i.putExtra(RESULT_NAME, selected.location.name);
        i.putExtra(RESULT_ID, selected.location.station);
        setResult(Activity.RESULT_OK, i);

        Status.get().getQuickSelectDb().addOrIncrement(selected.location.name,
                selected.location.station,
                selected.location.lat,
                selected.location.lng,
                selected.location.symbol);

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
        Observable<List<GuesserListItem>> favoriteGuesses =
                RxTextView.textChangeEvents(searchInput)
                        .map(TextViewTextChangeEvent::text)
                        .map(CharSequence::toString)
                        .startWith(init)
                        .map(String::toLowerCase)
                        .flatMap(in -> Status.get().getQuickSelectDb().getFavorites(in))
                        .map(locations -> {
                            List<GuesserListItem> guesses = new ArrayList<>(locations.size());
                            for (QuickSelectDataSource.Location l : locations) {
                                guesses.add(new GuesserListItem(l, GuesserListItem.FAVORITE));
                            }
                            return guesses;
                        });

        Observable<List<GuesserListItem>> stationGuesses =
                RxTextView.textChangeEvents(searchInput)
                        .map(TextViewTextChangeEvent::text)
                        .map(CharSequence::toString)
                        .startWith(init)
                        .map(String::toLowerCase)
                        .filter(in -> in.length() >= 3)
                        .flatMap(in -> Status.get().getServer().guessStation(in))
                        .map(res -> {
                            List<GuesserListItem> guesses = new ArrayList<>(res.guessesLength());
                            for (int i = 0; i < res.guessesLength(); i++) {
                                QuickSelectDataSource.Location l = new QuickSelectDataSource.Location(
                                        res.guesses(i).name(),
                                        null,
                                        res.guesses(i).id(),
                                        res.guesses(i).pos().lat(),
                                        res.guesses(i).pos().lng(),
                                        -i - 1);
                                guesses.add(new GuesserListItem(l, GuesserListItem.STATION));
                            }
                            return guesses;
                        })
                        .startWith(new ArrayList<GuesserListItem>());

        Observable<List<GuesserListItem>> addressGuesses =
                RxTextView.textChangeEvents(searchInput)
                        .map(TextViewTextChangeEvent::text)
                        .map(CharSequence::toString)
                        .startWith(init)
                        .map(String::toLowerCase)
                        .filter(in -> in.length() >= 3)
                        .flatMap(in -> Status.get().getServer().guessAddress(in))
                        .map(res -> {
                            List<GuesserListItem> guesses = new ArrayList<>(res.guessesLength());
                            for (int i = 0; i < res.guessesLength(); i++) {
                                QuickSelectDataSource.Location l = new QuickSelectDataSource.Location(
                                        res.guesses(i).name(),
                                        null,
                                        null,
                                        res.guesses(i).pos().lat(),
                                        res.guesses(i).pos().lng(),
                                        -i - 1);
                                guesses.add(new GuesserListItem(l, GuesserListItem.ADDRESS));
                            }
                            return guesses;
                        })
                        .startWith(new ArrayList<GuesserListItem>());

        return Observable
                .combineLatest(favoriteGuesses, stationGuesses, addressGuesses, (f, s, a) -> {
                    final List<GuesserListItem> guesses = new ArrayList<>();
                    s.removeAll(f);
                    a.removeAll(f);
                    guesses.addAll(s);
                    guesses.addAll(a);
                    guesses.sort((i1, i2) -> {
                        String ref = searchInput.getText().toString();
                        return Integer.compare(i2.cosineSimilarity(ref), i1.cosineSimilarity(ref));
                    });
                    guesses.addAll(0, f);
                    return guesses;
                })
                .subscribe(new Subscriber<List<GuesserListItem>>() {
                    @Override
                    public void onCompleted() {
                    }

                    @Override
                    public void onError(Throwable e) {
                    }

                    @Override
                    public void onNext(List<GuesserListItem> guesses) {
                        runOnUiThread(() -> adapter.setContent(guesses));
                    }
                });
    }
}