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
    $content = $items[0]['ncit:citation'];

    $element[0] = array(
      '#type' => 'markup',
      '#markup' => $content,
    );

    return $element;
  }


  /**
   * @see TripalFieldFormatter::settingsForm()
   *
   **/
  public function settingsForm($view_mode, $form, &$form_state) {
    $element = parent::settingsForm($view_mode, $form, $form_state);
    
    $display = $this->instance['display'][$view_mode];
    $settings = $display['settings'];

    // Citation template - text and tokens.
    $element['template'] = array(
      '#type' => 'textarea',
      '#title' => t('Citation Template'),
      '#default_value' => $settings['template'],
    );
    
    // Date token default to CURRENT_DATE.
    $element['date'] = array(
      '#type' => 'textfield',
      '#title' => t('Date Token'),
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
    $display = $this->instance['display'][$view_mode];
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