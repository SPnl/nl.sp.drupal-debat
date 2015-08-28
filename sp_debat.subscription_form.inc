<?php

function sp_debat_subscription_form($form, &$form_state) {
  // Get data
  global $user;
  $voorzitter = sp_debat_get_civi_contact($user->uid);
  $afdelingen = sp_debat_get_voorzitter_afdelingen($voorzitter['id']);

  // Create ajax form.
  $form = array();
  $form['#prefix'] = '<div id="debat_form">';
  $form['#suffix'] = '</div>';
  // Create a form for each afdeling.
  if ($afdelingen) {
    foreach($afdelingen as $afdeling) {
      // Set data.
      $province = sp_debat_get_province($afdeling['id']);
      $event = sp_debat_get_event($province['id']);
      $members = sp_debat_get_afdeling_members($afdeling['id']);
      $participants = sp_debat_get_participants($event['id']);
      $allowed = sp_debat_get_allowed_participants($afdeling['id']) - count($participants);

      // Store mapping afdeling id -> event id.
      $form_state['event']['afd_event'][$afdeling['id']] = $event['id'];

      $form[$afdeling['id']] = array(
        '#type' => 'fieldset',
      );
      $form[$afdeling['id']]['intro_' . $afdeling['id']] = array(
        '#type' => 'markup',
        '#markup' => '<h1>' . $afdeling['display_name'] . '</h1><h2>' . $event['title'] . '</h2>',
      );
      if ($allowed > 0) {
        // Create select participant button.
        $form[$afdeling['id']]['participant_' . $afdeling['id']] = array(
          '#type' => 'textfield',
          '#title' => 'naam / lidnummer',
          '#autocomplete_path' => 'form/aanmelden-debat/autocomplete_members/' . $afdeling['id'] . '/' . $event['id'],
        );
        // create add participant button.
        $form[$afdeling['id']]['add_button_' . $afdeling['id']] = array(
          '#type' => 'button',
          '#value' => 'Meld aan',
          '#name' => 'add_button_' . $afdeling['id'],
          '#ajax' => array(
            'callback' => 'sp_debat_participants_form_callback',
            'wrapper' => 'debat_form',
          ),
        );
      }
      // Add status messages.
      $form[$afdeling['id']]['message_' . $afdeling['id']] = array(
        '#type' => 'markup',
        '#markup' => '',
      );
      $form[$afdeling['id']]['allowed_' . $afdeling['id']] = array(
        '#type' => 'markup',
        '#markup' => '<p>Beschikbare plekken: ' . $allowed . '</p>',
      );
      // Create delete buttons for all participants.
      foreach ($participants as $participant) {
        // filter on afdeling.
        if (isset($members[$participant['contact_id']])) {
          $form[$afdeling['id']]['del_button_' . $participant['id']] = array(
            '#type' => 'button',
            '#value' => 'x',
            '#name' => 'del_button_' . $participant['id'],
            '#prefix' => '<p>',
            '#suffix' =>  $participant['display_name'] . '</p> ',
            '#ajax' => array(
              'callback' => 'sp_debat_participants_form_callback',
              'wrapper' => 'debat_form',
            ),
          );
        }
      }
    }
  }
  return $form;
}

function sp_debat_autocomplete_members($afdeling_id, $event_id, $string) {
  if (strlen($string) > 2) {
    // Check if participant is already subscribed.
    $participants = sp_debat_get_participants($event_id);
    $members = sp_debat_get_afdeling_members($afdeling_id);
    // Remove participants from memberlist.
    foreach ($participants as $participant) {
      unset($members[$participant['contact_id']]);
    }
    foreach ($members as $member_id => $member_name) {
      if (count($selected_members) > 9) {
        break;
      }
      if (strpos(strtolower($member_name), strtolower($string)) !== FALSE) {
          $selected_members[$member_id] = $member_name;
      }
    }
    drupal_json_output($selected_members);
  }
  exit;
}

function sp_debat_participants_form_callback($form, &$form_state) {
  $values = $form_state['values'];

  // Get data.
  $afdeling_id = $form_state['triggering_element']['#array_parents'][0];
  $event_id = $form_state['event']['afd_event'][$afdeling_id];
  
  // Get action info from trigger element.
  $trigger = $form_state['triggering_element']['#name'];
  $pattern = '/^(add|del)_button_(\d+)$/';
  if (preg_match($pattern, $trigger, $matches)) {
    switch ($matches[1]) {
    case 'add':
      $contact_id = $values['participant_' . $afdeling_id];
      // Check if member afdeling.
      if (sp_debat_contact_member_afdeling($contact_id, $afdeling_id)) {
        // Create event subscription.
        sp_debat_add_participant($contact_id, $event_id);
        $form[$afdeling_id]['participant_' . $afdeling_id]['#suffix'] = '<p>' . $result . '</p>';
        unset($form_state['input']['participant_' . $afdeling_id]);
      }
      else {
        drupal_set_message('Contact kon niet worden aangemeld', 'error');
      }
      break;
    case 'del':
      $participant_id = $matches[2];
      // Remove event subscription.
      sp_debat_remove_participant($participant_id, $event_id);
      break;
    }
  }

  // Rebuild form.
  $form = drupal_rebuild_form('sp_debat_subscription_form', $form_state);
  // Show drupal messages in form.
  $form[$afdeling_id]['message_' . $afdeling_id]['#markup'] = theme_status_messages();
  return $form;
}
