/**
 * @file 
 * Script containing events from configuration form.
 */

(function ($) {
  Drupal.behaviors.scriptStyleCitation = {
    attach: function (context, settings) {   

      // Load styling when a style is selected.
      $('#tripal-citation-select-style').change(function() {
        var newClass = 'tripal-citation-style-' + $(this).val();
      
        // Apply style only to first instance of citation.
        $('.tripal-citation-cite').eq(0)
          .find('div').eq(0).attr('class', newClass);
      });

      // Select field value when title field is selected.
      $('#tripal-citation-text-title').click(function() {
        if ($(this).val()) {
          $(this).select();
        }
      });
}};}(jQuery));