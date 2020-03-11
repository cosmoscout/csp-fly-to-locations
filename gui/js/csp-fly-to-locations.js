/* global IApi, CosmoScout */

/**
 * FlyTo Api
 */
class FlyToLocationsApi extends IApi {
  /**
   * @inheritDoc
   * @type {string}
   */
  name = 'flyToLocations';

  flyTo(planet, location, time) {
    if (typeof location === 'undefined') {
      CosmoScout.callbacks.flyToLocations.flyTo(planet);
    } else {
      CosmoScout.callbacks.flyToLocations.flyTo(
          planet, location.longitude, location.latitude, location.height, time);
    }

    CosmoScout.notifications.print('Traveling', `to ${planet}`, 'send');
  }

  /**
   * Adds a celestial body button to #celestial-bodies
   *
   * @param name {string}
   * @param icon {string}
   */
  addCelestialBody(name, icon) {
    const button = CosmoScout.gui.loadTemplateContent('celestial-body');
    if (button === false) {
      return;
    }

    button.innerHTML = button.innerHTML.replace(/%NAME%/g, name).replace(/%ICON%/g, icon).trim();

    button.addEventListener('click', () => {
      CosmoScout.callbacks.navigation.setBody(name);
    });

    const area = document.getElementById('celestial-bodies');

    if (area === null) {
      console.error("'#celestial-bodies' not found.");
      return;
    }

    area.appendChild(button);
  }

  /**
   * csp-fly-to-locations
   *
   * @param group {string}
   * @param text {string}
   */
  addLocation(group, text) {
    let first     = false;
    const tabArea = document.getElementById('location-tabs-area');

    if (tabArea.childNodes.length === 0) {
      first = true;
      tabArea.appendChild(CosmoScout.gui.loadTemplateContent('location-tab'));
    }

    const locationsTab = document.getElementById('location-tabs');
    const tabContents  = document.getElementById('nav-tabContents');

    let groupTab = document.getElementById(`nav-${group}`);

    if (groupTab === null) {
      const active = first ? 'active' : '';

      const locationTabContent = CosmoScout.gui.loadTemplateContent('location-tab-link');
      const element            = document.createElement('template');

      element.innerHTML = locationTabContent.outerHTML.replace(/%ACTIVE%/g, active)
                              .replace(/%GROUP%/g, group)
                              .replace(/%FIRST%/g, first.toString())
                              .trim();

      locationsTab.appendChild(element.content);

      const show = first ? 'show' : '';

      const tabContent = CosmoScout.gui.loadTemplateContent('location-tab-pane');

      element.innerHTML = tabContent.outerHTML.replace(/%SHOW%/g, show)
                              .replace(/%ACTIVE%/g, active)
                              .replace(/%GROUP%/g, group)
                              .trim();

      tabContents.appendChild(element.content);

      groupTab = document.getElementById(`nav-${group}`);
    }

    const groupTabContent = CosmoScout.gui.loadTemplateContent('location-group');

    groupTabContent.innerHTML = groupTabContent.innerHTML.replace(/%TEXT%/g, text).trim();

    groupTab.appendChild(groupTabContent);

    CosmoScout.gui.initTooltips();
  }
}

(() => {
  CosmoScout.init(FlyToLocationsApi);
})();
