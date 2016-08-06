package de.motis_project.app.query;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;

import java.util.ArrayList;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.OnItemClick;
import butterknife.OnTextChanged;
import de.motis_project.app.R;
import de.motis_project.app.io.MessageBuilder;
import de.motis_project.app.io.Server;
import de.motis_project.app.io.State;
import motis.Message;
import motis.MsgContent;
import motis.guesser.StationGuesserResponse;

public class GuesserActivity extends FragmentActivity implements Server.Listener {
    @BindView(R.id.suggestionslist)
    ListView suggestionList;

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
        setResults(new ArrayList<String>());
    }

    @OnItemClick(R.id.suggestionslist)
    void onSuggestionSelected(int pos) {
        Intent i = new Intent();
        i.putExtra("result",
                   suggestionList.getAdapter().getItem(pos).toString());
        setResult(Activity.RESULT_OK, i);
        finish();
    }

    @OnTextChanged(R.id.searchInput)
    void getSuggestions(CharSequence inputText) {
        if (inputText.length() < 3) {
            return;
        }

        byte[] msg = MessageBuilder.guess(1, inputText.toString());
        State.get().getServer().send(msg);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        State.get().getServer().addListener(this);

        setContentView(R.layout.activity_guesser);
        ButterKnife.bind(this);
        setResults(new ArrayList<String>());

        String query = getIntent().getStringExtra("query");
        if (query != null) {
            searchInput.setText(query);
            searchInput.setSelection(query.length());
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        State.get().getServer().removeListener(this);
    }

    public void setResults(ArrayList<String> r) {
        ArrayAdapter<String> adapter =
                new ArrayAdapter<String>(GuesserActivity.this,
                                         R.layout.search_list_item,
                                         R.id.guess_text, r);
        suggestionList.setAdapter(adapter);
    }

    @Override
    public void onMessage(Message m) {
        if (m.contentType() != MsgContent.StationGuesserResponse) {
            return;
        }

        StationGuesserResponse guessesResponse = new StationGuesserResponse();
        guessesResponse = (StationGuesserResponse) m.content(guessesResponse);

        final ArrayList<String> guesses = new ArrayList<String>();
        for (int i = 0; i < guessesResponse.guessesLength(); i++) {
            guesses.add(guessesResponse.guesses(i).name());
        }

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                setResults(guesses);
            }
        });
    }

    @Override
    public void onConnect() {
    }

    @Override
    public void onDisconnect() {
    }
}
