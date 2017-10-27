package de.motis_project.app.query.guesser;


import android.support.annotation.NonNull;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

import de.motis_project.app.quickselect.QuickSelectDataSource;

public class GuesserListItem {
    static final int STATION = 0;
    static final int FAVORITE = 1;
    static final int ADDRESS = 2;

    final QuickSelectDataSource.Location location;
    final int type;

    public GuesserListItem(QuickSelectDataSource.Location location, int type) {
        this.location = location;
        this.type = type;
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof GuesserListItem) {
            GuesserListItem other = (GuesserListItem) obj;
            return location.equals(other);
        }
        return false;
    }

    /*
        From org.apache.commons.text.similarity
     */
    public int cosineSimilarity(String ref) {
        Map<String, Integer> trigrams1 = trigram(this.location.name);
        Map<String, Integer> trigrams2 = trigram(ref);

        HashSet<String> intersection = new HashSet<>(trigrams1.keySet());
        intersection.retainAll(trigrams2.keySet());

        long dotProduct = 0;
        for (final CharSequence key : intersection) {
            dotProduct += trigrams1.get(key) * trigrams2.get(key);
        }

        float d1 = 0.0f;
        float d2 = 0.0f;
        for (final Integer value : trigrams1.values()) {
            d1 += Math.pow(value, 2);
        }
        for (final Integer value : trigrams2.values()) {
            d2 += Math.pow(value, 2);
        }

        float cosineSimilarity;
        if (d1 <= 0.0f || d2 <= 0.0f) {
            cosineSimilarity = 0.0f;
        } else {
            cosineSimilarity = (float) (dotProduct / (float) (Math.sqrt(d1) * Math.sqrt(d2)));
        }

        return (int) (cosineSimilarity * 10000);
    }

    @Override
    public int hashCode() {
        return location.hashCode() * 31 + Integer.valueOf(type).hashCode();
    }

    @NonNull
    public Map<String, Integer> trigram(String s) {
        Map<String, Integer> trigrams = new HashMap<>();
        for (int i = 2; i < s.length(); i++) {
            String trigram = s.substring(i - 2, i);
            if (!trigrams.containsKey(trigram)) {
                trigrams.put(trigram, 0);
            }
            trigrams.put(trigram, trigrams.get(trigram) + 1);
        }
        return trigrams;
    }
}