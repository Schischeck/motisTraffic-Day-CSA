var RailViz = RailViz || {};

RailViz.Preprocessing = (function() {

    function preprocess(data) {
        data.routes.forEach(route => route.segments.forEach(preprocessSegment));
    }

    function preprocessSegment(segment) {
        const coords = segment.coordinates.coordinates;
        const n_subsegments = (coords.length / 2) - 1;
        segment.subSegmentLengths = new Array(n_subsegments);
        segment.subSegmentOffsets = new Array(n_subsegments);
        let offset = 0;
        for (let i = 0; i < n_subsegments; i++) {
            const from_base = i * 2;
            const to_base = (i + 1) * 2;
            const x_dist = coords[to_base] - coords[from_base];
            const y_dist = coords[to_base + 1] - coords[from_base + 1];
            const length = Math.sqrt(x_dist * x_dist + y_dist * y_dist);
            segment.subSegmentLengths[i] = length;
            segment.subSegmentOffsets[i] = offset;
            offset += length;
        }
        segment.totalLength = offset;
    }

    const initialResolution = 2 * Math.PI * 6378137 / 256;
    const originShift = Math.PI * 6378137;

    function geoToWorldCoords(lat, lng) {
        const mx = lng * originShift / 180;
        const my1 = Math.log(Math.tan((90 + lat) * Math.PI / 360)) / (pi / 180);
        const my = my1 * originShift / 180;
        const x = (mx + originShift) / initialResolution;
        const y = 256 - ((my + originShift) / initialResolution);

        return {x: x, y: y};
    }

    return {
        preprocess: preprocess
    };

})();
