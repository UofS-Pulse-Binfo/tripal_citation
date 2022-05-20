# Tripal Citation

Tripal Fields specific to providing automated citations for data based on properties + publications

## Core Purpose

Make a field available for most Tripal Content Types to provide an easily copy/pastable citation for data. This citation should link directly to the actual publlication, if available, and not to the Tripal Publication content type. If there is no publication, this field should use an admin entered template and chado property records associated either 1) with the data directly or 2) associated with a chado project or chado analysis that the data belong to. The template used is specific to the type of data.

## Current Data of focus

- Phenotypic data: citation is on the experiment (chado.project-based) + trait (chado.cvterm-based) page the data is associated with (no phenotypic data pages)
- Genotypic data: citation is on the experiment page the data is associated with (no genotypic data pages).
- Studies, Experiments: clearly it's on these pages directly as they are chado.project-based.
- Sequence Variants: on the chado.feature-based sequence variant page but pulls information from the associated project/analysis record.
- Genetic Markers: on the chado.feature-based genetic marker page but pulls information from the associated project/analysis record.
- Genetic Maps: on the chado.featuremap-based genetic map page page but pulls information from the associated project/analysis record.
- Genome Assemblies: on the chado.analysis-based genome assembly page and properties are associated with the analysis.
- Germplasm: on the chado.stock-based germplasm pages and properties are associated with the specific germplasm.

## How the field is configured

Each instance of the field will have a field formatter settings which can be accessed through the cog button on the manage display page. This settings form will include 
- a large textarea where the admin can enter a template with tokens to be used if there is no associated publication. The tokens are denoted with curley braces (i.e. `{accession)`) used will be replaced with the value of the indicated chado property. 
- a date configuration form element to specify configuration of the `{CURRENT_DATE}` token.
- a checkbox to indicate if a publication should be the preferred citation if available. Default is TRUE.

There are a few generic, non-cvterm associated tokens as follows:
- `{CURRENT_URL}`: the full URL of the page the citation is displayed on
- `{CURRENT_DATE}`: the current date. Formatting of the date will be specified in the field formatter settings separately from the template.

Example Template:

> {local:authors} ({NCIT:date}) Unpublished Phenotypic data for {loca:genus} species. Data collected by {AGRO:00000379}, {local:research group} and funded by {local:funder_name}. Data was accessed from KnowPulse ({CURRENT_URL}) on {CURRENT_DATE}. All rights retained by the original authors.

### How the citation is generated

1. Check if the token is one of the supported generic, non-cvterm associated tokens and replace accordingly.
2. If the field instance has the "Publication Preferred" checkbox checked then:

    - check for a publication connection table for the current chado record and if it exists, check for a publication associated with this chado record.
    - check for a project connection table for the current chado record and if it exists, check for a project associated with this chado record and use it's publication if available.
    - check for an analysis connection table for the current chado record and if it exists, check for an analysis associated with this chado record and use it's publication if available.

3. Use the configured template to generate a citation based on chado properties. Replace tokens based on the following priority:

    - replace any tokens that match properties associated with the current chado record.
    - find an associated project, check for any remaining tokens that match properties associated with the project.
    - find an associated analysis, check for any remaining tokens that match properties associated with the analysis.
    - any remaining un-replaced tokens should be replaced with `[MISSING INFORMATION: cvterm.name]` if the cvterm exists OR `[MISSING INFORMATION]` if the cvterm doesn't exist at all. In this last case a Tripal warning should also be added to the logs that indicates the missing data so that administrators can provide it.


#### Tokens

For example, a token of `{local:source_institute}` used in a template on a stock-based germplasm page would be replaced with the chado.stockprop.value for the property with a type_id referencing the `local:source_institute` cvterm (i.e. db.name = 'local', dbxref.accession = 'source_institute'). If there is more then one property referencing the specified term then they will be ordered by the stockprop.rank and comma-separated.

## Terms of Interest

### Citation

- CV: NCIT
- Accession: NCIT:C41196
- Definition: An extract or quotation from or reference to an authoritative source, e.g. a book or author, used, for example, to support an idea, theory, or argument.
- URL:  https://ncithesaurus.nci.nih.gov/ncitbrowser/ConceptReport.jsp?dictionary=NCI_Thesaurus&ns=ncit&code=C41196
- Usage: We use this term as the default term for the citation field itself.

### Data Collector

- CV: Agronomy Ontology 
- Accesion: AGRO:00000379
- Definition: A person that has a role of collecting data related to the experiment.
- URL: http://purl.obolibrary.org/obo/AGRO_00000379
- Usage: We associate this term with phenotypic data to indicate the researcher who collected the phenotypic data.

## Data Custodian

- CV: NCIT
- Accession: NCIT:C165210
- Definition: The person responsible for a specific data set.
- URL: http://purl.obolibrary.org/obo/NCIT_C165210
- Usage: We use this for the name of the PI for the grant funding this data.

## Data Release

- CV: NCIT
- Accession: NCIT:C172217
- Definition:The act of making data or other structured information accessible to the public or to the user group of a database.
- URL: http://purl.obolibrary.org/obo/NCIT_C172217
- Usage: We use this for the full text of an experiment-specific data release statement. We generally use the default Tripal "Chado Property" field to display this text above the citation if it's available.

## Data Source

- CV: NCIT
- Accession: NCIT:C16493
- Definition: The person or authoritative body who provided the information.
- URL: http://purl.obolibrary.org/obo/NCIT_C16493
- Usage: We use this to indicate the research group who provided the data to our Tripal site.

## Data Originator

- CV: NCIT
- Accession: NCIT:C142490
- Definition: The creating entity for each information element that identifies the source of the information capture in the electronic case report form for a clinical investigation. The source could be a person, computer system, device or instrument.
- URL: http://purl.obolibrary.org/obo/NCIT_C142490
- Usage: We use this to indicate the person who generated the data through analysis. This applies to genotypic data for example.

## Curator

- CV: NCIT
- Accession: NCIT:C69141
- Definition: The person in charge of the care and superintendence of something, especially a collection.
- URL: http://purl.obolibrary.org/obo/NCIT_C69141
- Usage: We use this to indicate data that was specifically curated. Both the specific persons name and the team is indicated. For example, "Ruobin Liu, KnowPulse Curation Team".

