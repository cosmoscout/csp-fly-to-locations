/* global IApi, CosmoScout */

/**
 * FlyTo Api
 */
class FlyToApi extends IApi {
  /**
   * @inheritDoc
   * @type {string}
   */
  name = 'flyto';
 
 // Variables for the minimap.
  wmslayer = null;
  minimap = null;
  marker = null;

 // The active planet.
  activePlanet = null;

 // The circumfence of the planet.
  circumfencevar = 0;

  // Store last frame's observer position.
  lastLong = 0.0;
  lastLat = 0.0;
  lastHeight = 0.0;

  /**
   * @inheritDoc
   */
  init() {
    // Show the minimap and make an invisible border.
    this.minimap = L.map(document.getElementById('minimap'), {
        center: [52.315625, 10.562169],
        zoom: 3,
        worldCopyJump: true,
        maxBoundsViscosity: 0.5,
        maxBounds:[
          [-90, -180],
          [90, 180]
      ]});
    this.wmslayer= L.tileLayer.wms('http://maps.sc.bs.dlr.de/mapserv?', {
      layers: 'earth.bluemarble.rgb'
    }).addTo(this.minimap);
    this.marker = L.marker([50.5, 30.5]).addTo(this.minimap);

    // Moving the planet with the minimap.
    this.minimap.on('click', (e) => {
      var location = { 
        longitude: parseFloat(e.latlng.lng),
        latitude: parseFloat(e.latlng.lat),
        height: parseFloat(this.circumfencevar/ Math.pow(2, this.minimap.getZoom() 
      ))};
      this.flyTo(this.activePlanet, location, 5);
    });   
  } 
   // Reset the minimap to the center.
  reset() {
    this.minimap.setView([0,0], 1)
  }

  flyTo(planet, location, time) {
    if (typeof location === 'undefined') {
      CosmoScout.callNative('fly_to_location', planet);
    } else {
      CosmoScout.callNative('fly_to_location', planet, location.longitude, location.latitude, location.height, time);
    }

    CosmoScout.notifications.printNotification('Traveling', `to ${planet}`, 'send');
  }
  
  setActivePlanet(name) {
    this.activePlanet = name;
  }
// The marker move to the position on the planet.
  setUserPosition(long, lat, height) {
    this.marker.setLatLng([parseFloat(lat),parseFloat(long)])
// The lockmap option and go to old position.
    if ((this.lastLong != long || this.lastLat != lat || this.lastHeight != height) 
        && document.getElementById("set_enable_lock_minimap").checked) {
      this.minimap.setView([parseFloat(lat),parseFloat(long)],[this.minimap.getZoom()]);
    }

    this.lastLong = long;
    this.lastLat = lat;
    this.lastHeight = height;
  }

  /**
   * Adds a celestial body button to #celestial-bodies
   *
   * @param name {string}
   * @param icon {string}
   */
  addCelestialBody(name, icon) {
    const button = CosmoScout.loadTemplateContent('celestial-body');
    if (button === false) {
      return;
    }

    button.innerHTML = button.innerHTML
      .replace(this.regex('NAME'), name)
      .replace(this.regex('ICON'), icon)
      .trim();

    button.addEventListener('click', () => {
      CosmoScout.callNative('set_celestial_body', name);
    });

    const area = document.getElementById('celestial-bodies');

    if (area === null) {
      console.error("'#celestial-bodies' not found.");
      return;
    }

    area.appendChild(button);
  }

 // Hide and show the minimap.
  enableMinimap(enable) {
    if (enable) {
      document.getElementById ('navi').classList.remove('hidden')
    } else {
      document.getElementById ('navi').classList.add('hidden')
      document.getElementById ('nav-minimap').classList.remove('active')
      document.getElementById ('tab-minimap').classList.remove('show')
      document.getElementById ('tab-minimap').classList.remove('active')
      document.getElementById ('nav-celestial-bodies').classList.add('active')
      document.getElementById ('tab-celestial-bodies').classList.add('active')
      document.getElementById ('tab-celestial-bodies').classList.add('show')
    }
  }

  // Update the map server and layer for the minimap.
  configureMinimap(mapserver, layer, circumfence) {
    this.wmslayer.setParams({
      "baseUrl": mapserver,
      "layers": layer
    });
    // Change the circumfence.
    this.circumfencevar = circumfence;
  }

  /**
   * csp-fly-to-locations
   *
   * @param group {string}
   * @param text {string}
   */
  addLocation(group, text) {
    let first = false;
    const tabArea = document.getElementById('location-tabs-area');

    if (tabArea.childNodes.length === 0) {
      first = true;
      tabArea.appendChild(CosmoScout.loadTemplateContent('location-tab'));
    }

    const locationsTab = document.getElementById('location-tabs');
    const tabContents = document.getElementById('nav-tabContents');

    let groupTab = document.getElementById(`nav-${group}`);

    if (groupTab === null) {
      const active = first ? 'active' : '';

      const locationTabContent = CosmoScout.loadTemplateContent('location-tab-link');
      const element = document.createElement('template');

      element.innerHTML = locationTabContent.outerHTML
        .replace(this.regex('ACTIVE'), active)
        .replace(this.regex('GROUP'), group)
        .replace(this.regex('FIRST'), first.toString())
        .trim();

      locationsTab.appendChild(element.content);

      const show = first ? 'show' : '';

      const tabContent = CosmoScout.loadTemplateContent('location-tab-pane');

      element.innerHTML = tabContent.outerHTML
        .replace(this.regex('SHOW'), show)
        .replace(this.regex('ACTIVE'), active)
        .replace(this.regex('GROUP'), group)
        .trim();

      tabContents.appendChild(element.content);

      groupTab = document.getElementById(`nav-${group}`);
    }

    const groupTabContent = CosmoScout.loadTemplateContent('location-group');

    groupTabContent.innerHTML = groupTabContent.innerHTML
      .replace(this.regex('TEXT'), text)
      .trim();

    groupTab.appendChild(groupTabContent);

    CosmoScout.initTooltips();
    CosmoScout.initDataCalls();
  }
}

(() => {
  CosmoScout.init(FlyToApi);
})();
