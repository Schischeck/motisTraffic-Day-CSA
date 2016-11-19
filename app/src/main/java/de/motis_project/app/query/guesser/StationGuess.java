package de.motis_project.app.query.guesser;

public class StationGuess {
    final String eva;
    final String name;
    final int count;

    public StationGuess(String eva, String name, int count) {
        this.eva = eva;
        this.name = name;
        this.count = count;
    }

    @Override
    public String toString() {
        return name;
    }
}