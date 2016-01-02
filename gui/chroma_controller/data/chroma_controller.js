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
    $(to).fadeIn();
  } else {
    $(from).fadeOut();
    setTimeout(function(){ $(to).fadeIn(); }, 400 );
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
    window.location.href = 'cmd://' + parameters
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
