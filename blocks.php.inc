<?php

// blocks.inc.php
/**
 * Implementation of hook_block().
 *
 * @param string $op one of "list", "view", "save" and "configure"
 * @param integer $delta code to identify the block
 * @param array $edit only for "save" operation
 * @return array
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

  // Sort branches by key
  $valid_branches = $tallantoschedule_parsed_data['valid_branches'];
  ksort($valid_branches);

  foreach ($valid_branches as $branch => $placeholder) {
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//ДОБАВЛЕНИЕ ФИЛИАЛА
function Laddfilial($filial) {  //На выходе код  <option value="1">Тургеневская</option>
  $output = '<option value="' . $filial . '">';

  if ($filial == 0) {
    $output .= 'Тургеневская';
  }
  else {
    $output .= 'Семеновская';
  }

  $output .= '</option>';

  return $output;
}

//ДОБАВЛЕНИЕ ДНЯ
function Laddday($filial, $day, $name) {  //На выходе код  <option class="filial-1" value="1-1">$name</option>
  $output = '<option';
  $output .= ' class="filial-' . $filial . '"';  //код  class="filial-1"
  $output .= ' value="' . $filial . '-' . $day . '">';  //код  value="1-1">
  $output .= $name . '</option>';
  return $output;
}

//ДОБАВЛЕНИЕ ЗАНЯТИЯ
function Laddzanyatie($filial, $day, $data) {  //На выходе код  <option class="day-1-1" value="1-1-1">$name</option>
  $output = '';
  foreach ($data as $i => $name) {       // $i - номер занятия, $name - занятие
    $output .= '<option';
    $output .= ' class="day-' . $filial . '-' . $day . '"';  //код  class="day-1-1"
    $output .= ' value="' . $filial . '-' . $day . '-' . $i . '">';  //код  value="1-1-1">
    $output .= $name . '</option>';
  }
  return $output;
}

//ДОБАВЛЕНИЕ ССЫЛОК
function Laddhref($filial, $day, $datahref) {  //На выходе код  <li class="zanyatie-1-1-1">$name</li>
  $output = '';
  foreach ($datahref as $i => $name) {       // $i - номер занятие, $name - ссылка
    $output .= '<li';
    $output .= ' class="zanyatie-' . $filial . '-' . $day . '-' . $i . '">';  //код  class="zanyatie-1-1-1">
    $output .= $name . '</li>';
  }
  return $output;
}

//ПОИСК И ВЫВОД СПИСКА ЗАНЯТИЙ
function LvTSB() {
  // Get branches
  $tallantoschedule_parsed_data = variable_get('tallantoschedule_parsed_data', array());

  $branch = array(0, 1); // Массив
  $branch[0] = variable_get('tallantoschedule_branch_block_' . 0, FALSE);
  $branch[1] = variable_get('tallantoschedule_branch_block_' . 1, FALSE);

  $data = array();   //Массив занятий
  $datahref = array(); //Массив ссылок

  $difficulty_levels = preg_split('/\n|\r\n?/', variable_get('tallantoschedule_difficulty_levels', t('Suitable for all'))); //Уровни занятий

  $filials = ''; //Список филиалов
  $days = ''; //Список дней
  $zanyatie = ''; //Список занятий
  $hrefs = ''; //Список ссылок

  $fillinGlob = isset($GLOBALS['fillin']);
  if ($fillinGlob) {
    $filtr = $GLOBALS['fillin'];
  }

  for ($filial = 0; $filial <= 1; ++$filial)     //Цикл для филиалов
  {
    $truefil = FALSE;
    $valid_dates = listValidDates(TRUE);
    $i = 0;
    $events_week = listTallantoSchedule($branch[$filial]);   //Выбор филиала

    foreach ($events_week as $events_day) {     //Цикл для дней
      foreach ($events_day as $event_item) {  //Цикл для занятий
        if (substr(formatSignUpUrl($event_item), 0, 2) == "<a") //Если занятие проводится
        {
          // Предмет
          $subject = variable_get('tallantoschedule_subject_visibletitle_' . $event_item->subject_id, $tallantoschedule_parsed_data['valid_subjects'][$event_item->ect_id]);
          // Учитель
          $teacher = $tallantoschedule_parsed_data['valid_employees'][$event_item->employees_ids];

          // Prepare level
          if ($event_item->parent_event_id != '') {
            $level = variable_get('tallantoschedule_parent_event_level_' . $event_item->parent_event_id, 0);
          }
          else {
            $level = variable_get('tallantoschedule_parent_event_level_' . $event_item->id, 0);
          }

          // Фильтр
          if ($fillinGlob) {                       //Если указан фильтр
            $pos = stripos($subject . $difficulty_levels[$level], $filtr);    //Поиск слова
            $gofil = !($pos === FALSE);           //Если слово нейдено
          }
          else {
            $gofil = TRUE;
          }

          //Добавление занятий и ссылок в массивы
          if ($gofil) {
            $pos = strpos($subject, 'Каула-йога');
            if ($pos === FALSE) {
              $zanya = $subject;
            }
            else {
              $zanya = 'Каула-йога';
            }

            $data[] = formatEventDate($event_item) . ' ' . $zanya . ' (' . $difficulty_levels[$level] . ')';

            $datahref[] = formatSignUpUrl($event_item);
          } //endif
        } //endif
      } //endforeach
      if (count($data) > 0) {
        $zanyatie .= Laddzanyatie($filial, $i, $data); //Функция доб занятия
        $hrefs .= Laddhref($filial, $i, $datahref);  //Функция доб ссылок
        $days .= Laddday($filial, $i, sprintf('%s', $valid_dates[$i])); //Функция доб дня
        $truefil = TRUE;    //В этом филиале есть занятия
      }

      $i++;
      $data = array();
      $datahref = array();
    } //endforeach

    if ($truefil) {
      $filials .= Laddfilial($filial);
    } //Функция доб филиала
  } //endfor

  if ($filials == '') {
    $content = '<form id="zapis"><div class="no">Еще рано! Запись на это занятие откроется чуть позже, смотрите <a href="/schedule">Расписание</a>.</div></form>';
  }
  else {
    $begin = '<form id="zapis"><div>Записаться на занятия онлайн</div>'; //Начало формы
    $filials = '<p>Филиал</p> <select id="filial">' . $filials . '</select>'; //Список филиалов
    $days = '<p>День</p> <select id="day">' . $days . '</select>'; //Список дней
    $zanyatie = '<p>Ближайшее занятие</p> <select id="zanyatie">' . $zanyatie . '</select>'; //Список занятий
    $end = '<input type="button" value="Записаться"> </form>'; //Конец формы
    $content = $begin . $filials . $days . $zanyatie . $hrefs . $end; // Собираем форму
  }

  return $content;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Get content for the branch block
function viewTallantoScheduleBlock($delta = 0) {
  if ($GLOBALS['zapis'] == 1)  //ДЛЯ СТАТЕЙ
  {
    $content = LvTSB();
  }
  else {
    $content = '';

    // Get branches
    $tallantoschedule_parsed_data = variable_get('tallantoschedule_parsed_data', array());

    // Find branch connected to this block
    $branch = variable_get('tallantoschedule_branch_block_' . $delta, FALSE);
    if ($branch === FALSE) {
      drupal_set_message(t('Requested Tallanto Schedule block is not configured (delta: @delta)', array('@delta' => $delta)));
    }
    else {
      $header = array(
        array(
          'data'  => t('Time'),
          'width' => '15%',
          'style' => 'font-size: 0.875em;',
        ),
        array(
          'data'  => t('Subject'),
          'width' => '31%',
          'style' => 'font-size: 0.875em;',
        ),
        array(
          'data'  => t('Teacher'),
          'width' => '21%',
          'style' => 'font-size: 0.875em;',
        ),
        array(
          'data'  => t('Level'),
          'width' => '17%',
          'style' => 'font-size: 0.875em;',
        ),
        array(
          'data'  => t('Sign up'),
          'width' => '*',
          'style' => 'font-size: 0.875em;',
        ),
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
          }
          else {
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
          }
          else {
            $teacher = $tallantoschedule_parsed_data['valid_employees'][$event_item->employees_ids];
          }

          // Prepare level
          if ($event_item->parent_event_id != '') {
            $level = variable_get('tallantoschedule_parent_event_level_' . $event_item->parent_event_id, 0);
          }
          else {
            $level = variable_get('tallantoschedule_parent_event_level_' . $event_item->id, 0);
          }

          // Prepare CSS class name
          $css_class = _tallantoschedule_translit_css_class($tallantoschedule_parsed_data['valid_subjects'][$event_item->subject_id]);

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
        }

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

        // Cleanup data
        $data = array();
      }

      //print_r($difficulty_levels);
      //print_r($events_week);die;


    }
  }
  return $content;

}

// Print debug information
function tallantoschedule_debug() {
  $content = '';

  // Get branches
  $tallantoschedule_parsed_data = variable_get('tallantoschedule_parsed_data', array());

  // Find branch connected to this block
  $branch = variable_get('tallantoschedule_branch_block_' . '0', FALSE);
  if ($branch === FALSE) {
    drupal_set_message(t('Requested Tallanto Schedule block is not configured (delta: @delta)', array('@delta' => $delta)));
  }
  else {
    $header = array(
      array(
        'data'  => t('Time'),
        'width' => '15%',
        'style' => 'font-size: 0.875em;',
      ),
      array(
        'data'  => t('Subject'),
        'width' => '31%',
        'style' => 'font-size: 0.875em;',
      ),
      array(
        'data'  => t('Teacher'),
        'width' => '21%',
        'style' => 'font-size: 0.875em;',
      ),
      array(
        'data'  => t('Level'),
        'width' => '17%',
        'style' => 'font-size: 0.875em;',
      ),
      array(
        'data'  => t('Sign up'),
        'width' => '*',
        'style' => 'font-size: 0.875em;',
      ),
    );
    $data = array();

    // Levels
    $difficulty_levels = preg_split('/\n|\r\n?/', variable_get('tallantoschedule_difficulty_levels', t('Suitable for all')));

    // Days of week
    $valid_dates = listValidDates(TRUE);
    $i = 0;

    // Get schedule prepared
    $events_week = listTallantoSchedule($branch);

    //print_r('<pre>day_of_week=' . date('N') . '</pre>');
    //print_r($events_week);die('???');

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
        }
        else {
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
        }
        else {
          $teacher = $tallantoschedule_parsed_data['valid_employees'][$event_item->employees_ids];
        }

        // Prepare level
        if ($event_item->parent_event_id != '') {
          $level = variable_get('tallantoschedule_parent_event_level_' . $event_item->parent_event_id, 0);
        }
        else {
          $level = variable_get('tallantoschedule_parent_event_level_' . $event_item->id, 0);
        }

        // Prepare CSS class name
        $css_class = _tallantoschedule_translit_css_class($tallantoschedule_parsed_data['valid_subjects'][$event_item->subject_id]);

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
      }

      // Are there any events?
      if (count($events_day)) {
        $content .= sprintf('<h2>%s</h2>', $valid_dates[$i++]) . theme_table($header, $data, array(
            'width' => '647px',
            'class' => 'tallantoschedule_table',
          ));
      }
      else {
        $content .= sprintf('<h2>%s</h2><p>%s</p>', $valid_dates[$i++], t('No events on this day.'));
      }

      // Cleanup data
      $data = array();
    }

    //print_r($difficulty_levels);
    //print_r($events_week);die;


  }

  print_r($content);

  die;
}