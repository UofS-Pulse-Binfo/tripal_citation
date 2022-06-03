<?php
/**
 * @class
 * Purpose: Provide automated citations for data.
 *
 * Display: Rendered HTML
 * Configuration: Provide a content type level configuration for
 * citation template, use citation in a publication, time conversion format 
 * to use and an override to title set in main configuration.
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
    // System variables (with default key) defined in hook_bundle_instances_info().
    // @see TripalFields/tripal_citation.fields.inc.
    $display = $this->instance['display']['default'];
    $settings = $display['settings'];

    // Raw citation template. Contains all tokens as entered in the
    // citation template settings.
    $citation_template = trim($settings['template']);  

    // The final citation text generated.
    $citation = '';  
    
    // Flag that a completed citation coming off a publication (pub field)
    // has been found and used instead of citation template (if preferred).
    $citation_found = 0;

    // Inspect citaion settings variable and see if this content type has 
    // citation template set.
    if ($items[0]['value'] && !empty($citation_template)) {    
      // Prepare array of property types to inspect for tokens based
      // on the settings with regards to preference to publication.
      if ($settings['prefer_publication']) {
        // Prefer publication - inspect publication column,
        // publication property, project property and analysis property.
        // Except: when publication has citation stored in a field.        
        $property_set = array(          
          'tripal_citation:publication', 
          'tripal_citation_project', 
          'tripal_citation_analysis'
        );
        
        // Exception, here citation is completed.
        $pub_citation_set = 'tripal_citation:pub_column';

        if ($tokens_dataset[ $pub_citation_set ]) {
          $values = tripal_citation_prepare_values($tokens_dataset[ $pub_citation_set ]);
          $citation = tripal_citation_rank_token_values($values[ $pub_citation_set ]);
          
          if ($citation != '') {
            // Flag citation found and skip token transposition
            // method below.
            $citation_found = 1;
          }
        }

        // Otherwise, proceed to using citation template inspecting
        // the remaining property type in property set.
      }
      else {
        // Use citation template inspect project property, analysis property
        // and prop property.
        $property_set = array(
          'tripal_citation:project', 
          'tripal_citation:analysis',
          'tripal_citation:property'
        );
      }

      // Field has data and template has been set.
      // # TRANSPOSE ALL CVTERM TOKENS.
      // ALL CVTERM TOKENS.

      // Get all tokens from the template.
      $tokens_template = tripal_citation_get_tokens($citation_template);

      if (count($tokens_template) > 0 && $citation_found == 0) { 
        // All dataset for this entity keyed by property type token.
        $tokens_dataset = $items[0]['value']['hydra:member'];
        // Citation gets the template, save raw copy in the
        // event nothing was transposed and final process could
        // reference this copy for missing terms.
        $citation = $citation_template;

        // Each token in the template, find in every property type.
        foreach($tokens_template as $token) {
          $values = '';
          
          foreach($property_set as $set) {
            $values = tripal_citation_prepare_values($tokens_dataset[ $set ]);
            // Isolate the dataset.
            $dataset = $values[ $token ];
            // Token match a data from this property type.
            // Process (sort by rank) and render comma separate values
            // (if applicable) as the value for this token.
            
            // dataset: token = [rank => value], [rank => value] ...
            // Rank the values (if multiple values) and comma separate.
            // If only 2 values use and instead of comma symbol (ie: You and me).
            $token_value = tripal_citation_rank_token_values($dataset);
            
            // Transpose this token back to the current citation.
            if ($token_value != '') {
              $citation = tripal_citation_transpose_cvterm($citation, $token, $token_value);
              // No need to inspect other terms once token has been indentified
              // in a specific property type.
              break;
            }
          }
        }
      
        // If no transposition has occurred, revert the template to the
        // raw template citation and begin transposition of non-cvterm and missing token.
        $citation = ($citation == $citation_template) ? $citation_template : $citation;

        // # TRANSPOSE ALL NON CVTERM / MISSING TOKENS.
        // At this point we used the updated citation where all (attempted) cvterms 
        // have been resolved or back to the raw template.
        $citation = tripal_citation_transpose_non_cvterm($citation, 
          array('current_date' => trim($settings['date']))
        );

        // # Citation is almost in its final form. Inspect any missing
        // Token and transpose accordingly.
        $tokens_template = array();
        $tokens_template = tripal_citation_get_tokens($citation);
        if (count($tokens_template) > 0) {
          // Still has tokens, got to be a Missing token.
          foreach($tokens_template as $token) {
            $citation = tripal_citation_transpose_missing($citation, $token);
          }
        }      
      }
    }

    // # FINALLY, ANY OVERRIDE TITLE?
    // Apply template to citation using style set in configuration.
    $override = (trim($settings['title']) == '') ? array() : array('title' => $settings['title']);
    $markup = tripal_citation_theme_citation($citation, $override);
    
    $module_path = drupal_get_path('module', 'tripal_citation');
    
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
    
    // Date format to apply to data token.
    $element['date'] = array(
      '#type' => 'select',
      '#title' => t('Date format to use when converting {CURRENT_DATE} date token'),
      '#options' => $format_options,
      '#default_value' => $settings['date'],
    );

    // Title override.
    $element['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Citation alternate title text for this content type'),
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

    $summary = t('<strong>Citation Template</strong> @template<br /><strong>Date Token</strong> @date<br /><strong>Alternate Title</strong> @title<br /><strong>Prefer Publication</strong> @prefer_publication',
      array(
        '@template' => $settings['template'],
        '@date' => $settings['date'] . ' (' . tripal_citation_format_date($settings['date']) . ')',
        '@title' => $settings['title'],
        '@prefer_publication' => $prefer_publication
    ));

    return $summary;
  }
}