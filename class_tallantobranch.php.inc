<?php

/**
 * Created by PhpStorm.
 * User: Ант
 * Date: 20.07.2016
 * Time: 13:29
 */
class TallantoBranch {
  /**
   * @var
   */
  protected $id;
  /**
   * @var
   */
  protected $title;

  /**
   * TallantoSubject constructor.
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