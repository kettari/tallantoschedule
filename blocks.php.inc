<?php

// blocks.inc.php

/**
 * Implementation of hook_block().
 * @param string $op one of "list", "view", "save" and "configure"
 * @param integer $delta code to identify the block
 * @param array $edit only for "save" operation
*/
function tallantoschedule_block($op = 'list', $delta = 0, $edit = array()) {  
    $block = array();
    switch ($op) {
        case "list":
            // Generate listing of blocks from this module, for the admin/block page
            $block = listTallantoScheduleBlocks();
            break;
        case "view":
            $block['content'] = viewTallantoScheduleBlock($delta);
            break;
        case "save":
            break;
        case "configure":
            break;
    }
    return $block;
} // function tallantoschedule_block

// Get an array of blocks using user selected branches
function listTallantoScheduleBlocks() {
    $blocks = array();

    // Get branches
    $tallantoschedule_parsed_data = variable_get('tallantoschedule_parsed_data', array());

    foreach ($tallantoschedule_parsed_data['valid_branches'] as $branch => $placeholder) {
        // Is branch selected?
        $safe_branch = preg_replace("/[^a-zA-Z0-9]+/", "", $branch);
        if (!variable_get('tallantoschedule_branch_check_' . $safe_branch, 0)) {
            continue;
        }

        $the_block = array();
        $the_block['info'] = sprintf('%s: %s', t('Tallanto Schedule'), variable_get('tallantoschedule_branch_title_' . $safe_branch, $branch));
        $the_block['cache'] = BLOCK_NO_CACHE;
        $blocks[] = $the_block;

        // Save block index to prevent mixup
        variable_set('tallantoschedule_branch_block_' . (count($blocks) - 1), $branch);
    }

    return $blocks;
}

// Get content for the branch block
function viewTallantoScheduleBlock($delta = 0) {
    $content = '';
    
    // Get branches
    $tallantoschedule_parsed_data = variable_get('tallantoschedule_parsed_data', array());
    
    // Find branch connected to this block
    $branch = variable_get('tallantoschedule_branch_block_' . $delta, FALSE);
    if ($branch === FALSE) {
        drupal_set_message(t('Requested Tallanto Schedule block is not configured (delta: @delta)', array('@delta' => $delta)));
    } else {
        $header = array(
            array('data' => t('Time'), 'width' => '15%', 'style' => 'font-size: 0.875em;'),
            array('data' => t('Subject'), 'width' => '31%', 'style' => 'font-size: 0.875em;'),
            array('data' => t('Teacher'), 'width' => '21%', 'style' => 'font-size: 0.875em;'),
            array('data' => t('Level'), 'width' => '17%', 'style' => 'font-size: 0.875em;'),
            array('data' => t('Sign up'), 'width' => '*', 'style' => 'font-size: 0.875em;'),
        );
        $data = array();
        
        // Levels
        $difficulty_levels = preg_split('/\n|\r\n?/', variable_get('tallantoschedule_difficulty_levels', t('Suitable for all')));
        
        // Days of week
        $valid_dates = listValidDates(TRUE);
        $i = 0;

        // Get schedule prepared
        $events_week = listTallantoSchedule($branch);
        
        foreach ($events_week as $events_day) {
            foreach ($events_day as $event_item) {
                
                // Prepare subject
                $subject = '';
                $subject_url = variable_get('tallantoschedule_subject_url_' . $event_item->subject_id, '');
                $subject_title = variable_get('tallantoschedule_subject_visibletitle_' . $event_item->subject_id, $tallantoschedule_parsed_data['valid_subjects'][$event_item->subject_id]);
                if ($subject_url != '') {
                    if ((substr($subject_url, 0, 5) != 'http:') && (substr($subject_url, 0, 6) != 'https:')) {
                        $subject_url = 'http://' . $subject_url;
                    }
                    $subject = sprintf('<a href="%s">%s</a>', $subject_url, $subject_title);
                } else {
                    $subject = $subject_title;
                }
                
                // Prepare teacher
                $teacher = '';
                $teacher_url = variable_get('tallantoschedule_teacher_url_' . $event_item->employees_ids, '');
                if ($teacher_url != '') {
                    if ((substr($teacher_url, 0, 5) != 'http:') && (substr($teacher_url, 0, 6) != 'https:')) {
                        $teacher_url = 'http://' . $teacher_url;
                    }
                    $teacher = sprintf('<a href="%s">%s</a>', $teacher_url, $tallantoschedule_parsed_data['valid_employees'][$event_item->employees_ids]);
                } else {
                    $teacher = $tallantoschedule_parsed_data['valid_employees'][$event_item->employees_ids];
                }
                
                // Prepare level
                if ($event_item->parent_event_id != '') {
                    $level = variable_get('tallantoschedule_parent_event_level_'. $event_item->parent_event_id, 0);
                } else {
                    $level = variable_get('tallantoschedule_parent_event_level_'. $event_item->id, 0);
                }
                
                // Prepare CSS class name
                $css_class = _tallantoschedule_translit_css_class($tallantoschedule_parsed_data['valid_subjects'][$event_item->subject_id]);
                
                $data[] = array(
                    array('data' => formatEventDate($event_item), 'style' => 'font-size: 0.875em;', 'class' => $css_class),
                    array('data' => $subject, 'style' => 'font-size: 0.875em;', 'class' => $css_class),
                    array('data' => $teacher, 'style' => 'font-size: 0.875em;', 'class' => $css_class),
                    array('data' => $difficulty_levels[$level], 'style' => 'font-size: 0.875em;', 'class' => $css_class),
                    array('data' => formatSignUpUrl($event_item), 'style' => 'font-size: 0.875em;', 'class' => $css_class),
                );
            }
            
            $content .= sprintf('<h2>%s</h2>', $valid_dates[$i++]) . theme_table($header, $data, array('width' => '647px'));
            
            // Cleanup data
            $data = array();
        }
        
        //print_r($difficulty_levels);
        //print_r($events_week);die;
        
        
    }
    
    return $content;
}
