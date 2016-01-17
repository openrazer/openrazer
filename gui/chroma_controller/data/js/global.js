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


 ** Functions that are persistent on all pages.
 */


/**
 * Fade in and out between 2 elements
 *
 * @param from {string} Element
 * @param to {string} Element
 */
function smooth_fade(from, to) {
    if ($(from).is(":visible") == false) {
        $(to).fadeIn('fast');
    } else {
        $(from).fadeOut('fast');
        setTimeout(function () {
            $(to).fadeIn('fast');
        }, 200);
    }
}


/**
 * Change the header of the page.
 *
 * @param text {string} Header text
 */
function change_header(text) {
    var header = $('#page-header');
    header.fadeOut();

    setTimeout(function () {
        header.fadeIn();
        header.html(text);
    }, 400);
}


/**
 * Sends command to python.
 *
 * @param parameters {string} Command parameters
 */
function cmd(parameters) {
    // TODO Change this to use the page title as that is quicker and the title is not used.
    // If we use the title then we can then split up the main UI into seperate HMTL files
    // Also will redo the command to pass through JSON as that way there is less
    // string splitting and can do better communication.
    window.location.href = 'cmd://' + parameters;
}


/**
 * Change the cursor of the page.
 *
 * @param type {string} Cursor type
 */
function set_cursor(type) {
    if (type == 'normal') {
        $('html').removeClass('cursor-wait');
    } else if (type == 'wait') {
        $('html').addClass('cursor-wait');
    }
}

/**
 * Set a preference
 *
 */
function set_pref(setting, element) {
    state = $(element).is(':checked');
    cmd('pref-set?' + setting + '?' + state)
}

/**
 * Run once document has loaded
 */
// Always fade in the page.
$('.content').fadeIn('slow');
