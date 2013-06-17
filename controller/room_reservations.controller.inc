<?php
/**
 * @file
 * Miscellaneous controller functionality.
 */

/**
 * Display a list of the user's current reservations.
 *
 * @global object $user
 *   Drupal user object.
 * @global type $base_url
 *   The base URL of the Drupal installation.
 *
 * @return string
 *   A list of the user's reservations in HTML format.
 */
function room_reservations_res_list() {
  global $user;
  global $base_url;
  $user_reservations = NULL;
  $count = 0;
  if ($user->uid) {
    // User is logged in.
    $user_reservations = _room_reservations_user_reservations();
    $count = count($user_reservations);
  }
  // TODO Please change this theme call to use an associative array for the $variables parameter.
  $output .= theme('room_reservations_list', $user, $base_url,
      $user_reservations, $count);
  return $output;
}

/**
 * Display a page showing the reservation system policies.
 *
 * @return string
 *   The policies for the reservation system as entered by the administrator
 *   using the Display Text configuration page.
 */
function room_reservations_policies() {
  $output = check_markup(_room_reservations_get_variable('policies'));
  return $output;
}

/**
 * Display a page showing a description of each room that can be reserved.
 *
 * @return string
 *   A description of all the rooms that can be reserved in HTML format.
 */
function room_reservations_descriptions() {
  $categories = _room_reservations_categories();
  $rooms = _room_reservations_rooms();
  // TODO Please change this theme call to use an associative array for the $variables parameter.
  $output .= theme('room_reservations_descriptions', $categories, $rooms);
  return $output;
}
