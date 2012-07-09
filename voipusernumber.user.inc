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
 */
function voipusernumber_settings_add_form_validate($form, &$form_state) {
  global $user;
  $error = '';
  variable_set('pin_variable_ask', $form['add']['pin']['#value']);
  // Basic VoIPNumber validation.
  _voipnumber_valid_int_phone_number($form_state['values']['number'], $error);
  if (! empty($error)) {
    form_set_error('number', t('Phone number is %error', array('%error' => $error)));
  }
  // Make a call to confirm the pin  
  module_load_include('inc', 'voipusernumber', 'voipusernumber.script');
  $num = $form_state['values']['number'];
  _voipusernumber_auth_script($num);
  // Pin verification
  if(_voipusernumber_validate_pin() == FALSE) {
    form_set_error('pin', t('Try again, unexpected error.'));
  }
// Unique validation.
  $current_user = $user->uid;
  $assoc_user =VoipUserNumber::getUidFromNumber($form_state['values']['number'];
  if ($assoc_user != $assoc_user )) {
    voipusernumber_do_user_merge($current_user,$assoc_user);
    //    form_set_error('number', t('Phone number is already registered.'));
  }
}

function voipusernumber_do_user_merge($current_user,$assoc_user) {
    module_invoke_all('user_merge', $assoc_user, $current_user);
}
/**
 * Implement hook_usermerge_merge_users().
 */
function voipusernumber_user_merge($assoc_user, $current_user) {
  // Comment: change the uid to the new one.
  db_query('UPDATE {comments} SET uid = %d where uid = %d', $current_user->uid, $assoc_user->uid);

  // Node: change uid of node and revisions to the new one.
  db_query('UPDATE {node} SET uid = %d where uid = %d', $current_user->uid, $assoc_user->uid);
  db_query('UPDATE {node_revisions} SET uid = %d where uid = %d', $current_user->uid, $assoc_user->uid);

  //Update roles.
  $merged_roles = $current_user->roles + $assoc_user->roles;

  // First, remove all roles from the user to keep to prevent duplicate key errors.
  db_query("DELETE FROM {users_roles} WHERE uid = %d", $current_user->uid);
  // Then add them on to the user to keep.
  foreach (array_keys($merged_roles) as $rid) {
    if (!in_array($rid, array(DRUPAL_ANONYMOUS_RID, DRUPAL_AUTHENTICATED_RID))) {
      db_query('INSERT INTO {users_roles} (uid, rid) VALUES (%d, %d)', current_user->uid, $rid);
    }
  }
  // Block the old user.
  user_block_user_action($assoc_user);

  // Success!
  drupal_set_message(t('We successfully merged %user_to_delete into %user_to_keep', array('%user_to_delete' => $user_to_delete->name, '%user_to_keep' => $user_to_keep->name)));
}

/**
 * Implement hook_usermerge_merge_users() on behalf of og.
 */
function og_user_merge($assoc_user, $current_user) {
  // Get groups kept-user is not a member of but blocked-user is.
  $result = db_query("SELECT nid FROM {og_uid} WHERE uid = %d AND nid NOT IN (SELECT nid FROM {og_uid} WHERE uid = %d)", $assoc_user->uid, $current_user->uid);
  while ($record = db_fetch_array($result)) {
    $groups[] = $record['nid'];
  }
  if (!empty($groups)) {
    // Update old user's groups to kept-user if kept-user is not already a member.
    db_query("UPDATE {og_uid} SET uid = %d WHERE uid = %d AND nid IN (%s)", $current_user->uid, $assoc_user->uid, implode(',', $groups));
  }
  // Set blocked-user to inactive.
  db_query("UPDATE {og_uid} SET is_active = 0 WHERE uid = %d", $assoc_user->uid);
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