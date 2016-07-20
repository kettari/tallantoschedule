<?php

/**
 * Created by PhpStorm.
 * User: Ант
 * Date: 20.07.2016
 * Time: 13:00
 */
class TallantoTeacher {

  protected $id;
  protected $url;
  protected $name;

  /**
   * TallantoTeacher constructor.
   *
   * @param $id
   * @param $url
   * @param $name
   */
  public function __construct($id, $url, $name) {
    $this->id = $id;
    $this->url = $url;
    $this->name = $name;
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
  public function getUrl() {
    return $this->url;
  }

  /**
   * @return mixed
   */
  public function getName() {
    return $this->name;
  }
}