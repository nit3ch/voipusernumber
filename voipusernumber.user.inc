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
 */
function voipusernumber_settings_add_form(&$form_state, $account) {
  $form = array();
  $form['uid'] = array('#type' => 'value', '#value' => $account->uid);

  // would be good to have a standard (outside CCK implementation of voipnumber 
  // elements?
  $form['add']['number'] = array(
    '#type' => 'textfield',
    '#title' => t('Phone number'),
    '#default_value' => '', // do we ever want one
    '#required' => TRUE,
  );
  $form['add']['pin'] = array(
    '#type' => 'textfield',
    '#title' => t('PIN'),
    '#description'=> t('Verify this pin over phone'),
    '#default_value' => '', // do we ever want one
    '#required' => TRUE,
  );
  $form['add']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
  );

  return $form;
}

/**
 * Form validation handler for voipusernumber_settings_add_form().
 *
 * @see voipusernumber_settings_add_form_submit().
 */
function voipusernumber_settings_add_form_validate($form, &$form_state) {
  $error = '';
  // Basic VoIPNumber validation.
  _voipnumber_valid_int_phone_number($form_state['values']['number'], $error);
  if (! empty($error)) {
    form_set_error('number', t('Phone number is %error', array('%error' => $error)));
  }
  // Unique validation.
  if (VoipUserNumber::getUidFromNumber($form_state['values']['number'])) {
    form_set_error('number', t('Phone number is already registered.'));
  }
}

/**
 * Form submission handler for voipusernumber_settings_add_form().
 *
 * @see voipusernumber_settings_add_form_validate().
 */
function voipusernumber_settings_add_form_submit($form, &$form_state) {
  module_load_include('inc', 'voipusernumber', 'voipusernumber.script');
  if (_voipusernumber_auth_script() != FALSE) {
  $voipnumber = new VoipUserNumber($form_state['values']['number'], $form_state['values']['uid']);
  drupal_write_record('voipusernumber_numbers', $voipnumber);
  $account->voipnumber[$voipnumber->getNumber()] = $voipnumber;
  
    } 
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
