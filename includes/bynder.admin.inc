<?php

/**
 * @file
 * Content of administration settings page.
 */

/**
 * Administration settings form.
 *
 * Menu page callback for admin/config/media/-bynder
 */
function bynder_admin_form($form, &$form_state) {
  $form['bynder_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Bynder API'),
    '#description' => t('You will first need to apply for an API Developer Key'),
    '#collapsible' => TRUE
  );

  $form['bynder_settings']['bynder_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Bynder url'),
    '#default_value' => variable_get('bynder_url', ''),
    '#size' => 50,
    '#description' => t('The Bynder url.'),
    '#required' => TRUE
  );

  $form['bynder_settings']['bynder_oauth_consumer'] = array(
    '#type' => 'textfield',
    '#title' => t('OAuth Consumer Token'),
    '#default_value' => variable_get('bynder_oauth_consumer', ''),
    '#size' => 50,
    '#description' => t('The consumer token to be used by the module to communicate with Bynder.'),
    '#required' => TRUE
  );

  $form['bynder_settings']['bynder_oauth_consumer_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('OAuth Consumer Secret'),
    '#default_value' => variable_get('bynder_oauth_consumer_secret', ''),
    '#size' => 50,
    '#description' => t('The consumer secret to be used by the module to communicate with Bynder.'),
    '#required' => TRUE
  );

  $form['bynder_settings']['bynder_oauth_token'] = array(
    '#type' => 'textfield',
    '#title' => t('OAuth Token'),
    '#default_value' => variable_get('bynder_oauth_token', ''),
    '#size' => 50,
    '#description' => t('The token to be used by the module to communicate with Bynder.'),
    '#required' => TRUE
  );

  $form['bynder_settings']['bynder_oauth_token_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('OAuth Token Secret'),
    '#default_value' => variable_get('bynder_oauth_token_secret', ''),
    '#size' => 50,
    '#description' => t('The token secret to be used by the module to communicate with Bynder.'),
    '#required' => TRUE
  );

  return system_settings_form($form);
}

/**
 * Validate function for bynder_admin_form()
 */
function bynder_admin_form_validate($form, &$form_state) {
  if ($form_state['values']['bynder_url']) {
    $form_state['values']['bynder_url'] = trim($form_state['values']['bynder_url']);
    if (!filter_var($form_state['values']['bynder_url'], FILTER_VALIDATE_URL)) {
      form_set_error('bynder_url', t('Bynder url is not a valid url.'));
    }
  }
}

