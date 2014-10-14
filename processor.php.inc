<?php

// processor.php.inc

// Check if XML with schedule is outdated and load if yes
function tallantoschedule_boot() {
	if ((variable_get('tallantoschedule_cache_timestamp', 0) + variable_get('tallantoschedule_cache_timeout', 900)) < time()) {
        // Load XML data and parse it
        $xml_data = tallantoschedule_load_xml_source();

        tallantoschedule_parse_xml_data($xml_data);

        // Update cache timestamp
        variable_set('tallantoschedule_cache_timestamp', time());
	}

    //print_r(variable_get('tallantoschedule_parsed_data', 'NO DATA'));
}

// Load XML schedule data
function tallantoschedule_load_xml_source() {
    // Get start time
    $time_start = _tallantoschedule_microtime_float();

    // Load XML data
    $xml_data = file_get_contents(variable_get('tallantoschedule_source_url', 'http://kaula.tallanto.ru/index.php?entryPoint=ProcessCalendarData'));

    // Get end time
    $time_end = _tallantoschedule_microtime_float();
    $load_time = $time_end - $time_start;

    //drupal_set_message(sprintf('XML loaded, time elapsed: %.1f seconds; XML raw size: %.1f MB', $load_time, (strlen($xml_data) / 1024 / 1024)));

    return $xml_data;
}

// Parse XML schedule data
function tallantoschedule_parse_xml_data($xml_data) {
    // Get start time
    $time_start = _tallantoschedule_microtime_float();

    //
    // 1) Parse events
    //

    $pattern = "@<event id='(.+)' >"
        . "<start_date><!\[CDATA\[(.+) (.+)\]\]></start_date>.*"
        . "<end_date><!\[CDATA\[(.+) (.+)\]\]></end_date>.*"
        . "<subject_id><!\[CDATA\[(.+)\]\]></subject_id>.*"
        . "<employees_ids><!\[CDATA\[(.+)\]\]></employees_ids>.*"
        . "<event_pid><!\[CDATA\[(.*)\]\]></event_pid>.*"
        . "<filial><!\[CDATA\[(.+)\]\]></filial>"
        . ".*</event>@i";

    preg_match_all($pattern, $xml_data, $matches);

    // Copy records which fall into [today..today+6] days period

    // Prepare list of 'valid' dates
    $valid_dates = listValidDates();
    //print_r($valid_dates);

    // Filter records
    $valid_events = array();
    for ($i=0;$i<count($matches[0])-1;$i++) {
        foreach ($valid_dates as $val_date) {
            if ($matches[2][$i] == $val_date) { // date is one of valid dates
                $tallanto_event = new TallantoEvent(
                    $matches[1][$i], // event id
                    $matches[2][$i], // start_date
                    $matches[3][$i], // start_time
                    $matches[4][$i], // end_date
                    $matches[5][$i], // end_time
                    $matches[6][$i], // subject_id
                    $matches[7][$i], // employees_ids
                    $matches[8][$i], // parent event id
                    $matches[9][$i]  // branch
                );
                $valid_events[$matches[1][$i]] = $tallanto_event;
                break;
            }
        }
    }
    // Save event records to process it once again later (see below)
    $events_matches = $matches;

    //
    // 2) Parse subjects
    //

    // Extract subject options
    $pattern = "@<coll_options for='subjectsList'>(.+)</coll_options>@iUs";

    preg_match_all($pattern, $xml_data, $matches);

    $valid_subjects = array();

    if (isset($matches[1][0])) {
        // Extract subjects
        $pattern = "@<item value='(.+)' label='(.+)'></item>@i";

        preg_match_all($pattern, $matches[1][0], $matches);

        for ($i=0;$i<count($matches[0])-1;$i++) {
            $valid_subjects[$matches[1][$i]] = $matches[2][$i];
        }
    } else {
        drupal_set_message('Unable to locate subject options in the XML document.');
    }

    //
    // 3) Parse employees
    //

    // Extract employees options
    $pattern = "@<coll_options for='teachersList'>(.+)</coll_options>@iUs";

    preg_match_all($pattern, $xml_data, $matches);

    $valid_employees = array();
    
    if (isset($matches[1][0])) {
        // Extract subjects
        $pattern = "@<item value='(.+)' label='(.+)'></item>@i";

        preg_match_all($pattern, $matches[1][0], $matches);

        for ($i=0;$i<count($matches[0])-1;$i++) {
            $valid_employees[$matches[1][$i]] = $matches[2][$i];
        }
    } else {
        drupal_set_message('Unable to locate employee options in the XML document.');
    }

    //
    // 4) Prepare branches and parent events
    //
    $valid_branches = array();
    $valid_parent_events = array();
    $standalone_events = array();
    // Collect events with parent ID
    foreach ($valid_events as $tallanto_event) {
        $valid_branches[$tallanto_event->branch] = TRUE;
        if (strlen($tallanto_event->parent_event_id) > 0) { // Event has parent
            $valid_parent_events[$tallanto_event->parent_event_id] = TRUE; // Mark to resolve it later
        }
    }
    // Collect standalone events
    foreach ($valid_events as $tallanto_event) {
        if ((strlen($tallanto_event->parent_event_id) == 0) // Event has no parent
                && !isset($valid_parent_events[$tallanto_event->id])) { // and is not listed as parent for some other event
            $valid_parent_events[$tallanto_event->id] = TRUE; // Mark to resolve it later
            $valid_events[$tallanto_event->id]->standalone = TRUE;
            $standalone_events[$tallanto_event->id] = TRUE;
        }
    }/*
    // Fill parent events array with objects
    foreach ($valid_parent_events as $parent_event_id => $placeholder) {
        if (isset($valid_events[$parent_event_id])) {
            $valid_parent_events[$parent_event_id] = $valid_events[$parent_event_id];
            
            // Check if it's a standalone event
            if (isset($standalone_events[$parent_event_id])) {
                $valid_parent_events[$parent_event_id]->standalone = TRUE;
            }
        }
    }*/
    
    //
    // 5) ...
    //
    for ($i=0;$i<count($events_matches[0])-1;$i++) {
        // If event id exists in the parents array pick it
        if (isset($valid_parent_events[$events_matches[1][$i]])) {
       
            $tallanto_event = new TallantoEvent(
                $events_matches[1][$i], // event id
                $events_matches[2][$i], // start_date
                $events_matches[3][$i], // start_time
                $events_matches[4][$i], // end_date
                $events_matches[5][$i], // end_time
                $events_matches[6][$i], // subject_id
                $events_matches[7][$i], // employees_ids
                $events_matches[8][$i], // parent event id
                $events_matches[9][$i]  // branch
            );
            
            // Check if it's a standalone event
            if (isset($standalone_events[$events_matches[1][$i]])) {
                $tallanto_event->standalone = TRUE;
            }
            
            $valid_parent_events[$events_matches[1][$i]] = $tallanto_event;
        }
    }
    
    
    //
    // 6) Resolve employee names
    //
    foreach ($valid_events as $event_id => $event_item) {
        $valid_events[$event_id]->resolve_employees_names($valid_employees);
    }
    foreach ($valid_parent_events as $event_id => $event_item) {
        if (is_object($valid_parent_events[$event_id])) {
            $valid_parent_events[$event_id]->resolve_employees_names($valid_employees);
        } else {
            unset($valid_parent_events[$event_id]); // not an object...
        }
    }
    
    // Sort parent events
    uasort($valid_parent_events, '_tallantoschedule_sort_parents');
    //print_r($valid_parent_events);die;
    //print_r(_tallantoschedule_analyze_event_array($valid_events));die;

    // Pack all data
    $tallantoschedule_parsed_data['valid_events'] = $valid_events;
    $tallantoschedule_parsed_data['valid_subjects'] = $valid_subjects;
    $tallantoschedule_parsed_data['valid_employees'] = $valid_employees;
    $tallantoschedule_parsed_data['valid_branches'] = $valid_branches;
    $tallantoschedule_parsed_data['parent_events'] = $valid_parent_events;

    //print_r($tallantoschedule_parsed_data);
    //print_r($valid_subjects);
    //print_r(substr($xml_data, 0, 16384));
    //die();

    // Get end time
    $time_end = _tallantoschedule_microtime_float();
    $parse_time = $time_end - $time_start;

    //drupal_set_message(sprintf('XML parsed, time elapsed: %.1f seconds; event objects: %d; subjects: %d; employees: %d',
    //    $parse_time, count($valid_events), count($valid_subjects), count($valid_employees)));

    variable_set('tallantoschedule_parsed_data', $tallantoschedule_parsed_data);
}

// Sort parents
function _tallantoschedule_sort_parents($a, $b) {
    // Compare employees
    $names_comparison = strcmp($a->employees_names, $b->employees_names);
    if (0 == $names_comparison) {
        // Compare times
        if ($a->timestamp < $b->timestamp) {
            return -1;
        } elseif ($a->timestamp > $b->timestamp) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return $names_comparison;
    }
}

// Used to measure time of execution
function _tallantoschedule_microtime_float()
{
    list($usec, $sec) = explode(" ", microtime());
    return ((float)$usec + (float)$sec);
}

// Filter events using user settings
function listValidEvents() {
    // Get parsed data
    $tallantoschedule_parsed_data = variable_get('tallantoschedule_parsed_data', array());

    // Valid events
    $valid_events = array();

    // Iterate events and pick with IDs matching parent
    foreach ($tallantoschedule_parsed_data['valid_parent_events'] as $parent_event_id => $placeholder) {
        foreach ($tallantoschedule_parsed_data['valid_events'] as $tallanto_event) {
            if ($parent_event_id == $tallanto_event->subject_id) {
                $valid_events[] = $tallanto_event;
            }
        }
    }

    return $valid_events;
}

// Analyze event array
function _tallantoschedule_analyze_event_array(&$event_array) {
    $total = count($event_array);
    $with_parents = 0;
    $without_parents = 0;
    $standalone = 0;
    foreach ($event_array as $event_id => $event_item) {
        if (strlen($event_item->parent_event_id) == 0) {
            $without_parents++;
        } else {
            $with_parents++;
        }
        if ($event_item->standalone) {
            $standalone++;
        }
    }
    
    return array(
            'total' => $total,
            'with_parents' => $with_parents,
            'without_parents' => $without_parents,
            'standalone' => $standalone,
        );
}