<?php

/**
 * @file
 *   VoipUserNumber class file.
 */

/**
 * Unique VoIP User Number's instance of VoIP Number.
 *
 * This may add confusion but: (better way?)
 *
 * Static methods remain from parent, although those based
 * on a node context aren't relevant here. Parent methods
 * will return a VoipNumber.
 *
 * Additional status handling for VoIP User number types
 * required and added.
 *
 * To keep in line with database naming internal variables
 * are different as well. Allows for just saving object,
 * but...
 */
class VoipUserNumber extends VoipNumber {
  public $phone;
  public $uid;
  public $number_prefix;
  public $status;

  /**
   * Construct from arguments.
   *
   * @param string $phone
   *   Phone number.
   * @param string $uid
   *   Name of number. Here just use uid.
   * @param string $number_prefix
   *   Type, or prefex, of the number. Usually unknown. @todo confirm.
   * @param int $status
   *   VoIP User Number status.
   */
  function __construct($phone, $uid, $number_prefix = '', $status = 0) {
    $this->phone = $phone;
    $this->uid = $uid;
    $this->number_prefix = $number_prefix;
    $this->status = $status;
  }

  /**
   * Return unique id for a VoipUserNumber.
   *
   * This is specific to this module, so no hook, but database prodding.
   * Breaking the model of the parent object again.
   *
   * @param int|string $number
   *   Number to look up.
   *
   * @return int|bool
   *   User ID. FALSE if none.
   */
  public static function getUidFromNumber($number) {
    return db_result(db_query("SELECT uid FROM {voipusernumber_numbers} WHERE phone = '%s'", $number));
   }

  /**
   * Return default VoIP User number associated with a user.
   *
   * Overrides VoipNumber::getDefaultNumberFromUid(). This only returns the
   * first number from an array. We return the user chosen default number.
   *
   * @param int $uid
   *   The User ID.
   *
   * @return object
   *   VoipUserNumber object. The Default number if it exists. The first number
   *   if not. FALSE if there is are no user numbers associated with the UID.
   */
  public static function getDefaultNumberFromUid($uid) {
    $numbers = self::getNumbersFromUid($uid);
    if (! empty($numbers)) {
      if (! empty($numbers['default'])) {
        return $numbers['default'];
      }
      else {
        return current($numbers);
      }
    }
    else {
      return FALSE;
    }
  }

  /**
   * Return array of VoIP User numbers associated with a user.
   *
   * If you require all VoIP numbers use
   * VoipNumber::getNumbersFromUid().
   *
   * @todo This gets called a lot? What by? Cache?
   *
   * @param int $uid
   *   The User ID.
   *
   * @return object
   *   Keyed array of VoipUserNumber Objects. Keyed by phone number, with
   *   a duplicate 'default' keyed number. An empty array is returned if there
   *   are no numbers.
   */
  public static function getNumbersFromUid($uid) {
    $numbers = array();

    $fields = drupal_schema_fields_sql('voipusernumber_numbers');
    $result = db_query('SELECT ' . implode(',', $fields) . ' FROM {voipusernumber_numbers} WHERE uid = %d', $uid);
    while ($row = db_fetch_object($result)) {
      $numbers[$row->phone] = new VoipUserNumber($row->phone, $row->uid, $row->number_prefix, $row->status);
      if ($row->status & VOIPUSERNUMBER_DEFAULT) {
        $numbers['default'] = $numbers[$row->phone];
      }
    }

    return $numbers;
  }

  /**
   * Override get node id as it makes no sense in this context.
   */
  public function getNid() {
    return FALSE;
  }

  /**
   * Override get name, as we're using UID.
   */
  public function getName() {
    return $this->uid;
  }

  /**
   * Override get number.
   *
   * @todo could move schema to use 'number' rather than phone.
   */
  public function getNumber() {
    return $this->phone;
  }

  /**
   * Make number in/active.
   *
   * @param bool $active
   *   TRUE active; FALSE inactive.
   */
  public function setActive($active) {
    if ($active) {
      $this->status = $this->status | VOIPUSERNUMBER_ACTIVE;
    }
    else {
      $this->status = $this->status ^ VOIPUSERNUMBER_ACTIVE;
    }
  }

  /**
   * Make number un/confirmed.
   *
   * @param bool $confirmed
   *   (optional) TRUE confirmed, default; FALSE unconfirmed.
   */
  public function setConfirmed($confirmed = TRUE) {
    if ($confirmed) {
      $this->status = $this->status | VOIPUSERNUMBER_CONFIRMED;
    }
    else {
      $this->status = $this->status ^ VOIPUSERNUMBER_CONFIRMED;
    }
  }

  /**
   * Make number (not) default.
   *
   * NOTE at the moment this isn't handling removing default
   * status from any other numbers attached to this uid.
   *
   * @param bool $default
   *   (optional) TRUE default, default; FALSE not default.
   */
  public function setDefault($default) {
    if ($default) {
      $this->status = $this->status | VOIPUSERNUMBER_DEFAULT;
    }
    else {
      $this->status = $this->status ^ VOIPUSERNUMBER_DEFAULT;
    }
  }

  /**
   * Get active status.
   *
   * @return bool
   *   TRUE if active.
   */
  public function getActive() {
    return (bool) $this->status & VOIPUSERNUMBER_ACTIVE;
  }

  /**
   * Get confirmed status.
   *
   * @return bool
   *   TRUE if confirmed.
   */
  public function getConfirmed() {
    return (bool) $this->status & VOIPUSERNUMBER_CONFIRMED;
  }

  /**
   * Get default status.
   *
   * @return bool
   *   TRUE if this is a default number.
   */
  public function getDefault() {
    return (bool) $this->status & VOIPUSERNUMBER_DEFAULT;
  }
}
