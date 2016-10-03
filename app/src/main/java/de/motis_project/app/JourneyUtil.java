package de.motis_project.app;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.support.annotation.Nullable;
import android.support.v4.content.ContextCompat;
import android.support.v4.graphics.drawable.DrawableCompat;
import android.support.v4.util.LongSparseArray;
import android.view.View;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

import motis.Connection;
import motis.Move;
import motis.MoveWrapper;
import motis.Range;
import motis.Transport;
import motis.Trip;
import motis.Walk;

public class JourneyUtil {
    public static final LongSparseArray<Integer> colors = new LongSparseArray<>();

    static {
        colors.put(0, R.color.colorPrimary);
        colors.put(1, R.color.colorPrimaryDark);
        colors.put(2, R.color.colorPrimaryLight);
        colors.put(3, R.color.teal_500);
    }

    public static final LongSparseArray<Integer> icons = new LongSparseArray<>();

    static {
        icons.put(0, R.drawable.ic_directions_railway_black_24dp);
        icons.put(1, R.drawable.ic_directions_railway_black_24dp);
        icons.put(2, R.drawable.ic_directions_railway_black_24dp);
        icons.put(3, R.drawable.ic_directions_railway_black_24dp);
    }

    public static int getColor(Context c, long clasz) {
        int id = colors.get(clasz, R.color.colorAccent);
        return ContextCompat.getColor(c, id);
    }

    public static int getIcon(Context c, long clasz) {
        return icons.get(clasz, R.drawable.ic_directions_bus_black_24dp);
    }

    public static void tintBackground(Context context, View view, long clasz) {
        Drawable bg = DrawableCompat.wrap(view.getBackground());
        DrawableCompat.setTint(bg.mutate(), JourneyUtil.getColor(context, clasz));
        view.setBackground(bg);
    }

    public static void setBackgroundColor(Context context, View view, long clasz) {
        view.setBackgroundColor(JourneyUtil.getColor(context, clasz));
    }

    public static void setIcon(Context context, TextView view, long clasz) {
        view.setCompoundDrawablesWithIntrinsicBounds(JourneyUtil.getIcon(context, clasz), 0, 0, 0);
    }

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
        return SectionsExtractor.getSections(con);
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

    public static String getTransportName(Connection c, Section s) {
        Transport transport = JourneyUtil.getTransport(c, s);
        if (transport == null) {
            return "";
        }
        return Str.san(transport.name());
    }

    public static void printJourney(Connection con) {
        System.out.println("Stops:");
        for (int stopIdx = 0; stopIdx  < con.stopsLength(); ++stopIdx) {
            String sectionEnd = con.stops(stopIdx).interchange() || stopIdx == con.stopsLength() - 1 ? " [section end]" : "";
            System.out.println("  " + stopIdx + ": " + con.stops(stopIdx).station().name() + sectionEnd);
        }

        System.out.println("Transports:");
        for (int trIdx = 0; trIdx  < con.transportsLength(); ++trIdx) {
            System.out.print("  " + trIdx + ": ");
            switch (con.transports(trIdx).moveType()) {
                case Move.Transport: {
                    Transport tr = new Transport();
                    con.transports(trIdx).move(tr);
                    System.out.print(" from=" + tr.range().from() + ", to=" + tr.range().to());
                    System.out.println(" | transport " + tr.name());
                    break;
                }

                case Move.Walk: {
                    Walk tr = new Walk();
                    con.transports(trIdx).move(tr);
                    System.out.print(" from=" + tr.range().from() + ", to=" + tr.range().to());
                    System.out.println(" | walk");
                    break;
                }
            }
        }

        System.out.println("Trips:");
        for (int trpIdx = 0; trpIdx < con.tripsLength(); ++trpIdx) {
            Trip trp = con.trips(trpIdx);
            Range range = trp.range();
            System.out.println("  trip [" + range.from() + ", " + range.to() + "]: " + trp.id().trainNr());
        }
    }
}
