/*
 Chroma Controller is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
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


 ** Functions for editing profiles and interactions with keyboard SVG.
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
     * Return the colour of the selected key
     * @param row {int} Row integer
     * @param col {int} Column integer
     * @returns {string|null} Hex colour value or null
     */
    this.get_key_colour = function (row, col) {
        console.log("Row: " + row + " Col: " + col);
        var key_id = '#key' + row + '-' + col;
        var key = snap_object.select("#" + active_layout).select(key_id);

        if(key) {
            return key.selectAll("path, rect, ellipse")[0].attr("stroke");
        }
        return null;
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
            key.selectAll("path, rect, ellipse").attr({stroke: hex_colour});
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
        key.selectAll("path, rect, ellipse").attr({stroke: hex_colour});
    };
    /**
     * Clear the colour of a specified key
     *
     * @param key_id {string} Key ID like "key1-5"
     */
    this.clear_key_colour_by_id = function (key_id) {
        var key = snap_object.select("#" + active_layout).select('#' + key_id);

        key.selectAll("text").attr({fill: "#777777"});
        key.selectAll("path, rect, ellipse").attr({stroke: "#777777"});
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
        snap_object.select("#" + active_layout).selectAll(".key path, rect, ellipse").attr({stroke: "#777777"});
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

        if (!exists) {
            console.error("Layout \"" + layout + "\" does not exist! Using 'kb-gb'.");
            layout = "kb-gb";
        }

        snap_object.select("#" + active_layout).attr({display: "none"});
        snap_object.select("#" + layout).attr({display: "inline"});
        active_layout = layout;
        this.setup();

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


// Initialise keyboard object
var keyboard_obj = new Keyboard("keyboard-div", "img/blackwidow-chroma-keyboard-layout.svg");


/**
 * onclick function of the keyboard SVG
 *
 * @param elem {object} This object of each key
 * @param row {int} Key row
 * @param col {int} Key column
 */
function key(elem, row, col)
{
    // Get colour box
    var colour_box = $('#rgb_tmp_preview');
    var picker_color = colour_box.css("background-color");

    if(mode == 'set') {
        // Set key colour
        keyboard_obj.set_key_colour_by_id(elem.id, picker_color);
        cmd('set-key?' + row + '?' + col + '?' + picker_color);
    } else if(mode == 'clear') {
        // Clear key colour
        keyboard_obj.clear_key_colour_by_id(elem.id);
        cmd('clear-key?' + row + '?' + col);
    } else if(mode == 'picker') {
        // Get colour from key
        var current_colour = keyboard_obj.get_key_colour(row, col);
        colour_box.css("background-color", current_colour);
        set_mode("set");
    }
}

/**
 * Change the mode of left clicking
 *
 * @param id {string} Mode ID
 */
var mode;
function set_mode(id) {
    if (id == 'set') {
        mode = 'set';
        $('#current_mode').html("<b class='mode-text'>Set</b> - Click on a key to assign a colour here.");
        set_cursor('#keyboard-div','mode-set')
    } else if (id == 'picker') {
        mode = 'picker';
        $('#current_mode').html("<b class='mode-text'>Picker</b> - Click on a key to grab its colour.");
        set_cursor('#keyboard-div','mode-picker')
    } else if (id == 'clear') {
        mode = 'clear';
        $('#current_mode').html("<b class='mode-text'>Clear</b> - Click on a key to clear.");
        set_cursor('#keyboard-div','mode-clear')
    }
}


/**
 * Run once document has loaded
 */
$(document).ready(function () {

    // Set initial key mode
    set_mode('set');

});
