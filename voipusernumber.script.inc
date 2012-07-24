<?php

/**
 * @file
 *   Default VoIP scripts for interecting with VoIP user.
 */

/**
 * Automatically log in dialled-in user, by telephone number.
 *
 * If a user is related to the $number, or a user is created
 * they, will be logged into the Drupal global $user. The
 * script %uid and %username will also be set.
 *
 * If there is no user associated with the number, and none is created user
 * 0 (anonymous) will be stored in the variables.
 *
 * In the script that calls this script use of global $user
 * will return the value set at the initialization of the script. Further
 * scripts called and initialized after this script is called will have the
 * global available. The scrip variables %user_uid and %user_name values within
 * the calling script script.
 *
 * Example:
 *   $script->addGosub(
 *     'voipusernumber_autologin',
 *     array(
 *       'number' => '%caller_number',
 *       'create' => TRUE,
 *     )
 *   );
 *
 * @param string $number
 *   Phone number. For example from the script %caller_number variable.
 * @param bool $create
 *   (optional) If number is not known create a new user. Default FALSE.
 */
function _voipusernumber_autologin_script($number, $create = FALSE) {
  global $user;
  $script = new VoipScript('voipusernumber_autologin');
  // Autoload _should_ negate the need for this.
  module_load_include('inc', 'voipusernumber', 'voipusernumber.voipusernumber');

  $uid = VoipUserNumber::getUidFromNumber($number);
  if ($uid) {
    $user = user_load($uid);
    watchdog('voipusernumber', 'Voip call associated with user %name.', array('%name' => $user->name));
  }
  elseif ($create) {
    $uid = voipusernumber_user_create($number);
    $user = user_load($uid);
    watchdog('voipusernumber', 'Voip call account created and associated with user %name.', array('%name' => $user->name));
  }
  else {
    $user = user_load(0);
  }

  $script->addSet('user_uid', $user->uid);
  $script->addSet('user_name', $user->name);

  return $script;
}

/**
 * Log out user.
 *
 * Sets global $user and script %user_uid, %user_name to user 0.
 */
function _voipusernumber_logout_script() {
  $script = new VoipScript('voipusernumber_logout');
  global $user;
  watchdog('user', 'Voip call disassociated with user %name.', array('%name' => $user->name));
  $user = user_load(0);
  $script->addSet('user_uid', $user->uid);
  $script->addSet('user_name', $user->name);
  return $script;
}
/*
 * Script for making call to the phone no. which is going to be added.

function _voipusernumber_auth_script($num) {
  $start_time = time();
  $script = new VoipScript('voipusernumber_auth');
  $script-> addLabel('start');
  $call = new VoipCall();
  $call-> setDestNumber($num);
  $script_name = "voipusernumber_ask_pin";
  $script = VoipScript::loadScript($script_name);
  $call->setScript($script);
  $result = voip_dial($call);
  while(!$call->isHangup()) {
    sleep(2); 
    $call = VoipCall::load($call->getCid());
// Check Maximum execution time.
    if (time() - $start_time > 60) {
      watchdog('voipusernumber', 'Reached maximum execution time. Status: ' . var_export($call->getCallStatus(), TRUE));
      break;
    }
  }
  if ($result) {
     watchdog('voipusernumber', "call successfully queued for $num");
  }
  else {
     $msg = $call->getErrorMessage();
     $details = array('@msg' => $msg);
     watchdog('voipusernumber', 'Call failed with the following error: @msg',$details, WATCHDOG_WARNING);
  }
}
*/
/*
 * Script to verify pin, if the pin matches phone no. is added
 */
function _voipusernumber_ask_pin() {
    $form_pin= variable_get('pin_variable_ask','');
    $script = new VoipScript('voipusernumber_ask_pin');

// welcome script 
    $script->addLabel('welcome');
    $script->addSay('welcome to voxvox');
    $script->addGoto('getpin');
    
// get pin script
    $script->addLabel('getpin');    
    $prompt = 'Please enter your 4 digit Pin, and press pound key to confirm.';
    $script->addGetInput($prompt,4, '#' ,5);
    $script->addSet('pin',"%input_digits");
    $script->addSay(v('you entered %pin'));

//call a drupal function to vaildate the pin provided by the phone user
 //   $script->addSet('success', '^_voipusernumber_validate_pin(%pin)');
    $script->addGotoIf('success_message', "^%input_digits == '$form_pin");
    $script->addGotoIf('invalid_pin', "^%input_digits == ''");
    
    // Invalid pin messages.
    $script->addLabel('invalid_pin');
    $script->addSay('Invalid Pin number please try again.');
    $script->addGoto('getpin');

    // Correct pin given message.
    $script->addLabel('success_message');
    $script->addSet('success','^_voipusernumber_writerecord()');
    $script->addSay('Thank you. Phone number is successfully added.');
    $script->addHangup();
    return $script;
}