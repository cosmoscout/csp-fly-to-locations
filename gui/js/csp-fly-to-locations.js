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
  bookmarker = L.icon({
    iconUrl: 'third-party/leaflet/images/marker-icon.png',
    iconSize:     [20, 35], // size of the icon
    iconAnchor:   [10, 30], // point of the icon which will correspond to marker's location
    popupAnchor:  [5, 5] // point from which the popup should open relative to the iconAnchor
  });
  bookmarks = [];
  usericon = L.icon({
    iconUrl: 'third-party/leaflet/images/Untitled.png',
    iconSize:     [10, 10], // size of the icon
    iconAnchor:   [5, 5], // point of the icon which will correspond to marker's location
    popupAnchor:  [0, 0] // point from which the popup should open relative to the iconAnchor
  });
  
// The active planet.
  activePlanet = null;
 // The circumfence of the planet.
  circumferencevar = 0;

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
        center: [0, 0],
        zoom: 3,
        worldCopyJump: true,
        maxBoundsViscosity: 0.5,
        maxBounds:[
          [-90, -180],
          [90, 180]],
          
      });
      
      this.marker   =L.marker([50.5 ,30.5 ],{icon: this.usericon}).addTo(this.minimap);

      
    // Moving the planet with the minimap.
    this.minimap.on('click', (e) => {
      var location = { 
        longitude: parseFloat(e.latlng.lng),
        latitude: parseFloat(e.latlng.lat),
        height: parseFloat(this.circumferencevar/ Math.pow(2, this.minimap.getZoom() 
      ))};
      this.flyTo(this.activePlanet, location, 5);
    });   
  }

  update() {
    this.setObserverPosition(...CosmoScout.statusbar.getObserverPosition())
  }


  
  configureMinimap(mapserver, layer, attribution, circumference) {
    if (this.wmslayer != null) {
        this.wmslayer.removeFrom(this.minimap);
    } 
    
    if (layer == ""){
      this.wmslayer =L.tileLayer(mapserver,{ 
        attribution: '&copy; ' + attribution
      }).addTo(this.minimap);
    } else {
    this.wmslayer = L.tileLayer.wms(mapserver, { attribution: '&copy; ' + attribution, layers: layer })
                    .addTo(this.minimap);
  }
    // Change the circumfence.
    this.circumferencevar = circumference;
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
  setObserverPosition(long, lat, height) {
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
      document.getElementById('navi').classList.add('hidden')
      document.getElementById('nav-minimap').classList.remove('active')
      document.getElementById('tab-minimap').classList.remove('show')
      document.getElementById('tab-minimap').classList.remove('active')
      document.getElementById('nav-celestial-bodies').classList.add('active')
      document.getElementById('tab-celestial-bodies').classList.add('active')
      document.getElementById('tab-celestial-bodies').classList.add('show')
      document.getElementById('bookmarks').classList.add('hidden')
    }
  }
  
  clearBookmarks() {
    for (var i of this.bookmarks) {
      i.removeFrom(this.minimap);
    }
    
    this.bookmarks = [];

    const bookmarkArea = document.getElementById('location-tabs-area');
    document.getElementById('bookmarks').classList.add('hidden')
  }

  /**
   * csp-fly-to-locations
   *
   * @param text {string}
   */
  addBookmark(text, lat, lng) {

    var bookmark = L.marker([lat,lng],{icon: this.bookmarker}).addTo(this.minimap);
    bookmark.bindPopup(text);
    this.bookmarks.push(bookmark)

    
    let first = false;
    const bookmarkArea = document.getElementById('location-tabs-area');
    if (bookmarkArea.childNodes.length >= 0){
      document.getElementById('bookmarks').classList.remove('hidden')
    }

    // Loads a template for a bookmark row containing a name and a button.
    const bookmarkRow = CosmoScout.loadTemplateContent('location-group');

    // Sets the name of the bookmark row
    bookmarkRow.innerHTML = bookmarkRow.innerHTML
      .replace(this.regex('TEXT'), text)
      .trim();

    bookmarkArea.appendChild(bookmarkRow);

    CosmoScout.initTooltips();
    CosmoScout.initDataCalls();
  }
}

(() => {
  CosmoScout.init(FlyToApi);
})();
