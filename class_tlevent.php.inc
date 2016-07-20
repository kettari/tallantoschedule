<?php

// class_tlevent.php.inc

class TallantoEvent {
  public $id = 0;
  public $start_date = '';
  public $start_time = '';
  public $end_date = '';
  public $end_time = '';
  public $subject_id = '';
  public $employees_ids = '';
  public $parent_event_id = '';
  public $branch = '';
  public $timestamp = 0;
  public $employees_names = '';
  public $standalone = FALSE; // Standalone -- event without parent ID

  function __construct($id, $start_date, $start_time, $end_date, $end_time, $subject_id, $employees_ids, $parent_event_id, $branch) {
    $this->id = $id;
    $this->start_date = $start_date;
    $this->start_time = $start_time;
    $this->end_date = $end_date;
    $this->end_time = $end_time;
    $this->subject_id = $subject_id;
    $this->employees_ids = $employees_ids;
    $this->parent_event_id = $parent_event_id;
    $this->branch = $branch;
    $this->timestamp = mktime(
      substr($this->start_time, 0, 2),
      substr($this->start_time, 3, 2),
      0,
      substr($this->start_date, 5, 2),
      substr($this->start_date, 8, 2),
      substr($this->start_date, 0, 4)
    );
  }

  // Resolve employees_ids to name
  function resolve_employees_names($valid_employees) {
    $this->employees_names = $valid_employees[$this->employees_ids];
  }
}

/*
see http://stackoverflow.com/questions/965611/forcing-access-to-php-incomplete-class-object-properties

This issue appends when you un serialize an object of a class that hasn't been included yet. 
For exemple, if you call session_start before include the class.

A PHPIncompleteClass object can't be accessed directly, but it's ok with foreach, serialize 
and gettype. Calling is_object with an PHPIncompleteClass object will result false.

So, if you find a '__PHP_Incomplete_Class' object in your session and you've included your 
class after the session_load, you can use this function :
*/
function fixObject(&$object) {
  if (!is_object($object) && gettype($object) == 'object') {
    return ($object = unserialize(serialize($object)));
  }
  return $object;
}