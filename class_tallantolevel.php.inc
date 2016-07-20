<?php

/**
 * Created by PhpStorm.
 * User: Ант
 * Date: 20.07.2016
 * Time: 13:59
 */
class TallantoLevel {
  protected $id;
  protected $title;

  /**
   * TallantoTeacher constructor.
   *
   * @param $id
   * @param $title
   */
  public function __construct($id, $title) {
    $this->id = $id;
    $this->title = $title;
  }

  /**
   * @return mixed
   */
  public function getId() {
    return $this->id;
  }

  /**
   * @return mixed
   */
  public function getTitle() {
    return $this->title;
  }

}