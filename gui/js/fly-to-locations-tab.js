function add_location(group, text) {
    CosmoScout.call('sidebar', 'addLocation', group, text);
}


function add_celestial_body(name, icon) {
    CosmoScout.call('sidebar', 'addCelestialBody', name, icon);
}