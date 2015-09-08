<?php

function sp_debat_admin_form($form, &$form_state) {
  $events = sp_debat_get_events();
  $range = range(1,200);
  $regions = array(
    'noord' => 'Landsdeel noord',
    'midden' => 'Landsdeel midden',
    'zuid' => 'Landsdeel zuid',
  );
  foreach ($regions as $key => $name) {
    $default_event_id = variable_get('sp_debat_event_' . $key);

    $form[$key] = array(
      '#type' => 'fieldset',
      '#title' => $name,
    );
    $form[$key]['sp_debat_event_' . $key] = array(
      '#type' => 'select',
      '#title' => 'Selecteer een debat',
      '#options' => $events,
      '#default_value' => isset($default_event_id) ? $default_event_id : '',
      '#required' => TRUE,
    );
    $form[$key]['sp_debat_ratio_' . $key] = array(
      '#type' => 'select',
      '#title' => 'Aantal leden afdeling per deelnemer afdeling',
      '#options' => $range,
      '#default_value' => variable_get('sp_debat_ratio_' . $key),
      '#required' => TRUE,
    );
    $total = sp_debat_get_total_participants($default_event_id);
    $options = array(
      'Aanmeldingen beperken tot maximum per afdeling.',
      'Aanmeldingen per afdeling vrijgeven tot evenement maximum bereikt is.',
      'Aanmeldingen niet beperken, meer aanmeldingen mogelijk dan plaatsen.',
    );
    $form[$key]['sp_debat_free_' . $key] = array(
      '#type' => 'radios',
      '#title' => 'Aanmeldingen vrijgeven',
      '#default_value' => variable_get('sp_debat_free_' . $key, 0),
      '#options' => $options,
    );
    if (isset($default_event_id)) {
      $form[$key]['sp_debat_ratio_' . $key]['#suffix'] = 'Totaal aantal beschikbare plaatsen: ' . $total;
    }
  }
  return system_settings_form($form);
}
