/**
 * @file 
 * Script containing to handle copy button.
 */

(function ($) {
  Drupal.behaviors.scriptControlCitation = {
    attach: function (context, settings) {   

      if ($('.tripal-citation-control-copy')) {  
        var copy = $('.tripal-citation-control-copy');
        copy.click(function(e) {
          var text = $(this).parent().text().trim();  
          navigator.clipboard.writeText(text);

          // Indicate it has copied.
          var t = 0;
          var timer = setInterval(function() {
            if (t < 5) {
              copy.css('opacity', 0);
            } 
            else {
              copy.css('opacity', 1);
              clearInterval(timer);
            }

            t++;
          }, 200);
        });        
      }
}};}(jQuery));