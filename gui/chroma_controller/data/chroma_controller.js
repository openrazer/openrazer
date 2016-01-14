/*
 Chroma Controller is free software: you can redistribute it and/or modify
 it under the temms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Chroma Controller is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Chroma Controller. If not, see <http://www.gnu.org/licenses/>.

 Copyright (C) 2015-2016 Luke Horwell <lukehorwell37+code@gmail.com>
 2015-2016 Terry Cain <terry@terrys-home.co.uk>
 */

/**
 * Keyboard class
 *
 * @param keyboard_element_id {string} ID for container of keyboard
 * @param keyboard_svg_path {string} Path to keyboard SVG
 */
function Keyboard(keyboard_element_id, keyboard_svg_path) {

    var keyboard_id = keyboard_element_id;
    var svg_path = keyboard_svg_path;
    var snap_object;

    var layouts = [];
    var active_layout = null;

    /**
     * Loads the keyboard layouts available
     *
     * @private
     */
    var get_layouts = function () {
        var svg_layouts = snap_object.selectAll(".kblayout");
        for (var i = 0; i < svg_layouts.length; ++i) {
            var node = svg_layouts[i].node;
            layouts.push(node.id);

            //console.info("Found layout: " + node.id);

            if (node.style["display"] != "none") {
                active_layout = node.id;
                //console.info("Active layout: " + node.id);
            }
        }

        if (active_layout == null) {
            console.error("No active layout found")
        }
    };

    /**
     * Return the available keyboard layouts
     *
     * @return {Array} Array of keyboard layouts
     */
    this.available_layouts = function () {
        return layouts;
    };

    /**
     * Set the colour of a specified key
     *
     * @param row {int} Row integer
     * @param col {int} Column integer
     * @param hex_colour {string} Hex representation of colours like in HTML/CSS E.g #FF0000
     */
    this.set_key_colour = function (row, col, hex_colour) {
        var key_id = '#key' + row + '-' + col;
        var key = snap_object.select("#" + active_layout).select(key_id);

        if(key) {
            key.selectAll("text").attr({fill: hex_colour});
            key.selectAll("path, rect").attr({stroke: hex_colour});
        }
    };
    /**
     * Set the colour of a specified key
     *
     * @param key_id {string} Key ID like "key1-5"
     * @param hex_colour {string} Hex representation of colours like in HTML/CSS E.g #FF0000
     */
    this.set_key_colour_by_id = function (key_id, hex_colour) {
        var key = snap_object.select("#" + active_layout).select('#' + key_id);

        key.selectAll("text").attr({fill: hex_colour});
        key.selectAll("path, rect").attr({stroke: hex_colour});
    };
    /**
     * Clear the colour of a specified key
     *
     * @param key_id {string} Key ID like "key1-5"
     */
    this.clear_key_colour_by_id = function (key_id) {
        var key = snap_object.select("#" + active_layout).select('#' + key_id);

        key.selectAll("text").attr({fill: "#777777"});
        key.selectAll("path, rect").attr({stroke: "#777777"});
    };


    /**
     * Set the effect mode of the keyboard
     *
     * This will set the colour of the background of the keyboard behind the keys
     *
     * @param mode {string} Set the effect mode of the keyboard
     */
    this.set_effect_mode = function (mode) {
        if (mode == "none") {
            var node = snap_object.select("#effect-layer").node;

            snap_object.select("#effect-layer").selectAll('rect').attr({fill: "#222222"});
        }
    };

    /**
     * Set LED state
     *
     * @param led_id {string} ID of the led to set
     * @param enable {boolean} Enable or disable
     *
     * @private
     */
    var set_led = function (led_id, enable) {
        if (enable) {
            snap_object.select("#" + led_id).attr({display: "inline"});
        } else {
            snap_object.select("#" + led_id).attr({display: "none"});
        }
    };

    /**
     * Set Caps Lock LED
     *
     * @param enable {boolean} Enable or disable
     */
    this.set_caps_lock = function (enable) {
        set_led("caps-lock", enable);
    };
    /**
     * Set Num Lock LED
     *
     * @param enable {boolean} Enable or disable
     */
    this.set_num_lock = function (enable) {
        set_led("num-lock", enable);
    };
    /**
     * Set Scroll Lock LED
     *
     * @param enable {boolean} Enable or disable
     */
    this.set_scroll_lock = function (enable) {
        set_led("scroll-lock", enable);
    };
    /**
     * Set Game Mode LED
     *
     * @param enable {boolean} Enable or disable
     */
    this.set_game_mode = function (enable) {
        set_led("game-mode", enable);
    };
    /**
     * Set Macro LED
     *
     * @param enable {boolean} Enable or disable
     */
    this.set_macro_led = function (enable) {
        set_led("macro-led", enable);
    };

    /**
     * Disable Key
     *
     * @param row {int} Row ID
     * @param col {int} Column ID
     */
    this.disable_key = function (row, col) {
        snap_object.select("#" + active_layout).select('#key' + row + '-' + col).attr({onclick: null});
    };
    /**
     * Enable Key
     *
     * @param row {int} Row ID
     * @param col {int} Column ID
     */
    this.enable_key = function (row, col) {
        var onclick_string = "key(this," + row + "," + col + ")";

        snap_object.select("#" + active_layout).select('#key' + row + '-' + col).attr({onclick: onclick_string});
    };

    /**
     * Reset all the keys colour
     *
     * @private
     */
    this.clear_all_keys = function () {
        snap_object.select("#" + active_layout).selectAll(".key text").attr({fill: "#777777"});
        snap_object.select("#" + active_layout).selectAll(".key path, rect").attr({stroke: "#777777"});
    };

    /**
     * Perform initial setup
     * - Gets layouts
     * - Clears keys
     *
     * @private
     */
    this.setup = function () {
        get_layouts();
        this.clear_all_keys();
        this.set_effect_mode("none");
        /*this.set_caps_lock(false);
         this.set_num_lock(false);
         this.set_scroll_lock(false);
         this.set_game_mode(false);
         this.set_macro_led(false);
         this.disable_key(5, 7);
         this.disable_key(5, 12);*/
    };

    /**
     * Change keyboard layout
     *
     * @param layout {string} Keyboard layout
     *
     * @return {boolean} True if layout change is successful
     */
    this.set_layout = function (layout) {
        var exists = false;

        for (var i = 0; i < layouts.length; ++i) {
            if (layout == layouts[i]) {
                exists = true;
                break;
            }
        }

        if (exists) {
            snap_object.select("#" + active_layout).attr({display: "none"});
            snap_object.select("#" + layout).attr({display: "inline"});
            active_layout = layout;
            this.setup();
        } else {
            console.error("Layout \"" + layout + "\" does not exist!");
        }

        return exists;
    };

    /**
     * Load the keyboard SVG into the keyboard container
     */
    this.load = function () {
        snap_object = Snap("#" + keyboard_id);



        Snap.load(svg_path, function (svg_contents) {
            snap_object.append(svg_contents);

            keyboard_obj.setup();
        });
    }
}

var keyboard_obj = new Keyboard("keyboard-div", "img/blackwidow-chroma-keyboard-layout.svg");

// Smoothly fade between two elements.
function smoothFade(from, to) {
    if ($(from).is(":visible") == false) {
        $(to).fadeIn('fast');
    } else {
        $(from).fadeOut('fast');
        setTimeout(function () {
            $(to).fadeIn('fast');
        }, 200);
    }
}

// Change the header title between page views.
function changeTitle(text) {
    $('#title').fadeOut();
    setTimeout(function () {
        $('#title').fadeIn();
        $('#title').html(text);
    }, 400);
}

// Communicate to Python via JavaScript
function cmd(parameters) {
    window.location.href = 'cmd://' + parameters;
}

// UI Elements
function setCursor(type) {
    if (type == 'normal') {
        $('html').removeClass('cursor-wait');
    } else if (type == 'wait') {
        $('html').addClass('cursor-wait');
    }
}

// Change brightness control
$(document).ready(function () {

    $("[type=range]").change(function () {
        var brightnessRaw = ($(this).val() / 255.0) * 100;
        $('#brightnessValue').text(Math.round(brightnessRaw) + "%");
        if (brightnessRaw == 0) {
            $(this).next().text("Off")
        }
        window.location.href = 'cmd://brightness?' + Math.round($(this).val());
    });

    keyboard_obj.load();
});

/**
 * onclick function of the keyboard SVG
 *
 * @param elem {object} This object of each key
 * @param row {int} Key row
 * @param row {int} Key column
 */
function key(elem, row, col)
{
    //var current_color = $('#' + pos).css("color");
    var picker_color = $('#rgb_tmp_preview').css("background-color");

    if(mode == 'set') {
        keyboard_obj.set_key_colour_by_id(elem.id, picker_color);
        cmd('set-key?' + row + '?' + col + '?' + picker_color);
    } else if(mode == 'clear') {
        keyboard_obj.clear_key_colour_by_id(elem.id);
        cmd('clear-key?' + row + '?' + col);
    } else if(mode == 'picker') {
        // TODO
    }
}

// Profile Management before the command is passed to Python.
function profile_list_change() {
    profileName = $("#profiles_list option:selected").text();

    // Change profile right away, depending on preferences.
    if (instantProfileSwitch == true) {
        cmd('profile-activate?' + profileName)
    } else {
        $('#profiles-activate').removeClass('btn-disabled')
        $('#profiles-edit').removeClass('btn-disabled')
        $('#profiles-delete').removeClass('btn-disabled')
    }
}

// Profiles
//// Colours are set to the 'color' and 'border' variables, but primarily from the 'color' attribute.
/*function key(pos) {
    current_color = $('#' + pos).css("color");
    picker_color = $('#rgb_tmp_preview').css("background-color");
    var key_id = $('#' + pos).attr('data-keyid');

    // Set Mode - Change this key's colour.
    if (mode == 'set') {
        $('#' + pos).css("color", picker_color);
        $('#' + pos).css("border", "2px solid " + picker_color);
        cmd('set-key?' + pos + '?' + picker_color + '?' + key_id);

        // Picker Mode - Take colour from this key.
    } else if (mode == 'picker') {
        $('#rgb_tmp_preview').css("background-color", current_color);
        set_mode('set');

        // Clear Mode - Remove customisation from this key.
    } else if (mode == 'clear') {
        $('#' + pos).css("color", cleared);
        $('#' + pos).css("border", "2px solid " + cleared_border);
        cmd('clear-key?' + pos + '?' + key_id);
    }
}*/

// Change mode of cursor
function set_mode(id) {
    if (id == 'set') {
        mode = 'set';
        $('#current_mode').html("<b>Set</b> - Click on a key to assign a colour here.");
    } else if (id == 'picker') {
        mode = 'picker';
        $('#current_mode').html("<b>Picker</b> - Click on a key to grab its colour.");
    } else if (id == 'clear') {
        mode = 'clear';
        $('#current_mode').html("<b>Clear</b> - Click on a key to clear.");
    }
}

// Set variables at start.
var mode;
var cleared = 'rgb(128, 128, 128)';
var cleared_border = 'rgb(70, 70, 70)';
set_mode('set');

// Prompt for a new profile name.
function profile_new() {
    response = window.prompt("Please name your new key profile.");
    if (response != null && response.length > 0) {
        cmd('profile-new?' + response)
    }
}

// Confirm deletion of a profile.
function profile_del(profileName) {
    response = window.confirm("Are you sure you wish to delete '" + profileName + "'?")
    if (response == true) {
        cmd('profile-del?' + profileName)
    }
}
