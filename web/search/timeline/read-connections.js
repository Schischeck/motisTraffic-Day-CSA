const moveColors = {
  1: '#FF0000',  // class IC
  2: '#708D91',  // class NIGHT
  3: '#19DD89',  // class RE
  4: '#FD8F3A',  // class RB
  5: '#94A507',  // class S
  6: '#156ab8',  // class U
  7: '#563AC9',  // class TRAM
  8: '#4E070D',  // class BUS
  9: '#7ED3FD',  // class OTHER
  'walk': '#000000',
  'fallback': '#D31996'
};

function transports(con, from, to) {
  const inRange = t => (t.move.range.from >= from && t.move.range.to <= to);
  return con.transports.filter(inRange);
}

function readConnections(connections, skipWalks) {
  return connections.map(c => {
    const walkSources = c.transports
                         .filter(move => move.move_type === 'Walk')
                         .map(walk => walk.move.range.from);
    const walkTargets = c.transports
                         .filter(move => move.move_type === 'Walk')
                         .map(walk => walk.move.range.to);

    const importantStops = c.stops
                            .map((stop, i) => new Object({ type: 'stop', stop, i }))
                            .filter(el => el.i === 0 ||
                                          el.i === c.stops.length - 1 ||
                                          el.stop.interchange ||
                                          walkTargets.indexOf(el.i) !== -1 ||
                                          walkSources.indexOf(el.i) !== -1)

    const elements = [];
    var depStop, arrStop, transport;
    for (var i = 0; i < importantStops.length - 1; i++) {
      depStop = importantStops[i];
      arrStop = importantStops[i + 1];
      transport = transports(c, depStop.i, arrStop.i)[0];

      if (transport && transport.move.name) {
        elements.push({
          transport,
          from: depStop,
          to: arrStop,
          color: moveColors[transport.move.clasz] || moveColors.fallback,
          begin: new Date(depStop.stop.departure.time * 1000),
          end: new Date(arrStop.stop.arrival.time * 1000)
        });
      }
    }
    return elements;
  });
}

export default readConnections;
