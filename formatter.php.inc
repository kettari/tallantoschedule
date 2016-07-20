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
  }
  else {
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

    for ($i = 0; $i < 7; $i++) {
      $day_of_week = date('N', $today + ($i * 60 * 60 * 24));
      switch ($day_of_week) {
        case 1:
          $day_text = t('Monday');
          break;
        case 2:
          $day_text = t('Tuesday');
          break;
        case 3:
          $day_text = t('Wednesday');
          break;
        case 4:
          $day_text = t('Thursday');
          break;
        case 5:
          $day_text = t('Friday');
          break;
        case 6:
          $day_text = t('Saturday');
          break;
        case 7:
          $day_text = t('Sunday');
          break;
      }
      $valid_dates[] = $day_text . date($format, $today + ($i * 60 * 60 * 24));
    }
  }
  else {
    $format = 'Y-m-d';

    for ($i = 0; $i < 7; $i++) {
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

  //print_r($selected_events);die('listTallantoSchedule() before ksort()');

  // Make sure all week days are present
  $example_week = array(
    1 => array(),
    2 => array(),
    3 => array(),
    4 => array(),
    5 => array(),
    6 => array(),
    7 => array(),
  );
  $selected_events = $selected_events + $example_week;

  // Sort events so 'today' always first
  ksort($selected_events);

  //print_r($selected_events);die('listTallantoSchedule() after ksort()');

  // Reorder days
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

  // Text strings for the 'sign up'
  $signup_text_not_opened = t('Not opened yet');
  $signup_text_closed = t('Closed');
  $signup_text_signup = t('Sign up');

  // If we want to customize...
  $use_custom_signup_text = variable_get('tallantoschedule_subject_custom_text_check_' . $tallanto_event->subject_id, 0);
  if ($use_custom_signup_text) {
    $signup_text_not_opened = $signup_text_signup = variable_get('tallantoschedule_signup_custom_text', t('Please call')); // Yes we skip 'closed' because closed means always closed
  }

  $tallantoschedule_signup_min = variable_get('tallantoschedule_signup_min', 30) * 60;
  $tallantoschedule_signup_max = variable_get('tallantoschedule_signup_max', 4320) * 60;
  $far_cutoff_timestamp = time() + $tallantoschedule_signup_max;
  if (variable_get('tallantoschedule_signup_roundup', 1)) {
    $far_cutoff_timestamp = mktime(23, 59, 59,
      date('m', $far_cutoff_timestamp), date('d', $far_cutoff_timestamp), date('Y', $far_cutoff_timestamp));
  }
  if ($tallanto_event->timestamp > $far_cutoff_timestamp) {                           // Too far in the future
    return $signup_text_not_opened;
  }
  elseif ($tallanto_event->timestamp < (time() + $tallantoschedule_signup_min)) {   // Too late already
    return $signup_text_closed;
  }
  else {                                                                            // ...and here we go
    $url = str_replace('@EVENT_ID@', $tallanto_event->id, variable_get('tallantoschedule_signup_url', '@EVENT_ID@'));

    $cookie_keys = array(
      'beforekaula'      => 'Источник перехода',
      'firstpageaddress' => 'Страница входа на сайт',
    );
    $for_new_description = '';
    foreach ($cookie_keys as $key => $lang) {
      $value = $_COOKIE[$key];
      $value = urldecode($value);
      if (empty($value)) {
        //@todo Костыль так как если первая страница с расписанием, то куки еще не заполены и может произойти беда
        if ($key == 'beforekaula') {
          $value = $_SERVER['HTTP_REFERER'];
          $_COOKIE['firstpageaddress'] = $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'];
        }
        else {
          $value = 'empty';
        }
      }
      if ($key == 'beforekaula') {
        $query = explode('?', $value);
        $search_domain = reset($query);
        foreach ($query as $q_val) {
          $q_val = explode('=', $q_val);
          if (sizeof($q_val) == 2 && $q_val[0] == 'q') {
            $value = $search_domain . "\nПоисковый запрос: " . $q_val[1];
            break;
          }
        }
      }
      $for_new_description .= $lang . ": " . $value . "\n";
    }

    $for_new_description = urlencode($for_new_description);
    $url .= '&for_new_description=' . $for_new_description;
    $result = sprintf('<a href="%s">%s</a>', $url, $signup_text_signup);

    // Custom sign up text? Avoid URLs
    if ($use_custom_signup_text) {
      $result = $signup_text_signup;
    }
    return $result;
  }
}

// Transliterator (for CSS classes for example)
function _tallantoschedule_rus2translit($string) {
  $converter = array(
    'а' => 'a',
    'б' => 'b',
    'в' => 'v',
    'г' => 'g',
    'д' => 'd',
    'е' => 'e',
    'ё' => 'e',
    'ж' => 'zh',
    'з' => 'z',
    'и' => 'i',
    'й' => 'y',
    'к' => 'k',
    'л' => 'l',
    'м' => 'm',
    'н' => 'n',
    'о' => 'o',
    'п' => 'p',
    'р' => 'r',
    'с' => 's',
    'т' => 't',
    'у' => 'u',
    'ф' => 'f',
    'х' => 'h',
    'ц' => 'c',
    'ч' => 'ch',
    'ш' => 'sh',
    'щ' => 'sch',
    'ь' => '',
    'ы' => 'y',
    'ъ' => '',
    'э' => 'e',
    'ю' => 'yu',
    'я' => 'ya',

    'А' => 'A',
    'Б' => 'B',
    'В' => 'V',
    'Г' => 'G',
    'Д' => 'D',
    'Е' => 'E',
    'Ё' => 'E',
    'Ж' => 'Zh',
    'З' => 'Z',
    'И' => 'I',
    'Й' => 'Y',
    'К' => 'K',
    'Л' => 'L',
    'М' => 'M',
    'Н' => 'N',
    'О' => 'O',
    'П' => 'P',
    'Р' => 'R',
    'С' => 'S',
    'Т' => 'T',
    'У' => 'U',
    'Ф' => 'F',
    'Х' => 'H',
    'Ц' => 'C',
    'Ч' => 'Ch',
    'Ш' => 'Sh',
    'Щ' => 'Sch',
    'Ь' => '',
    'Ы' => 'Y',
    'Ъ' => '',
    'Э' => 'E',
    'Ю' => 'Yu',
    'Я' => 'Ya',
  );
  return strtr($string, $converter);
}

// Transliterate CSS class
function _tallantoschedule_translit_css_class($string) {
  $result = strtolower(_tallantoschedule_rus2translit($string));

  $converter = array(
    ' ' => '-',
  );

  return strtr($result, $converter);
}
