package de.motis_project.app;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;

import com.google.flatbuffers.FlatBufferBuilder;

import java.util.ArrayList;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.OnItemClick;
import butterknife.OnTextChanged;
import motis.Message;
import motis.MotisError;
import motis.MsgContent;
import motis.guesser.StationGuesserRequest;
import motis.guesser.StationGuesserResponse;

public class SearchActivity extends FragmentActivity {
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
        i.putExtra("result", suggestionList.getAdapter().getItem(pos).toString());
        setResult(Activity.RESULT_OK, i);
        finish();
    }

    @OnTextChanged(R.id.searchInput)
    void getSuggestions(CharSequence inputText) {
        if(inputText.length() < 3) {
            return;
        }

        String input = inputText.toString();
        FlatBufferBuilder b = new FlatBufferBuilder();
        int guesserRequestOffset = StationGuesserRequest.createStationGuesserRequest(b, 10, b.createString(input));
        b.finish(Message.createMessage(b, MsgContent.StationGuesserRequest, guesserRequestOffset, 1));

        new FetchDataTask() {
            protected void onPostExecute(final Message result) {
                ArrayList<String> guessList = new ArrayList<>();


                if (result != null && result.contentType() == MsgContent.StationGuesserResponse) {
                    StationGuesserResponse guessesResponse = new StationGuesserResponse();
                    guessesResponse = (StationGuesserResponse) result.content(guessesResponse);
                    for (int i = 0; i < guessesResponse.guessesLength(); i++) {
                        guessList.add(guessesResponse.guesses(i).name());
                    }
                } else {
                    guessList.add("ERROR!");
                    if (result != null) {
                        System.err.println("content type match: " + (result.contentType() != MsgContent.StationGuesserResponse));
                        System.err.println("content type is error: " + (result.contentType() == MsgContent.MotisError));
                        MotisError error = new MotisError();
                        error = (MotisError) result.content(error);
                        System.err.println("ERROR: " + error.category() + ", " + error.reason());
                    }
                }
                setResults(guessList);
            }
        }.execute(b);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_search);
        ButterKnife.bind(this);
        setResults(new ArrayList<String>());

        String query = getIntent().getStringExtra("query");
        if (query != null) {
            searchInput.setText(query);
            searchInput.setSelection(query.length());
        }
    }

    public void setResults(ArrayList<String> r) {
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(SearchActivity.this,
                R.layout.search_list_item, R.id.guess_text, r);
        suggestionList.setAdapter(adapter);
    }
}
