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
    $form[$key] = array(
      '#type' => 'fieldset',
      '#title' => $name,
    );
    $form[$key]['sp_debat_event_' . $key] = array(
      '#type' => 'select',
      '#title' => 'Selecteer een debat',
      '#options' => $events,
      '#default_value' => variable_get('sp_debat_event_' . $key),
      '#required' => TRUE,
    );
    $form[$key]['sp_debat_ratio_' . $key] = array(
      '#type' => 'select',
      '#title' => 'Aantal leden afdeling per deelnemer afdeling',
      '#options' => $range,
      '#default_value' => variable_get('sp_debat_ratio_' . $key),
      '#required' => TRUE,
    );
  }
  return system_settings_form($form);
}
