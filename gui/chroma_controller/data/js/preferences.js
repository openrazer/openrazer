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


 ** Functions specific to the Preferences menu.
 */

function reset_all_prompt() {
    if ( confirm('Are you sure you wish to erase all configuration and profiles?') == true ) {
      cmd('pref-reset-all');
    }
}

function toggle_startup(element) {
    set_pref_chkstate('startup', 'enabled', element);
    state = $(element).is(':checked');
    if ( state == true ) {
        $('#startup-options').fadeIn()
    } else {
        $('#startup-options').fadeOut()
    }
}

$('#start-effect-dropdown').change(function() {
    selected = $("#start-effect-dropdown option:selected").val();
    set_pref_str('startup', 'start_effect', selected);
    if ( selected == 'profile' ) {
        $('#start-profile').show();
    } else {
        $('#start-profile').hide();
    }
});

$('#profiles-list').change(function() {
    selected = $("#profiles-list option:selected").val();
    set_pref_str('startup', 'start_profile', selected);
});

$(document).ready(function () {
    $("#start-brightness").change(function () {
        var brightnessRaw = ($(this).val() / 255.0) * 100;
        var brightnessRnd = Math.round(brightnessRaw).toString();
        $('#start-brightness-text').text(brightnessRnd + "%");
        if (brightnessRnd == 0) {
            $("#start-brightness-text").text("No Change");
        }
        set_pref_str('startup','start_brightness', $(this).val());
    });
});
