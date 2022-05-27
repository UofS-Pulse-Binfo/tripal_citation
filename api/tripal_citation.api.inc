<?php
/**
 * @file
 * Contains general API required to process and render citations.
 */

/**
 * Theme citation, create markup.
 *
 * @param $citation
 *   String, citation text required to apply citation theme to.
 * @param $override
 *   Array with keys: style, title and control, that when provided a value
 *   will override corresponding preconfigured citation settings for style, title and control.
 *   Default: empty array.
 * 
 * @return markup
 *   Citation in HTML, CSS and JS form.
 *   Theme applied: tripal_citation_citation theme @see hook_theme() in .module.
 */
function tripal_citation_theme_citation($citation, $override = array()) {
  // Fetch all global configuration on how to render citations.
  // Override when requested in override parameter.

  // Styling configuration.
  $conf_style = variable_get('tripal_citation_appearance');
  $style = ($override['style'] && in_array($override['style'], array('smart', 'banner', 'simple'))) 
    ? $override['style'] : $conf_style; 

  // Title text.
  $conf_title  = variable_get('tripal_citation_title');
  $title = ($override['title'] && $override['title'] != '') 
    ? $override['title'] : $conf_title;

  // Copy button.
  $conf_control = variable_get('tripal_citation_control');
  $control = ($override['control'] && in_array($override['control'], array('none', 'copy'))) 
    ? $override['control'] : $conf_control;
  
  // Prepare markup.
  $markup = theme('tripal_citation_citation', array(
    'citation' => $citation,
    'style'  => $style,
    'title' => $title,
    'control' => $control
  ));

  return $markup;
}

/**
 * Get entity record properties.
 * 
 * @param $record_id  
 *   Integer, id number, the primary key to match.
 * @param $source_table
 *   String, table name of the source relation where the record resides.
 *   For example, if the entity is project, source table is chado.project.
 * @param $type
 *   String: prop - in table prop, project - in project, analysis - in featureanalysis.
 *   Default: prop.
 * 
 * @return object
 *   All properties contained in the prop table of the source table -
 *   type_id expanded to CV NAME:CVTERM NAME, rank and value.
 */
function tripal_citation_get_properties($record_id, $source_table, $property_type = 'prop') {
  $prop_sql = "
    SELECT CONCAT(t3.name, ':', t2.name) AS token, t1.value AS token_value, t1.rank AS token_rank  
    FROM {%s} AS t1 LEFT JOIN {cvterm} AS t2 ON t1.type_id = t2.cvterm_id LEFT JOIN {cv} AS t3 ON t2.cv_id = t3.cv_id
    WHERE t1.%s IN (:record_id) ORDER BY t1.type_id ASC
  ";

  if ($type == 'project') {
    // Table name as project_[table_name].
    $table_project = 'project_' . $source_table;
    $table_project_id = $source_table . '_id';
    
    // Get all projects for a record id.
    $sql = "SELECT project_id FROM {%s} WHERE %s = :record_id ORDER BY project_id ASC";
    $sql = sprintf($sql, $table_project, $table_project_id);

    $projects = chado_query($sql, array(':record_id' => $record_id));
    if ($projects->rowCount() > 0) {
      $project_ids = $project->fetchCol();
      // Get all properties for projects in prop table.
      $sql = sprintf($prop_sql, 'projectprop', 'project_id');
      $properties = chado_query($sql, array(':record_id' => $ids));
    }

  }
  elseif ($type == 'analysis') {
    $table_analysis = 'analysisfeature';
    // @TODO: resolved multiple analysis returned which one to select.
    //        check if table exists
  }
  else {
    // Assumptions: that every table in chado has prop table where the table name is
    // table+prop (ie. project - projectprop, stock - stockprop) and in each table
    // primary key (record id) name would be the table name + _id (ie. project - project_id).
    $table_prop = $source_table . 'prop';
    $table_id   = $source_table . '_id';
    
    // Fetch all properties of record id since there is no access to the citation template tokens and all, 
    // for this content type.
    $sql = sprintf($prop_sql, $table_prop, $table_id);
    $properties = chado_query($sql, array(':record_id' => array($record_id)));
  }
  
  return ($properties->rowCount() > 0) ? $properties : 0;
}

/**
 * Format datatime.
 * 
 * @param format
 *   String, drupal datetime format provided by Drupal 7 system_get_date_types().
 *
 * @return string
 *   Current date and time formatted to format requested.
 */
function tripal_citation_format_date($format) {
  $current = time();

  return format_date($current, $format);
}

/**
 * Transpose not-cvterm tokens.
 * 
 * @param $template
 *   String, citation template containing non-cvterms
 *   {CURRENT_DATE} and/or {CURRENT_URL}.
 * @param $options
 *   Array, additional options/parameter to apply when transposing.
 *   Keys: current_date = {CURRENT_DATE}, current_url = {CURRENT_URL}
 * 
 * @return string
 *   Same citation template with all non-cvterm tokens
 *   transposed with appropriate values.
 */
function tripal_citation_transpose_non_cvterm($template, $options) {
  // Resolve non-token cvterms.
  // {CURRENT_DATE}
  $current_date = tripal_citation_format_date($options['current_date']);
  // {CURRENT_URL}
  $url  = url(current_path(), array('absolute' => TRUE));
  $current_url = l($url, $url);
  
  // More here: Resove token here and add to array below:

  // Expected non-cvterms.
  $non_cvterms = array(
    '{CURRENT_DATE}' => $current_date, 
    '{CURRENT_URL}' => $current_url,
    // Add more here.
  );
  
  // Replace all not-cvterms token with resolved value of each token.
  $result = str_replace(array_keys($non_cvterms), array_values($non_cvterms), $template);

  return $result;
}

/**
 * Transpose cvterm tokens.
 * 
 * @param $template
 *   String, citation template as entered in citation template field settings.
 * @param $token
 *   String, token to replace.
 * @param $token_value
 *   String, resolved value to replace a token.
 * 
 * @return string
 *   Same citation template with a cvterm token resolved.
 */
function tripal_citation_transpose_cvterm($template, $token, $token_value) {
  // Restore token to expected format as entered in template field.
  $token = '{' . $token . '}';
  // Replace token with resolved cvterm value.
  $result = str_replace($token, $token_value, $template);

  return $result;
}

/**
 * Transpose missing tokens.
 * 
 * @param $template
 *   String, citation template as entered in citation template field settings.
 * @param $token
 *   String, in the format {CV NAME:CVTERM NAME}
 * 
 * @return string
 *   Same citation template with all missing tokens resolved.
 */
function tripal_citation_transpose_missing($template, $token) {
  list($cv, $cvterm_name) = explode(':', $token);
  $term = tripal_get_cvterm(array('name' => $cvterm_name, 'cv_id' => array('name' => $cv)));

  // Found but no data else just missing token.
  $token_value = ($term) ? '[MISSING INFORMATION:'. $cvterm_name .']' : '[MISSING INFORMATION]';
  // Restore token to expected format as entered in template field.
  $token = '{' . $token . '}';
  $result = str_replace($token, $token_value, $template);

  return $result;
}


/**
 * Rank order token value based on rank and create a single
 * comma separated values.
 * 
 * @param $token_values
 *   Array, token value(s) keyed by rank number (rank field in the prop table).
 * 
 * @return string
 *   A comma separted values (csv) in the order rank value indicates.
 */
function tripal_citation_rank_token_values($token_values) {
  // Rank order (ASCENDING ORDER) by rank each item in a dataset by using the key
  // which is the rank value for a datapoint.
  $ranks = array_keys($token_values);

  array_multisort($ranks, SORT_ASC, $token_values);
  
  $rank_values = array();
  foreach($token_values as $value) {
    // This is the value in the right order (by rank).
    $rank_values[] = reset($value);
  }
   
  $result = implode(', ', $rank_values);
  return (count($rank_values) > 1) ? substr_replace($result, ' and', strrpos($result, ','), 1) : $result;
}

/**
 * Get all tokens from citation template.
 *
 * @param $template
 *   String, citation template.
 * 
 * @return array
 *   All tokens detected in the template maintaining the name - CV NAME:CVTERM NAME.
 */
function tripal_citation_get_tokens($template) {
  // Get all tokens - {token}.
  preg_match_all('/{(.*?)}/', $template, $match);
  // Tokens from the template settings.
  return $match[1];
}
