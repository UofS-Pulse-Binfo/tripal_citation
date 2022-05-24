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

  // APPEARANCE:
  $form['configure_appearance'] = array(
    '#type' => 'fieldset',
    '#title' => t('Appearance'),
  );

    // Citation styles:
    // Smart Citation = smart looking using of quotes.
    // Banner Citation = citation contained in a banner container.
    // Simple Citation = plain and simple purely text-based.
    $styles = array(
      'smart' => 'Smart Citation',
      'banner' => 'Banner Citation',
      'simple'  => 'Simple Citation' 
    );

    // Get default style (Smart Citation) set in the install file 
    // unless changed by admin after install.
    $default_style = variable_get('tripal_citation_appearance');
    $style = 'tripal-citation-style-' . $default_style;

    $form['configure_appearance']['select_style'] = array(
      '#type' => 'select',
      '#title' => t('Style Citation'),
      '#description' => t('Select a style to apply to all citations.'),
      '#options' => $styles,
      '#default_value' => $default_style,
      '#id' => 'tripal-citation-select-style'
    );

    // Render a citation with the style selected.
    $form['configure_appearance']['apply_style'] = array(
      '#type' => 'markup',
      '#markup' => '<div id="tripal-citation-cite">
        <div class="' . $style . '">
          Lorem ipsum dolor sit amet, consectetur adipiscing elit. 
          Cras ac dolor sed turpis ullamcorper pulvinar euismod ut nisl. 
          Vestibulum tincidunt orci vel nulla vulputate dignissim. 
          <br />Aenean consectetur malesuada convallis. 
          Fusce dapibus ipsum a dictum varius.
        </div>
        <div class="tripal-citation-control-none"></div>
      </div>'
    );
  
  // CONTROLS:
  $form['configure_controls'] = array(
    '#type' => 'fieldset',
    '#title' => t('Controls'),
  );  

    // Get default control set in the install file 
    // unless changed by admin after install.
    $default_control = variable_get('tripal_citation_control');
    $control = 'tripal-citation-control-' . $default_control;
    
    // Allow copying citation to clipboard.
    $form['configure_controls']['checkbox_copy'] = array(
      '#type' => 'checkbox',
      '#title' => t('Add button to copy citation to clipboard for quick copy and paste action.'),
      '#default_value' => ($default_control == 'copy') ? 1 : 0,
      '#id' => 'tripal-citation-checkbox-copy'
    );

    $form['configure_controls']['add_copy'] = array(
      '#type' => 'markup',
      '#markup' => 'EXAMPLE: <div id="tripal-citation-cite">
        <div class="' . $style . '">
          Lorem ipsum dolor sit amet, consectetur adipiscing elit. 
          Cras ac dolor sed turpis ullamcorper pulvinar euismod ut nisl. 
          Vestibulum tincidunt orci vel nulla vulputate dignissim. 
          <br />Aenean consectetur malesuada convallis. 
          Fusce dapibus ipsum a dictum varius.
        </div>
        <div id="tripal-citation-control" class="' . $control . '" title="Copy citation to clipboard"></div>
      </div>'
    );

  // SUbmit.
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
  $style = $form_state['values']['select_style'];
  variable_set('tripal_citation_appearance', $style);

  // Control:
  $control = $form_state['values']['checkbox_copy'];
  $control_value = ($control == 1) ? 'copy' : 'none'; 
  variable_set('tripal_citation_control', $control_value);  
}