<?php

/**
 * Implements hook_import_map().
 * Return mapkey => label array
 */
function entityimport_example_import_map() {
  return array(
    'ex1'  => t("My Example"),
  );
}

/**
 * Implements entityimport_import_map_settings_MAPID().
 * single: TRUE|FALSE: If this enable, the importing callback will hide other settings so the form will be sample.
 * term_to_entity: TRUE|FALSE: allow to predefined terms to be used in the importing
 * remove: TRUE; Allow remove feature to be disable
 * header: TRUE|FALSE: If CSV has the header row or not
 * offset: 0 The row that data will start from
 * map: this is a JSON of the following format:
 * field: valueformular
 * field is the field name
 * valueformular has formular that defined as below
 * Format of fieldvalue would be opfieldidentify-column0-opcolumn1|opdatatype-opdatatypesecond-opdatatypearg|opdataverify|optional_check_existed-optionalfind_existed
 * Ex: 2
 * Ex: 0-1
 * Ex: tid-7-25|check-practice_are
 */
function entityimport_import_map_settings_ex1() {
  return array(
    'single' => TRUE,
    'header' => FALSE,
    'map' => '{
    "title": "0|||1-1"
}',
    'to' => array('shop', 'shop|shop'),
  );
}
