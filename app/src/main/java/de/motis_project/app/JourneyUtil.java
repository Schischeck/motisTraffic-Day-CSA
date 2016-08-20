package de.motis_project.app;

import android.support.annotation.Nullable;

import java.util.ArrayList;
import java.util.List;

import motis.Connection;
import motis.Move;
import motis.MoveWrapper;
import motis.Transport;

public class JourneyUtil {
    public static class Section {
        public final int from, to;

        public Section(int from, int to) {
            this.from = from;
            this.to = to;
        }
    }

    public static class DisplayTransport {
        public final long clasz;
        public final String longName;
        public final String shortName;

        public DisplayTransport(Transport t) {
            longName = t.name();
            shortName = getShortName(t);
            clasz = t.clasz();
        }

        private static String getShortName(Transport t) {
            if (t.name().length() < 7) {
                return t.name();
            } else if (t.lineId().isEmpty() && t.trainNr() == 0) {
                return t.name();
            } else if (t.lineId().isEmpty()) {
                return Long.toString(t.trainNr());
            } else {
                return t.lineId();
            }
        }
    }

    public static List<Section> getSections(Connection con) {
        int lastSectionEnd = 0;
        List<Section> sections = new ArrayList<>();
        for (int i = 0; i < con.stopsLength(); i++) {
            if (con.stops(i).interchange() || i == con.stopsLength() - 1) {
                sections.add(new Section(lastSectionEnd, i));
                lastSectionEnd = i;
            }
        }
        return sections;
    }

    public static List<DisplayTransport> getTransports(Connection con) {
        List<DisplayTransport> displayTransports = new ArrayList<>();
        for (Section s : getSections(con)) {
            Transport t = getTransport(con, s);
            if (t != null) {
                displayTransports.add(new DisplayTransport(t));
            }
        }
        return displayTransports;
    }

    @Nullable
    public static Transport getTransport(Connection c, Section s) {
        for (int i = 0; i < c.transportsLength(); i++) {
            MoveWrapper m = c.transports(i);
            if (m.moveType() == Move.Transport) {
                Transport t = new Transport();
                m.move(t);

                if (t.range().from() <= s.to && t.range().to() > s.from) {
                    return t;
                }
            }
        }
        return null;
    }
}
