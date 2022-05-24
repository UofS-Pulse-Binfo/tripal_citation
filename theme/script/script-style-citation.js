/**
 * Export to data for Helium script.
 */

 (function ($) {
  Drupal.behaviors.scriptStyleCitation = {
    attach: function (context, settings) {   
    
    // Load styling when style is selected.
    $('#tripal-citation-select-style').change(function() {
      var newClass = 'tripal-citation-style-' + $(this).val();

      $('#tripal-citation-cite div:first-child').attr('class', newClass);
    });

    // Show copy citation icon.
    $('#tripal-citation-checkbox-copy').click(function() {
      var addCopyClass = ($(this).is(':checked')) 
        ? 'tripal-citation-control-copy' : 'tripal-citation-control-none';
      
      $('#tripal-citation-control').attr('class', addCopyClass);
    });
}};}(jQuery));