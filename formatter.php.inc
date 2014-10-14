<?php

// formatter.php.inc

// Return formatted date for TallantoEvent object
function formatEventDate(&$tallanto_event, $use_day_of_week = FALSE) {
    if ($use_day_of_week) {
        $timestamp = mktime(0, 0, 0, substr($tallanto_event->start_date, 5, 2), substr($tallanto_event->start_date, 8, 2), substr($tallanto_event->start_date, 0, 4));
        
        return sprintf('%s, %s–%s',
            date('D', $timestamp),
            substr($tallanto_event->start_time, 0, 5), 
            substr($tallanto_event->end_time, 0, 5)
        );
    } else {
        return sprintf('%s–%s', 
            substr($tallanto_event->start_time, 0, 5), 
            substr($tallanto_event->end_time, 0, 5)
        );
    }
}

// Return list of 'valid' dates for the schedule
function listValidDates($forBlockRender = FALSE) {
    
    $today = mktime(0, 0, 0, date('n'), date('j'), date('Y'));
    $valid_dates = array();
        
    if ($forBlockRender) {
        $format = ', d.m';
        
        for ($i=0;$i<7;$i++) {
            $day_of_week = date('N', $today + ($i * 60 * 60 * 24));
            switch ($day_of_week) {
                case 1: $day_text = t('Monday');
                   break;
                case 2: $day_text = t('Tuesday');
                   break;
                case 3: $day_text = t('Wednesday');
                   break;
                case 4: $day_text = t('Thursday');
                   break;
                case 5: $day_text = t('Friday');
                   break;
                case 6: $day_text = t('Saturday');
                   break;
                case 7: $day_text = t('Sunday');
                   break;
            }
            $valid_dates[] = $day_text . date($format, $today + ($i * 60 * 60 * 24));
        }
    } else {
        $format = 'Y-m-d';
        
        for ($i=0;$i<7;$i++) {
            $valid_dates[] = date($format, $today + ($i * 60 * 60 * 24));
        }
    }
    
    
    
    return $valid_dates;
}

// Sort days in the schedule
function cmpTallantoDayEvents($a, $b) {
    if ($a->timestamp == $b->timestamp) {
        return 0;
    }
    return ($a->timestamp < $b->timestamp) ? -1 : 1;
}

// Prepare schedule for display (sort)
function listTallantoSchedule($branch) {
    // Get data
    $tallantoschedule_parsed_data = variable_get('tallantoschedule_parsed_data', array());
    
    // Select events for active branch
    $selected_events = array();
    foreach ($tallantoschedule_parsed_data['valid_events'] as $event_id => $tallanto_event) {
        // fix object
        $tallanto_event = fixObject($tallanto_event);
        
        // Is branch selected?
        if ($branch == $tallanto_event->branch) {
            // Is subject selected?
            if (variable_get('tallantoschedule_subject_check_' . $tallanto_event->subject_id, 0)) {
                $selected_events[date('N', $tallanto_event->timestamp)][] = $tallanto_event;
            }
        }
    }
    
    // Sort events so 'today' always first
    ksort($selected_events);
    $today = date('N');
    $before_today = array_slice($selected_events, 0, ($today - 1), TRUE);
    $today_and_further = array_slice($selected_events, ($today - 1), 7, TRUE);
    $selected_events = $today_and_further + $before_today;
    
    // Sort events inside each day
    foreach ($selected_events as $day_of_week => $day_events) {
        usort($day_events, 'cmpTallantoDayEvents');
        $selected_events[$day_of_week] = $day_events;
    }
    
    //print_r($selected_events);die;
    
    return $selected_events;
}

// Format sign up URL
function formatSignUpUrl($tallanto_event) {
    $tallantoschedule_signup_min = variable_get('tallantoschedule_signup_min', 30) * 60;
    $tallantoschedule_signup_max = variable_get('tallantoschedule_signup_max', 4320) * 60;
    $far_cutoff_timestamp = time() + $tallantoschedule_signup_max;
    if (variable_get('tallantoschedule_signup_roundup', 1)) {
        $far_cutoff_timestamp = mktime(23, 59, 59, 
            date('m', $far_cutoff_timestamp), date('d', $far_cutoff_timestamp), date('Y', $far_cutoff_timestamp));
    }
    if ($tallanto_event->timestamp > $far_cutoff_timestamp) {                           // Too far in the future
        return t('Not opened yet');
    } elseif ($tallanto_event->timestamp < (time() + $tallantoschedule_signup_min)) {   // Too late already
        return t('Closed');
    } else {                                                                            // ...and here we go
        $url = str_replace('@EVENT_ID@', $tallanto_event->id, variable_get('tallantoschedule_signup_url', '@EVENT_ID@'));
        return sprintf('<a href="%s">%s</a>', $url, t('Sign up'));
    }
}