package de.motis_project.app;

import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

import com.google.flatbuffers.FlatBufferBuilder;

import java.util.ArrayList;
import java.util.Arrays;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.OnTextChanged;
import motis.Message;
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
        finish();
    }

    @OnClick(R.id.clearButton)
    void clearInput() {
        searchInput.setText("");
    }

    @OnTextChanged(R.id.searchInput)
    void getSuggestions(CharSequence inputText) {
        String input = inputText.toString();
        System.err.println(input);
        FlatBufferBuilder b = new FlatBufferBuilder();
        int guesserRequestOffset = StationGuesserRequest.createStationGuesserRequest(b, 10, b.createString(input));
        b.finish(Message.createMessage(b, MsgContent.StationGuesserRequest, guesserRequestOffset, 1));

        new FetchDataTask() {
            protected void onPostExecute(Message result) {
                ArrayList<String> guessList = new ArrayList<>();

                if (result == null || result.contentType() != MsgContent.StationGuesserResponse) {
                    System.err.println("result == null: " + (result == null));
                    if (result != null) {
                        System.err.println("content type match: " + (result.contentType() != MsgContent.StationGuesserResponse));
                    }
                    guessList.add("ERROR!");
                } else {
                    StationGuesserResponse guesses = new StationGuesserResponse();
                    guesses = (StationGuesserResponse) result.content(guesses);

                    for (int i = 0; i < guesses.guessesLength(); i++) {
                        guessList.add(guesses.guesses(i).name());
                    }
                }

                ArrayAdapter<String> adapter = new ArrayAdapter<String>(SearchActivity.this,
                        android.R.layout.simple_list_item_1, android.R.id.text1, guessList);


                // Assign adapter to ListView
                suggestionList.setAdapter(adapter);
            }
        }.execute(b);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_search);
        ButterKnife.bind(this);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(SearchActivity.this,
                android.R.layout.simple_list_item_1, android.R.id.text1, new String[] {"A","B","C"});
        suggestionList.setAdapter(adapter);
    }
}
