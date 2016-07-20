<?php

/**
 * Class TallantoSubject
 */
class TallantoSubject {
  /**
   * @var
   */
  protected $id;
  /**
   * @var
   */
  protected $url;
  /**
   * @var
   */
  protected $title;

  /**
   * TallantoSubject constructor.
   *
   * @param $id
   * @param $url
   * @param $title
   */
  public function __construct($id, $url, $title) {
    $this->id = $id;
    $this->url = $url;
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
  public function getUrl() {
    return $this->url;
  }

  /**
   * @return mixed
   */
  public function getTitle() {
    return $this->title;
  }
}