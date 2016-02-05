<gb-map>
  <div id="map"></div>

  <style scoped>
  #map {
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    z-index: 0;
  }
  </style>

  <script>
  this.on('mount', () => {
    var map = L.map('map').setView([49.8728, 8.6512], 14);

    L.tileLayer('https://tiles.motis-project.de/osm_light/{z}/{x}/{y}.png?token={accessToken}', {
      attribution: 'Map data &copy; OpenStreetMap contributors, CC-BY-SA',
      maxZoom: 18,
      accessToken: '862bdec137edd4e88029304609458291f0ec760b668c5816ccdd83d0beae76a4'
    }).addTo(map);

    this.trigger('map-mounted', map);
  });
  </script>
</gb-map>
