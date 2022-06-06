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
    
      
      // Content type will return record id and chado table
      // record is stored in.
      $record_id = $my_stock->stock_id;
      $table_name = 'stock';

      // TEST: stock citation:
      $properties = array();


      /// GET PROPERTIES:

      // # PUBLICATIONS:
      // Get entity publications connection.
      $publication = tripal_citation_get_publication($record_id, $table_name);
      // In chado.pub and properties in pubprop.
      $properties['tripal_citation:publication'] = ($publication) 
        ? tripal_citation_get_property($publication, 'pub') : 0;
      // In cahdo.pub, uniquename (replace param #2 to field where citation resides) column.
      // If preferred, and has a value, this will become the citation.
      $properties['tripal_citation:pub_column'] = ($publication) 
        ? tripal_citation_get_publication_column($publication, 'uniquename') : 0;
  
        
      // # PROJECTS:
      // Get entity project connection.
      $project = tripal_citation_get_project($record_id, $table_name);
      // In chado.project and properties in projectprop.
      $properties['tripal_citation:project'] = ($project)
        ? tripal_citation_get_property($project, 'project') : 0;


        // # ANALYSIS:
      // Get entity analysis connection.
      $analysis = tripal_citation_get_analysis(2, $table_name);
      // In chado.pub and properties in pubprop.
      $properties['tripal_citation:analysis'] = ($analysis) 
        ? tripal_citation_get_property($record_id, 'analysis') : 0;

        
      // # PROPERTIES:
      // Get entity property from prop table.
      // In chado.tablename + prop table of the table.
      $prop = tripal_citation_get_property($record_id, $table_name);
      $properties['tripal_citation:property'] = ($prop) ? $prop : 0;


      // CITATION SETTINGS:
      $citation_template = '{CURRENT_DATE} + {CURRENT_URL} + {local:authors}';
      $citation_prefer_project = 0;
      $citation_date = 'short';
      $citation_title = '';
      
      // 1. TEST: TOKEN RESOLVED USING PROPERTY 
      // FROM PROJECT CONNECTION (PROJECTPROP) TABLE OF STOCK.
      // Tokens - current date, current url and local:authors.
      $tokens_template = tripal_citation_get_tokens($citation_template);
      $this->assertEquals($tokens_template, array('CURRENT_DATE', 'CURRENT_URL', 'local:authors'), 
        'Tokens do not match the citation template.');
    
      // Inspect these property set if publication citaiton 
      // is NOT preferred in the configuration.
      $property_set = array(
        'tripal_citation:project', 
        'tripal_citation:analysis',
        'tripal_citation:property'
      );

      $citation = $citation_template;
      foreach($tokens_template as $token) {
        $values = '';

        foreach($property_set as $set) {
          // Isolate the values in a specific property set and
          // prepare array so values can be referenced using token.
          $values = tripal_citation_prepare_values($properties[ $set ]);

          if (is_array($values[ $token ])) {
            $dataset = $values[ $token ];
            // Attempt to rank values or just return text if single value
            // has been returned.
            $token_value = tripal_citation_rank_token_values($dataset);
            
            // ASSERT: the value is the same as stock-project-projectprop for token authors.
            $this->assertEquals($token_value, $seeder->my_stock['citations']['authors'], 
              'Token value does not match expected value for project citations/authors.');

            if($token_value != '') {
              // Transpose the token.
              $citation = tripal_citation_transpose_cvterm($citation, $token, $token_value);

              // At this stage the cvterm token has been transposed but non-cvterms are as is.
              // ASSERT: the token in citation template has been resolved.
              $assert_value = '{CURRENT_DATE} + {CURRENT_URL} + ' . $seeder->my_stock['citations']['authors'];
              $this->assertEquals($citation, $assert_value, 
                'The citation template with cvterm tokens resolved does not match expected result.');

              break;
            }
          }
        }
      }

      // Resolve all non-cvterm tokens after all cvterm tokens established.
      $citation = tripal_citation_transpose_non_cvterm($citation, 
        array('current_date' => $citation_date)
      );      

      // Final citation should look like this:
      $url = url(current_path(), array('absolute' => TRUE));
      $current_url = l($url, $url);
      $final_citation = tripal_citation_format_date($citation_date) . ' + ' . $current_url . ' + ' . $seeder->my_stock['citations']['authors'];

      // ASSERT: At this stage citation is ready to be rendered.
      $this->assertEquals($citation, $final_citation, 'Completed citation does not match expected citation.');
    }


    // 2. TEST PREFER PUBLICATION
    // FROM PUB TABLE CONNECTION (STOCK_PUB) AND USING
    // UNIQUENAME FIELD (CONTAINING THE CITATION TEXT).
    
    // CITATION SETTINGS:
    // Same configuration as as the test above except
    // here prefer project is set to true.
    $citation_template = '{CURRENT_DATE} + {CURRENT_URL} + {local:authors}';
    $citation_prefer_project = 1;
    $citation_date = 'short';
    $citation_title = '';
    
    // Isolate data from tripal publication pub column property set.
    $pub_citation_set = 'tripal_citation:pub_column';
    $values = tripal_citation_prepare_values($properties[ $pub_citation_set ]);
    $citation = tripal_citation_rank_token_values($values[ $pub_citation_set ]);

    // ASSERT: citation bypassed and citation is the uniquename field
    // of chado.pub as established by connection table.
    $this->assertEquals($citation, $seeder->my_stock['citations']['publication'], 
      'Completed citation does not match expected pub citation.');


    // 3. TEST: RANKING OF INFORMATION.
    // In seeder stock property writer for MY STOCk has 3 writers 
    // with rank value (rank field).
    // Rank order.
    $prop_citation_set = 'tripal_citation:property';
    $token = 'local:writers';
    $citation_template = 'WRITERS: {' . $token . '}';
    // Isolate data from tripal tableprop (property) property set.
    $values = tripal_citation_prepare_values($properties[ $prop_citation_set ]);
    $token_value = tripal_citation_rank_token_values($values[ $token ]);
    $citation = tripal_citation_transpose_cvterm($citation_template, $token, $token_value);

    // Result ranked and converted to modified comma separted value
    // using and as the delimeter for the last item in a list.
    // WRITERS: Ms. Sofia G., Mr. James E. and Mr. Luke S.
    $writers = 'WRITERS: ' . $seeder->my_stock['citations']['writers'][1]['name'] . ', ' . 
               $seeder->my_stock['citations']['writers'][0]['name'] . ' and ' . 
               $seeder->my_stock['citations']['writers'][2]['name'];

    // ASSERT: multiple values are transformed into csv format.
    $this->assertEquals($citation, $writers, 
      'Ranked and completed citation do not match expected citation.');
    
    // Single value - no ranking.
    $prop_citation_set = 'tripal_citation:property';
    $token = 'local:lead writer';
    $citation_template = 'LEAD WRITER: {' . $token . '}';
    // Isolate data from tripal tableprop (property) property set.
    $values = tripal_citation_prepare_values($properties[ $prop_citation_set ]);
    $token_value = tripal_citation_rank_token_values($values[ $token ]);
    $citation = tripal_citation_transpose_cvterm($citation_template, $token, $token_value);
    
    $writer = 'LEAD WRITER: ' . $seeder->my_stock['citations']['lead writer']; 

    // ASSERT: just single value property - return value/text as is.
    $this->assertEquals($citation, $writer, 
      'Single-value property and completed citation do not match expected citation.');


    // 4. TEST: token missing information and value.
    // Does not exist term.
    $token = 'i do not exist:kinda term';
    $citation_template = '{' . $token . '}';
    $citation = tripal_citation_transpose_missing($citation_template, $token);
    // ASSERT: non-existent term replaced with missing information token.
    $this->assertEquals($citation, '[MISSING INFORMATION]', 
      'Missing token (does not exist) resolved value does not match expected value.');
    
    // Term exists but no value found.
    $token = 'local:glass';
    $citation_template = '{' . $token . '}';
    $citation = tripal_citation_transpose_missing($citation_template, $token);
    // ASSERT: existing term replaced with missing information but show the term name.
    $this->assertEquals($citation, '[MISSING INFORMATION:glass]', 
      'Missing token (no data) resolved value does not match expected value.');

    
      // 5. TEST: configuration title override.
    // Set global title, and override title on content type level setting.
    $set_title = 'GLOBAL TITLE';
    $title = variable_set('tripal_citation_title', $set_title);
    $override_title = 'OVERRIDE TITLE';
    $markup = tripal_citation_theme_citation('TEST CITATION - NOTHING FANCY', array('title' => $override_title));
    // See if gobal title has been replaced by override title 
    // in the final markup.
    $text = strip_tags($markup);
    $replaced = (str_contains($text, $override_title)) ? TRUE : FALSE;
    $this->assertTrue($replaced, 'Could not replace title text with override title');
  }
}
