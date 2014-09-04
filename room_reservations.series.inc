<?php

/**
 * @file
 * Reservation CRUD.
 */

/**
 * Create REPEATING reservations.
 *
 * @param mixed $node
 *   $node
 */
function room_reservations_insert($node) {
  if ($node->type != 'room_reservations_reservation') {
    return;
  }

  // When we do a node_save below; it will hit this hook;
  // so if this routine just did the save lets bail.
  if (isset($node->saved)) {
    return;
  }

  $rtype = $node->reservation_repeat_type['und'][0]['value'];
  $start = $node->reservation_date['und'][0]['value'];
  $start_yyyy_mm_dd = date('Y-m-d', strtotime($start));
  $end = date('Y-m-d', strtotime($node->reservation_repeat_until['und'][0]['value']));
  $time = $node->reservation_time['und'][0]['value'];
  $length = $node->reservation_length['und'][0]['value'];
  $rid = $node->reservation_room['und'][0]['target_id'];
  $day = date('l', strtotime($start));

  switch ($rtype) {
    // No repeats.
    case 1:
      return;

    // Every day until....
    case 2:
      $skip = '+1 day';
      $back = '-1 day';
      $msg = t('You have booked every day from %start until %end', array('%start' => $start_yyyy_mm_dd, '%end' => $end));
      break;

    // This day of the week until..
    case 3:
      $skip = '+7 day';
      $back = '-7 day';
      $msg = t('You have booked every %day from %start until %end', array(
        '%day' => $day,
        '%start' => $start_yyyy_mm_dd,
        '%end' => $end)
      );
      break;
  }

  // Set NID as Series ID for both the primary node and the repeat nodes.
  $node->reservation_series_id['und'][0]['value'] = $node->nid;

  $tmp = clone($node);
  unset($tmp->nid, $tmp->vid);
  // And must unset default date that we just used
  // in hook_node_presave for the initial node.
  unset($tmp->date_default);
  $date = $start;
  while (strtotime($date) <= strtotime($back, strtotime($end))) {
    $date = date('Y-m-d', strtotime($skip, strtotime($date)));

    // Must check to see if next booking is available
    // the first one we don't check as we could not have picked it if it wasn't
    if (_room_reservations_is_slot_free($rid, $date, $time, $length)) {
      $new = clone($tmp);
      // Added 12:00:00 to fix TZ issues;
      // this is unlikely correct way to do this.
      $new->reservation_date['und'][0]['value'] = $date . ' 00:00:00';
      $new->saved = TRUE;
      node_save($new);
    }
    else {
      $booked[] = $date;
    }
  }

  // Lets spit out some useful msgs;
  // First clear the msg stating we just created the reservation node.
  drupal_get_messages('status');
  drupal_set_message(t('Your reservation series has been booked.'));
  drupal_set_message($msg);
  if (count($booked)) {
    $dates = '<br/>' . implode('<br/>', $booked);
    // CHANGED - !dates changed to @dates for escaping characters
    // drupal_set_message(t('
    // The following dates were not booked due to scheduling conflicts:
    // !dates', array('!dates' => $dates)));
    drupal_set_message(t('The following dates were not booked due to scheduling conflicts: !dates', array('!dates' => $dates)));
  }
}

/**
 * Handles Series edits.
 *
 * @param mixed $node
 *   $node
 */
function room_reservations_update($node) {
  if ($node->type != 'room_reservations_reservation') {
    return;
  }

  // When we do a node_save below; it will hit this hook;
  // so if this routine just did the save lets bail.
  if ($node->op = "Save") {
    return;
  }

  // If not part of a Series or special single only url; do nothing.
  if (!($sid = $node->reservation_series_id['und'][0]['value']) || $_GET['single']) {
    return;
  }

  // Reservation details.
  $start = $node->reservation_date['und'][0]['value'];
  $end = date('Y-m-d', strtotime($node->reservation_repeat_until['und'][0]['value']));
  $time = $node->reservation_time['und'][0]['value'];
  $length = $node->reservation_length['und'][0]['value'];
  $rid = $node->reservation_room['und'][0]['target_id'];
  $day = date('l', strtotime($start));

  // Grab all reservations in this series.
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'room_reservations_reservation')
    ->fieldCondition('reservation_series_id', 'value', $sid, '=');
  $result = $query->execute();
  if (isset($result['node'])) {
    $result_nids = array_keys($result['node']);
    $results = entity_load('node', $result_nids);
    foreach ($results as $data) {
      // For now, only allow changing Title, Blocked title, Length.
      // First 2 are easy; but to change length we need to check if
      // that period is available and if not delete entry from Series.
      $data->title = $node->title;
      $data->reservation_block_title['und'][0]['value'] = $node->reservation_block_title['und'][0]['value'];

      $date = date('Y-m-d', strtotime($data->reservation_date['und'][0]['value']));
      if (_room_reservations_is_slot_free($rid, $date, $time, $length)) {
        $data->reservation_length['und'][0]['value'] = $node->reservation_length['und'][0]['value'];
        $data->saved = TRUE;
        node_save($data);
      }
      // If slot is not available; delete that entry.
      else {
        $deleted[] = $date;
        node_delete($data->nid);
      }
    }
  }

  // Lets spit out some useful messages.
  drupal_set_message(t('Your reservation series has been modified.'));
  drupal_set_message(check_plain($msg));
  if (count($deleted)) {
    $dates = '<br/>' . implode('<br/>', $deleted);
    // CHANGED - Replaced !dates with @dates to filter characters
    // drupal_set_message(t('The following dates were deleted from the series
    // due to scheduling conflicts: !dates', array('!dates' => $dates)));
    drupal_set_message(t('The following dates were deleted from the series
      due to scheduling conflicts: !dates', array('!dates' => $dates)));
  }
}

/**
 * Description.
 *
 * @param array $form
 *   $node
 */
function _room_reservations_series_delete($form) {
  $sid = $form['#node']->reservation_series_id['und'][0]['value'];
  // Grab all reservations in this series.
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'room_reservations_reservation')
    ->fieldCondition('reservation_series_id', 'value', $sid, '=');
  $result = $query->execute();
  if (isset($result['node'])) {
    $nids = array_keys($result['node']);
    foreach ($nids as $nid) {
      node_delete($nid);
    }
  }
  $title = $form['#node']->title;
  drupal_set_message(t('The reservation series @title was deleted.', array('@title' => $title)));
  drupal_goto('room_reservations');
}
