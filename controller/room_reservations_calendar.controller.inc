<?php
/**
 * @file
 * Calendar controller functionality.
 */

/**
 * Display the reservation calendar.
 *
 * @param int $selected_month
 *   (optional) The month of the day selected by the user.
 * @param int $selected_day
 *   (optional) The day of the month selected by the user.
 * @param string $selected_category
 *   (optional) The room category selected by the user.
 *
 * @return string
 *   The page display formatted as HTML.
 */
function room_reservations($selected_month = NULL, $selected_day = NULL,
  $selected_category = NULL) {
  $dates = _room_reservations_dates($selected_month, $selected_day);
  // Determine which day has been selected by the user.  If
  // the user has entered a url that specifies a day outside of the
  // allowable reservation window, then set the current day
  // as the selected day.
  $yyyymmdd = $dates[0]['yyyymmdd'];
  foreach ($dates as $day) {
    if ($day['selected']) {
      $yyyymmdd = $day['yyyymmdd'];
    }
  }
  if ($yyyymmdd == $dates[0]['yyyymmdd']) {
    $dates[0]['selected'] = TRUE;
    $dates[0]['today'] = TRUE;
  }
  $categories = _room_reservations_categories();
  // If the day being displayed is today, only display time slots that are
  // later than the current time minus two hours.
  $today_displayed = FALSE;
  foreach ($dates as $day) {
    if (($day['selected']) && $day['today']) {
      $today_displayed = TRUE;
    }
  }
  if ($today_displayed) {
    $hours = _room_reservations_hours('limited');
  }
  else {
    $hours = _room_reservations_hours();
  }
  $_room_reservations_building_hours = _room_reservations_facility_hours();
  $display_hours = _room_reservations_hours();
  // Determine the open hours (display version) for the date selected by the
  // user.
  $building_hours_day = $_room_reservations_building_hours[$yyyymmdd];
  $building_hours_display = $building_hours_day['display'];
  // For each time slot, determine if the rooms are open or closed.
  $building_hours_day = $_room_reservations_building_hours[$yyyymmdd];
  $int_first_shift_open = intval($building_hours_day['first_shift_open']);
  $int_first_shift_close = intval($building_hours_day['first_shift_close']);
  $int_second_shift_open = intval($building_hours_day['second_shift_open']);
  $int_second_shift_close = intval($building_hours_day['second_shift_close']);
  $open_24 = $building_hours_day['open_24_hours'];
  $x = 0;
  foreach ($hours as $time_slot) {
    $int_time_slot_time = intval($time_slot['time']);
    if ($building_hours_day['open_24_hours']) {
      $time_slot['open'] = TRUE;
    }
    elseif ((($int_time_slot_time >= $int_first_shift_open) &&
      ($int_time_slot_time < $int_first_shift_close)) ||
      (($int_time_slot_time >= $int_second_shift_open) &&
      ($int_time_slot_time < $int_second_shift_close))) {
      $time_slot['open'] = TRUE;
    }
    else {
      $time_slot['open'] = FALSE;
    }
    $hours[$x] = $time_slot;
    $x++;
  }
  $all_hours = _room_reservations_hours();
  $times = _room_reservations_times();
  $rooms = _room_reservations_rooms();
  // Initialize the $reservations array.
  $reservations = array();
  foreach ($rooms as $room) {
    $room_name = $room['name'];
    foreach ($hours as $time_slot) {
      $time = $time_slot['time'];
      $reservations[$room_name][$time] = array(
        'time' => $time,
        'display' => $time_slot['display'],
        'class' => $time_slot['class'],
        'id' => '',
        'booked' => FALSE,
        'start' => FALSE,
        'end' => FALSE,
        'user' => '',
        'name' => '',
      );
    }
  }
  // Get all room_reservation records for the selected day.
  // Build the $reservations array that will be used for displaying the
  // information in a calendar style grid.
  $mobile_db = _room_reservations_detect_mobile_db();
  if ($mobile_db) {
    $mainsite = trim(_room_reservations_get_variable('main_database'));
    db_set_active($mainsite);
  }
  $sql = "
    SELECT * FROM room_reservations
    WHERE date = :date
    AND deleted = :delete
    ORDER BY time
  ";
  $result = db_query($sql, array(':date' => $yyyymmdd, ':delete' => 'N'));
  foreach ($result as $data) {
    $id = $data->id;
    $time = $data->time;
    $room_name = $data->room;
    $name = check_plain($data->name);
    $user_name = $data->user_name;
    $reservations[$room_name][$time]['booked'] = TRUE;
    $reservations[$room_name][$time]['name'] = $name;
    $reservations[$room_name][$time]['user_name'] = $user_name;
    $reservations[$room_name][$time]['id'] = $id;
    $length = $data->length;
    $time_slots = $length / 30;
    $remainder = $length % 30;
    if ($remainder > 1) {
      $time_slots++;
    }
    $key = array_search($time, $times);
    $x = 2;
    while ($x <= $time_slots) {
      $key++;
      $next_time = $times[$key];
      $reservations[$room_name][$next_time]['booked'] = TRUE;
      $reservations[$room_name][$next_time]['user_name'] = '';
      $x++;
    }
  }
  if ($mobile_db) {
    db_set_active('default');
  }
  // Get any room_reservation records for the previous day
  // that might carry over into the selected day.
  // Continue to update the $reservation array with that information.
  $previous_day = date('Y-m-d', strtotime("$yyyymmdd -1 days"));
  $max_length = variable_get('room_reservations_max_length', 120);
  $lhours = _room_reservations_hours();
  $rhours = array_reverse($lhours);
  $slots = ($max_length / 30) - 1;
  $late_times = array();
  $search_times = array(
    '9999',
    '9999',
    '9999',
    '9999',
    '9999',
    '9999',
    '9999',
  );
  for ($j = 0; $j < $slots; $j++) {
    $hour = $rhours[$j];
    $late_times[] = $hour['time'];
    $search_times[$j] = $hour['time'];
  }
  $late_times = array_reverse($late_times);
  if ($mobile_db) {
    $mainsite = trim(_room_reservations_get_variable('main_database'));
    db_set_active($mainsite);
  }
  $sql = "
    SELECT id, time, length, room, name, user_name
    FROM {room_reservations}
    WHERE date = :previous_day
    AND time IN (:search_times0, :search_times1, :search_times2, :search_times3, :search_times4, :search_times5, :search_times6)
    AND deleted = :delete
    ORDER BY time
  ";
  $result = db_query($sql, array(
    ':previous_day' => $previous_day,
    ':search_times0' => $search_times[0],
    ':search_times1' => $search_times[1],
    ':search_times2' => $search_times[2],
    ':search_times3' => $search_times[3],
    ':search_times4' => $search_times[4],
    ':search_times5' => $search_times[5],
    ':search_times6' => $search_times[6],
    ':delete' => 'N',
  ));
  foreach ($result as $data) {
    $id = $data->id;
    $time = $data->time;
    $length = $data->length;
    $room_name = $data->room;
    $name = check_plain($data->name);
    $user_name = $data->user_name;
    $time_slots = $length / 30;
    $remainder = $length % 30;
    if ($remainder > 1) {
      $time_slots++;
    }
    for ($j = 0; $j < (($max_length / 30) - 1); $j++) {
      if ($late_times[$j] == $time) {
        $carry_over_time_slots = ($time_slots + $j - (($max_length / 30) - 1));
        break;
      }
    }
    if ($carry_over_time_slots > 0) {
      $reservations[$room_name]['0000']['booked'] = TRUE;
      $reservations[$room_name]['0000']['name'] = $name;
      $reservations[$room_name]['0000']['user_name'] = $user_name;
      $reservations[$room_name]['0000']['id'] = $id;
      $carry_over_time_slots--;
    }
    while ($carry_over_time_slots > 0) {
      $next_time = $times[$carry_over_time_slots];
      $reservations[$room_name][$next_time]['booked'] = TRUE;
      $carry_over_time_slots--;
    }
  }
  if ($mobile_db) {
    db_set_active('default');
  }
  $user_reservations = _room_reservations_user_reservations();
  // TODO Please change this theme call to use an associative
  // TODO array for the $variables parameter.
  $output = theme('room_reservations', array($dates, $categories, $reservations,
    $hours, $_room_reservations_building_hours, $building_hours_display, $rooms,
    $selected_category, $user_reservations));
  return $output;
}
