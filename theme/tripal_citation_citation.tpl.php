<?php

/**
 * @file
 * Master template file of citations.
 */

/**
 * Class styles for citations:
 * smart - tripal-citation-style-smart (use quotes)
 * banner - tripal-citation-style-banner (banner style)
 * simple  - tripal-citation-style-simple (plain text)
 * 
 * Ckass for controls:
 * copy - tripal-citation-control-copy (copy to clipboard)
 * none - tripal-citation-control-none (no button)
 */

$citation = $variables;

// Add title to copy button when required.
$copy_title = ($citation['control'] == 'copy') 
  ? 'Copy citation to clipboard' : '';
?>

<div class="tripal-citation">
  <strong><?php print $citation['title']; ?></strong>
  <div class="tripal-citation-cite">
    <div class="<?php print 'tripal-citation-style-' . $citation['style']; ?>">
      <?php print $citation['citation']; ?>
    </div>
    <div title="<?php print $copy_title; ?>" class="<?php print 'tripal-citation-control-' . $citation['control']; ?>"></div>
  </div>
 </div>