class FlyToApi extends IApi {
    name = 'flyto';

    flyTo(planet, location, time) {
        if (typeof location === 'undefined') {
            CosmoScout.callNative('fly_to', planet);
        } else {
            CosmoScout.callNative('fly_to', planet, location.longitude, location.latitude, location.height, time);
        }

        CosmoScout.call('notifications', 'printNotification', 'Traveling', `to ${planet}`, 'send');
    }

    setCelestialBody(name) {
        CosmoScout.callNative('set_celestial_body', name);
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

CosmoScout.init(FlyToApi);
