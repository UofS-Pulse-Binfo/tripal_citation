<?php

/**
 * @file 
 * Include file containing function callback that generates configuration page.
 */

/**
 * Configuration form render array.
 */
function tripal_citation_configure_form($form, &$form_state) {
  $module = drupal_get_path('module', 'tripal_citation');
  drupal_add_css($module . '/theme/style/style-citation.css');
  drupal_add_js($module . '/theme/script/script-style-citation.js');

  // Example citation:
  $example_citation = 'Lorem ipsum dolor sit amet, consectetur adipiscing elit. 
    Cras ac dolor sed turpis ullamcorper pulvinar euismod ut nisl. 
    Vestibulum tincidunt orci vel nulla vulputate dignissim. 
    <br />Aenean consectetur malesuada convallis. Fusce dapibus ipsum a dictum varius.';


  // APPEARANCE:
  $form['configure_appearance'] = array(
    '#type' => 'fieldset',
    '#title' => t('Appearance'),
  );

    // # Citation styles.
    $default_style = variable_get('tripal_citation_appearance');
    // Smart Citation = smart looking using of quotes.
    // Banner Citation = citation contained in a banner container.
    // Simple Citation  = plain and simple purely text-based.
    $styles = array(
      'smart' => 'Smart Citation',
      'banner' => 'Banner Citation',
      'simple'  => 'Simple Citation' 
    );

    $form['configure_appearance']['select_style'] = array(
      '#type' => 'select',
      '#title' => t('Style Citation'),
      '#description' => t('Select a style to apply to all citations.'),
      '#options' => $styles,
      '#default_value' => $default_style,
      '#id' => 'tripal-citation-select-style'
    );
    
    // Render a citation with the style selected applied.
    $markup = theme('tripal_citation_citation', array(
      'style' => $default_style,
      'citation' => $example_citation,
      'title' => '',
      'control' => 'none'
    ));
    
    $form['configure_appearance']['apply_style'] = array(
      '#type' => 'markup',
      '#markup' => $markup
    );

    // # Title text.
    // Inform user when this field is not set.
    $default_title = variable_get('tripal_citation_title');
    $title_alert = (trim($default_title) == '') ? '(No title text set)' : '';

    // Citation title/text:
    $form['configure_appearance']['text_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title Text @alert', array('@alert' => $title_alert)),
      '#description' => t('Add title or text that will appear on top of citations.'),
      '#default_value' => $default_title,
      '#id' => 'tripal-citation-text-title'
    );

    // Render a citation with title text.
    $markup = theme('tripal_citation_citation', array(
      'style' => 'banner',
      'citation' => $example_citation,
      'title' => 'This is an example title text',
      'control' => 'none'
    ));
    
    $form['configure_appearance']['apply_title'] = array(
      '#type' => 'markup',
      '#markup' => 'EXAMPLE: ' . $markup
    );

    
  // CONTROLS:
  $form['configure_controls'] = array(
    '#type' => 'fieldset',
    '#title' => t('Controls'),
  );  

    // # Copy to clipboard.
    $default_control = variable_get('tripal_citation_control');
    
    // Allow copying citation to clipboard.
    $form['configure_controls']['checkbox_copy'] = array(
      '#type' => 'checkbox',
      '#title' => t('Add button to copy citation to clipboard for quick copy and paste action.'),
      '#default_value' => ($default_control == 'copy') ? 1 : 0,
      '#id' => 'tripal-citation-checkbox-copy'
    );
    
    // Render cittion (matching the selected style) with copy button.
    $markup = theme('tripal_citation_citation', array(
      'style' => 'banner',
      'citation' => $example_citation,
      'title' => '',
      'control' => 'copy'
    ));

    $form['configure_controls']['add_copy'] = array(
      '#type' => 'markup',
      '#markup' => 'EXAMPLE (see button to right hand side): ' . $markup
    );


  // SUBMIT.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save Configuration'
  );


  return $form;
}


/**
 * Implements hook_form_submit().
 */
function tripal_citation_configure_form_submit($form, $form_state) {
  // Appearance:

  // Style.
  $style = $form_state['values']['select_style'];
  variable_set('tripal_citation_appearance', $style);

  // Title.
  $title = $form_state['values']['text_title'];
  variable_set('tripal_citation_title', $title);
 

  // Control:
  // NOTE: to disable control use none.
  $control = $form_state['values']['checkbox_copy'];
  $control_value = ($control == 1) ? 'copy' : 'none'; 
  variable_set('tripal_citation_control', $control_value);  
}