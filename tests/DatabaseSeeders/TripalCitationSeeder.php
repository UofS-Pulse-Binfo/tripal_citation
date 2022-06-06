<?php

namespace Tests\DatabaseSeeders;

use StatonLab\TripalTestSuite\Database\Seeder;

class TripalCitationSeeder extends Seeder
{
  // Stock profile used to use in creating sample citation.
  public $my_stock = array(
    'name' => 'MY STOCK',
    'description' => 'Test stock/line',
    'citations' => array(
      'publication' => 'Lorem Ipsum, Dolor Sit Amet, Consectetur Adipiscing Elit et al. Copyright 2022-2023', 
      'authors' => 'Lorem Ipsum, Dolor Sit Amet, Consectetur Adipiscing Elit et al', 
      'writers' => array(
        array('name' => 'Mr. Luke S.', 'rank' => 5),
        array('name' => 'Mr. Sophia G.', 'rank' => 0),
        array('name' => 'Mr. Benjamin F.', 'rank' => 27),
      ),
      'lead writer' => 'Mr. Jacob Z.',
    )
  );

  public function up() {
    $data = array();

    // Create user. This user is authenticated user
    // but does not have any experiments assigned.
    $new_user = array(
      'name' => 'Tripal Citation',
      'pass' => 'secret',
      'mail' => 'tripal_citation@mytripalsite.com',
      'status' => 1,
      'init' => 'Email',
      'roles' => array(
          DRUPAL_AUTHENTICATED_RID => 'authenticated user',
      ),
    );

    // The first parameter is sent blank so a new user is created.
    $user = user_save(new \stdClass(), $new_user);
    $data['user'] = $user->uid;
    
    // TERMS: install terms used to store citation information
    // in prop table. Id will be referenced in type_id of prop table
    // during table insert.
    $terms = array_keys($this->my_stock['citations']);
    $id = array();
    foreach($terms as $term) {
      $i = tripal_insert_cvterm(array(
        'name' => $term,
        'definition' => 'define',
        'accession' => $term,
        'cv_name' => 'local',
        'db_name' => 'local',
      ));

      $id[ $term ] = $i->cvterm_id;
    }

    // STOCK:
    // CREATE A STOCK RECORD.
    // Create a stock record in chado.stock the create following connections
    // * A publication in chado.pub linked via chado.stock_pub table
    // * A project in chado.project linked via chado.project_stock table
    // * A direct property in chado.stockprop table. 
    // * A property in feature-analysis.
    $organism = chado_insert_record('organism', array(
      'abbreviation' => 'T. databasica',
      'genus' => 'Tripalus',
      'species' => 'databasica',
      'common_name' => 'Cultivated Tripal'
    ));

    $accession = chado_query("SELECT cvterm_id FROM {cvterm} WHERE name = 'accession' LIMIT 1")
      ->fetchField();

    $my_stock = chado_insert_record('stock', array(
      'organism_id' => $organism['organism_id'],
      'name' => $this->my_stock['name'],
      'uniquename' => 'uname-' . uniqid(),
      'type_id' => $accession,
      'dbxref_id' => null,
      'description' => $this->my_stock['description'],
    ));

    $my_stock_pub = chado_insert_record('pub', array(
      'title' => 'MY STOCK PUBLICATION',
      'volumetitle' => 'Unabridged Volume',
      'volume' => 'Vol. X',
      'series_name' => '10th Series',
      'issue' => 'Pandemic Issue',
      'pyear' => '2022',
      'pages' => '12345',
      'uniquename' => $this->my_stock['citations']['publication'],
      'miniref' => 'my_stock_publication', 
      'type_id' => $id['publication'],
      'is_obsolete' => FALSE,
      'publisher' => 'Self Publishing Inc.',
      'pubplace' => 'Canada'
    ));

    chado_insert_record('stock_pub', array(
      'stock_id' => $my_stock['stock_id'],
      'pub_id' => $my_stock_pub['pub_id'],
    ));

    $my_stock_project = chado_insert_record('project', array(
      'name' => 'MY STOCK PROJECT',
      'description' => 'my stock project'
    ));

    chado_insert_record('projectprop', array(
      'type_id' => $id['authors'],
      'project_id' => $my_stock_project['project_id'],
      'value' => $this->my_stock['citations']['authors'],
      'rank' => 0,
    ));

    chado_query("DELETE FROM {project_stock} WHERE stock_id > 0");
    chado_insert_record('project_stock', array(
      'stock_id' => $my_stock['stock_id'],
      'project_id' => $my_stock_project['project_id'],
    ));

    // To test ranking of information.
    foreach($this->my_stock['citations']['writers'] as $writer) {
      chado_insert_record('stockprop', array(
        'stock_id' => $my_stock['stock_id'],
        'type_id' => $id['writers'],
        'value' => $writer['name'],
        'rank' => $writer['rank']
      ));
    }
    
    // To test single value returned - no ranking.
    chado_insert_record('stockprop', array(
      'stock_id' => $my_stock['stock_id'],
      'type_id' => $id['lead writer'],
      'value' => $this->my_stock['citations']['lead writer'],
      'rank' => 0
    ));

    /* ERROR: currval of sequence "stock_feature_stock_feature_id_seq" is not yet defined in this session
    // Sample feature id.
    $feature = chado_query("SELECT feature_id FROM {feature} LIMIT 1")
      ->fetchField();
    
    chado_insert_record('stock_feature', array(
      'feature_id' => $feature,
      'stock_id' => $my_stock['stock_id'],
      'type_id' => $accession,
      'rank' => 5
    )); */

      
    return $data;
  }
}