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
        // Test if property is more than one, therefore apply csv formatting
        // but the last item should use and not comma.
        array('name' => 'Mr. Luke S.', 'rank' => 5),
        array('name' => 'Mr. Sophia G.', 'rank' => 0),
        array('name' => 'Mr. Benjamin F.', 'rank' => 27),
      ),
      'lead writer' => 'Mr. Jacob Z.',
    )
  );

  public $my_analysisfeature = array(
    'analysis' => array(
      'name' => 'MY ANALYSIS',
    ),
    'feature' => array(
      'name' => 'MY FEATURE',
    ),
    'citations' => array(
      'analysis author' => array(
        // Test if property is single value, therefore no csv formatting.
        array('name' => 'Dr. Helen S.', 'rank' => 3)
      ),
      // Term used to create a feature (feature.type_id).
      'feature' => ''
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

    unset($terms);
    $terms = array_keys($this->my_analysisfeature['citations']);
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
    // Create a stock record in chado.stock then create following connections
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
    
    // ANALYSIS
    // Create feature record in chado.feature then create following connections
    // * An entry in chado.analysis
    // * An entry in chado.analysisfeature linked via foreign key feature_id and analysis_id (created above)
    // * An entry in analysisprop where the analysis_id is the id created in item 2.
    //   and the value is analysis author in the term array above.
    $my_analysis = chado_insert_record('analysis', array(
      'name' => $this->my_analysisfeature['analysis']['name'],
      'description' => 'this is a test analysis connected to MY FEATURE',
      'program' => 'Program Search',
      'programversion' => 'v1.3',
      'algorithm' => 'Fibonacci Search',
      'sourcename' => 'Source Entity',
      'sourceversion' => 'v1.1',
      'sourceuri' => 'programsearch-test.ca',
      'timeexecuted' => '2014-03-03 11:50:04.747584'
    ));

    $my_feature = chado_insert_record('feature', array(
      'dbxref_id' => 1,
      'organism_id' => $organism['organism_id'],
      'name' => $this->my_analysisfeature['feature']['name'],
      'uniquename' => 'MY FEATURE UNIQUENAME',
      'residues' => 'AAAAACCCCCGGGGG',
      'seqlen' => 100,
      'md5checksum' => '82839238hf',
      'type_id' => $id['feature'],
      'is_analysis' => 1,
      'is_obsolete' => 0,
      'timeaccessioned' => '2011-11-10 09:50:17.927864',
      'timelastmodified' => '2011-11-10 09:50:17.927864',
    ));

    chado_insert_record('analysisfeature', array(
      'feature_id' => $my_feature['feature_id'],
      'analysis_id' => $my_analysis['analysis_id'],
    ));

    chado_insert_record('analysisprop', array(
      'analysis_id' => $my_analysis['analysis_id'],
      'type_id' => $id['analysis author'],
      'value' => $this->my_analysisfeature['citations']['analysis author'][0]['name'],
      'rank' => $this->my_analysisfeature['citations']['analysis author'][0]['rank']
    ));
    
    return $data;
  }
}