<?php
namespace Tests;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use Tests\DatabaseSeeders\TripalCitationSeeder;

class TripalCitationTest extends TripalTestCase {
  use DBTransaction;
  
  /**
   * Test Tripal Citation module.
   */
  public function testCitation() {  
    // INSTALL:
    
    // Module.
    $is_enabled = module_exists('tripal_citation');
    $this->assertTrue($is_enabled, 'Module enabled');
    
    // Terms and configuration values:
    // Terms used:
    // publication - refers to publications.
    // pub_column  - refers to a value of a field in chado.pub.
    // project  - refers to projects.
    // analysis - refers to analysis.
    // property - referes to property in a chado.tableprop table.
    $terms = array('publication', 'pub_column', 'project', 'analysis', 'property');
    $cv = 'tripal_citation';

    foreach($terms as $term) {
      $cvterm = tripal_get_cvterm(array('name' => $term, 'cv_id' => array('name' => $cv)));
      // Required term created during install, found.
      $this->assertEquals($cvterm->name, $term, 'The required term ' . $term . ' is not found.');
    }

    // Configurations:
    // Style - default to smart.
    $style = variable_get('tripal_citation_appearance');
    $this->assertEquals($style, 'smart', 
      'The configuration for style (default to smart) is default to incorrect value: ' . $style );

    // Title - default to blank/empty.
    $title = variable_get('tripal_citation_title');
    $this->assertEquals($title, '', 
      'The configuration for style (default to empty) is default to incorrect value: ' . $title );

    // Copy paste button - default to copy.
    $copy  = variable_get('tripal_citation_control');
    $this->assertEquals($copy, 'copy', 
      'The configuration for style (default to copy) is default to incorrect value: ' . $copy );
    
    // Test connections.
    $seeder = new TripalCitationSeeder();
    $data = $seeder->up();

    if ($data) {
      // Test if my stock is in.
      $stock = 'MY STOCK';
      $my_stock = chado_query("SELECT * FROM {stock} WHERE name = :stock LIMIT 1", array(':stock' => $stock))
        ->fetchObject();
      
      $this->assertEquals($my_stock->name, $stock, 'Stock MY STOCK is not found.');
    
      // TEST: stock citation:
      // Stock has property authors in stockprop.
      // Stock has project connection (stock_project) and project has authors property in projectprop.
      // Stock has publication connection (stock_pub) and pub field uniquename has citation.
      
      // 1. Use citation template to resolve tokens directly from stock property.

      // 2. Use citation template to resolve tokens directly from stock project - project prop

      // 3. Use citation template but prefer publicaiton citation.
    
    }
  }
}
