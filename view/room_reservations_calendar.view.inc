<?php
/**
 * @file
 * Calendar view functionality.
 */

module_load_include('inc', 'room_reservations', '/view/room_reservations.view');

/**
 * Theme the room reservation calendar page.
 *
 * @global object $user
 *   Drupal user object.
 * @global type $base_url
 *   Application base url.
 *
 * @param array $dates
 *   An array containing information about all the possible days for which a
 *   reservtion can be made.
 * @param array $categories
 *   An array of all the room categories.
 * @param array $reservations
 *   An array representing all the reservations that have been made for the
 *   day that is being displayed on the calendar.
 * @param array $hours
 *   An array representing every half hour time slot in a single day.
 * @param array $building_hours
 *   A two dimensional array, with the first dimension representing a single
 *   day for which a reservation can be made, and the second dimension
 *   representing information about the facility's open hours for that day, such
 *   as whether the facility is open that day, the number of open shifts,
 *   open and close hours, and a string that can be used to display the hours
 *   in a user friendly way.
 * @param string $building_hours_display
 *   A user friendly string of the open hours for the day being displayed
 *   on the calendar.
 * @param array $rooms
 *   An array representing all of the rooms that can be reserved.
 * @param string $selected_category
 *   The current category the user has selected, if any.
 * @param array $user_reservations
 *   An array representing the current user's reservations for the allowable
 *   reservation period.
 *
 * @return string
 *   The html for displaying the page.
 */
function theme_room_reservations($variables) {
  $dates = $variables['0'];
  $categories = $variables['1'];
  $reservations = $variables['2'];
  $hours = $variables['3'];
  $building_hours = $variables['4'];
  $building_hours_display = $variables['5'];
  $rooms = $variables['6'];
  $selected_category = $variables['7'];
  // TODO Number of parameters in this theme funcion does not match number of parameters found in hook_theme.
  // User information.
  $full_access = FALSE;
  $user_login_name = NULL;
  global $user;
  global $base_url;
  if ($user->uid) {
    $user_login_name = $user->name;
    $full_access = _room_reservations_full_access();
  }
  // Determine which date has been selected by the user.
  foreach ($dates as $day) {
    if ($day['selected'] == TRUE) {
      $day_of_week = $day['day-of-week'];
      $month_number = $day['month-number'];
      $month = $day['month'];
      $xday = $day['day'];
      $year = $day['year'];
      $yyyymmdd = $day['yyyymmdd'];
    }
  }
  // Determine mobile or desktop.
  $mobile = _room_reservations_detect_mobile();
  // Mobile display.
  if ($mobile) {
    $output = "<div id='rooms-mobile'><div id='info'>";
    // Room.
    if ($selected_category) {
      $display_room = $selected_category;
    }
    else {
      foreach ($rooms as $room) {
        $display_room = $room['name'];
        break;
      }
    }
    // Form to change day and or room.
    $form = drupal_get_form('room_reservations_select_room_date_form',
      $building_hours, $dates, $rooms, $categories, $selected_category);
    $output .= drupal_render($form);
    // Calendar date.
    $display_date = $day_of_week . ', ' . $month . ' ' . $xday . ', ' . $year;
    $output .= "<b>" . t('Reservation Calendar') . "</b>
      <div class='hours'>" . t($display_date) . '<br />' .
      t($building_hours_display) . '<br /><b>' . $display_room .
      '</b></br></div>';
    // Table to display times and reservations.
    $output .= '<table>';
    foreach ($hours as $time_slot) {
      $output .= '<tr><td>' . t($time_slot['display']) .
        '&nbsp;&nbsp;</td><td>';
      if (!$time_slot['open']) {
        // The building is closed.
        $output .= t('Closed');
      }
      else {
        $time_slot_time = $time_slot['time'];
        if (($reservations[$display_room][$time_slot_time]['booked']) &&
          (_room_reservations_full_access())) {
          // The room has already been reserved for this time slot
          // and the user is allowed to view any reservation.
          if ($reservations[$display_room][$time_slot_time]['name']) {
            $output .= "<a href='" . $base_url . '/room_reservations/view/' .
              $reservations[$display_room][$time_slot_time]['id'] . "'>" .
              check_plain($reservations[$display_room][$time_slot_time]['name'])
              . '</a>';
          }
          else {
            $output .= '&nbsp;&nbsp;';
          }
        }
        elseif (($reservations[$display_room][$time_slot_time]['booked']) &&
          ($user_login_name ==
          $reservations[$display_room][$time_slot_time]['user_name'])) {
          // The room has already been reserved for this time slot by the
          // current user.
          if ($reservations[$display_room][$time_slot_time]['name']) {
            $output .= "<a href='" . $base_url . '/room_reservations/view/' .
              $reservations[$display_room][$time_slot_time]['id'] . "'>" .
              check_plain($reservations[$display_room][$time_slot_time]['name'])
              . '</a>';
          }
          else {
            $output .= '&nbsp;&nbsp;"';
          }
        }
        elseif (($reservations[$display_room][$time_slot_time]['booked']) &&
          ($user_login_name !=
          $reservations[$display_room][$time_slot_time]['user_name'])) {
          // The room has already been reserved for this time slot by a
          // different user.
          if ($reservations[$display_room][$time_slot_time]['name']) {
            $output .= check_plain(
              $reservations[$display_room][$time_slot_time]['name']);
          }
          else {
            $output .= '&nbsp;&nbsp;"';
          }
        }
        else {
          if ($user->uid) {
            // The time slot is open and the user is already logged in.
            $output .= "<a href='" . $base_url . '/room_reservations/add/' .
              $month_number . '/' . $xday . '/' .
              $time_slot_time . '/' . $display_room . "'>" .
              t('Reserve') . '</a>';
          }
          else {
            // The time slot is open and the user must still log in.
            $output .= "<a href='" . $base_url .
              '/user/login?destination=room_reservations/add/' .
              $month_number . '/' . $xday . '/' .
              $time_slot_time . '/' . $display_room . "'>" .
              t('Reserve') . '</a>';
          }
        }
      }
      $output .= '</td></tr>';
    }
    $output .= '</table>';
    $output .= '</div>';
  }
  // Desktop display.
  else {
    $calendar_text
      = check_markup(_room_reservations_get_variable('calendar_text'));
    $reserve_room_instructions_text = check_markup(
      _room_reservations_get_variable('reserve_instructions'));
    if (!$reserve_room_instructions_text) {
      $reserve_room_instructions_text
        = t('To reserve a room, click on the desired time/day in the calendar
        below.  You will be asked to login.');
    }
    // Room reservations container.
    $output = "<div id='rooms'>";
    $output .= "<div id='tabbedPanels'>";
    // Info box - user reservations, maps, link to policies.
    $output .= "<div id='info'><div>$calendar_text</div>
      <div><img id='arrow' src='" .
      base_path() . drupal_get_path('module', 'room_reservations') .
      "/view/images/007354-blue-jelly-icon-arrows-arrow-sparkle.png' />
      <span id='start'>" . $reserve_room_instructions_text .
      '</span></div></div>';
    // Calendar date.
    $output .= "<h3 class='date'>" . t('Reservation Calendar') . '</h3>' .
      "<div class='hours'>"  . $day_of_week . ", " . $month . ' ' . $xday .
      ", " . $year . '&nbsp;&nbsp;' . $building_hours_display . '</div>' .
      "<div id='date-change'><select id='date-selectbox'><option value='#'>" .
      t('View a different day') . '</option>';
    foreach ($dates as $day) {
      $building_hours_day = $building_hours[$day['yyyymmdd']];
      if (($building_hours_day['open']) && (!$day['selected'])) {
        $output .= "<option value='" . $day['month-number'] . '/' .
        $day['day'] . "'>" . t($day['display']) . '</option>';
      }
    }
    $output .= '</select></div>';
    // Reservation calendar grid:
    //
    // Each block on the grid is assigned one of the following classes:
    //
    // closed - the building is closed;
    // booked - the building is open and the time slot has been reserved;
    // open - the building is open, the time slot has not been reserved, but
    // the user must login to reserve the time slot;
    // reservable - the building is open, the time slot has not been reserved,
    // and the user is able to reserve the time slot.
    // Tabs.
    $output .= "<ul class='room-tabs'>";
    $i = 0;
    foreach ($categories as $category) {
      // Set the active tab - first tab if none has been selected yet.
      if (!$selected_category) {
        $active = ($i == 0) ? " class='active'" : "";
        $i++;
      }
      else {
        $active = ($category['category'] == $selected_category) ?
          " class='active'" : "";
      }
      $output .= '<li><a' . $active . " href='#" . $category['category'] . "'>"
        . $category['display'] . '</a></li>';
    }
    // Panel container.
    $output .= "</ul><div class='panelContainer'>";
    // If the user is logged in, the class for each unbooked time slot is
    // 'reservable'.  If the user is not logged in, the class is 'open'.
    // Only logged in users can click a block of time to open the reservation
    // form.
    $unbooked_class = ($user->uid) ? 'reservable' : 'open';
    // Create a tab for each room category.  Each tab contains a grid of time
    // slots and rooms.
    $i = 0;
    foreach ($categories as $category) {
      // Show the first tab and hide all others.
      if (!$selected_category) {
        $show = ($i == 0) ? 'show' : 'hide';
        $i++;
      }
      else {
        $show = ($category['category'] == $selected_category) ? 'show' : 'hide';
      }
      // Date and building hours.
      $output .= "<div id='" . t($category['category']) . "' class='panel " .
        $show . "'><div class='gcolumns'>
        <div class='grid-column hours-column'><ul><li class='room-info'>" .
        t('Room') . '</li>';
      $output .= "<li class='room-info room-info-heading'>" . t('Capacity') .
        '</li>';
      // Available hours column.
      foreach ($hours as $time_slot) {
        $time_display = ($time_slot['class'] == 'odd') ?
          t($time_slot['display']) : "";
        $output .= "<li class='" . $time_slot['class'] . " timeslot' time='" .
          $time_slot['time'] . "' " .
          '>' . $time_display . '</li>';
      }
      $output .= "<li class='room-info room-info-footer'>" . t('Room') .
        '</li>';
      $output .= "<li class='room-info'>" . t('Capacity') . '</li>';
      $output .= '</ul></div>';
      // Count the number of rooms in the selected category.
      $rooms_per_category = 0;
      foreach ($rooms as $room) {
        if ($room['category'] == $category['category']) {
          $rooms_per_category++;
        }
      }
      // Column for each room in the category.
      foreach ($rooms as $room) {
        $room_name = $room['name'];
        if ($room['category'] == $category['category']) {
          // Room name, capacity.
          $output .= "
            <div class='grid-column room-column'><ul><li class='room-info'>" .
            "<a href='" . $base_url . "/room_reservations/rooms/#" .
            $room['name'] . "'>" . $room['name'] . '</a></li>';
          $output .= "<li class='room-info room-info-heading'>" .
            $room['capacity'] . '</li>';
          // Populate each time slot.
          foreach ($hours as $time_slot) {
            $time = $time_slot['time'];
            $open = $time_slot['open'];
            // Determine if the building is open or closed for this time slot.
            if ($open) {
              $booked_class = ($reservations[$room_name][$time]['booked']) ?
                'booked' : $unbooked_class;
            }
            else {
              $booked_class = 'closed';
            }
            // The time slot can be reserved by the current user.
            $viewable_class=null;
            if ($booked_class == 'reservable') {
              $link = "<a href='" . $base_url . "/room_reservations/add/" .
                $month_number . "/" . $xday . "/" .
                $time_slot['time'] . "/" . $room['name'] . "'><img src='" .
                base_path() . drupal_get_path('module', 'room_reservations') .
                "/view/images/clear.png' /></a>";
              $viewable = '';
            }
            // The time slot can be reserved by the current user,
            // but the user must login first.
            elseif ($booked_class == 'open') {
              $link = "<a href='" . $base_url .
                "/user/login?destination=room_reservations/add/" .
                $month_number . "/" . $xday . "/" .
                $time_slot['time'] . "/" . $room['name'] . "'><img src='" .
                base_path() . drupal_get_path('module', 'room_reservations') .
                "/view/images/clear.png' /></a>";
              $viewable = '';
            }
            elseif ($booked_class == 'closed') {
              $link = '';
            }
            else {
              // The time slot has a reservation that can be viewed by the
              // current user. 
              $user_name = $reservations[$room_name][$time]['user_name'];
              $viewable_class = (($full_access) ||
                ($user_login_name == $user_name)) ? 'viewable' : '';
              if ($viewable_class == 'viewable') {
                $id = $reservations[$room_name][$time]['id'];
                $link = "<a href='" . $base_url .
                "/room_reservations/view/" . $id . "'>" .
                check_plain($reservations[$room_name][$time]['name']) . '</a>';
              }
              else {
                // The time slot has a reservation that cannot be viewed by
                // the current user.
                $link = check_plain($reservations[$room_name][$time]['name']);
              }
            }
            // Room name and capacity.
            $output .= "<li class='" . $time_slot['class'] . ' ' .
              $booked_class .  " " . $viewable_class . "'>" . $link . "</li>";
          }
          $output .= "<li class='room-info room-info-footer'><a href='" .
            $base_url . "/room_reservations/rooms/#" .
            $room['name'] . "'>" .
            $room['name'] . "</a></li><li class='room-info'>" .
            $room['capacity'] . "</li>";
          $output .= '</ul></div>';
        }
      }
      $output .=
        '</div></div>';
    }
    $output .= '</div>';
    $output .= '</div>';
    $output .= '</div>';
  }
  return $output;
}
