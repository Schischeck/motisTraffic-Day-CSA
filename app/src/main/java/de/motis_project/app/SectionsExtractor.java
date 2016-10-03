package de.motis_project.app;

import java.util.ArrayList;
import java.util.List;

import motis.Connection;
import motis.Move;
import motis.MoveWrapper;
import motis.Walk;

public class SectionsExtractor {
    static private class Range {
        final int from, to;
        Range(int from, int to) {
            this.from = from;
            this.to = to;
        }
    }

    static public List<JourneyUtil.Section> getSections(Connection con) {
        return getSections(con, getWalks(con));
    }

    static private List<JourneyUtil.Section> getSections(Connection con, List<Range> walks) {
        int lastSectionEnd = 0;
        List<JourneyUtil.Section> sections = new ArrayList<>();
        for (int i = 0; i < con.stopsLength(); i++) {
            if (con.stops(i).interchange() || i == con.stopsLength() - 1) {
                sections.add(new JourneyUtil.Section(lastSectionEnd, i));
                lastSectionEnd = getNextSectionBegin(i, walks);
            }
        }
        return sections;
    }

    private static int getNextSectionBegin(int i, List<Range> walks) {
        for (Range walk : walks) {
            if (walk.from == i) {
                return walk.to;
            }
        }
        return i;
    }

    static private List<Range> getWalks(Connection con) {
        final List<Range> walks = new ArrayList<>();
        for (int trIdx = 0; trIdx < con.transportsLength(); ++trIdx) {
            MoveWrapper tr = con.transports(trIdx);
            if (tr.moveType() == Move.Walk) {
                Walk walk = new Walk();
                tr.move(walk);
                walks.add(new Range(walk.range().from(), walk.range().to()));
            }
        }
        return walks;
    }
}
