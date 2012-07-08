<?php

/**
 * @file
 *   Voip user module user settings pages and forms.
 */

/**
 * Menu callback: User number settings page.
 */
function voipusernumber_settings_page($account) {
  $content = array();

  if (voipusernumber_edit_access($account)) {
    $content['add'] = l(t('Add number'), 'user/' . $account->uid . '/edit/voipnumber/add');
  }

  $content['numbers'] = drupal_get_form('voipusernumber_settings_numbers_form', $account);

  return theme('voipusernumber_settings_page', $content);
}

/**
 * Form callback: User phone numbers - set default.
 */
function voipusernumber_settings_numbers_form(&$form_state, $account) {
  $form = array();
  $form['uid'] = array('#type' => 'value', '#value' => $account->uid);

  // This would/should be loaded onto $account, but allowing for
  // use of form without user_load:
  $numbers = VoipUserNumber::getNumbersFromUid($account->uid);
  foreach ($numbers as $number) {
    $row = array();
    $row['number'] = array(
      '#type' => 'item',
      '#title' => t('Number'),
      '#value' => check_plain($number->getNumber()),
      '#description' => t('Phone number.'),
    );
    // Need check boxes / radios / links to change
    $row['active'] = array(
      '#type' => 'item',
      '#title' => t('Active'),
      '#value' => $number->getActive(),
      '#description' => t('Phone number available for calls.'),
    );
    $row['confirmed'] = array(
      '#type' => 'item',
      '#title' => t('Confirmed'),
      '#value' => $number->getConfirmed(),
      '#description' => t('Number confirmed.'),
    );
    $row['default'] = array(
      '#type' => 'item',
      '#title' => t('Default'),
      '#value' => $number->getDefault(),
      '#description' => t('The default number.'),
    );
    $row['delete'] = array(
      '#type' => 'item',
      '#title' => t('Delete'),
      '#value' => l(t('Delete'), 'user/' . $account->uid . '/edit/voipnumber/' . $number->getNumber() . '/delete'),
      '#access' => voipusernumber_edit_access($account),
    );
    $form[$number->getNumber()] = $row;
  }

  if (count($form) > 1) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );
  }

  return $form;
}

/**
 * Form constructor for user add number page.
 *
 * @see voipusernumber_settings_add_form_validate()
 * @see voipusernumber_settings_add_form_submit()
 * @ingroup forms

function voipusernumber_settings_add_form(&$form_state, $account) {
  $form = array();
  $form['uid'] = array('#type' => 'value', '#value' => $account->uid);
  // would be good to have a standard (outside CCK implementation of voipnumber
  // elements?
 
 /  possible implementation using multi-step form, to break validation in steps
  
  if (!empty($form_state['storage']['step'])) {
    $function = $form_state['storage']['step'];
    return $function($form_state);
  }
  else {
    return _add_number();
  }
}
function _add_number() {
  $form['add']['number'] = array(
    '#type' => 'textfield',
    '#title' => t('Phone number'),
    '#required' => TRUE,
  );
  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => 'Continue',
  );
  $form['this_step'] = array(
    '#type' => 'value',
    '#value' => 'start',
  );
  $form['step_next'] = array(
    '#type' => 'value',
    '#value' => '_get_pin',
  );
  $form['#validate'] = '_add_number_validate';
  return $form;
}

function _get_pin() {
  $form['add']['pin'] = array(
    '#type' => 'textfield',
    '#title' => t('PIN'),
    '#description'=> t('Verify this pin over phone'),
    '#required' => TRUE,
  );
  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => 'Verify',
  );
  $form['this_step'] = array(
    '#type' => 'value',
    '#value' => 'pin',
  );
  $form['#validate'] = '_get_pin_validate';

  / aaha implemtation 
   $form['add']['apin'] = array(
   '#type' => 'submit',
   '#value' => t('verify'),
   '#description' => t("Verify code."),
   '#weight' => 1,
   //    '#submit' => array('poll_more_choices_submit'),
   '#ahah' => array(
   'event' => 'click',
   'path' => 'voipusernumber/js',
   'wrapper' => 'add-wrapper',
   'method' => 'replace',
   'effect' => 'fade',
   ),
   );

   $form['add']['submit'] = array(
   '#type' => 'submit',
   '#value' => t('Add'),
   );
   
   /ahaha ends here
   
  return $form;
}


function _add_number_validate($form, &$form_state) {
  print dsm($form_state);
}
function _get_pin_validate($form, &$form_state) {
  print dsm($form_state);
  }
  */
/**
 * callback function for voipusernumber/js
 * The return HTML will be outputted by AHAH

 function voipusernumber_js() {
 //module_load_include('inc', 'voipusernumber', 'voipusernumber.user');
 $output = 'Hello Drupal World';
 //print_r(drupal_json(array('status' => TRUE, 'data' => $output)));exit;
 drupal_json(array('status' => TRUE, 'data' => $output));
 }
 */

 function voipusernumber_settings_add_form(&$form_state, $account) {
  $form = array();
  $form['uid'] = array('#type' => 'value', '#value' => $account->uid);
  $pin = rand(1000,9999);
  // would be good to have a standard (outside CCK implementation of voipnumber
  // elements?
  $form['add']['number'] = array(
    '#type' => 'textfield',
    '#title' => t('Phone number'),
    '#default_value' => '', // do we ever want one
    '#required' => TRUE,
  );
  $form['add']['pin'] = array(
    '#type' => 'item',
    '#title' => t('PIN'),
    '#description'=> t('Verify this pin over phone'),
    '#value' => $pin,
  );  
  $form['add']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Call to verify'),
  );  
  $form['add']['hangup'] = array(
    '#type' => 'submit',
    '#value' => t('Hang up'),
  );  
  return $form;
}


/**
 * Form validation handler for voipusernumber_settings_add_form().
 *
 * @see voipusernumber_settings_add_form_submit().
 *
 */
function voipusernumber_settings_add_form_validate($form, &$form_state) {
  $error = '';
  $default='';
  variable_set('pin_variable_ask', $form['add']['pin']['#value']);
  $pin= variable_get('pin_variable_ask',$default);
  // Basic VoIPNumber validation.
  _voipnumber_valid_int_phone_number($form_state['values']['number'], $error);
  if (! empty($error)) {
    form_set_error('number', t('Phone number is %error', array('%error' => $error)));
  }
  // Unique validation.
  if (VoipUserNumber::getUidFromNumber($form_state['values']['number'])) {
    form_set_error('number', t('Phone number is already registered.'));
  }
  module_load_include('inc', 'voipusernumber', 'voipusernumber.script');
  _voipusernumber_auth_script();
  
    if(_voipusernumber_validate_pin() == FALSE) {
    form_set_error('pin', t('Wrong pin.'));
  }
  //exit;
/** For user merge
  *
      $no_to_be_added = $form_state['values']['number'];
  $assoc_phone = db_result(db_query("select count(*) as nop From voipusernumber_numbers where phone=%d",$no_to_be_added));
  if ($assoc_phone = 0){
    echo "hello";
  }
  */
}

/**
 * Form submission handler for voipusernumber_settings_add_form().
 *
 * @see voipusernumber_settings_add_form_validate().
 */
function voipusernumber_settings_add_form_submit($form, &$form_state) {
  $voipnumber = new VoipUserNumber($form_state['values']['number'], $form_state['values']['uid']);
  drupal_write_record('voipusernumber_numbers', $voipnumber);
  $account->voipnumber[$voipnumber->getNumber()] = $voipnumber;
}

/**
 * Form callback: User delete number page.
 *
 * @ingroup forms
 * @see voipusernumber_settings_delete_form_submit().
 */
function voipusernumber_settings_delete_form(&$form_state, $account, $number) {
  $form = array();
  $form['uid'] = array('#type' => 'value', '#value' => $account->uid);
  $form['number'] = array('#type' => 'value', '#value' => $number);

  return confirm_form(
  $form,
  t('Are you sure you want to delete phone number %number?', array('%number' => $number)),
  isset($_GET['destination']) ? $_GET['destination'] : 'user/' . $account->uid . '/edit/voipnumber',
  NULL,
  t('Delete'),
  t('Cancel')
  );
}

/**
 * Form submission handler for
 * voipusernumber_settings_delete_form().
 */
function voipusernumber_settings_delete_form_submit($form, &$form_state) {
  if ($form_state['clicked_button']['#value'] == t('Delete')) {
    voipusernumber_delete_number($form_state['values']['uid'], $form_state['values']['number']);
  }
  drupal_goto();
}

/**
 * Theme user settings page.
 *
 * @param array $content
 *   'add' link and 'numbers' form.
 *
 * @ingroup themeable
 */
function theme_voipusernumber_settings_page($content) {
  $output = '<div class="voipusernumber-add">' . $content['add'] . '</div>';
  $output .= $content['numbers'];
  return $output;
}
// user merge implementation
function voipusernumber_user_merge() {
}