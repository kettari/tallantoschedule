<?php

// class_tlevent.php.inc

class TallantoEvent {
    public $id = 0;
    public $start_date = '';
    public $start_time = '';
    public $subject_id = '';
    public $employees_ids = '';
    public $parent_event_id = '';
    public $branch = '';
    
    function __construct($id, $start_date, $start_time, $subject_id, $employees_ids, $parent_event_id, $branch) {
        $this->id = $id;
        $this->start_date = $start_date;
        $this->start_time = $start_time;
        $this->subject_id = $subject_id;
        $this->employees_ids = $employees_ids;
        $this->parent_event_id = $parent_event_id;
        $this->branch = $branch;
    }
}
