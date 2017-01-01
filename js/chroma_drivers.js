/*
 *  Designed by @lah7 for the Razer Chroma Linux drivers project.
 *
 * Copyright (C) 2016-2017 Luke Horwell (lah7)
 * Licensed under CC BY-SA 4.0
 */

// Floating "Mini" Navigation Bar
$(document).ready(function () {
  $(document).scroll(function () {
    if ( $(this).scrollTop() > 128 ) {
        $('#nav-menu').addClass('nav-sticky');
        $('#nav-menu').removeClass('nav-top');
        $('#mini-logo-text').fadeIn();
    } else {
        $('#nav-menu').removeClass('nav-sticky');
        $('#nav-menu').addClass('nav-top');
        $('#mini-logo-text').fadeOut();
    }
  });
});

// Add smooth scrolling to all links inside a navbar
$("#navbar a").on('click', function(event){

  // Prevent default anchor click behavior
  event.preventDefault();

  // Store hash (#)
  var hash = this.hash;

  // Using jQuery's animate() method to add smooth page scroll
  // The optional number (800) specifies the number of milliseconds it takes to scroll to the specified area (the speed of the animation)
  $('html, body').animate({
    scrollTop: $(hash).offset().top
  }, 800, function(){

    // Add hash (#) to URL when done scrolling (default click behavior)
    window.location.hash = hash;
  });
});

// Auto-open OS installation instructions if specified with hash.
if (window.location.hash) {
    var hash = window.location.hash.substring(1);
    $('#' + hash).collapse('show');
    $('body').animate({ scrollTop: $('#'+hash).position().top - 32 });
}
