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

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof StationGuess) {
            StationGuess other = (StationGuess) obj;
            return eva.equals(other.eva);
        }
        return false;
    }

    @Override
    public int hashCode() {
        return eva.hashCode();
    }
}