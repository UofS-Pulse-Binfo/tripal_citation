<?php
/**
 * @file
 * Configure Tripal Citation.
 */

/**
 * Function callback - construct configure Tripal Citation page
 */
function tripal_citation_configure($form, &$form_state) {
  // Main fieldset.
  $form['tripal_citaiton_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Configure Tripal Citation Template'),
    '#collapsed' => FALSE,
    '#collapsible' => TRUE,
  );

  return $form;
}
