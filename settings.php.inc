<?php

// settings.php.inc

// General settings
function tallantoschedule_settings() {

    $form = array();
    $form['tallantoschedule_source_url'] = array(
        '#type' => 'textfield',
        '#title' => t('Source URL'),
        '#default_value' => variable_get('tallantoschedule_source_url', 'http://kaula.tallanto.ru/index.php?entryPoint=ProcessCalendarData'),
        '#size' => 100,
        '#maxlength' => 400,
        '#description' => t("URL for import schedule in the XML format."),
        '#required' => TRUE,
    );
    $form['tallantoschedule_cache_timeout'] = array(
        '#type' => 'textfield',
        '#title' => t('Cache timeout, seconds'),
        '#default_value' => variable_get('tallantoschedule_cache_timeout', 900),
        '#size' => 10,
        '#maxlength' => 20,
        '#description' => t("How long the module should cache XML document."),
        '#required' => TRUE,
    );
    $form['tallantoschedule_signin_url'] = array(
        '#type' => 'textfield',
        '#title' => t('Sign in URL'),
        '#default_value' => variable_get('tallantoschedule_signin_url', 'http://kaula.tallanto.ru/index.php?entryPoint=wtcs&from_calendar=1&write_to=class&write_to_id=@EVENT_ID@'),
        '#size' => 100,
        '#maxlength' => 400,
        '#description' => t("URL for sign in to the event. '@EVENT_ID@' variable will be substituted for event ID."),
        '#required' => TRUE,
    );
    $form['tallantoschedule_difficulty_levels'] = array(
        '#type' => 'textarea',
        '#title' => t('Difficulty levels'),
        '#default_value' => variable_get('tallantoschedule_difficulty_levels', t('Suitable for all
Easy
Medium
Intensive')),
        '#description' => t("Difficulty levels to be used for events."),
        '#required' => TRUE,
    );
    $form['cache_flush'] = array(
        '#type' => 'submit',
        '#value' => t('Flush cache'),
        '#submit' => array('tallantoschedule_settings_cache_flush'),
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

// Branches settings
function tallantoschedule_settings_branches() {
    $form = array();
    
    // Get branches
    $tallantoschedule_parsed_data = variable_get('tallantoschedule_parsed_data', array());
    
    if (isset($tallantoschedule_parsed_data['valid_branches'])) {
        
        foreach ($tallantoschedule_parsed_data['valid_branches'] as $branch => $placeholder) {
            $safe_branch = preg_replace("/[^a-zA-Z0-9]+/", "", $branch);
            $form['tallantoschedule_branch_' . $safe_branch] = array(
                '#type' => 'textfield',
                '#title' => $branch,
                '#default_value' => variable_get('tallantoschedule_branch_' . $safe_branch, ''),
                '#size' => 80,
                '#maxlength' => 400,
            );
        }
        
    } else {
        drupal_set_message(t('No branches loaded.'));
    }
    
    return system_settings_form($form);
}