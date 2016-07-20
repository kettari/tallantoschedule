<?php
/**
 * Created by PhpStorm.
 * User: Ант
 * Date: 20.07.2016
 * Time: 11:39
 */


/**
 * Class SimpleXMLExtended
 */
class SimpleXMLExtended extends SimpleXMLElement {
  public function addCData($cdata_text) {
    $node = dom_import_simplexml($this);
    $no = $node->ownerDocument;
    $node->appendChild($no->createCDATASection($cdata_text));
  }
}


/**
 * Generate XML document of the schedule
 */
function tallantoschedule_api_axport() {
  // Get branches
  $tallantoschedule_parsed_data = variable_get('tallantoschedule_parsed_data', array());

  $xml_schedule = new SimpleXMLElement('<schedule></schedule>');
  $xml_schedule->addAttribute('server_timestamp', time());
  $xml_schedule->addAttribute('cached_timestamp', variable_get('tallantoschedule_cache_timestamp', 0));
  $xml_schedule->addAttribute('expires_timestamp',
    variable_get('tallantoschedule_cache_timestamp', 0)
    + variable_get('tallantoschedule_cache_timeout', 900));
  //$xml_schedule->addChild('raw_data', print_r($tallantoschedule_parsed_data, TRUE));
  $xml_branches_schedule = $xml_schedule->addChild('branches_schedule');

  // Levels
  $difficulty_levels = preg_split('/\n|\r\n?/',
    variable_get('tallantoschedule_difficulty_levels', t('Suitable for all')));

  // Dictionaries
  $branches_dict = array();
  $subjects_dict = array();
  $teachers_dict = array();
  $levels_dict = array();

  foreach ($tallantoschedule_parsed_data['valid_branches'] as $branch => $placeholder) {
    $safe_branch = preg_replace("/[^a-zA-Z0-9]+/", "", $branch);

    // Add branch to the dictionary
    $branches_dict[$branch] = new TallantoBranch(
      $branch,
      variable_get('tallantoschedule_branch_title_' . $safe_branch, $branch)
    );

    // Get schedule prepared
    $events_week = listTallantoSchedule($branch);

    // Add XML nodes
    $xml_events = $xml_branches_schedule->addChild('events');
    $xml_events->addAttribute('branch_id', $branch);

    // Process events day by day, item by item
    foreach ($events_week as $events_day) {
      foreach ($events_day as $event_item) {

        // Prepare subject
        $subject_url = variable_get('tallantoschedule_subject_url_' . $event_item->subject_id, '');
        $subject_title = variable_get('tallantoschedule_subject_visibletitle_' . $event_item->subject_id, $tallantoschedule_parsed_data['valid_subjects'][$event_item->subject_id]);
        $subject = new TallantoSubject($event_item->subject_id, $subject_url, $subject_title);
        $subjects_dict[$event_item->subject_id] = $subject;

        // Prepare teacher
        $teacher_url = variable_get('tallantoschedule_teacher_url_' . $event_item->employees_ids, '');
        if ($teacher_url != '') {
          if ((substr($teacher_url, 0, 5) != 'http:') && (substr($teacher_url, 0, 6) != 'https:')) {
            $teacher_url = 'http://' . $teacher_url;
          }
        }
        $teacher_title = $tallantoschedule_parsed_data['valid_employees'][$event_item->employees_ids];
        $teacher = new TallantoTeacher($event_item->employees_ids, $teacher_url, $teacher_title);
        $teachers_dict[$event_item->employees_ids] = $teacher;

        // Prepare level
        if ($event_item->parent_event_id != '') {
          $level_id = variable_get('tallantoschedule_parent_event_level_' . $event_item->parent_event_id, 0);
        }
        else {
          $level_id = variable_get('tallantoschedule_parent_event_level_' . $event_item->id, 0);
        }
        $level = new TallantoLevel($level_id, $difficulty_levels[$level_id]);
        $levels_dict[$level_id] = $level;

        // Sign up
        // If we want to customize...
        $use_custom_signup_text = variable_get('tallantoschedule_subject_custom_text_check_' . $event_item->subject_id, 0);
        // Check if time has come
        $tallantoschedule_signup_min = variable_get('tallantoschedule_signup_min', 30) * 60;
        $tallantoschedule_signup_max = variable_get('tallantoschedule_signup_max', 4320) * 60;
        $far_cutoff_timestamp = time() + $tallantoschedule_signup_max;
        if (variable_get('tallantoschedule_signup_roundup', 1)) {
          $far_cutoff_timestamp = mktime(23, 59, 59,
            date('m', $far_cutoff_timestamp), date('d', $far_cutoff_timestamp), date('Y', $far_cutoff_timestamp));
        }
        if ($event_item->timestamp > $far_cutoff_timestamp) {                           // Too far in the future
          $signup_open = FALSE;
        }
        elseif ($event_item->timestamp < (time() + $tallantoschedule_signup_min)) {   // Too late already
          $signup_open = FALSE;
        }
        else {
          $signup_open = TRUE;
        }

          // Add XML event node
        $xml_event = $xml_events->addChild('event');
        $xml_event->addAttribute('id', $event_item->id);
        $xml_event->addChild('start_date', sprintf('%s %s',
          $event_item->start_date, $event_item->start_time));
        $xml_event->addChild('end_date', sprintf('%s %s',
          $event_item->end_date, $event_item->end_time));
        $xml_event->addChild('subject_id', $subject->getId());
        $xml_event->addChild('teacher_id', $teacher->getId());
        $xml_event->addChild('level_id', $level->getId());
        $xml_event->addChild('signup_open', $signup_open ? 'true' : 'false');
        $xml_event->addChild('allow_self_signup', $use_custom_signup_text ? 'false' : 'true');

        /*
        $data[] = array(
          array(
            'data'  => formatEventDate($event_item),
            'style' => 'font-size: 0.875em;',
            'class' => $css_class . ' tl-date',
          ),
          array(
            'data'  => $subject,
            'style' => 'font-size: 0.875em;',
            'class' => $css_class . ' tl-subject',
          ),
          array(
            'data'  => $teacher,
            'style' => 'font-size: 0.875em;',
            'class' => $css_class . ' tl-teacher',
          ),
          array(
            'data'  => $difficulty_levels[$level],
            'style' => 'font-size: 0.875em;',
            'class' => $css_class . ' tl-difficulty',
          ),
          array(
            'data'  => formatSignUpUrl($event_item),
            'style' => 'font-size: 0.875em;',
            'class' => $css_class . ' tl-signup',
          ),
        );
        */
      }

      /*
      // Are there any events?
      if (count($events_day)) {
        $content .= sprintf('<h2>%s</h2>', $valid_dates[$i++]) . theme_table($header, $data, array(
            'width' => '646px',
            'class' => 'tallantoschedule_table',
          ));
      }
      else {
        $content .= sprintf('<h2>%s</h2><p>%s</p>', $valid_dates[$i++], t('No events on this day.'));
      }
      */

      // Cleanup data
      $data = array();
    }
  }

  // Add dictionaries to the XML
  // Branches dictionary
  $xml_branches = $xml_schedule->addChild('branches');
  foreach ($branches_dict as $item) {
    $xml_branch_item = $xml_branches->addChild('branch');
    $xml_branch_item->addAttribute('id', $item->getId());
    $xml_branch_item->addAttribute('title', $item->getTitle());
  }
  // Subjects dictionary
  $xml_subjects = $xml_schedule->addChild('subjects');
  foreach ($subjects_dict as $item) {
    $xml_subject_item = $xml_subjects->addChild('subject');
    $xml_subject_item->addAttribute('id', $item->getId());
    $xml_subject_item->addAttribute('url', $item->getUrl());
    $xml_subject_item->addAttribute('title', $item->getTitle());
  }
  // Teachers dictionary
  $xml_teachers = $xml_schedule->addChild('teachers');
  foreach ($teachers_dict as $item) {
    $xml_teacher_item = $xml_teachers->addChild('teacher');
    $xml_teacher_item->addAttribute('id', $item->getId());
    $xml_teacher_item->addAttribute('url', $item->getUrl());
    $xml_teacher_item->addAttribute('name', $item->getName());
  }
  // Levels dictionary
  $xml_levels = $xml_schedule->addChild('levels');
  foreach ($levels_dict as $item) {
    $xml_level_item = $xml_levels->addChild('level');
    $xml_level_item->addAttribute('id', $item->getId());
    $xml_level_item->addAttribute('title', $item->getTitle());
  }

  echo $xml_schedule->asXML();

  die;
}