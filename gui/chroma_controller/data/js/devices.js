/**
 * Adds a detected device to the list.
 */
function add_device(serial, hardware_type) {
    // Determine the icons and menus for this type of hardware.
    if (hardware_type == 'blackwidow_chroma') {
        var device_name = 'BlackWidow Chroma';
        var image_name = 'blackwidow_chroma';
        var config_menu = 'chroma_menu';
    }
    // Append to the list.
    $('#detected-devices').append('<tr><td class="device-image"><img src="img/devices/' + image_name + '.png"/></td><td><h2>' + device_name + '</h2></td><td><a class="btn" onclick=\'configure_device("' + serial + '")\'>Configure</a></td></tr>');
}

function configure_device(serial) {
    window.location.href = 'cmd://set-device?' + serial;
}
