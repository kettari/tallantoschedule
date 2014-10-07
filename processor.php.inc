<?php

// processor.php.inc

// Check if XML with schedule is outdated and load if yes
function tallantoschedule_boot() {
	if ((variable_get('tallantoschedule_cache_timestamp', 0) + variable_get('tallantoschedule_cache_timeout', 0)) < time()) {
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
    $xml_data = file_get_contents(variable_get('tallantoschedule_source_url', ''));
    
    // Get end time
    $time_end = _tallantoschedule_microtime_float();
    $load_time = $time_end - $time_start;
    
    drupal_set_message(sprintf('XML loaded, time elapsed: %.1f seconds; XML raw size: %.1f MB', $load_time, (strlen($xml_data) / 1024 / 1024)));
    
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
        . "<subject_id><!\[CDATA\[(.+)\]\]></subject_id>.*"
        . "<employees_ids><!\[CDATA\[(.+)\]\]></employees_ids>.*"
        . "<event_pid><!\[CDATA\[(.*)\]\]></event_pid>.*"
        . "<filial><!\[CDATA\[(.+)\]\]></filial>"
        . ".*</event>@i";
    
    preg_match_all($pattern, $xml_data, $matches);
    
    // Copy records which fall into [today..today+6] days period
    
    // Prepare list of 'valid' dates
    $today = mktime(0, 0, 0, date('n'), date('j'), date('Y'));
    $valid_dates = array();
    for ($i=0;$i<7;$i++) {
        $valid_dates[] = date('Y-m-d', $today + ($i * 60 * 60 * 24));
    }
    //print_r($valid_dates);
    
    // Filter records
    $valid_events = array();
    for ($i=0;$i<count($matches[0])-1;$i++) {
        foreach ($valid_dates as $val_date) {
            if ($matches[2][$i] == $val_date) {
                $tallanto_event = new TallantoEvent(
                    $matches[1][$i], // event id
                    $matches[2][$i], // start_date
                    $matches[3][$i], // start_time - not in XML
                    $matches[4][$i], // subject_id
                    $matches[5][$i], // employees_ids
                    $matches[6][$i], // parent event id
                    $matches[7][$i]  // branch
                );
                $valid_events[] = $tallanto_event;
                break;
            }
        }
    }
    
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
    foreach ($valid_events as $tallanto_event) {
        $valid_branches[$tallanto_event->branch] = TRUE;
        if (strlen($tallanto_event->parent_event_id) > 0) {
            $valid_parent_events[$tallanto_event->parent_event_id] = TRUE;
        }
    }   
    
    // Pack all data
    $tallantoschedule_parsed_data['valid_events'] = $valid_events;
    $tallantoschedule_parsed_data['valid_subjects'] = $valid_subjects;
    $tallantoschedule_parsed_data['valid_employees'] = $valid_employees;
    $tallantoschedule_parsed_data['valid_branches'] = $valid_branches;
    $tallantoschedule_parsed_data['valid_parent_events'] = $valid_parent_events;
    
    //print_r($tallantoschedule_parsed_data);
    //print_r($valid_subjects);
    //print_r(substr($xml_data, 0, 16384));
    //die();
    
    // Get end time
    $time_end = _tallantoschedule_microtime_float();
    $parse_time = $time_end - $time_start;
    
    drupal_set_message(sprintf('XML parsed, time elapsed: %.1f seconds; event objects: %d; subjects: %d; employees: %d', 
        $parse_time, count($valid_events), count($valid_subjects), count($valid_employees)));
    
    variable_set('tallantoschedule_parsed_data', $tallantoschedule_parsed_data);
}

// Used to measure time of execution
function _tallantoschedule_microtime_float()
{
    list($usec, $sec) = explode(" ", microtime());
    return ((float)$usec + (float)$sec);
}