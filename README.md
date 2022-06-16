![Tripal Dependency](https://img.shields.io/badge/tripal-%3E=3.0-brightgreen)
[![Run PHPUnit Tests](https://github.com/UofS-Pulse-Binfo/tripal_citation/actions/workflows/phpunit.yml/badge.svg)](https://github.com/UofS-Pulse-Binfo/tripal_citation/actions/workflows/phpunit.yml)

![tripal-citation-banner](https://user-images.githubusercontent.com/15472253/174119538-0b9698ab-bcbb-40a8-8cc8-be62dca72044.png)

Tripal website instance houses data from many sources whether from an individual researcher or institutions. Tripal Citation module provides automated citations for various data based on properties and/or publication. Citations text can easily be copied and pasted into research papers or publications to accurately attribute an entity.


## Dependencies

1. [Drupal 7](https://www.drupal.org/)
2. [Tripal 3.x](http://tripal.info/)


## Installation
1. Add Tripal Citation module into you Drupal host modules directory. 
2. Install and enable Tripal Citation module like you would any other Drupal module.
3. Configure module after installation. Link to configuration page is displayed next to the enabled module in the Modules page of Drupal.
<br /><br />

![image](https://user-images.githubusercontent.com/15472253/174120403-47e992a8-c0bb-4ea9-9645-be2abf5fd207.png)

## Resolving Citation Tokens

Compose citation templates with tokens in the form of {CV NAME:CVTERM NAME} format. Each template can be positioned to specific Tripal Content Type to enable targetted citaiton and/or attribution to specific data. Automated resolution of all tokens in a template are based on properties + publications information stored in CHADO prop and pub tables as shown in the illustration below.
<br /><br />

![image](https://user-images.githubusercontent.com/15472253/174138584-f6e0dfee-801f-43b0-b313-6c316565ce32.png)

## Configuration

Tripal Citation module provides predefined style options to customize overall appearance of citation elements in relation to the page it is attached to. Click the link below for general information on configuring citation style and setting up citation template for a content type.

[Configure Tripal Citation Module](https://github.com/UofS-Pulse-Binfo/tripal_citation)

