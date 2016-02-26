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


 ** Functions for the Chroma Main Menu.
 */


/**
 * Enable buttons when a profile is clicked
 */
function profile_list_change() {
    $('#profiles-activate, #profiles-edit, #profiles-delete').removeClass('btn-disabled');
}

/**
 * Get the name for a new profile.
 */
function profile_new() {
    var dialog_response = window.prompt("Please name your new key profile.");
    if (dialog_response != null && dialog_response.length > 0) {
        cmd('profile-new?' + dialog_response);
    }
}

/**
 * Edit profile.
 */
function profile_edit() {
    var selected_profile = $("#profiles-list option:selected").text();
    cmd('profile-edit?' + selected_profile);
}

/**
 * Delete a profile confirmation box
 */
function profile_del() {
    var selected_profile = $("#profiles-list option:selected").text();

    var dialog_response = window.confirm("Are you sure you wish to delete '" + selected_profile + "'?");
    if (dialog_response == true) {
        cmd('profile-del?' + selected_profile);
    }
}

/**
 * Activate profile.
 */
function profile_activate() {
    var selected_profile = $("#profiles-list option:selected").text();
    cmd('profile-activate?'+selected_profile);
}

/**
 * Run once document has loaded
 */
$(document).ready(function () {

    // Change brightness control
    $("[type=range]").change(function () {
        var brightnessRaw = ($(this).val() / 255.0) * 100;
        $('#brightnessValue').text(Math.round(brightnessRaw) + "%");
        if (brightnessRaw == 0) {
            $(this).next().text("Off")
        }
        window.location.href = 'cmd://brightness?' + Math.round($(this).val());
    });

    // Instant profile activation (if 'live_switch' is enabled in preferences)
    $('#profiles-list').change(function() {
        if ( live_switch == true ) {
            profile_activate()
        }
    });

});
