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
import de.motis_project.app.io.Server;
import de.motis_project.app.io.State;
import motis.Message;
import motis.MsgContent;
import motis.guesser.StationGuesserResponse;

public class GuesserActivity extends FragmentActivity
        implements Server.Listener {
    public static final String RESULT_NAME = "result_name";
    public static final String RESULT_ID = "result_id";
    public static final String QUERY = "route";

    private List<String> resultIds = new ArrayList<String>();

    private int nextResponseId;

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
        setResults(new ArrayList<String>(), new ArrayList<String>());
    }

    @OnItemClick(R.id.suggestionslist)
    void onSuggestionSelected(int pos) {
        Intent i = new Intent();
        i.putExtra(RESULT_NAME,
                   suggestionList.getAdapter().getItem(pos).toString());
        i.putExtra(RESULT_ID, resultIds.get(pos).toString());
        setResult(Activity.RESULT_OK, i);
        finish();
    }

    @OnTextChanged(R.id.searchInput)
    void getSuggestions(CharSequence inputText) {
        if (inputText.length() < 3) {
            return;
        }
        nextResponseId = State.get().getServer().guess(inputText.toString());
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        State.get().getServer().addListener(this);

        setContentView(R.layout.query_guesser_activity);
        ButterKnife.bind(this);
        setResults(new ArrayList<String>(), new ArrayList<String>());

        String query = getIntent().getStringExtra(QUERY);
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

    public void setResults(ArrayList<String> names, ArrayList<String> ids) {
        ArrayAdapter<String> adapter =
                new ArrayAdapter<String>(GuesserActivity.this,
                                         R.layout.query_guesser_list_item,
                                         R.id.guess_text, names);
        suggestionList.setAdapter(adapter);
        resultIds = ids;
    }

    @Override
    public void onMessage(Message m) {
        if (m.id() != nextResponseId ||
                m.contentType() != MsgContent.StationGuesserResponse) {
            return;
        }

        StationGuesserResponse res = new StationGuesserResponse();
        m.content(res);

        final ArrayList<String>
                names = new ArrayList<>(res.guessesLength()),
                ids = new ArrayList<>(res.guessesLength());
        for (int i = 0; i < res.guessesLength(); i++) {
            names.add(res.guesses(i).name());
            ids.add(res.guesses(i).id());
        }

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                setResults(names, ids);
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
