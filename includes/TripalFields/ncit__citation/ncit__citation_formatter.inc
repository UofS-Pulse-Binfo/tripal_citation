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
    $citation  = '';

    $link = url(current_path(), array('absolute' => TRUE));
    $non_cvterms = array(
      '{CURRENT_DATE}' => format_date(time(), $citation_date), 
      '{CURRENT_URL}' => l($link, $link)
    );

    if ($items[0]['value'] && !empty($citation_template)) {
      // 1. TRANSPOSE ALL NON-CVTERM TOKENS.
      $citation = str_replace(array_keys($non_cvterms), array_values($non_cvterms), $citation_template);

      // 2. TRANSPOSE ALL CVTERM TOKENS.
      // From this point use already revised version of template where
      // all non-cvterms are omitted.
      
      // Get all tokens.
      preg_match_all('/{(.*?)}/', $citation, $match);
      // Tokens from the template settings.
      $tokens_template = $match[1];
      // All dataset for this entity keyed by cvterm token.
      $tokens_dataset = $items[0]['value']['hydra:member'];

      if ($tokens_template) {
        // If there are tokens to parse coming off citation template settings.
        // Work only on the tokens that are in the template and excess
        // tokens from instance can be ignored.
        foreach($tokens_template as $token) {         
          if ($tokens_dataset[ $token ]) {
            // Token has data.
            $dataset = $tokens_dataset[ $token ];
   
            // Rank order by rank each item in a dataset by using the key
            // which is the rank value for a datapoint.
            array_multisort(array_keys($dataset), SORT_DESC, $dataset);
            
            $token_values = array();
            foreach($dataset as $value) {
              // This is the value in the right order (by rank).
              $token_values[] = reset($value);
            }

            // Transpose this token back to the current citation.
            $replace = '{' . $token . '}';
            $citation = str_replace($replace, implode(', ', $token_values), $citation);
          }
        }
      }

      $module_path = drupal_get_path('module', 'tripal_citation');
      // Apply template to citation using style set in configuration.
      $conf_style = variable_get('tripal_citation_appearance');
      $conf_title  = variable_get('tripal_citation_title');
      $conf_control = variable_get('tripal_citation_control');

      $markup = theme('tripal_citation_citation', array(
        'style' => $conf_style,
        'citation' => $citation,
        'title' => $conf_title,
        'control' => $conf_control
      ));

      $element[0] = array(
        '#type' => 'markup',
        '#markup' => $markup,
        '#attached' => array(
          'css' => array($module_path . '/theme/style/style-citation.css'),
          'js'  => array()
        )
      );
    }

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
    $this_time = time();

    foreach($system_format as $format => $prop) {
      // Date format machine name - FORMAT TITLE - (Example applied to current time).
      $format_options[ $format ] = strtoupper($prop['title']) . ': (' . format_date($this_time, $format) .')';
    }

    $element['date'] = array(
      '#type' => 'select',
      '#title' => t('Date format to use when converting {CURRENT_DATE} date token'),
      '#options' => $format_options,
      '#default_value' => $settings['date'],
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

    $summary = t('<strong>Citation Template</strong> @template<br /><strong>Date Token</strong> @date<br /><strong>Prefer Publication</strong> @prefer_publication',
      array(
        '@template' => $settings['template'],
        '@date' => $settings['date'],
        '@prefer_publication' => $prefer_publication
    ));

    return $summary;
  }
}