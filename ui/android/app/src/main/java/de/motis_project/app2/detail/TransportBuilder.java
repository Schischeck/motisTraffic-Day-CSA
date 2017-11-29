package de.motis_project.app2.detail;

import android.view.LayoutInflater;
import android.view.ViewGroup;

import java.util.HashSet;
import java.util.List;

import de.motis_project.app2.JourneyUtil;
import motis.Connection;

public class TransportBuilder {
    public static void setConnection(
            LayoutInflater inflater,
            ViewGroup journeyDetails,
            Connection con,
            HashSet<JourneyUtil.Section> expanded) {
        journeyDetails.removeAllViews();

        JourneyUtil.printJourney(con);

        List<JourneyUtil.Section> sections = JourneyUtil.getSections(con);
        for (int i = 0; i < sections.size(); i++) {
            boolean isFirst = (i == 0);
            boolean isLast = (i == sections.size() - 1);
            JourneyUtil.Section section = sections.get(i);
            JourneyUtil.Section prevSection = isFirst ? null : sections.get(i - 1);
            boolean expand = expanded.contains(section);
            addTransport(inflater, journeyDetails, con, prevSection, section, isFirst, isLast, expand);
        }
    }

    public static void addTransport(
            LayoutInflater inflater,
            ViewGroup journeyDetails,
            Connection con,
            JourneyUtil.Section prevSection,
            JourneyUtil.Section section,
            boolean isFirst, boolean isLast, boolean expand) {
        if (isFirst) {
            journeyDetails.addView(
                    new FirstTransportHeader(con, section, journeyDetails, inflater).getView(), 0);
        } else {
            journeyDetails.addView(
                    new TransportHeader(con, prevSection, section, journeyDetails, inflater).getView());
        }

        journeyDetails.addView(new TransportDetail(con, section, journeyDetails, inflater).getView());

        journeyDetails.addView(new TransportStops(con, section, journeyDetails, inflater, expand).getView());
        if (expand) {
            for (int i = section.from + 1; i < section.to; i++) {
                journeyDetails.addView(new StopOver(con, section, con.stops(i), journeyDetails, inflater).getView());
            }
        }

        if (isLast) {
            journeyDetails.addView(new FinalArrival(con, section, journeyDetails, inflater).getView());
        } else {
            journeyDetails.addView(new TransportTargetStation(con, section, journeyDetails, inflater).getView());
        }
    }
}
