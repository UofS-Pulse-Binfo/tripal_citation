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

Tripal Citation module provides predefined style options to customize overall appearance of citation elements in relation to the page it is attached to. Below are general information on configuring citation style and setting up citation template for a content type.

### Configure Appearance and Controls

[See Tripal Citation Module Confiuration Page](https://user-images.githubusercontent.com/15472253/174154849-5f44d6f6-c74a-44ed-94ed-100a093d31dc.png)

**`Style Citation`** <br/> Prepared styling options applied to citation element that will define its overall form and appearance. See Style Options table below.

#### Styles Options
| Style | Description | Example |
|----------------|-------------|---------|
| SMART | Uses quotation marks to create design interests that appear smart and formal |![image](https://user-images.githubusercontent.com/15472253/174156578-13f6a74a-9c51-47aa-b3ff-2e82e40317d8.png)|
| BANNER | Encapsulates a citation into a rectangular container (banner-like) | ![image](https://user-images.githubusercontent.com/15472253/174156749-4edc10fc-5322-4f01-8301-adf8fab17f8e.png)|
| SIMPLE | Plain text and no styling applied to citation | ![image](https://user-images.githubusercontent.com/15472253/174156966-658e08ee-f7bd-4d9c-97b3-48a4721a1eeb.png)|


**`Title Text`** <br/> A text field to set a title placed on every citation instance. An override title in Citation Template can be used to apply title to specific content type. See Configure Citation Template below.

**`Copy Button`** <br/> Enable or disable option for citation text to be copied to clipboard by providing a copy button placed on the upper right-hand corner of a citation block. When text is copied, it can be pasted onto other documents by using the text editor paste icon or through paste key combination command.

![image](https://user-images.githubusercontent.com/15472253/174156143-834c4d62-e91f-4802-b4a9-f5f39ab07297.png)


### Configure Citation Template 
This configuration becomes available when Tripal Citation Custom Field is attached to a content type.

[See Tripal Citation Content Type Configuration](https://user-images.githubusercontent.com/15472253/174160067-3e9e9728-426a-44a5-8385-307ca11e4180.png)

**`Citation Template`** <br/> This setting will define a set of tokens in the form of {CV NAME:CVTERM NAME}, also called cvterm tokens, which corresponds to a value in a chado table. Apart from cvterm tokens, the following non-cvterm tokens are supported.

Examples: {local:authors}, {ncit:citation}


| Token | Description |
|-------|-------------|
|{CURRENT_DATE} | token resolves to the current date and time. The date and time format to apply can be customized using the Date format setting.|
|{CURRENT_URL} | token resolves to the current URL (page) the citation is attached or viewed. |


**`Date Format`** <br/> {CURRENT_DATE} token output can be formatted using these settings. Each option in this field shows an example to evaluate the format to match desired format.

**`Alternate Title`** <br/> Module configuration for citation title that applies to every citation instance can be overridden by this alternate citation text setting. Title text set to setting applies only to the content type.

**`Prefer Publication`** <br/> When a content is referenced in a publication that has attribution, this setting will use that attribution information instead of the citation template defined by Citation Template setting.

## Composing Citation Template

Compose citation template in the content type citation template settings field using the following illustration to form a valid CVTERM tokens. Other non-cvterms tokens namely {CURRENT_DATE} and {CURRENT_URL} are also supported and must be encoded as they appear in the table above. 

<p align="center">
<br />
<img src="https://user-images.githubusercontent.com/15472253/174162249-a6b5ddb1-84db-40e2-9dd7-23f642a2d45c.png" />
<br />
</p>

Each token in a citation template contains specific terms to query and match against a relevant properties + publication chado tables the content type is mapped to.

Example: In this example, the citation template below was attached to Germplasm Accession Tripal Content Type which mapped to chado.stock table.

```
{local:authors} Unpublished Phenotypic data. Data collected by and funded by Project B. 
Data was accessed from mysite. All rights reserved.
```

The token {local:authors} will be resolved by inspecting chado.project+projectprop (project connection) and chado.stockprop table for citation/author record where the column type_id (cvterm) is equals to authors. All value or values will be ranked and processed to generate the final citation text.

```
James Y., Robert F., Rose B. and Crystal Q. Unpublished Phenotypic data. Data collected by and funded by Project B. 
Data was accessed from mysite. All rights reserved.
```




