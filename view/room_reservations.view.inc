<?php
/**
 * @file
 * Miscellaneous view functionality.
 */

/**
 * Change date form.
 *
 * Constructor for the small form that appears just above the reservation
 * calendar and allows the user to change the date for which reservations
 * are displayed on the calendar.
 *
 * @param array $building_hours
 *   Contains hrs information for each day for which a reservation can be made.
 * @param array $dates
 *   An array containing information about all the possible days for which a
 *   reservtion can be made.
 * @param array $rooms
 *   An array representing all of the rooms that can be reserved.
 * @param array $categories
 *   An array of all the room categories.
 * @param string $selected_category
 *   The current category the user has selected, if any.
 */
function room_reservations_select_room_date_form(&$form_state, $building_hours,
  $dates, $rooms, $categories, $selected_category) {
  // Select box options.
  $date_options = array();
  foreach ($dates as $day) {
    $building_hours_day = $building_hours[$day['yyyymmdd']];
    if ($building_hours_day['open']) {
      $date_options[$day['month-number'] . "/" . $day['day']]
        = t($day['display']);
    }
  }
  $first = TRUE;
  foreach ($rooms as $room) {
    $name = $room['name'];
    $key = 'room ' . $name;
    $capacity = $room['capacity'];
    $room_category = $room['category'];
    $value = t('@room (capacity: !capacity persons)',
      array('@room' => $name, '!capacity' => $capacity));
    $room_options[$key] = $value;
    if ($first) {
      $room_default = $key;
      $first = FALSE;
    }
  }
  ksort($room_options);
  // Defaults.
  if ($selected_category) {
    $room_default = 'room ' . $selected_category;
  }
  foreach ($dates as $day) {
    if ($day['selected']) {
      $date_default = $day['month-number'] . '/' . $day['day'];
      break;
    }
  }
  // Form.
  $form['date'] = array(
    '#title' => t('View a Different Day'),
    '#type' => 'select',
    '#options' => $date_options,
    '#default_value' => $date_default,
    '#weight' => 10,
  );
  $form['room'] = array(
    '#title' => t('View a Different Room'),
    '#type' => 'select',
    '#options' => $room_options,
    '#default_value' => $room_default,
    '#weight' => 20,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Change Day or Room'),
    '#weight' => 30,
  );
  return $form;
}

/**
 * Form submission for the change date form.
 *
 * @param string $form_id
 *   Drupal form id.
 * @param array $form_state
 *   Drupal form state object.
 */
function room_reservations_select_room_date_form_submit($form_id,
  &$form_state) {
  $date = $form_state['values']['date'];
  $room = drupal_substr($form_state['values']['room'], 5);
  $redirect = "room_reservations/" .
    $date . "/" .
    $room;
  $form_state['redirect'] = $redirect;
}
/**
 * Theme the list of the current user's reservations.
 *
 * @param object $user
 *   Drupal user object.
 * @param string $base_url
 *   The base URL of the Drupal installation.
 * @param array $user_reservations
 *   Information about the user's reservations.
 * @param int $count
 *   The number of reservations to be displayed.
 *
 * @return string
 *   A list of the current user's reservations in HTML format.
 */
function theme_room_reservations_list($variables) {
  $user = $variables['0'];
  // TODO Number of parameters in this theme funcion does not match number of parameters found in hook_theme.
  $output = '';
  if ($user->uid) {
    // User is logged in.
    $output .= "<div id='reservations'>";
    if ($count > 0) {
      // User is logged in and has current reservations.
      $output .= '<br />' . t('Click on a reservation to view, cancel or modify
        it.') . '<br /><ul>';
      foreach ($user_reservations as $user_reservation) {
        $output .= "<li><a href='" . $base_url . '/room_reservations/view/' .
          $user_reservation['id'] . "'>" . t($user_reservation['date']) .
          " - " . t($user_reservation['time'])  . '</a></li>';
      }
      $output .= '</ul>';
    }
    else {
      // User is logged in and has no current reservations.
      $output .= '<br />' . t('No current reservations');
    }
  }
  else {
    // User is not logged in.
    $output .= "<br /><a href='" . $base_url .
      "/user/login?destination=room_reservations'>" .
      t('Login to view, cancel or modify your reservations.') . '</a>';
  }
  $output .= '</div>';
  return $output;
}

/**
 * Theme the room description page /room_reservations/rooms.
 *
 * @param array $categories
 *   An array of all the room categories.
 * @param array $rooms
 *   An array of all the rooms.
 *
 * @return string
 *   The html for displaying the page.
 */
function theme_room_reservations_descriptions($variables) {
  $categories = $variables['0'];
  // TODO Number of parameters in this theme funcion does not match number of parameters found in hook_theme.
  $output .= '<br />';
  foreach ($categories as $category) {
    $output .= '<h2>' . t($category['display']) . '</h2><br />';
    foreach ($rooms as $room) {
      if ($room['category'] == $category['category']) {
        $output .= '<a name="' . $room['name'] . '"><h3>' . $room['name'] .
          '</h3></a><br />' . t('Capacity') . ':&nbsp;&nbsp;' .
          $room['capacity'] . '<br /><br />' . t($room['description']) .
          '<br /><br />';
      }
    }
  }
  return $output;
}
