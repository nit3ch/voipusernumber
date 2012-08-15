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
  $pin = rand(1000,9999);

  // would be good to have a standard (outside CCK implementation of voipnumber 
  // elements?
  $form['add']['pin'] = array(
    '#type' => 'item',
    '#title' => t('Pin'),
    '#description' => t('Enter the pin when asked during call verification.'),
    '#value' => $pin,
  );
  $form['add']['numbers'] = array(
      '#type' => 'item',
      '#title' => t('Verify your phone number'),
      '#value' => _voipusernumber_user_user_view($account),
    );
    variable_set('pin_variable_ask', $form['add']['pin']['#value']);
  return $form;
}
function _voipusernumber_user_user_view($account) {
  
  drupal_add_js(drupal_get_path('module', 'voipusernumber') . '/js/voipusernumber.js');
  drupal_add_css(drupal_get_path('module', 'voipusernumber') . '/css/voipusernumber.css');
  
  $field_name='user';
  $delta=$account->uid; //Delta is uid of callee
  $script_desc= 'Click here to call';
  
  $output='<a href="javascript:void(0)" onclick="click2call_display(\'' . $field_name . '\',\'' . $delta . '\');" class="click2call-link">' . $script_desc . '</a>';
  $output .= '<div id="click2call-group-' . $field_name . '-' . $delta . '" class="click2call-hidden">';
  //Hidden div to store the phone list
  $output .= '<div id="click2call-' . $field_name . '-' . $delta . '-hidden" class="click2call-hidden"></div>';
  //Hidden div to store the call status in case of error
  $output .= '<div id="click2call-' . $field_name . '-' . $delta . '-status"></div>';
  //Hidden input to store the call nid.
  $output .= '<input type="hidden" value="" id="click2call-' . $field_name . '-' . $delta . '-callnid"/>';
  $output .= theme('voipusernumber_phone_numbers', $field_name, $delta, '');
  //Call/Remove button
  $output .= '<input id="click2call-button-' . $field_name . '-' . $delta . '"type="button" value="Call Me" onclick="click2call_call(\'' . $field_name . '\',' . $delta . ',\'\')" />';
  
  //Hangup button
  $output .= '<input class="click2call-hangup-button" id="click2call-hangup-button-' . $field_name . '-' . $delta . '"type="button" value="' . t('Hangup') . '" onclick="click2call_hangup(\'' . $field_name . '\',' . $delta . ')" />';
  $output .= '</div>';
  return $output;
}
/**
 * Form submission handler for voipusernumber_settings_add_form().
 */
function voipusernumber_settings_add_form_submit($form, &$form_state) {
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