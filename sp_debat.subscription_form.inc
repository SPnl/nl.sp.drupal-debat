<?php

function sp_debat_subscription_form($form, &$form_state) {
  global $conf;
  if (!isset($conf['sp_debat_event_noord'])) {
    drupal_set_message('Debat module is nog niet ingesteld.', 'error');
    return;
  }
  // Get data
  global $user;
  $voorzitter = sp_debat_get_civi_contact($user->uid);
  $afdelingen = sp_debat_get_voorzitter_afdelingen($voorzitter['id']);
  $event_noord = variable_get('sp_debat_event_noord');
  $event_midden = variable_get('sp_debat_event_midden');
  $event_zuid = variable_get('sp_debat_event_zuid');

  // Intro texts.
  $intro[$event_noord] = '<p>Dinsdag 6 oktober<br/>Groninger Forum, Hereplein 73 te Groningen</p>';
  $intro[$event_midden] = '<p>Maandag 28 september<br/>De Moed, Snouckaertlaan 70 te Amersfoort</p>';
  $intro[$event_zuid] = '<p>Dinsdag 22 september<br/>Verkadefabriek, Boschdijkstraat 45 te \'s Hertogenbosch</p>';

  // Create ajax form.
  $form = array(
    '#prefix' => '<div class="content" id="debat_form"><p>Meldt hier de leden aan die aanwezig zullen zijn bij het  voorzittersdebat voor jouw regio.<br/>Voor iedere afdeling is een aantal plaatsen gegarandeerd. Meer leden aanmelden is mogelijk, plaatsen die vrij komen als afdelingen geen gebruik maken van hun gegarandeerde aanmeldingen komen beschikbaar voor andere afdelingen. Minimaal drie werkdagen voor het debat krijg jij en krijgen de leden die jij hebt opgegeven daarover bericht.</p>',
    '#suffix' => '</div>',
    '#attached' => array('css' => array(drupal_get_path('module', 'sp_debat') . '/sp_debat.css',),),
    '#attributes' => array('class' => array('sp-debat-form'),),
  );
  // Create a form for each afdeling.
  if ($afdelingen) {
    foreach($afdelingen as $afdeling) {
      // Set data.
      $event = sp_debat_get_event($afdeling['id']);
      $free = sp_debat_get_free($afdeling['id']); 
      $members = sp_debat_get_afdeling_members($afdeling['id']);
      $participants = sp_debat_get_participants($event['id'], $afdeling['id']);
      $allowed = sp_debat_get_allowed_participants($afdeling['id']);
      $afdelingsname = str_replace('SP-afdeling', '', $afdeling['display_name']);

      // Store mapping afdeling id -> event id.
      $form_state['event']['afd_event'][$afdeling['id']] = $event['id'];

      $form[$afdeling['id']] = array(
        '#type' => 'fieldset',
      );
      $reserved_text = ($free == 2) ? '<br/>' . $allowed . ' plaatsen gegarandeerd' :'';
      $intro_text = '<h1>Afdeling ' . $afdelingsname . '</h1><p><strong>Aanmelding voor ' . $event['title'] . $reserved_text . '</strong></p>' . $intro[$event['id']];
      $form[$afdeling['id']]['intro_' . $afdeling['id']] = array(
        '#type' => 'markup',
        '#markup' => $intro_text,
      );
      if (
        ($allowed > 0 && in_array($free, array(0, 1))) || // Beperkt aanmelden, alleen als limiet niet bereikt.
        $free == 2 // Geheel vrij aanmelden.
      ) {
        // Create select participant button.
        $form[$afdeling['id']]['participant_' . $afdeling['id']] = array(
          '#type' => 'textfield',
          '#title' => 'naam / lidnummer',
          '#autocomplete_path' => 'aanmelden/voorzittersdebat/autocomplete_members/' . $afdeling['id'] . '/' . $event['id'],
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
      if ($free == 2) {
        $text = '';
      }
      elseif ($allowed < 0) {
        $text = 'Het maximale aantal aanmeldingen is overschreden.';
      }
      elseif ($allowed == 0) {
        $text = 'Het maximale aantal aanmeldingen is bereikt.';
      }
      elseif ($allowed == 1) {
        $text = 'Er kan nog één lid worden aangemeld.';
      }
      else {
        $text = 'Er kunnen nog ' . $allowed . ' leden worden aangemeld.';
      }
      $form[$afdeling['id']]['allowed_' . $afdeling['id']] = array(
        '#type' => 'markup',
        '#markup' => '<p>' . $text . '</p>',
      );
      // Create delete buttons for all participants.
      $count = 0;
      foreach ($participants as $participant) {
        $count ++;
        $guaranteed = ($count <= $allowed) ? 'guaranteed' : 'not-guaranteed';
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
          '#attributes' => array('class' => array('sp_debat_del_button', $guaranteed)),
        );
      }
    }
    $form['sp_debat_ready_button'] = array(
      '#type' => 'submit',
      '#value' => 'Klaar',
      '#name' => 'sp_debat_ready_button',
      '#ajax' => array(
        'callback' => 'sp_debat_participants_form_callback',
        'wrapper' => 'debat_form',
      ),
      '#attributes' => array('class' => array('sp-debat-ready-submit-button')),
    );
  }
  return $form;
}

function sp_debat_ready_button_submit($form, &$form_state) {
  drupal_set_message('Uw aanmeldingen zijn opgeslagen.', 'error');
  drupal_goto('<front>');
}

function sp_debat_autocomplete_members($afdeling_id, $event_id, $string) {
  $selected_members = sp_debat_find_members($afdeling_id, $event_id, $string); 
  drupal_json_output($selected_members);
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
      // Check if is lidnummer.
      if(!filter_var($contact_id, FILTER_VALIDATE_INT)) {
        $selected_members = sp_debat_find_members($afdeling_id, $event_id, $contact_id); 
        if (count($selected_members) == 1) {
          $contact_id = key($selected_members);
        }
      }
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
    case 'del':
      $participant_id = $matches[2];
      // Remove event subscription.
      sp_debat_remove_participant($participant_id, $event_id);
      break;
    }
    // Rebuild form.
    $form = drupal_rebuild_form('sp_debat_subscription_form', $form_state);
    // Show drupal messages in form.
    $form[$afdeling_id]['message_' . $afdeling_id]['#markup'] = theme_status_messages();
  }

  if ($trigger === 'sp_debat_ready_button') {
    drupal_set_message('Uw aanmeldingen zijn opgeslagen.');
    ctools_include('ajax');
    ctools_add_js('ajax-responder');
    $commands[] = ctools_ajax_command_redirect('<front>');
    print ajax_render($commands);
    exit;
  }

  return $form;
}
