<?php
/**
 * @class
 * Purpose: Provide automated citations for data.
 *
 * Display: A simple textfield search form.
 * Configuration:
 */
class ncit__citation_formatter extends TripalFieldFormatter {
  // The default label for this field.
  public static $default_label = 'Tripal Citation';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('ncit__citation');

  // IMPORTANT: this will allow settings form variables
  // to be persistent.
  public static $default_settings = array(
    'template' => '',
    'date' => '',
    'title' => '',
    'prefer_publication' => 1
  );

  /**
   *  Provides the display for a field
   *
   * This function corresponds to the hook_field_formatter_view()
   * function of the Drupal Field API.
   *
   *  This function provides the display for a field when it is viewed on
   *  the web page.  The content returned by the formatter should only include
   *  what is present in the $items[$delta]['values] array. This way, the
   *  contents that are displayed on the page, via webservices and downloaded
   *  into a CSV file will always be identical.  The view need not show all
   *  of the data in the 'values' array.
   *
   *  @param $element
   *  @param $entity_type
   *  @param $entity
   *  @param $langcode
   *  @param $items
   *  @param $display
   *
   *  @return
   *    An element array compatible with that returned by the
   *    hook_field_formatter_view() function.
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {
    // Inspect settings variable and see if this content type has 
    // citation template set.

    // System variables (with default key) defined in hook_bundle_instances_info().
    // @see TripalFields/tripal_citation.fields.inc.
    $display = $this->instance['display']['default'];
    $settings = $display['settings'];

    $citation_template = trim($settings['template']);
    $citation_date = trim($settings['date']);   

    // The final citation text generated.
    $citation = '';  
    
    if ($items[0]['value'] && !empty($citation_template)) {
      // Field has data and template has been set.

      // # 1. TRANSPOSE ALL CVTERM TOKENS.
      
      // Get all tokens from the template.
      $tokens_template = tripal_citation_get_tokens($citation_template);
      // All dataset for this entity keyed by cvterm token.
      $tokens_dataset = $items[0]['value']['hydra:member'];
    
      // Citation template or publication?
      if ($settings['prefer_publication']) {
        // Prefer publication citation.
        // Check for publication, project and analysis table connection.
        // PUBLICATION, PROJECT AND ANALYSISFEATURE.
        $property_set = array(0, 1, 2);
      }
      else {
        // Citation template. 
        // Replace tokens using the table prop table and check project and analysis if
        // prop did not house the tokens.
        // PROJECT, ANALYSISFEATURE AND PROP TABLE.
        $property_set = array(1, 2, 3);
      }

      // Transpose all non-cvterms and missing information.
      // Using the base properties defined.
      if (count($tokens_template) > 0) { 
        // If there are tokens to parse coming off citation template settings.
        foreach($tokens_template as $token) {
          foreach($property_set as $set) { 
            if (is_array($tokens_dataset[ $set ]) && $tokens_dataset[ $set ][ $token ]) { 
              // Token match a data from the properties.
              // Process (sort by rank) and render comma separate values
              // (if applicable) as the value for this token.
              $dataset = $tokens_dataset[ $set ][ $token ];

              // dataset: token = [rank => value], [rank => value] ...
              // Rank the values (if multiple values) and comma separate.
              $token_value = tripal_citation_rank_token_values($dataset);

              // Transpose this token back to the current citation.
              $citation = tripal_citation_transpose_cvterm($citation, $token, $token_value);
              // No need to inspect other terms one token has been indentified.
              break;
            }
          }
        }
      }
    }

    $module_path = drupal_get_path('module', 'tripal_citation');
    // Apply template to citation using style set in configuration.
    $override = (trim($settings['title']) == '') ? array() : array('title' => $settings['title']);
    $markup = tripal_citation_theme_citation($citation, $override);

    $element[0] = array(
      '#type' => 'markup',
      '#markup' => $markup,
      '#attached' => array(
        'css' => array($module_path . '/theme/style/style-citation.css'),
        'js'  => array($module_path . '/theme/script/script-copy-citation.js')
      )
    );
    
    return $element;
  }


  /**
   * @see TripalFieldFormatter::settingsForm()
   *
   **/
  public function settingsForm($view_mode, $form, &$form_state) {    
    $display = $this->instance['display'][$view_mode];
    $settings = $display['settings'];

    // Citation template - text and tokens.
    $element['template'] = array(
      '#type' => 'textarea',
      '#title' => t('Citation Template'),
      '#default_value' => $settings['template'],
    );
    
    // Date format to use to convert date token {CURRENT_DATE}
    $system_format = system_get_date_types(); 
    $format_options = array();
    
    foreach($system_format as $format => $prop) {
      // Date format machine name - FORMAT TITLE - (Example applied to current time).
      $format_options[ $format ] = strtoupper($prop['title']) . ': (' . tripal_citation_format_date($format) .')';
    }

    $element['date'] = array(
      '#type' => 'select',
      '#title' => t('Date format to use when converting {CURRENT_DATE} date token'),
      '#options' => $format_options,
      '#default_value' => $settings['date'],
    );

    $element['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Citation title text for this content type'),
      '#description' => t('This title will override title set in the main configuration'),
      '#default_value' => $settings['title'],
    );
    
    // Use publication when available as citation.
    $element['prefer_publication'] = array(
      '#type' => 'checkbox',
      '#title' => t('Prefer publication as citation (when available)'),
      '#default_value' => $settings['prefer_publication'],
    );

    return $element;
  }

  /**
   * @see TripalFieldFormatter::settingsSummary()
   *
   **/
  public function settingsSummary($view_mode) {
    $display = $this->instance['display'][ $view_mode ];
    $settings = $display['settings'];

    // Translate value of prefer publication to yes or no.
    $prefer_publication = ($settings['prefer_publication']) ? 'yes' : 'no';

    $summary = t('<strong>Citation Template</strong> @template<br /><strong>Date Token</strong> @date<br /><strong>Title</strong> @title<br /><strong>Prefer Publication</strong> @prefer_publication',
      array(
        '@template' => $settings['template'],
        '@date' => $settings['date'] . ' (' . tripal_citation_format_date($settings['date']) . ')',
        '@title' => $settings['title'],
        '@prefer_publication' => $prefer_publication
    ));

    return $summary;
  }
}