<?php

/**
 * @file
 * Administration interface for the ims module.
 *
 * Allows configuration of ims end-point and credentials.
 */

/**
 * Form builder; Configure IMS settings for this site.
 *
 * @ingroup forms
 *
 * @see system_settings_form()
 */
function ims_admin_settings_form($form, &$form_state) {
  $form['ims'] = array(
    '#type' => 'fieldset',
    '#title' => t('IMS settings'),
    '#description' => t('When settings are configured DDB CMS will start 
      fetching placement information about materials from the IMS service. <br>
      By default the IMS service exposes no placements at all. Use the 
      IMS Webclient to configure which application types should expose placements.'),
    '#tree' => FALSE,
  );

  $form['ims']['ims_wsdl_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Service URL'),
    '#description' => t('URL of the IMS webservice. The url template 
      looks like this: https://[library-short-name].lyngsoe-imms.com/ImsWs/soap/Ims4Ils?wsdl<br> 
      E.g. https://bibsdb.lyngsoe-imms.com/ImsWs/soap/Ims4Ils?wsdl'),
    '#required' => TRUE,
    '#default_value' => variable_get('ims_wsdl_url', ''),
  );

  $form['ims']['ims_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#description' => t('Username of a user created in the IMS webclient. 
      No privileges needs to be granted to the user.'),
    '#required' => TRUE,
    '#default_value' => variable_get('ims_username', ''),
  );

  $form['ims']['ims_password'] = array(
    '#type' => 'textfield',
    '#title' => t('Password'),
    '#description' => t('Password of the user.'),
    '#required' => TRUE,
    '#default_value' => variable_get('ims_password', ''),
  );

  $form['ims']['ims_enable_logging'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable logging'),
    '#default_value' => variable_get('ims_enable_logging', FALSE),
  );

  $form['ims']['ims_hide_zero_holdings'] = array(
    '#type' => 'checkbox',
    '#title' => t('Filter away holdings with 0 available items'),
    '#default_value' => variable_get('ims_hide_zero_holdings', FALSE),
  );

  return system_settings_form($form);
}
