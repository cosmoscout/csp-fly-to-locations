function add_location(group, text) {
    const tab_name = 'list-location-' + group;
    const list = $('#' + tab_name);
    const area = $('#location-tabs-area');

    if (area.children().length === 0) {
        area.append('<div class=\'col s12\'> \
                        <ul class=\'tabs\' id=\'location-tabs\'> \
                        </ul> \
                    </div>');
    }

    if (list.length === 0) {
        const tabs = $('#location-tabs');
        tabs.children().removeAttr('style');
        tabs.append('<li class=\'tab col s3\'><a href=\'#' + tab_name + '\'>' + group + '</a></li>');
        area.append('\
            <div class="col s12 scroll-box" style="max-height:250px">\
                <div id="' +
            tab_name + '" class="item-list scroll-box-content">\
                </div>\
            </div>\
        ');

        // initalize tabs again, then select new tab
        tabs.tabs();
    }

    const html = '\
        <div class=\'row\'>\
            <div class=\'col s8\'>\
                ' +
        text + '\
            </div>\
            <div class=\'col s4\'><a class=\'waves-effect waves-light btn glass tooltipped block\' data-position=\'top\' data-delay=\'50\' data-tooltip=\'Fly to ' +
        text + '\' onclick=\'window.call_native("fly_to", "' + text +
        '")\' ><i class=\'material-icons\'>send</i></a></div>\
   </div>';

    $('#' + tab_name).append(html);
}


function add_celestial_body(name, icon) {
    const area = $('#celestial-bodies');
    area.append('<div class=\'col s3 center\'>\
                    <label>\
                        <input id=\'set_body_' +
        name + '\' type=\'radio\' name=\'celestial-body\' />\
                        <div class=\'waves-effect waves-light block btn btn-large glass\'>\
                            <img style=\'pointer-events: none\' src=\'../icons/' +
        icon + '\' height=\'80\' width=\'80\'>' + name + '</div>\
                    </label>\
                </div>');

    $('#set_body_' + name).on('click', function() {
        window.call_native('set_celestial_body', name);
    });
}