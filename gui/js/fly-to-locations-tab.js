function add_location(group, text) {
    let first = false;
    const tabArea = $("#location-tabs-area");
    if (tabArea.children().length === 0) {
        first = true;
        tabArea.append(`
            <nav>
                <div class="row nav nav-tabs" id="location-tabs" role="tablist"></div>
            </nav>
            <div class="tab-content" id="nav-tabContents"></div>
        `)
    }

    const locationsTab = $("#location-tabs");
    const tabContents = $("#nav-tabContents");

    let groupTab = $(`#nav-${group}`);
    if (groupTab.length === 0) {
        const active = first ? "active" : "";
        locationsTab.append(`
            <a class="nav-item nav-link ${active} col-4" id="nav-${group}-tab" data-toggle="tab" href="#nav-${group}" 
                role="tab" aria-controls="nav-${group}" aria-selected="${first}">${group}</a>
        `);

        const show = first ? "show" : "";

        tabContents.append(`
            <div class="tab-pane fade ${show} ${active}" id="nav-${group}" role="tabpanel" aria-labelledby="nav-${group}-tab"></div>
        `);

        groupTab = $(`#nav-${group}`);
    }

    groupTab.append(`
        <div class='row'>
            <div class='col-8'>
                ${text}
            </div>
            <div class='col-4'>
                <a class='btn glass block fly-to' data-toggle="tooltip" title='Fly to ${text}' onclick='window.call_native("fly_to", "${text}")'>
                    <i class='material-icons'>send</i>
                </a>
            </div>
        </div>
    `);

    $('[data-toggle="tooltip"]').tooltip({ delay: 500, placement: "auto", html: false });
}


function add_celestial_body(name, icon) {
    const area = $('#celestial-bodies');
    area.append(`<div class='col-3 center' style='padding: 3px'>
                    <a class='block btn glass' id='set_body_${name}'>
                        <img style='pointer-events: none' src='../icons/${icon}' height='80' width='80'>
                        ${name}
                    </a>
                </div>`);

    $('#set_body_' + name).on('click', function () {
        window.call_native('set_celestial_body', name);
    });
}