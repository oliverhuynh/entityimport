<?php

function entityimport_import_form($form, &$form_state) {
  $form = array();
  $form['description'] = array(
    '#markup' => t('Notes: <ul><li>This form cannot be run multiple instances at the same time<li>With existed entities, the data will be appended.</li></ul>'),
  );
  $form['import_source'] = array(
    '#type' => 'radios',
    '#title' => 'Import source',
    '#options' => array(
      'file' => t('File'),
      'text' => t('Copy paste'),
    ),
    '#default_value' => 'file'
  );

  $form['file_upload'] = array(
    '#type'  => 'file',
    '#title' => t('File'),
    '#description' => t('Entities spreadsheet. The CSV file\'s encoding should be UTF-8 to avoid invalid characters. If you leave this as blank, the old form state as well as rencent uploading file will be used.'),
    '#states' => array(
      'visible' => array(
       ':input[name="import_source"]' => array(
          array('value' => 'file')
       ),
      ),
    ),
  );

  $form['text_upload'] = array(
    '#type'  => 'textarea',
    '#title' => t('Entities spreadsheet content'),
    '#description' => t('Entities spreadsheet content. If you leave this as blank, the old form state as well as rencent uploading file will be used.'),
    '#states' => array(
      'visible' => array(
       ':input[name="import_source"]' => array(
          array('value' => 'text')
       ),
      ),
    ),
  );

  $import_maps = module_invoke_all("import_map");
  if ($import_maps && count($import_maps)) {
    $form['file_type'] = array(
      '#type' => 'radios',
      '#title' => 'File type',
      '#options' => $import_maps
    );
    if (count($import_maps) == 1) {
      $form['file_type']['#default_value'] = key($form['file_type']['#options']);
      $form['file_type']['#access'] = FALSE;
    }
    $hide_custom_map = FALSE;
    $hide_remove = FALSE;
    foreach ($import_maps as $import_map => $label) {
      $settings = entityimport_my_get_settings($import_map);
      if ($settings['single']) {
        $form['file_type']['#default_value'] = $import_map;
        $hide_custom_map = TRUE;
        $hide_remove = TRUE;
        break;
      }
    }
  }

  $form['import_map'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => $import_maps && count($import_maps),
    '#title' => $import_maps && count($import_maps) ? t('Custom import map') : t("Import map")
  );
  if ($hide_custom_map) {
    $form['import_map']['#access'] = FALSE;
  }

  $form['import_map']['to'] = array(
    '#type' => 'hierarchical_select',
    '#title' => 'Entity | Bundle',
    '#default_value' => array('node', 'article'),
    '#config' => array(
      'module' => 'entityimport',
      'params' => array(),
      'save_lineage'    => 1,
      'enforce_deepest' => 1,
      'resizable'       => 0,
      'level_labels' => array(
        'status' => 1,
        'labels' => array(
          0 => t('Type'),
          1 => t('Bundle')
        ),
      ),
      'dropbox' => array(
        'status'   => 0,
      ),
      'editability' => array(
        'status'           => 0,
      ),
      'entity_count'    => array(
        'enabled' => 0,
        'require_entity' => 0,
      ),
      'render_flat_select' => 0,
    ),
  );
  $form['import_map']['term_to_entity'] = array(
    '#type' => 'checkbox',
    '#title' => t("Fill taxonomy terms to entity")
  );
  $form['import_map']['offset'] = array(
    '#type' => 'textfield',
    '#title' => t("Data(including header) row offset"),
    '#default_value' => 0,
  );
  $form['import_map']['header'] = array(
    '#type' => 'checkbox',
    '#title' => t("There is a header row"),
    '#default_value' => TRUE
  );
  $form['import_map']['map'] = array(
    '#type' => 'textarea',
    '#title' => t("Text area JSON"),
    '#default_value' => '{"title":0}',
    '#description' => nl2br(' * Format of fieldvalue would be opfieldidentify-column0-opcolumn1|opdatatype-opdatatypesecond-opdatatypearg|data_verify - data_singlealter
    opfieldidentify-column0-opcolumn1 | opdatatype-opdatatypesecond-opdatatypearg | data_verify - data_singlealter | optional_check_existed-optionalfind_existed
 * Simple Ex column 2: 2 =
 * Ex: 0-1
 * Ex: null-2 => skip using field_identify, 2 will be the data to set to the field
 * Ex: $name allows column to be in string, not by index
 * Ex: 0|||1-1 => Get col 1, find existed item by this, and use data from existed entity (use data from existed:1 - find existed:1)
 * Ex: tid-7-25|check-practice_are
 * Ex: TextToStore|string
 * Ex: mail-1||required
 ')
  );
  $form['keywords'] = array(
    '#type' => 'textfield',
    '#title' => 'Taxonomy terms',
    '#autocomplete_path' => 'keywords/autocomplete',
    '#states' => array(
      'visible' => array(array(
        array(':input[name="import_map[term_to_entity]"]' => array('checked' => TRUE))
      )),
    ),
  );
  if ($hide_custom_map) {
    $form['keywords']['#states'] = array();
  }

  foreach ($import_maps as $import_map => $label) {
    $settings = entityimport_my_get_settings($import_map);
    if ($settings['term_to_entity']) {
      $form['keywords']['#states']['visible'][0][] = array(':input[name="file_type"]' => array('value' => $import_map));
    }
  }
  if (!$form['keywords']['#states']) {
    $form['keywords']['#access'] = FALSE;
  }

  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Import'),
  );
  if (!$hide_remove) {
    $form['submit-remove'] = array(
      '#type'  => 'submit',
      '#value' => t('Remove entities'),
    );
  }
  if ($f = variable_get('entityimport_recent_file', (object) array())) {
    if (!empty($f->filename)) {
      $form['recent_file'] = array(
        '#type'  => 'value',
        '#value' => $f,
      );
      $form['submit'] = array(
        '#type'  => 'submit',
        '#value' => t('Import from the file input/text or recent file !file...', array('!file' => substr($f->filename, 0, 8))),
      );
      if (!$hide_remove) {
        $form['submit-remove'] = array(
          '#type'  => 'submit',
          '#value' => t('Remove recent imported entities'),
        );
      }
    }
  }
  return $form;
}



function entityimport_import_form_validate_exec_keywords($form, &$form_state, $settings) {
  // Validate the keywords
  if ($settings['term_to_entity'])  {
    $tags = $form_state['values']['keywords'];
    $typed_terms = drupal_explode_tags($tags);
    $value = array();
    $out = array();
    foreach ($typed_terms as $typed_term) {
      preg_match('/\(([^\)]+)\)/', $typed_term, $m);
      if (isset($m[1])) {
        $out[] = $m[1];
      }
    }
    $form_state['values']['keywords'] = $out;
  }
}

function entityimport_import_form_validate_make_text_to_file($form, &$form_state) {
  if (empty($_FILES['files']['name']['file_upload'])) {
    if (!($form_state['values']['text_upload'])) {
      return FALSE;
    }

    _prepare_data_help_with_message($form_state, $form_state['values']['text_upload']);
  }
}

function _prepare_data_help_with_message(&$form_state, $data = "") {
  $file = _prepare_data_help($data);
  if (!$file) {
    form_set_error('file_upload', t("Failed to save the file."));
    return ;
  }
  else {
    file_usage_add($file, 'services', 'files', $file->fid);
    $file->status = 1;
    file_save($file);
    $form_state['values']['file_upload'] = $file;
    $form_state['values']['file_upload']->formvalues = $form_state['values'];
    $form_state['values']['recent_file'] = $form_state['values']['file_upload'];
  }
}

function _prepare_data_help($data = "") {
  $dest = 'public://entityimport';
  if (!file_prepare_directory($dest, FILE_CREATE_DIRECTORY)) {
    watchdog('file', 'The upload directory %directory for entities spreadsheet could not be created or is not accessible.', array('%directory' => $dest));
    form_set_error('file_upload', t('The file could not be uploaded.'));
    return ;
  }
  if ($data) {
    $csv_file = 'textupload.csv';
    return file_save_data($data, $dest . '/' . $csv_file, FILE_EXISTS_REPLACE);
  }

  $validators = array(
    'file_validate_extensions' => array('csv'),
  );
  return file_save_upload('file_upload', $validators, $dest, FILE_EXISTS_REPLACE);
}

function entityimport_import_form_validate($form, &$form_state) {
  $settings = entityimport_my_get_settings($form_state['values']['file_type'], $form_state['values']['import_map']);
  entityimport_import_form_validate_exec_keywords($form, $form_state, $settings);
  entityimport_import_form_validate_make_text_to_file($form, $form_state);

  // Save the file, check the folder
  if (!empty($_FILES['files']['name']['file_upload'])) {
    _prepare_data_help_with_message($form_state);
  }
  else {
    $form_state['values']['file_upload'] = isset($form_state['values']['recent_file']) ? $form_state['values']['recent_file'] : FALSE;
  }

  if ((!$form_state['values']['file_upload']) || !is_file($form_state['values']['file_upload']->uri)) {
    $uri = isset($form_state['values']['file_upload']) && $form_state['values']['file_upload'] && isset($form_state['values']['file_upload']->uri) ? $form_state['values']['file_upload']->uri : "N/A";
    form_set_error('file_upload', t("Failed to read the file !file.", array('!file' => $uri)));
    return ;
  }
  else {
    unset($form_state['values']['file_upload']->formvalues['recent_file']);
    variable_set('entityimport_recent_file', $form_state['values']['file_upload']);
  }
}

function entityimport_import_form_submit($form, &$form_state) {
  if (!isset($form['submit-remove']) || $form_state['clicked_button']['#value'] != $form['submit-remove']['#value']) {
    $batch = array(
      'operations' => array(
        array('entityimport_import_batch_do_import', array())
      ),
      'finished' => 'entityimport_import_batch_finished',
      'title' => t('Processing entities importing'),
      'init_message' => t('Entities import is starting.'),
      'progress_message' => t('Processed @current out of @total.'),
      'error_message' => t('Entities importing has encountered an error.'),
      'file' => drupal_get_path('module', 'entityimport') . '/entityimport.pages.inc',
    );
    batch_set($batch);
  }
  else {
    $batch = array(
      'operations' => array(
        array('entityimport_import_batch_do_remove', array())
      ),
      'finished' => 'entityimport_import_batch_finished',
      'title' => t('Processing entities removing'),
      'init_message' => t('Entities removing is starting.'),
      'progress_message' => t('Processed @current out of @total.'),
      'error_message' => t('Entities removing has encountered an error.'),
      'file' => drupal_get_path('module', 'entityimport') . '/entityimport.pages.inc',
    );
    batch_set($batch);
  }

  return 'submitted';
}

function entityimport_import_batch_do_import(&$context) {
  if (empty($context['sandbox'])) {
    $context['results'] = array('results' => array(), 'skips' => array());
    $context['sandbox'] = array();
    $context['sandbox']['progress'] = 0;
    $f = variable_get('entityimport_recent_file', (object) array());
    $fp = file($f->uri);
    $context['sandbox']['max'] = count($fp);
    $context['sandbox']['file'] = $f;
    $context['sandbox']['file-pointer'] = 0;
  }

  $max_per_run = 20;
  // Seek to previous file point
  $handle = fopen($context['sandbox']['file']->uri, "r");
  fseek($handle, $context['sandbox']['file-pointer']);
  $formvalues = $context['sandbox']['file']->formvalues;
  $settings = entityimport_my_get_settings($formvalues['file_type'], $formvalues['import_map']);

  $map = entityimport_get_map($context);
  while ($max_per_run-- && $handle !== FALSE) {
    // Get row data
    $row = fgetcsv($handle, 10000, ",");
    $context['sandbox']['file-pointer'] = ftell($handle);

    if ($row === FALSE) {
      $context['sandbox']['progress'] = 1000000; // Stop the batch
      break;
    }

    $context['sandbox']['progress']++;
    $context['message'] = t('Importing row !row/!total', array('!row' => $context['sandbox']['progress'], '!total' => $context['sandbox']['max']));
    $context['results']['results'][] = 'Row data: ' . print_r($row, TRUE);

    $next = isset($settings['offset']) ? $settings['offset'] : 0;
    if ($context['sandbox']['progress'] > $next) {
      // Store the first row and skip if needed
      $continue_load = TRUE;
      if (!isset($context['sandbox']['headlines'])) {
        $context['sandbox']['headlines'] = $row;
        if ($settings['header']) {
          $continue_load = FALSE;
          // Remove saved record because it is header
          unset($context['results']['results']);
        }
      }

      if ($continue_load) {
        // Do the importing with next rows
        $skips = entityimport_import_do_import($row, $context['sandbox']['headlines'], $context);
        foreach ($skips as $skip) {
          $context['results']['skips'][] = str_replace(':row', $context['sandbox']['progress'], $skip);
        }
      }
    }
  }

  @fclose($handle);
  $context['finished'] = ($context['sandbox']['progress'] >= $context['sandbox']['max']);
}

function entityimport_import_batch_do_remove(&$context) {
  if (empty($context['sandbox'])) {
    $context['results'] = array('results' => array(), 'skips' => array());
    $context['sandbox'] = array();
    $context['sandbox']['progress'] = 0;
    $f = variable_get('entityimport_recent_file', (object) array());
    $fp = file($f->uri);
    $context['sandbox']['max'] = count($fp);
    $context['sandbox']['file'] = $f;
    $context['sandbox']['file-pointer'] = 0;
  }

  $map = entityimport_get_map($context);
  $max_per_run = 20;
  // Seek to previous file point
  $handle = fopen($context['sandbox']['file']->uri, "r");
  fseek($handle, $context['sandbox']['file-pointer']);
  while ($max_per_run-- && $handle !== FALSE) {
    // Get row data
    $row = fgetcsv($handle, 10000, ",");
    $context['sandbox']['file-pointer'] = ftell($handle);

    if ($row === FALSE) {
      $context['sandbox']['progress'] = 1000000; // Stop the batch
      break;
    }

    $context['sandbox']['progress']++;
    $context['message'] = t('Removing user !user !row/!total', array('!row' => $context['sandbox']['progress'], '!user' => $row[$map['mail']], '!total' => $context['sandbox']['max']));
    $context['results']['results'][] = 'Row data: ' . print_r($row, TRUE);

    // Store the first row
    if (!isset($context['sandbox']['headlines'])) {
      $context['sandbox']['headlines'] = $row;
    }
    else {
      // Do the importing with next rows
      $skips = entityimport_import_do_remove($row, $context['sandbox']['headlines'], $context);
      foreach ($skips as $skip) {
        $context['results']['skips'][] = str_replace(':row', $context['sandbox']['progress'], $skip);
      }
    }
  }

  @fclose($handle);
  $context['finished'] = ($context['sandbox']['progress'] >= $context['sandbox']['max']);
}

function entityimport_import_batch_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t('@count items processed.', array('@count' => count($results['results']))));
    $results['skips'] = array_unique($results['skips']);
    foreach ($results['skips'] as $skip) {
      drupal_set_message($skip, 'error');
    }
  }
  else {
    $error_operation = reset($operations);
    drupal_set_message(t('An error occurred while processing @operation with arguments : @args', array(
      '@operation' => $error_operation[0],
      '@args' => print_r($error_operation[0], TRUE),
    )));
  }
}

/**
 * 0-based columns array, offset is the first row of the header
 */
function entityimport_get_map($context) {
  $formvalues = $context['sandbox']['file']->formvalues;
  $settings = entityimport_my_get_settings($formvalues['file_type'], $formvalues['import_map']);
  return $settings['map'];
}

/**
 * Format of fieldvalue would be opfieldidentify-column0-opcolumn1 | opdatatype-opdatatypesecond-opdatatypearg | data_verify- data_singlealter | optional_check_existed-optionalfind_existed
 * Ex: 2
 * Ex: 0-1
 * Ex: null-2 => skip using field_identify, 2 will be the data to set to the field
 * Ex: 0|||1-1 => Get col 1, find existed item by this, and use data from existed entity (use data from existed:1 - find existed:1)
 * Ex: $name allow column to be in string, not by index
 * Ex: TextToStore | string
 * Ex: tid-7-25|check-practice_are
 */
function _is_col_help($t)  {
  return is_numeric($t) || $t[0] == '$';
}

function extract_callback_filters($data) { // Should be $format
  $data_filter = explode('|', $data);
  $tmp = isset($data_filter[3]) ? $data_filter[3] : '';
  $tmp = explode("-", $tmp);
  $tmp = $tmp + array(0,0);
  list($data_callback_filters['data_compare_existed'], $data_callback_filters['find_existed']) = $tmp;

  // Data Verify filter
  $tmp = isset($data_filter[2]) ? $data_filter[2] : '';
  $tmp = explode("-", $tmp);
  $tmp = $tmp + array('','');
  list($data_callback_filters['data_verify'], $data_callback_filters['data_singlealter']) = $tmp;

  $data_cols = explode('-', $data_filter[0]);
  $data_filter = count($data_filter) > 1 ? $data_filter[1] : 'plain';
  $data_filter = explode('-', $data_filter);
  $data_callback_filters['data_type'] = array_shift($data_filter);
  $data_callback_filters['data_type_second'] = count($data_filter) >= 2 ? array_shift($data_filter) : ($data_callback_filters['data_type'] == 'plain' ? '' : '');
  $data_callback_filters['data_type_arg'] = $data_filter;

  $data_callback_filters['field_identify'] = _is_col_help($data_cols[0]) ? 'value' : $data_cols[0];
  // Default for vocabselect
  if ($data_callback_filters['field_identify'] == 'tid' && $data_callback_filters['data_type_second'] == '') {
    $data_callback_filters['data_type_second'] = 'vocabselect';
  }

  if (!_is_col_help($data_cols[0])) {
    array_shift($data_cols);
  }
  $data_cols[0] = is_numeric($data_cols[0]) ? $data_cols[0] : substr($data_cols[0], 1);
  $data_callback_filters['data_cols'] = $data_cols;
  return $data_callback_filters;
}

/**
 * Implement of data_type_call_special_SPECIALCALL
 */
function data_type_call_special_arraykeys($row, $skips, $context, $data_cols, $data_filter, $headline) {
  return array_combine($data_cols, $data_cols);;
}

/**
 * Implement of data_type_call_special_SPECIALCALL
 */
function data_type_call_special_string($row, $skips, $context, $data_cols, $data_filter, $headline) {
  $text = isset($data_cols[0]) ? $data_cols[0] : $data_filter['field_identify'];
  return $text;
}

/**
 * Implement of data_type_call_special_SPECIALCALL
 */
function data_type_call_special_array($row, $skips, $context, $data_cols, $data_filter, $headline) {
  return $row[$data_cols[0]];
}

/**
 * Implement of data_type_call_special_SPECIALCALL
 */
function data_type_call_special_generated_from_mail($row, $skips, $context, $data_cols, $data_filter, $headline, $account) {
  if (!isset($account['mail'])){
    return "";
  }
  $name = preg_replace('/@.*$/', '', $account['mail']);
  $name = entityimport_cleanup_username($name);
  $name = entityimport_unique_username($name);
  return $name;
}

/**
 * Implement of data_type_call_special_SPECIALCALL
 */
function data_type_call_special_user_pass_rehash($row, $skips, $context, $data_cols, $data_filter, $headline, $account) {
  if (!isset($account['mail'])){
    return "";
  }
  return user_pass_rehash($account['mail'], REQUEST_TIME, data_type_call_special_generated_from_mail($row, $skips, $context, $data_cols, $data_filter, $headline, $account));
}

/**
 * Implement of data_type_call_special_SPECIALCALL
 */
function data_type_call_special_defined($row, $skips, $context, $data_cols, $data_filter, $headline) {
  return isset($context['sandbox']['file']->formvalues[$data_filter['data_cols'][0]]) ? $context['sandbox']['file']->formvalues[$data_filter['data_cols'][0]] : FALSE;
}

/**
 * Implement of data_type_call__NORMALCALL
 */
function data_type_call__check(&$row, $skips, $context, $data_col, $data_filter, $headlines) {
  if ($row[$data_col] == 'x') {
    return $headlines[$data_col];
  }
  return $row[$data_col];
}

/**
 * Implement of data_singlealter_ALTER
 * TODO: Ensure the old file to be removed
 */
function data_singlealter_url2image($return_value, $row, $skips, $context, $data_filter, $headlines) {
  $file = file_save_data(file_get_contents($return_value), file_default_scheme().'://field/image/' . basename($return_value));
  $file->status = 1;
  return (array) $file;
}

/**
 * Implement of data_type_call__NORMALCALL
 */
function data_type_call__explode(&$row, $skips, $context, $data_col, $data_filter, $headlines) {
  $parts = explode(' ', $row[$data_col]);
  $ret = isset($data_filter['data_type_arg'][0]) && isset($parts[$data_filter['data_type_arg'][0]]) ? $parts[$data_filter['data_type_arg'][0]] : FALSE;
  return $ret;
}

/**
 * Implement of data_type_call__NORMALCALL
 */
function data_type_call__explodelast(&$row, $skips, $context, $data_col, $data_filter, $headlines) {
  $parts = explode(' ', $row[$data_col]);
  if (count($parts) >= 2) {
    $new_parts = array();
    $new_parts[1] = array_pop($parts);
    $new_parts[0] = implode(' ', $parts);
    $parts = $new_parts;
  }
  $ret = isset($data_filter['data_type_arg'][0]) && isset($parts[$data_filter['data_type_arg'][0]]) ? $parts[$data_filter['data_type_arg'][0]] : FALSE;
  return $ret;
}


function entityimport_import_find_existed_entity($map, $account) {
  // Build find_existed keys
  $find_existed_array = array();
  foreach (element_children($map) as $field) {
    $data = $map[$field];
    // Data filter
    $data_filter = extract_callback_filters($data);
    extract($data_filter);
    if ($find_existed) {
      $find_existed_array[$field] = $account[$field];
    }
  }
  return $find_existed_array;
}

function entityimport_import_do_import($row, $headlines, &$context) {
  // Prepare entity type
  $formvalues = $context['sandbox']['file']->formvalues;
  $settings = entityimport_my_get_settings($formvalues['file_type'], $formvalues['import_map']);
  list($type, $bundle) = explode("|", $settings['to'][1]);
  $typeinfo = entity_get_info($type);

  $skips = array();
  $skip_this = FALSE;
  $map = entityimport_get_map($context);
  $account = array();
  foreach (element_children($map) as $field) {
    $data = $map[$field];
    // Data filter
    $data_filter = extract_callback_filters($data);

    // Build $data_cols
    extract($data_filter);

    // Data field structure,
    $parsing_data = FALSE;
    $parsing_data_is_array = FALSE;
    // handle special data_type
    $special_call = 'data_type_call_special_' . $data_type;
    if ($data_type && !function_exists($special_call) && $special_call != 'data_type_call_special_plain') {
      $skips[] = t('Data type !type call is not defined!', array("!type" => $special_call));
    }
    if (function_exists($special_call)) {
      $parsing_data = $special_call($row, $skips, $context, $data_cols, $data_filter, $headlines, $account);
      if (is_array($parsing_data)) {
        $parsing_data_is_array = TRUE;
        foreach ($parsing_data as $k => $parsing_data_v) {
          $parsing_data[$k] = roschier_import_data_clean_single_value($parsing_data[$k], $row, $skip_this, $skips, $context, $data_filter, $headlines, array('ignore_error' => TRUE));
        }
      }
      else {
        $parsing_data = roschier_import_data_clean_single_value($parsing_data, $row, $skip_this, $skips, $context, $data_filter, $headlines);
      }
      if ($skip_this) {
        $skips[] = t('Data special_call wrong');
        break ;
      }
    }
    else {
      $data_cols = count($data_cols) <= 1 ? array($data_cols[0], $data_cols[0]) : $data_cols;
      $parsing_data_is_array = (strpos($field, 'field_') === 0);
      if ($parsing_data_is_array) {
        $parsing_data = array();
      }
      for ($data_col = $data_cols[0]; $data_col <= $data_cols[1]; $data_col++) {
        if ($parsing_data_is_array) {
          $parsing_data[] = roschier_import_data_cell_to_user_field($row, $skips, $context, $data_col, $data_filter, $headlines, $return);
          if (isset($return['error'])) {
            $skip_this = TRUE;
            break ;
          }
        }
        else {
          $parsing_data = $row[$data_col];
        }
      }
      if ($skip_this) {
        $skips[] = t('Data cell wrong');
        break ;
      }
    }

    // Set parsing_data to field of account
    if ($parsing_data !== FALSE) {
      if (strpos($field, 'field_') === 0) {
        $parsing_data = $parsing_data_is_array ? $parsing_data : array($parsing_data);
        foreach ($parsing_data as $parsing_data_value) {
          if ($parsing_data_value !== FALSE) {
            if ($field_identify == 'null') {
              $account[$field][LANGUAGE_NONE][] = $parsing_data_value;
            }
            else {
              $account[$field][LANGUAGE_NONE][] = array($field_identify => $parsing_data_value,);
            }
          }
        }
      }
      else {
        if ($field == 'mail') {
          $parsing_data = drupal_strtolower($parsing_data);
        }
        $account[$field] = $parsing_data;
      }
    }

    // Validate the mail
    if (!$skip_this && !valid_email_address($account[$field]) && $data_filter['data_verify'] == "required") {
      $skip_this = TRUE;
      $skips[] = t('@field is required at row :row', array('@field' => $field));
    }
  }

  if (!$skip_this) {
    $find_existed_array = entityimport_import_find_existed_entity($map, $account);
    $existed_account = $find_existed_array ? my_entity_load($type, array(), $find_existed_array, 1) : array();
    if ($existed_account) {
      $existed_account = reset($existed_account);
      $existed_account = (array) $existed_account;
      // Merge if the user is existed.
      $account += $existed_account;
      // Merge keywords
      merge_keywords($account, $existed_account);
    }

    $account = (object) $account;
    foreach (element_children($map) as $field) {
      $data = $map[$field];
      // Data filter
      $data_filter = extract_callback_filters($data);
      extract($data_filter);
      if ($data_compare_existed  == '1') {
        $account->{$field}   = $existed_account && isset($existed_account->{$field}) ? $existed_account->{$field} : $account->{$field};
      }
    }


    // Save user
    if (isset($typeinfo['bundle keys']) && isset($typeinfo['bundle keys']['bundle'])) {
      $_key = $typeinfo['bundle keys']['bundle'];
      $account->{$_key} = $bundle;
    }
    if (!$settings['debug']) {
      entity_save($type, $account);
    }
    else {
      //-----------DEBUGGING----------
      watchdog('DEBUG', __FUNCTION__ . print_r($account, TRUE));
      //-----------END DEBUGGING----------
    }
  }

  return $skips;
}

function entityimport_import_do_remove($row, $headlines, &$context) {
  $formvalues = $context['sandbox']['file']->formvalues;
  $settings = entityimport_my_get_settings($formvalues['file_type'], $formvalues['import_map']);
  list($type, $bundle) = explode("|", $settings['to'][1]);
  $skips = array();
  if ($type != 'user') {
    $skips[] = t('Removing feature is currently developed for only user entity.');
    return $skips;
  }

  $map = entityimport_get_map($context);
  $existed_account = user_load_multiple(array(), array('mail' => $row[$map['mail']]));


  if (!empty($existed_account)) {
    $existed_account = reset($existed_account);
    if ($existed_account->uid && $existed_account->uid != 1) {
      user_delete($existed_account->uid);
    }
  }
  else {
    $skips[] = t('Email !email does not belong to any user', array('!email' => $row[$map['mail']]));
  }

  return $skips;
}

function roschier_import_data_clean_single_value($return_value, $row, &$skip, &$skips, $context, $data_filter, $headlines, $options = array()) {
  $options += array(
    'ignore_error' => FALSE,
  );
  extract($options);

  if ($data_filter['data_type_second'] == 'vocabselect') {
    $return_value = trim($return_value);

    if (!isset($context['sandbox']['terms'][$data_filter['data_type_arg'][0]][$return_value])) {
      $d = $context['sandbox']['terms'][$data_filter['data_type_arg'][0]][$return_value] = db_query('SELECT * FROM {taxonomy_term_data} d INNER JOIN {taxonomy_vocabulary} v ON v.vid = d.vid WHERE v.machine_name=:vocab AND ((d.tid=:name) OR (REPLACE(REPLACE(d.name, :string1, :string2), :string3, :string4) = REPLACE(REPLACE(:name, :string1, :string2), :string3, :string4)))', array(':string1' => ' ', ':string2' => '', ':string3' => '&', ':string4' => 'and', ':name' => $return_value, ':vocab' => $data_filter['data_type_arg'][0]))->fetchField();
    }
    else {
      $d = $context['sandbox']['terms'][$data_filter['data_type_arg'][0]][$return_value];
    }

    if (!$ignore_error && $d === FALSE) {
      $skip = TRUE;
      $skips[] = t('Could not retrieve data from value !value', array('!value' => $return_value));
    }

    $return_value = $d;
  }

  // Filter
  $verify_call = 'data_singlealter_' . $data_filter['data_singlealter'];
  if (function_exists($verify_call)) {
    $return_value = $verify_call($return_value, $row, $skips, $context, $data_filter, $headlines);
  }
  return $return_value;
}

function roschier_import_data_cell_to_user_field(&$row, &$skips, &$context, $data_col, $data_filter, $headlines, &$return = array()) {
  $skip = FALSE;
  $return_value = $row[$data_col];
  if ($return_value == '') {
    $skip = TRUE;
  }

  $normal_call = 'data_type_call__' . $data_filter['data_type'];
  if (function_exists($normal_call)) {
    $return_value = $normal_call($row, $skips, $context, $data_col, $data_filter, $headlines);
  }

  if ($data_filter['data_type'] == 'search') {
    // No need, below is already trimmed
    // $return_value = trim($return_value);
  }

  $return_value = roschier_import_data_clean_single_value($return_value, $row, $skip, $skips, $context, $data_filter, $headlines);

  $verified = TRUE;
  $verify_call = 'data_type_verify_' . $data_filter['data_verify'];
  if (function_exists($verify_call)) {
    $verified = $verify_call($return_value, $row, $skips, $context, $data_col, $data_filter, $headlines);
    if (!$verified) {
      $return['error'] = TRUE;
    }
  }
  $skip = !$verified;

  // Core verify
  if (is_string($return_value)) {
    if (!$skip && !drupal_validate_utf8($return_value)) {
      $d = iconv("UTF-8", "UTF-8//IGNORE", $return_value);
      if (!drupal_validate_utf8($d)) {
        $skip = TRUE;
      }
      $return_value = $d;
    }
  }

  if ($skip) {
    $skips[] = t('String !value is not valid. Check UTF-8 issue or the length. Please update this user !user. Row :row', array('!value' => $return_value, '!user' => $account['mail']));
  }

  if (!$skip) {
    return $return_value;
  }
  return FALSE;
}

/**
 * Implement of data_type_verify_VERIFYKEY
 */
function data_type_verify_2chars($return_value, $row, $skips, $context, $data_col, $data_filter, $headlines) {
  return strlen(trim($return_value)) >= 2;
}

/**
 * Function to clean up username.
 *
 * e.g.
 *     Replace two or more spaces with a single underscore
 *     Strip illegal characters.
 *
 * @param string $name
 *   The username to be cleaned up.
 *
 * @return string
 *   Cleaned up username.
 */
function entityimport_cleanup_username($name) {
  // Strip illegal characters.
  $name = preg_replace('/[^\x{80}-\x{F7} a-zA-Z0-9@_.\'-]/', '', $name);

  // Strip leading and trailing spaces.
  $name = trim($name);

  // Convert any other series of spaces to a single underscore.
  $name = preg_replace('/  +/', '_', $name);

  // If there's nothing left use a default.
  $name = ('' === $name) ? t('user') : $name;

  // Truncate to a reasonable size.
  $name = (drupal_strlen($name) > (USERNAME_MAX_LENGTH - 10)) ? drupal_substr($name, 0, USERNAME_MAX_LENGTH - 11) : $name;
  return $name;
}

/**
 * Create a legal, unique Drupal username.
 *
 * @param string $name
 *   A name from which to base the final user name.
 *
 * @return string
 *   A unique user name based on $name.
 */
function entityimport_unique_username($name) {
  // Iterate until we find a unique name.
  $i = 0;
  do {
    $new_name = empty($i) ? $name : $name . '_' . $i;
    $found = db_query_range("SELECT uid from {users} WHERE name = :name", 0, 1, array(':name' => $new_name))->fetchAssoc();
    $i++;
  } while (!empty($found));

  return $new_name;
}

function merge_keywords(&$account, $existed_account) {
  foreach (array('field_practice_area', 'field_industry_sector', 'field_author') as $field) {
    if (isset($existed_account[$field][LANGUAGE_NONE])) {
      foreach ($existed_account[$field][LANGUAGE_NONE] as $field_item) {
        // Check if this exist in account
        if (!_exists_in_account($account[$field][LANGUAGE_NONE], $field_item)) {
          array_unshift($account[$field][LANGUAGE_NONE], $field_item);
        }
      }
    }
  }
}
function _exists_in_account($field_holder, $field_item) {
  foreach ($field_holder as $k => $fh_item) {
    foreach (array('value', 'tid') as $identify) {
      if (isset($fh_item[$identify]) && $fh_item[$identify] == $field_item[$identify]) {
        return TRUE;
      }
    }
  }
  return FALSE;
}


function entityimport_keywords_retrieve($search) {
  $tags_typed = drupal_explode_tags($search);
  $search = drupal_strtolower(array_pop($tags_typed));
  $prefix = count($tags_typed) ? drupal_implode_tags($tags_typed) . ', ' : '';
  // $parts = explode(',', $search);
  // $search = trim(array_pop($parts));

  $matches = array();
  $result = db_select('taxonomy_term_data', 't')
    -> fields('t', array('tid', 'name', 'vid'))
    -> condition('name', $search.'%%', 'LIKE')
    -> range(0, 10)
    -> execute();
  foreach ($result as $term) {
    $n = $term->name;
    $v = taxonomy_vocabulary_load($term->vid);
    // Term names containing commas or quotes must be wrapped in quotes.
    if (strpos($n, ',') !== FALSE || strpos($n, '"') !== FALSE) {
      $n = '"' . str_replace('"', '""', $n) . '"';
    }
    $out = $v->name . ':' . $n;
    $key = $prefix . $n . '('. $term->tid .')';
    $matches[$key] = $out;
  }
  drupal_json_output($matches);
}

