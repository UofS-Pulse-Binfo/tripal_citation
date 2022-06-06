<?php

/**
 * @file
 * Contains general API required to process and render citations.
 */

/**
 * Theme citation, apply style, implement override and create markup.
 *
 * @param $citation
 *   String, citation text required to apply citation theming to.
 * @param $override
 *   Array with keys: style, title and control, that when provided a value
 *   will override corresponding preconfigured citation settings for style, title and control.
 *   Default: empty array - no override.
 * 
 * @return markup
 *   Citation in HTML/markup form.
 *   Theme applied: tripal_citation_citation theme.
 * 
 * @see hook_theme() implementation in .module.
 */
function tripal_citation_theme_citation($citation, $override = array()) {
  // Fetch all global configuration on how to render citations.
  // Override when requested in override parameter.

  // Styling configuration.
  $conf_style = variable_get('tripal_citation_appearance');
  // Any override to style set in main configuration?
  $citation_styles = array('smart', 'banner', 'simple');
  $style = ($override['style'] && in_array($override['style'], $citation_styles)) 
    ? $override['style'] : $conf_style; 

  // Title text.
  $conf_title  = variable_get('tripal_citation_title');
  // Any override to title set in main configuration?
  $title = ($override['title'] && $override['title'] != '') 
    ? $override['title'] : $conf_title;

  // Copy button.
  $conf_control = variable_get('tripal_citation_control');
  // Any override to control set in main configuration?
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
 * Get entity record publication(s).
 * 
 * @param $record_id  
 *   Integer, id number, the primary key to match.
 * @param $source_table
 *   String, table name of the source relation where the record resides.
 *   For example, if the entity is project, source table is chado.project.
 *   This will be used to construct connecting table tablename + _pub.
 * 
 * @return array
 *   Publication(s) (pub_id) after establishing publication 
 *   record relationship - table name + _pub.
 */
function tripal_citation_get_publication($record_id, $source_table) {  
  // Table connection pub table.
  $table_pub = $source_table . '_pub';
  // Foreign key connecting to the source table.
  $table_pub_id = $source_table . '_id';
  $publication = 0;

  if (chado_table_exists($table_pub)) {
    // Table exists in schema.
    $publ_sql = "SELECT pub_id FROM {%s} WHERE %s = :record_id ORDER BY pub_id ASC";
    $publ_sql = sprintf($publ_sql, $table_pub, $table_pub_id); 
   
    // All related publications.
    $result = chado_query($publ_sql, array(':record_id' => $record_id));
    $publication = ($result->rowCount() > 0) ? $result->fetchCol() : 0;
  }

  return $publication;
}

/**
 * Get a field column of publication(s) table chado.pub.
 * 
 * @param $publication
 *   Array, publication id numbers to match.
 * @param $column
 *   String, column or field name in chado.pub table to return.
 *   Default: miniref.
 *  
 * @return array
 *   Keyed by publication id with value as the corresponding field
 *   specified by $column parameter.
 */
function tripal_citation_get_publication_column($publication, $column = 'miniref') {
  // This is formatted like other property using tripal_citation:pub_column as the
  // token and id and value as rank and value respectively.
  $publ_sql = "
    SELECT 'tripal_citation:pub_column' AS token, %s AS token_value, pub_id AS token_rank 
    FROM {pub} WHERE pub_id IN (:publication) ORDER BY pub_id ASC
  ";

  $publ_sql = sprintf($publ_sql, $column);
  $result = chado_query($publ_sql, array(':publication' => array_values($publication)));
  
  return ($result->rowCount() > 0) ? $result->fetchAll() : 0;
}

/**
 * Get entity record analysis.
 * 
 * @param $record_id  
 *   Integer, id number, the primary key to match.
 * @param $source_table
 *   String, table name of the source relation where the record resides.
 *   For example, if the entity is project, source table is chado.project.
 *   This will be used to construct connecting table tablename + _feature.
 * 
 * @return array
 *   Analysis (feature id) after establishing analysis
 *   record relationship - table name + _feature - analysis.
 */
function tripal_citation_get_analysis($record, $source_table) {
  // From chado schema diagram.
  // The following tables support project connection.
  // stock_feature and project_feature.
  $table_features = array(
    'stock_feature',  // Table connection: stock.
    'project_feature' // Table connection: project.
  );

  // Table connection feature table.
  $table_feature = $source_table . '_feature';
  // Foreign key connecting to the source table.
  $table_feature_id = $source_table . '_id';
  $analysis = 0;

  // Source table - Source Table_feature - analysisfeature - analysis.
  if (in_array($table_feature, $table_features)) {
    // Table has connection.
    if (chado_table_exists($table_feature) && chado_table_exists('analysisfeature')) {
      // Table exists.
      $feat_sql = "SELECT feature_id FROM {%s} WHERE %s = :record_id ORDER BY feature_id ASC";
      $feat_sql = sprintf($feat_sql, $table_feature, $table_feature_id); 
      
      // All related feature.
      $result = chado_query($feat_sql, array(':record_id' => $record_id));
      if ($result->rowCount() > 0) {
        $feature = $result->fetchCol();

        $a = chado_query(
          "SELECT analysis_id FROM {analysisfeature} WHERE feature_id IN (:feature_id)",
          array(':feature_id' => $feature)
        );
         
        // Analysis ID.
        $analysis = ($a->rowCount() > 0) ? $a->fetchCol() : 0;
      }
    }
  }

  return $analysis;
}

/**
 * Get entity record project(s).
 *
 * @param $record_id  
 *   Integer, id number, the primary key to match.
 * @param $source_table
 *   String, table name of the source relation where the record resides.
 *   For example, if the entity is project, source table is chado.project.
 *   This will be used to construct connecting table project_ + tablename.
 * 
 * @return array
 *   Project (project id) after establishing project 
 *   record relationship - project + _table name.
 */
function tripal_citation_get_project($record_id, $source_table) {
  // From chado schema diagram.
  // The following tables support project connection.
  // project_analysis, project_contact, project_dbxref, project_feature,
  // project_pub, project_relationship and project_stock.
  $table_projects = array(
    'project_pub', // Table connection: pub.
    'project_stock', // Table connection: stock.
    'project_dbxref', // Table connection: dbxref.
    'project_contact', // Table connection: contact.
    'project_feature',  // Table connection: feature. 
    'project_analysis',   // Table connection: analysis. 
    'project_relationship', // Table connection: relationship.
  );
  
  // Table connection project table.
  $table_project = 'project_' . $source_table;
  // Foreign key connecting to the source table.
  $table_project_id = $source_table . '_id';
  $project = 0;

  if (in_array($table_project, $table_projects)) {
    // Table has connection.
    if (chado_table_exists($table_project)) {
      // Table exists.
      $proj_sql = "SELECT project_id FROM {%s} WHERE %s = :record_id ORDER BY project_id ASC";
      $proj_sql = sprintf($proj_sql, $table_project, $table_project_id); 
      
      // All related projects.
      $result = chado_query($proj_sql, array(':record_id' => $record_id));
      $project = ($result->rowCount() > 0) ? $result->fetchCol() : 0;
    }
  }

  return $project;
}

/**
 * Get properties from table prop table.
 * 
 * @param $record_id  
 *   Array, id number(s) to match.
 * @param $source_table
 *   String, table name of the source relation where the record resides.
 *   For example, if the entity is project, source table is chado.project.
 *   This will be used to construct connecting table tablename + prop.
 *   
 * @return object
 *   Properties rank and value from prop table where term ids (type_id) 
 *   are resolved to indicate the human-readable cvterm name.
 */
function tripal_citation_get_property($record_id, $source_table) {
  // Assumptions: that every table in chado has prop table where the table name is
  // table+prop (ie. project - projectprop, stock - stockprop) and in each table
  // primary key (record id) name would be the table name + _id (ie. project - project_id).
  $record_id = (array) $record_id;

  // This is formatted like other property using tripal_citation:pub_column as the
  // token and id and value as rank and value respectively.
  $prop_sql = "
    SELECT CONCAT(t3.name, ':', t2.name) AS token, t1.value AS token_value, t1.rank AS token_rank  
    FROM {%s} AS t1 LEFT JOIN {cvterm} AS t2 ON t1.type_id = t2.cvterm_id LEFT JOIN {cv} AS t3 ON t2.cv_id = t3.cv_id
    WHERE t1.%s IN (:record_id) ORDER BY t1.type_id ASC
  ";

  $table_prop = $source_table . 'prop';
  $table_id   = $source_table . '_id';
  $properties = 0;

  if (chado_table_exists($table_prop)) {
    // Fetch all properties of record id since there is no access to the citation template 
    // tokens and all, for this content type.
    $sql = sprintf($prop_sql, $table_prop, $table_id);
    $result = chado_query($sql, array(':record_id' => $record_id));

    $properties = ($result->rowCount() > 0) ? $result->fetchAll() : 0;
  }

  return $properties;
}

/**
 * Format datatime.
 * 
 * @param format
 *   String, drupal datetime format provided by Drupal 7 system_get_date_types().
 *   Default: short - YYYY Mon Day - HH:MMAM/PM.
 * 
 * @return string
 *   Current date and time formatted to format requested.
 */
function tripal_citation_format_date($format) {
  $current = time();

  return format_date($current, $format);
}

/**
 * Transpose non-cvterm tokens.
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
 * 
 * @dependencies
 *   tripal_citation_format_date().
 */
function tripal_citation_transpose_non_cvterm($template, $options) {
  // Resolve non-token cvterms.
  // {CURRENT_DATE} - default to short.
  // $options['current_date'] to apply formatting.
  $current_date = tripal_citation_format_date($options['current_date']);
  // {CURRENT_URL}
  // $options['current_url'] to apply formatting.
  $url = url(current_path(), array('absolute' => TRUE));
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
 *   String, resolved value to replace a token with.
 * 
 * @return string
 *   Same citation template with all cvterm token resolved.
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
  // Restore token to expected format as entered in template field
  // and replace accordingly.
  $token = '{' . $token . '}';
  $result = str_replace($token, $token_value, $template);

  return $result;
}

/**
 * Rank order token value based on rank and create a single
 * comma separated values. If there is just a single value return as string,
 * If there are 2 values, use the word and instead of comma.
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
  if (count($token_values) > 1) {
    foreach($token_values as $i => $value) {
      $ranks[] = array_keys($value)[0];
    }

    array_multisort($ranks, SORT_ASC, SORT_REGULAR, $token_values);
  
    $rank_values = array();
    foreach($token_values as $value) {
      // This is the value in the right order (by rank).
      $rank_values[] = trim(reset($value));
    }
   
    $result = implode(', ', $rank_values);
    return (count($rank_values) > 1) ? substr_replace($result, ' and', strrpos($result, ','), 1) : $result;
  }
  else {
    return reset($token_values[0]);
  }
}

/**
 * Get all tokens from citation template.
 *
 * @param $template
 *   String, citation template.
 * 
 * @return array
 *   All tokens detected in the template maintaining the name format - {CV NAME:CVTERM NAME}.
 */
function tripal_citation_get_tokens($template) {
  // Get all tokens - {token}.
  preg_match_all('/{(.*?)}/', $template, $match);

  // Tokens from the template settings.
  return $match[1];
}

/**
 * Prepare properties array into cv:name = rank => value format.
 * 
 * @param $property
 *   Array of property object.
 * 
 * @return array
 *   Array of property values in the described format.
 */
function tripal_citation_prepare_values($property) {
  $new_format = array();

  if (count($property) > 0) {
    foreach($property as $prop) {
      $new_format[ $prop->token ][] = array(
        $prop->token_rank => $prop->token_value
      );
    }
  }

  return $new_format;
}