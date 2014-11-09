<?php

// settings.php.inc

// 
// General settings
//
function tallantoschedule_settings() {
    $weight = -1000;

    $form = array();
    /*
     * URL settings
     */
    $form['url_settings'] = array(
      '#type' => 'fieldset', 
      '#title' => t('Links'), 
      '#weight' => $weight++, 
      '#collapsible' => TRUE, 
      '#collapsed' => TRUE,
    );
    $form['url_settings']['tallantoschedule_source_url'] = array(
        '#type' => 'textfield',
        '#title' => t('Source URL'),
        '#default_value' => variable_get('tallantoschedule_source_url', 'http://kaula.tallanto.ru/index.php?entryPoint=ProcessCalendarData'),
        '#size' => 100,
        '#maxlength' => 400,
        '#description' => t("URL for import schedule in the XML format."),
        '#required' => TRUE,
        
    );
    $form['url_settings']['tallantoschedule_signup_url'] = array(
        '#type' => 'textfield',
        '#title' => t('Sign up URL'),
        '#default_value' => variable_get('tallantoschedule_signup_url', 'http://kaula.tallanto.ru/index.php?entryPoint=wtcs&from_calendar=1&write_to=class&write_to_id=@EVENT_ID@'),
        '#size' => 100,
        '#maxlength' => 400,
        '#description' => t("URL for sign up to the event. '@EVENT_ID@' variable will be substituted for event ID."),
        '#required' => TRUE,
    );
    /*
     * Cache settings
     */
    $form['cache_settings'] = array(
      '#type' => 'fieldset', 
      '#title' => t('Cache'), 
      '#weight' => $weight++, 
      '#collapsible' => TRUE, 
      '#collapsed' => FALSE,
    );
    $form['cache_settings']['tallantoschedule_cache_timeout'] = array(
        '#type' => 'textfield',
        '#title' => t('Cache timeout, seconds'),
        '#default_value' => variable_get('tallantoschedule_cache_timeout', 900),
        '#size' => 10,
        '#maxlength' => 20,
        '#description' => t("How long the module should cache XML document."),
        '#required' => TRUE,
    );
    $time_to_flush = sprintf(' Cache will be auto-flushed in %d seconds.', 
        (variable_get('tallantoschedule_cache_timestamp', 0) + variable_get('tallantoschedule_cache_timeout', 900) - time()));
    $form['cache_settings']['cache_flush'] = array(
        '#type' => 'submit',
        '#value' => t('Flush cache'),
        '#submit' => array('tallantoschedule_settings_cache_flush'),
        '#suffix' => '<span class="description">&nbsp;' . t('Clear the cache and force to load XML document from the source.')
          . $time_to_flush . '</span>' . '<br />',
    );
    /*
     * Sign up settings
     */
    $form['signup_settings'] = array(
      '#type' => 'fieldset', 
      '#title' => t('Sign up window'), 
      '#weight' => $weight++, 
      '#collapsible' => TRUE, 
      '#collapsed' => FALSE,
    );
    $form['signup_settings']['tallantoschedule_signup_min'] = array(
        '#type' => 'textfield',
        '#title' => t('Minimal period, minutes'),
        '#default_value' => variable_get('tallantoschedule_signup_min', 30),
        '#size' => 10,
        '#maxlength' => 20,
        '#description' => t("Minimal period user will be allowed to sign up."),
        '#required' => TRUE,
    );
    $form['signup_settings']['tallantoschedule_signup_max'] = array(
        '#type' => 'textfield',
        '#title' => t('Maximal period, minutes'),
        '#default_value' => variable_get('tallantoschedule_signup_max', 4320),
        '#size' => 10,
        '#maxlength' => 20,
        '#description' => t("Maximal period user will be allowed to sign up."),
        '#required' => TRUE,
    );
    $form['signup_settings']['tallantoschedule_signup_roundup'] = array(
      '#type' => 'checkbox', 
      '#title' => t('Round up maximal period to the day'),
      '#default_value' => variable_get('tallantoschedule_signup_roundup', 1),
    );
    
    /*
     * Other settings
     */
    $form['other_settings'] = array(
      '#type' => 'fieldset', 
      '#title' => t('Other'), 
      '#weight' => $weight++, 
      '#collapsible' => TRUE, 
      '#collapsed' => TRUE,
    );
    $form['other_settings']['tallantoschedule_difficulty_levels'] = array(
        '#type' => 'textarea',
        '#title' => t('Difficulty levels'),
        '#default_value' => variable_get('tallantoschedule_difficulty_levels', t('Suitable for all
Easy
Medium
Intensive')),
        '#description' => t("Difficulty levels to be used for events."),
        '#required' => TRUE,
    );
    $form['other_settings']['tallantoschedule_signup_custom_text'] = array(
        '#type' => 'textfield',
        '#title' => t('Sign up custom text'),
        '#default_value' => variable_get('tallantoschedule_signup_custom_text', t('Please call')),
        '#size' => 80,
        '#maxlength' => 400,
        '#description' => t("Custom text to show in the 'Sign up' column when 'Custom text' choice is selected for the subject."),
        '#required' => FALSE,
    );
    
    // Hidden settings
    $form['tallantoschedule_cache_timestamp'] = array(
        '#type' => 'hidden',
        '#value' => variable_get('tallantoschedule_cache_timestamp', 0),
    );
    
    return system_settings_form($form);
}

// Handle 'flush cache button'
function tallantoschedule_settings_cache_flush($form, &$form_state) {
    // Set cache outdated so next time module will load XML from the source
    variable_set('tallantoschedule_cache_timestamp', 0);
    
    drupal_set_message(t('The cache was successfully flushed.'));
}

//
// Branches settings
//
function tallantoschedule_settings_branches() {
    $form = array();
    
    // Get branches
    $tallantoschedule_parsed_data = variable_get('tallantoschedule_parsed_data', array());
    
    if (isset($tallantoschedule_parsed_data['valid_branches']) && (count($tallantoschedule_parsed_data['valid_branches']) > 0)) {
    
        $form['help'] = array(
            '#value' => t('Please select branches should be visible in the schedule.'),
        );
    
        $form['header1'] = array(
            '#value' => ' ',
            '#prefix' => '<table><tr><th>',
            '#suffix' => '</th>',
        );
        $form['header2'] = array(
            '#value' => t('Branch name'),
            '#prefix' => '<th>',
            '#suffix' => '</th>',
        );
        $form['header3'] = array(
            '#value' => t('Title'),
            '#prefix' => '<th>',
            '#suffix' => '</th></tr>',
        );
                
        foreach ($tallantoschedule_parsed_data['valid_branches'] as $branch => $placeholder) {
            $safe_branch = preg_replace("/[^a-zA-Z0-9]+/", "", $branch);
            
            $form['tallantoschedule_branch_check_' . $safe_branch] = array(
                '#type' => 'checkbox',
                '#default_value' => variable_get('tallantoschedule_branch_check_' . $safe_branch, 0),
                '#prefix' => '<tr><td>',
                '#suffix' => '</td>',
            );
            $form['tallantoschedule_branch_key_' . $safe_branch] = array(
                '#type' => 'textfield',
                '#default_value' => $branch,
                '#size' => 60,
                '#maxlength' => 400,
                '#disabled' => TRUE,
                '#prefix' => '<td>',
                '#suffix' => '</td>',
            );
            $form['tallantoschedule_branch_title_' . $safe_branch] = array(
                '#type' => 'textfield',
                '#default_value' => variable_get('tallantoschedule_branch_title_' . $safe_branch, $branch),
                '#size' => 80,
                '#maxlength' => 400,
                '#prefix' => '<td>',
                '#suffix' => '</td></tr>',
            );
        }
    
        $form['footer1'] = array(
            '#value' => ' ',
            '#prefix' => '<tr><td>',
            '#suffix' => '</td>',
        );
        $form['footer2'] = array(
            '#value' => ' ',
            '#prefix' => '<td>',
            '#suffix' => '</td>',
        );
        $form['footer3'] = array(
            '#value' => ' ',
            '#prefix' => '<td>',
            '#suffix' => '</td></tr></table>',
        );
    
    } else {
        drupal_set_message(t('No branches loaded.'));
    }
    
    return system_settings_form($form);
}

//
// Subjects settings
//
function tallantoschedule_settings_subjects() {
    $form = array();
    
    // Get subjects
    $tallantoschedule_parsed_data = variable_get('tallantoschedule_parsed_data', array());
    
    if (isset($tallantoschedule_parsed_data['valid_subjects']) && (count($tallantoschedule_parsed_data['valid_subjects']) > 0)) {
    
        $form['help'] = array(
            '#value' => t('Please select subjects should be visible in the schedule.'),
        );
    
        $form['header1'] = array(
            '#value' => ' ',
            '#prefix' => '<table><tr><th>',
            '#suffix' => '</th>',
        );
        $form['header2'] = array(
            '#value' => t('Subject name'),
            '#prefix' => '<th>',
            '#suffix' => '</th>',
        );
        $form['header3'] = array(
            '#value' => t('Visible title'),
            '#prefix' => '<th>',
            '#suffix' => '</th>',
        );
        $form['header4'] = array(
            '#value' => t('Description URL'),
            '#prefix' => '<th>',
            '#suffix' => '</th>',
        );
        $form['header5'] = array(
            '#value' => t('Custom sign up?'),
            '#prefix' => '<th>',
            '#suffix' => '</th></tr>',
        );
                
        foreach ($tallantoschedule_parsed_data['valid_subjects'] as $subject => $title) {
            
            $form['tallantoschedule_subject_check_' . $subject] = array(
                '#type' => 'checkbox',
                '#default_value' => variable_get('tallantoschedule_subject_check_' . $subject, 0),
                '#prefix' => '<tr><td>',
                '#suffix' => '</td>',
            );
            $form['tallantoschedule_subject_title_' . $subject] = array(
                '#type' => 'textfield',
                '#value' => $title,
                '#size' => 30,
                '#maxlength' => 400,
                '#disabled' => TRUE,
                '#prefix' => '<td>',
                '#suffix' => '</td>',
            );
            $form['tallantoschedule_subject_visibletitle_' . $subject] = array(
                '#type' => 'textfield',
                '#default_value' => variable_get('tallantoschedule_subject_visibletitle_' . $subject, $title),
                '#size' => 45,
                '#maxlength' => 400,
                '#prefix' => '<td>',
                '#suffix' => '</td>',
            );
            $form['tallantoschedule_subject_url_' . $subject] = array(
                '#type' => 'textfield',
                '#default_value' => variable_get('tallantoschedule_subject_url_' . $subject, ''),
                '#size' => 35,
                '#maxlength' => 400,
                '#prefix' => '<td>',
                '#suffix' => '</td>',
            );
            $form['tallantoschedule_subject_custom_text_check_' . $subject] = array(
                '#type' => 'checkbox',
                '#default_value' => variable_get('tallantoschedule_subject_custom_text_check_' . $subject, 0),
                '#prefix' => '<td>',
                '#suffix' => '</td></tr>',
            );
        }
    
        $form['footer1'] = array(
            '#value' => ' ',
            '#prefix' => '<tr><td>',
            '#suffix' => '</td>',
        );
        $form['footer2'] = array(
            '#value' => ' ',
            '#prefix' => '<td>',
            '#suffix' => '</td>',
        );
        $form['footer3'] = array(
            '#value' => ' ',
            '#prefix' => '<td>',
            '#suffix' => '</td>',
        );
        $form['footer4'] = array(
            '#value' => ' ',
            '#prefix' => '<td>',
            '#suffix' => '</td>',
        );
        $form['footer5'] = array(
            '#value' => ' ',
            '#prefix' => '<td>',
            '#suffix' => '</td></tr></table>',
        );
    
    } else {
        drupal_set_message(t('No subjects loaded.'));
    }
    
    return system_settings_form($form);
}

//
// Teachers settings
//
function tallantoschedule_settings_teachers() {
    $form = array();
    
    // Get teachers
    $tallantoschedule_parsed_data = variable_get('tallantoschedule_parsed_data', array());
    
    if (isset($tallantoschedule_parsed_data['valid_employees']) && (count($tallantoschedule_parsed_data['valid_employees']) > 0)) {
    
        $form['help'] = array(
            '#value' => t('Please specify description URLs for teachers.'),
        );
    
        $form['header1'] = array(
            '#value' => 'Teacher',
            '#prefix' => '<table><tr><th>',
            '#suffix' => '</th>',
        );
        $form['header3'] = array(
            '#value' => t('Description URL'),
            '#prefix' => '<th>',
            '#suffix' => '</th></tr>',
        );
                
        foreach ($tallantoschedule_parsed_data['valid_employees'] as $teacher => $title) {

            $form['tallantoschedule_teacher_title_' . $teacher] = array(
                '#type' => 'textfield',
                '#value' => $title,
                '#size' => 60,
                '#maxlength' => 400,
                '#disabled' => TRUE,
                '#prefix' => '<tr><td>',
                '#suffix' => '</td>',
            );
            $form['tallantoschedule_teacher_url_' . $teacher] = array(
                '#type' => 'textfield',
                '#default_value' => variable_get('tallantoschedule_teacher_url_' . $teacher, ''),
                '#size' => 80,
                '#maxlength' => 400,
                '#prefix' => '<td>',
                '#suffix' => '</td></tr>',
            );
        }
    
        $form['footer1'] = array(
            '#value' => ' ',
            '#prefix' => '<tr><td>',
            '#suffix' => '</td>',
        );
        $form['footer3'] = array(
            '#value' => ' ',
            '#prefix' => '<td>',
            '#suffix' => '</td></tr></table>',
        );
    
    } else {
        drupal_set_message(t('No teachers loaded.'));
    }
    
    return system_settings_form($form);
}

//
// Difficulty settings
//
function tallantoschedule_settings_difficulty() {
    $form = array();
    
    // Get parent events
    $tallantoschedule_parsed_data = variable_get('tallantoschedule_parsed_data', array());
    
    //print_r($tallantoschedule_parsed_data['parent_events']);die;
    /*
    echo "valid events: ";print_r(_tallantoschedule_analyze_event_array($tallantoschedule_parsed_data['valid_events']));
    echo "parent events: ";print_r(_tallantoschedule_analyze_event_array($tallantoschedule_parsed_data['parent_events']));
    die;
    */
    
    if (isset($tallantoschedule_parsed_data['parent_events']) && (count($tallantoschedule_parsed_data['parent_events']) > 0)) {
    
        $form['help'] = array(
            '#value' => t('Please specify difficulty levels for events. Events marked with asterisk "*" are created as standalone (not as series).'),
        );
        
        // Pre-sort 
    
        // Iterate by branch
        $no_branches_selected = TRUE;
        $no_subjects_selected = TRUE;
        foreach ($tallantoschedule_parsed_data['valid_branches'] as $active_branch => $placeholder) {

            // Check if branch is selected
            $safe_active_branch = preg_replace("/[^a-zA-Z0-9]+/", "", $active_branch);
            if (!variable_get('tallantoschedule_branch_check_' . $safe_active_branch, 0)) {
                continue;
            }
            
            // Little check if there are any selected branches
            $no_branches_selected = FALSE;

            $form['header0_' . $safe_active_branch] = array(
                '#value' => variable_get('tallantoschedule_branch_title_' . $safe_active_branch, $active_branch),
                '#prefix' => '<table><tr><th colspan="4"><h2>',
                '#suffix' => '</h2></th></tr>',
            );
            $form['header1_' . $safe_active_branch] = array(
                '#value' => 'Date',
                '#prefix' => '<tr><th>',
                '#suffix' => '</th>',
            );
            $form['header2_' . $safe_active_branch] = array(
                '#value' => t('Subject'),
                '#prefix' => '<th>',
                '#suffix' => '</th>',
            );
            $form['header3_' . $safe_active_branch] = array(
                '#value' => t('Teacher'),
                '#prefix' => '<th>',
                '#suffix' => '</th>',
            );
            $form['header4_' . $safe_active_branch] = array(
                '#value' => t('Difficulty'),
                '#prefix' => '<th>',
                '#suffix' => '</th></tr>',
            );
                    
            foreach ($tallantoschedule_parsed_data['parent_events'] as $parent_event) {
                // fix object
                $parent_event = fixObject($parent_event);
                
                // Check event eligibility against settings
                // Is branch selected?
                $safe_branch = preg_replace("/[^a-zA-Z0-9]+/", "", $parent_event->branch);
                if (!variable_get('tallantoschedule_branch_check_' . $safe_branch, 0) || ($parent_event->branch != $active_branch)) {
                    continue;
                }
                // Is subject selected?
                if (!variable_get('tallantoschedule_subject_check_' . $parent_event->subject_id, 0)) {
                    continue;
                }
                
                // Little check if there are any selected subjects
                $no_subjects_selected = FALSE;
                
                $form['tallantoschedule_parent_event_date_' . $parent_event->id] = array(
                    '#type' => 'textfield',
                    '#value' => formatEventDate($parent_event, TRUE),
                    '#size' => 20,
                    '#disabled' => TRUE,
                    '#prefix' => '<tr><td>',
                    '#suffix' => '</td>',
                );
                $form['tallantoschedule_parent_event_title_' . $parent_event->id] = array(
                    '#type' => 'textfield',
                    '#value' => implode('', array($tallantoschedule_parsed_data['valid_subjects'][$parent_event->subject_id], ($parent_event->standalone) ? '*' : '')),
                    '#size' => 45, 
                    '#disabled' => TRUE,
                    '#prefix' => '<td>',
                    '#suffix' => '</td>',
                );
                $form['tallantoschedule_parent_event_teacher_' . $parent_event->id] = array(
                    '#type' => 'textfield',
                    '#value' => $tallantoschedule_parsed_data['valid_employees'][$parent_event->employees_ids],
                    '#size' => 45, 
                    '#disabled' => TRUE,
                    '#prefix' => '<td>',
                    '#suffix' => '</td>',
                );
                $form['tallantoschedule_parent_event_level_' . $parent_event->id] = array(
                    '#type' => 'select',
                    '#default_value' => variable_get('tallantoschedule_parent_event_level_'. $parent_event->id, ''),
                    '#options' => preg_split('/\n|\r\n?/', variable_get('tallantoschedule_difficulty_levels', t('Suitable for all'))),
                    '#prefix' => '<td>',
                    '#suffix' => '</td></tr>',
                );
            }
        
            $form['footer_' . $safe_active_branch] = array(
                '#value' => ' ',
                '#prefix' => '<tr><td colspan = "4">',
                '#suffix' => '</td></tr></table>',
            );
        }
        
        // Check to be nice :)
        if ($no_branches_selected) {
            drupal_set_message(t('No branches selected.'));
        }
        if ($no_subjects_selected) {
            drupal_set_message(t('No subjects selected.'));
        }
        if ($no_branches_selected || $no_subjects_selected) {
            $form['brbrbr'] = array(
                '#value' => ' ',
                '#suffix' => '<br>',
            );
        }
        
        // Add submit handler
        $form['#submit'][] = 'tallantoschedule_settings_difficulty_submit';
        
    } else {
        drupal_set_message(t('No parent events loaded.'));
    }
    
    return system_settings_form($form);
}

// Handle difficulty submit
function tallantoschedule_settings_difficulty_submit($form, &$form_state) {
   
    // Get parent events
    $tallantoschedule_parsed_data = variable_get('tallantoschedule_parsed_data', array());
    
    foreach ($tallantoschedule_parsed_data['valid_events'] as $valid_event) {
        // fix object
        $valid_event = fixObject($valid_event);

        // Clean variables...
        variable_del('tallantoschedule_parent_event_date_' . $valid_event->id);
        variable_del('tallantoschedule_parent_event_title_' . $valid_event->id);
        variable_del('tallantoschedule_parent_event_teacher_' . $valid_event->id);
        variable_del('tallantoschedule_parent_event_level_' . $valid_event->id);
    }
}