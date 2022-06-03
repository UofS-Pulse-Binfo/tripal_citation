<?php

namespace Tests\DatabaseSeeders;

use StatonLab\TripalTestSuite\Database\Seeder;

class TripalCitationSeeder extends Seeder
{
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
    
    // TERMS:
    $terms = array('citation', 'publication', 'authors');
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
      'name' => 'MY STOCK',
      'uniquename' => 'uname-' . uniqid(),
      'type_id' => $accession,
      'dbxref_id' => null,
      'description' => 'description my stock',
    ));

    $my_stock_pub = chado_insert_record('pub', array(
      'title' => 'MY STOCK PUBLICATION',
      'volumetitle' => 'Unabridged Volume',
      'volume' => 'Vol. X',
      'series_name' => '10th Series',
      'issue' => 'Pandemic Issue',
      'pyear' => '2022',
      'pages' => '12345',
      'uniquename' => 'Lorem Ipsum, Dolor Sit Amet, Consectetur Adipiscing Elit et al. Copyright 2022-2023',
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
      'value' => 'Lorem Ipsum, Dolor Sit Amet, Consectetur Adipiscing Elit et al',
      'rank' => 0,
    ));

    chado_insert_record('stock_project', array(
      'stock_id' => $my_stock['stock_id'],
      'project_id' => $my_stock_project['project_id'],
    ));

    chado_insert_record('stockprop', array(
      'stock_id' => $my_stock['stock_id'],
      'type_id' => $id['authors'],
      'value' => 'Mr. Benjamin Tan et al.',
      'rank' => 0
    ));

    /*
    // PROJECT:
    // CREATE A PROJECT RECORD
    // Create a project record in chado.project and create the following connections
    // * A direct property in chado.projectprop table.
    $my_project = chado_insert_record('project', array(
      'name' => 'MY PROJECT',
      'description' => 'my project'
    ));

    chado_insert_record('projectprop', array(
      'type_id' => $id['authors'],
      'project_id' => $my_stock_project['project_id'],
      'value' => 'Lorem Ipsum, Dolor Sit Amet, Consectetur Adipiscing Elit et al',
      'rank' => 0,
    ));

    chado_insert_record('projectprop', array(
      'type_id' => $id['authors'],
      'project_id' => $my_stock_project['project_id'],
      'value' => 'Information and Computational Sciences at My Project',
      'rank' => 5,
    ));
  
    chado_insert_record('projectprop', array(
      'type_id' => $id['citation'],
      'project_id' => $my_stock_project['project_id'],
      'value' => 'Lorem Ipsum, Dolor Sit Amet, Consectetur Adipiscing Elit et al. Information and Computational Sciences at My Project',
      'rank' => 0,
    ));
    */
      
    return $data;
  }
}