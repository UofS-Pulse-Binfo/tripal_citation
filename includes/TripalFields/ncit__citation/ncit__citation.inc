<?php
/**
 * @class
 * Purpose: Provide automated citations for data.
 *
 * Data: No data.
 * Assumptions:
 */
class ncit__citation extends TripalField {
  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendant class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------
  // The default label for this field.
  public static $default_label = 'Tripal Citation';
  // The default description for this field.
  public static $default_description = 'Tripal Field specific to providing automated citations for data based on properties + publications';
  // The default widget for this field.
  public static $default_widget = 'ncit__citation_widget';
  // The default formatter for this field.
  public static $default_formatter = 'ncit__citation_formatter';
  // The module that manages this field.
  public static $module = 'tripal_citation';

  // A list of global settings. These can be accessed within the
  // globalSettingsForm.  When the globalSettingsForm is submitted then
  // Drupal will automatically change these settings for all fields.
  // Once instances exist for a field type then these settings cannot be
  // changed.
  public static $default_settings = array(
    'storage' => 'tripal_no_storage',
    // It is expected that all fields set a 'value' in the load() function.
    // In many cases, the value may be an associative array of key/value pairs.
    // In order for Tripal to provide context for all data, the keys should
    // be a controlled vocabulary term (e.g. rdfs:type). Keys in the load()
    // function that are supported by the query() function should be
    // listed here.
    'browseable_keys' => array(),
  );

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instance.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings = array(
    // The short name for the vocabulary (e.g. schema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'ncit',
    // The name of the term.
    'term_name' => 'citation',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'citation',
    // Set to TRUE if the site admin is not allowed to change the term
    // type, otherwise the admin can change the term mapped to a field.
    'term_fixed' => FALSE,
    // Indicates if this field should be automatically attached to display
    // or web services or if this field should be loaded separately. This
    // is convenient for speed.  Fields that are slow should for loading
    // should have auto_attach set to FALSE so tha their values can be
    // attached asynchronously.
    'auto_attach' => FALSE,
    // The table where the options for this specific field are stored.
    // This can be one of trpfancy_browse_options or trpfancy_browse_options_per_entity
    // based on admin configuration. Default: trpfancy_browse_options.
    'option_storage' => '',
    // A list of browser types this field intends to provide.
    'browser_types' => '',
  );

  // A boolean specifying that users should not be allowed to create
  // fields and instances of this field type through the UI. Such
  // fields can only be created programmatically with field_create_field()
  // and field_create_instance().
  public static $no_ui = FALSE;
  // A boolean specifying that the field will not contain any data. This
  // should exclude the field from web services or downloads.  An example
  // could be a quick browse field that appears on the page that redirects
  // the user but otherwise provides no data.
  public static $no_data = TRUE;

 /**
   * Loads the field values from the underlying data store.
   *
   * @param $entity
   *
   * @return
   *   An array of the following format:
   *     $entity->{$field_name}['und'][0]['value'] = $value;
   *   where:
   *     - $entity is the entity object to which this field is attached.
   *     - $field_name is the name of this field
   *     - 'und' is the language code (in this case 'und' == undefined)
   *     - 0 is the cardinality.  Increment by 1 when more than one item is
   *       available.
   *     - 'value' is the key indicating the value of this field. It should
   *       always be set.  The value of the 'value' key will be the contents
   *       used for web services and for downloadable content.  The value
   *       should be of the follow format types: 1) A single value (text,
   *       numeric, etc.) 2) An array of key value pair. 3) If multiple entries
   *       then cardinality should incremented and format types 1 and 2 should
   *       be used for each item.
   *   The array may contain as many other keys at the same level as 'value'
   *   but those keys are for internal field use and are not considered the
   *   value of the field.
   */
  public function load($entity) {
    $field_name = $this->instance['field_name'];

    $table_name = $entity->chado_record->tablename;
    $table_prop = $table_name . 'prop';
    $table_id   = $table_name . '_id';
    $record_id  = $entity->chado_record->{$table_id};
    
    $template = "{project_property:Project Description} LOREM IPSUM DOLOR SIT AMET {GCP germplasm ontology:germplasm}. 
      Data was accessed from KnowPulse ({CURRENT_URL}) on {CURRENT_DATE}. All rights retained by the original authors.";


    // Get all tokens and locate values in the prop table.
    preg_match_all('/{(.*?)}/', $template, $match);
    $tokens = $match[1];

    // Store all cvterms as typeids used to match type_id column in a prop table.
    $type_ids = array();
    foreach($tokens as $token) {
      list($cv, $cvterm) = explode(':', $token);

      if ($cv && $cvterm) {
        $term = tripal_get_cvterm(
          array('name' => $cvterm, 'cv_id' => array('name' => $cv))
        );
        
        if ($term) {
          $type_ids[ $term->cvterm_id ] = array($cv => $cvterm);
        }
      }      
    }
    
    if ($type_ids) {
      // Query all items that makeup the citation.
      $sql = "SELECT type_id, value FROM {%s} WHERE type_id IN (:type_ids) AND %s = :table_id ORDER BY type_id ASC";
      $query = chado_query(sprintf($sql, $table_prop, $table_id), 
        array(':type_ids' => array_keys($type_ids), ':table_id' => $record_id)
      );

      if ($query->rowCount() > 0) {
        $data = $query->fetchAllKeyed(0, 1);
      }
    }

    // Set some defaults for the empty record.
    $entity->{$field_name}['und'][0] = array(
      'value' => array(),
    );


    $entity->{$field_name}['und'][0] = array(
      'ncit:citation' => $data,
      'hydra:member'  => $type_ids
    );
  }

  /**
   * @see TripalField::elementInfo()
   *
   **/
  public function elementInfo() {
  }

  /**
   * @see TripalField::query()
   *
   **/
  public function query($query, $condition) {
  }

  /**
  * @see TripalField::queryOrder()
  *
  **/
  public function queryOrder($query, $order) {
  }
}