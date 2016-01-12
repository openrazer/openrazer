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
*/

// Smoothly fade between two elements.
function smoothFade(from, to) {
  if ($(from).is(":visible") == false) {
    $(to).fadeIn('fast');
  } else {
    $(from).fadeOut('fast');
    setTimeout(function(){ $(to).fadeIn('fast'); }, 200 );
  }
}

// Change the header title between page views.
function changeTitle(text) {
  $('#title').fadeOut();
  setTimeout(function(){
    $('#title').fadeIn();
    $('#title').html(text);
  }, 400 );
}

// Communicate to Python via JavaScript
function cmd(parameters) {
    window.location.href = 'cmd://' + parameters;
}

// UI Elements
function setCursor(type) {
  if ( type == 'normal' ) {
      $('html').removeClass('cursor-wait');
  } else if ( type == 'wait' ) {
      $('html').addClass('cursor-wait');
  }
}

// Change brightness control
$(document).ready(function(){
  $("[type=range]").change(function(){
    var brightnessRaw=($(this).val() / 255.0)*100;
    $('#brightnessValue').text(Math.round(brightnessRaw)+"%");
    if ( brightnessRaw == 0 ) { $(this).next().text("Off") };
    window.location.href = 'cmd://brightness?' + Math.round($(this).val())
  });
});

// Profile Management before the command is passed to Python.
function profile_list_change() {
  profileName = $("#profiles_list option:selected").text();

  // Change profile right away, depending on preferences.
  if ( instantProfileSwitch == true ) {
    cmd('profile-activate?'+profileName)
  } else {
    $('#profiles-activate').removeClass('btn-disabled')
    $('#profiles-edit').removeClass('btn-disabled')
    $('#profiles-delete').removeClass('btn-disabled')
  }
}

// Profiles
//// Colours are set to the 'color' and 'border' variables, but primarily from the 'color' attribute.
function key(pos) {
  current_color = $('#'+pos).css("color");
  picker_color  = $('#rgb_tmp_preview').css("background-color");

  // Set Mode - Change this key's colour.
  if ( mode == 'set' ) {
    $('#'+pos).css("color",picker_color);
    $('#'+pos).css("border","2px solid "+picker_color);
    cmd('set-key?'+pos+'?'+picker_color)

  // Picker Mode - Take colour from this key.
  } else if ( mode == 'picker' ) {
    $('#rgb_tmp_preview').css("background-color",current_color);
    set_mode('set');

  // Clear Mode - Remove customisation from this key.
  } else if ( mode == 'clear' ) {
    $('#'+pos).css("color",cleared);
    $('#'+pos).css("border","2px solid "+cleared_border);
    cmd('clear-key?'+pos)
  }
}

// Change mode of cursor
function set_mode(id) {
  if ( id == 'set' ) {
    mode = 'set';
    $('#current_mode').html("<b>Set</b> - Click on a key to assign a colour here.");
  } else if ( id == 'picker' ) {
    mode = 'picker';
    $('#current_mode').html("<b>Picker</b> - Click on a key to grab its colour.");
  } else if ( id == 'clear' ) {
    mode = 'clear';
    $('#current_mode').html("<b>Clear</b> - Click on a key to clear.");
  }
}

// Set variables at start.
var mode;
var cleared = 'rgb(128, 128, 128)'
var cleared_border = 'rgb(70, 70, 70)'
set_mode('set');

// Prompt for a new profile name.
function profile_new() {
  response = window.prompt("Please name your new key profile.");
  if ( response != null ) {
    cmd('profile-new?'+response)
  }
}

// Confirm deletion of a profile.
function profile_del(profileName) {
  response = window.confirm("Are you sure you wish to delete '"+profileName+"'?")
  if ( response == true ) {
    cmd('profile-del?'+profileName)
  }
}
