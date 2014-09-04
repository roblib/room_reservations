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
 * @param array $variables
 *   Standard $variables array.
 *
 * @return string
 *   The html for displaying the page.
 */
function theme_room_reservations($variables) {
  global $user;
  global $base_url;

  extract($variables);
  // User information.
  $full_access = FALSE;
  $user_login_name = NULL;
  if ($user->uid) {
    $user_login_name = $user->name;
    $full_access = user_access('administer room reservations system');
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
  // $mobile = _room_reservations_detect_mobile();
  $mobile = _detect_mobile('mobile');

  // Mobile display.
  if ($mobile) {
    $output = "<div id='rooms-mobile'><div id='info'>";
    // Room.
    if ($selected_category) {
      $display_room = $selected_category;
    }
    else {
      foreach ($rooms as $room) {
        $display_room = $room['title'];
        break;
      }
    }
    // Form to change day and or room.
    $output .= drupal_get_form('room_reservations_select_room_date_form', $building_hours, $dates, $rooms, $categories, $selected_category);
    // Calendar date.
    $display_date = $day_of_week . ', ' . $month . ' ' . $xday . ', ' . $year;
    $output .= "<b>" . t('Reservation Calendar') . "</b><div class='hours'>" . $display_date . '<br />'
      . $building_hours_display . '<br /><b>' . $display_room . '</b></br></div>';

    // Table to display times and reservations.
    $output .= '<table>';
    foreach ($hours as $time_slot) {
      $output .= '<tr><td>' . $time_slot['display'] .
        '&nbsp;&nbsp;</td><td>';
      if (!$time_slot['open']) {
        // The building is closed.
        $output .= t('Closed');
      }
      else {
        $time_slot_time = $time_slot['time'];
        if (($reservations[$rid][$time_slot_time]['booked']) && _room_reservations_full_access()) {
          // The room has already been reserved for this time slot
          // and the user is allowed to view any reservation.
          if ($reservations[$display_room][$time_slot_time]['name']) {
            $output .= "<a href='" . $base_url . '/room_reservations/view/' . $reservations[$display_room][$time_slot_time]['id'] . "'>" .
              check_plain($reservations[$display_room][$time_slot_time]['name']) . '</a>';
          }
          else {
            $output .= '&nbsp;&nbsp;';
          }
        }
        elseif (($reservations[$rid][$time_slot_time]['booked']) &&
          ($user_login_name == $reservations[$room->rid][$time_slot_time]['user_name'])) {
          // The room has already been reserved for this time slot by the
          // current user.
          if ($reservations[$room->rid][$time_slot_time]['name']) {
            $output .= "<a href='" . $base_url . '/room_reservations/view/' . $reservations[$display_room][$time_slot_time]['id'] . "'>" .
              check_plain($reservations[$display_room][$time_slot_time]['name']) . '</a>';
          }
          else {
            $output .= '&nbsp;&nbsp;"';
          }
        }
        elseif (($reservations[$rid][$time_slot_time]['booked']) &&
          ($user_login_name != $reservations[$rid][$time_slot_time]['user_name'])) {
          // The room has already been reserved for this time slot by a
          // different user.
          if ($reservations[$rid][$time_slot_time]['name']) {
            $output .= check_plain($reservations[$rid][$time_slot_time]['name']);
          }
          else {
            $output .= '&nbsp;&nbsp;"';
          }
        }
        else {
          if ($user->uid) {
            // The time slot is open and the user is already logged in.
            $output .= "<a href='" . $base_url . '/room_reservations/add/' . $month_number . '/' . $xday . '/' . $time_slot_time . '/' . $rid . "'>" .
              t('Reserve') . '</a>';
          }
          else {
            // The time slot is open and the user must still log in.
            $output .= "<a href='" . $base_url . '/user/login?destination=room_reservations/add/' . $month_number . '/' . $xday . '/' . $time_slot_time . '/' . $rid . "'>" .
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
    $calendar_text = check_markup(_room_reservations_get_variable('calendar_text'));
    $reserve_room_instructions_text = check_markup(_room_reservations_get_variable('reserve_instructions'));
    if (!$reserve_room_instructions_text) {
      $reserve_room_instructions_text = t('To reserve a room, click on the desired time/day in the calendar below. You will be asked to login.');
    }
    // Room reservations container.
    $output = "<div id='rooms'>";
    $output .= "<div id='tabbedPanels'>";

    // Info box - user reservations, maps, link to policies.
    $output .= "<div id='info'><div>$calendar_text</div>
      <div><img id='arrow' src='" . base_path() . drupal_get_path('module', 'room_reservations') . "/images/007354-blue-jelly-icon-arrows-arrow-sparkle.png' />
      <span id='start'>" . $reserve_room_instructions_text . '</span></div></div>';

    // Calendar date.
    $output .= '<h3 class="date">' . t('Reservation Calendar') . '</h3>' .
      '<div class="hours">' . $day_of_week . ', ' . $month . ' ' . $xday . ', ' . $year . '&nbsp;&nbsp;' . $building_hours_display . '</div>';

    // Add new Day Selector as popup calendar since we now allow going
    // out up to 6 months rather than 15 days.
    // CHANGED - setting var for drupal_get_form and sending that to
    // drupal_render as it needs a var to be passed by reference
     //$output .= '<div id="date-change">'
     //. drupal_render(drupal_get_form('room_reservations_admin_date_picker'))
     //. '</div>';
    $date_picker_form = drupal_get_form('room_reservations_admin_date_picker');
    $output .= '<div id="date-change">' . drupal_render($date_picker_form) . '</div>';

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
        $active = ($category['title'] == $selected_category) ? " class='active'" : "";
      }

      $id = strtolower(preg_replace('/[^a-zA-Z0-9-]+/', '-', $category['title']));
      $output .= '<li><a' . $active . " href='#" . $id . "'>" . $category['title'] . '</a></li>';
    }

    // Panel container.
    $output .= "</ul><div class='panelContainer'>";
    // If the user is logged in, the class for each unbooked time slot is
    // 'reservable'. If the user is not logged in, the class is 'open'.
    // Only logged in users can click a block of time to open
    // the reservation form.
    $unbooked_class = ($user->uid) ? 'reservable' : 'open';
    // Create a tab for each room category.
    // Each tab contains a grid of time slots and rooms.
    $i = 0;
    foreach ($categories as $category) {
      // Show the first tab and hide all others.
      if (!$selected_category) {
        $show = ($i == 0) ? 'show' : 'hide';
        $i++;
      }
      else {
        $show = ($category['title'] == $selected_category) ? 'show' : 'hide';
      }

      $id = strtolower(preg_replace('/[^a-zA-Z0-9-]+/', '-', $category['title']));


      $output .= "<div id = '" . $id . "' class = 'panel " . $show . "'>";

      // CHANGED - Added new function call.
      $hour_column = room_reservations_build_hours_column($hours);
      $output .= $hour_column;


      // CHANGED - Moved to function.
      // Date and building hours.
/*
      $output .= "<div id='" . $id . "' class='panel " . $show . "'><div class='gcolumns'>
        <div class='grid-column hours-column'><ul><li class='room-info'>" . t('Room') . '</li>';
      $output .= "<li class='room-info room-info-heading'>" . t('Capacity') . '</li>';
      // Available hours column.
      foreach ($hours as $time_slot) {
        $time_display = ($time_slot['class'] == 'odd') ? $time_slot['display'] : "";
        $output .= "<li class='" . $time_slot['class'] . " timeslot' time='" . $time_slot['time'] . "' " . '>' . $time_display . '</li>';
      }
      $output .= "<li class='room-info room-info-footer'>" . t('Room') . '</li>';
      $output .= "<li class='room-info'>" . t('Capacity') . '</li>';
      $output .= '</ul></div>';
*/




      // Count the number of rooms in the selected category.
      $rooms_per_category = 0;
      foreach ($rooms as $room) {
        $rid = $room['nid'];
        if ($room['room_category']['und'][0]['target_id'] == $category['nid']) {
          $rooms_per_category++;
        }
      }
      // Column for each room in the category.
      $room_count = 0;
      foreach ($rooms as $room) {


        // CHANGED - ADDED
        if ($room['room_category']['und'][0]['target_id'] == $category['nid']) {
          $room_count++;

          // Wrap and repeat hours column after every 7th room per category.
          if ($room_count == 8 OR $room_count == 15 OR $room_count == 22 OR $room_count == 29
            OR $room_count == 36 OR $room_count == 43 OR $room_count == 50) {

            $output .= "</div>";
            //$output .= "<div id=" . $id . " class=\"panel\"" . $show . ">";
            $output .= room_reservations_build_hours_column($hours);
          }
        }





        $rid = $room['nid'];
        $room_name = $room['title'];
        $room_link = l($room_name, 'node/' . $rid);
        $room_desc = $room['body']['und'][0]['value'];

        // Use qtip if we have it; for now add as a dependency in
        // .info so don't really need this test.
        if (module_exists('qtip')) {
          $room_info = '<span class="qtip-link"><span class="qtip-tooltip">' . $room_desc . '</span>' . $room_link . '</span>';
        }
        else {
          $room_link;
        }

        if ($room['room_category']['und'][0]['target_id'] == $category['nid']) {
          // Room name, capacity.
          $output .= '<div class="grid-column room-column"><ul><li class="room-info">' . $room_info . '</li>';
          $output .= "<li class='room-info room-info-heading'>" . $room['room_capacity']['und'][0]['value'] . '</li>';
          // Populate each time slot.
          foreach ($hours as $time_slot) {
            $time = $time_slot['time'];
            $open = $time_slot['open'];
            // Determine if the building is open or closed for this time slot.
            if ($open) {
              $booked_class = ($reservations[$rid][$time]['booked']) ? 'booked' : $unbooked_class;
            }
            else {
              $booked_class = 'closed';
            }
            // The time slot can be reserved by the current user.
            if ($booked_class == 'reservable' && (user_access('create room reservations standard') || user_access('create room reservations extended') || user_access('administer room reservations system'))) {
              $text_temp = '<img src="' . base_path() . drupal_get_path('module', 'room_reservations') . '/images/clear.png" />';
              $link_temp = 'node/add/room-reservations-reservation/' . $month_number . '/' . $xday . '/' . $time_slot['time'] . '/' . $rid;

              // $link = l('<img src="' . base_path()
              // . drupal_get_path('module', 'room_reservations')
              // . '/images/clear.png" />',
              // 'node/add/room-reservations-reservation/' . $month_number
              // . '/' . $xday . '/' . $time_slot['time'] . '/' . $rid,
              // array('html' => TRUE)
              // );

              $link = l($text_temp, $link_temp, array('html' => TRUE));

              $viewable = '';
            }
            // The time slot can be reserved by the current user,
            // but the user must login first.
            elseif ($booked_class == 'open') {
              $link = "<a href='" . $base_url . "/user/login?destination=node/add/room-reservations-reservation/" . $month_number . "/" . $xday . "/" .
                $time_slot['time'] . "/" . $rid . "'><img src='" . base_path() . drupal_get_path('module', 'room_reservations') .
                "/images/clear.png' /></a>";
              $viewable = '';
            }
            elseif ($booked_class == 'closed') {
              $link = '';
            }
            else {
              // The time slot has a reservation that
              // can be viewed by the current user.
              $viewable_class = (($full_access) || ($reservations[$rid][$time]['user'] == $user->uid)) ? 'viewable' : '';

              if ($viewable_class == 'viewable') {
                $id = $reservations[$rid][$time]['id'];

                $link = l($reservations[$rid][$time]['name'], 'node/' . $id . '/edit');

              }
              // The time slot has a reservation that cannot
              // be viewed by the current user.
              // And we are NOT allowed to see the Title.
              elseif (!empty($reservations[$rid][$time]['blocked'])) {
                $link = t('Booked');
              }
              // The time slot has a reservation that cannot
              // be viewed by the current user.
              // But we are allowed to see the Title.
              else {
                $link = $reservations[$rid][$time]['name'];
              }
            }
            // Room name and capacity.

            // CHANGE - $viewable_class var being called
            // even if it's not defined
            // $output .= "<li class='" . $time_slot['class'] .
            // ' ' . $booked_class .  " " . $viewable_class . "'>"
            // . $link . "</li>";
            $output .= "<li class='" . $time_slot['class'] . ' ' . $booked_class . "'>" . $link . "</li>";
          }
          $output .= '
            <li class="room-info">' . $room_info . '</li>
            <li class="room-info">' . $room['room_capacity']['und'][0]['value'] . '</li>';
          $output .= '</ul></div>';
        }
      }
      $output .=
        '</div></div>';
    }
    $output .= '</div><div class="clear"></div>';
    $output .= '</div>';
    $output .= '</div>';
  }
  return $output;
}


/**
 * Description.
 *
 * @param array $form_state
 *   Standard $form_state array.
 *
 * @return string
 *   The html for displaying the page.
 */
function room_reservations_admin_date_picker($form_state) {
  if (($arg1 = arg(1)) && ($arg2 = arg(2))) {
    $default = $arg1 . '-' . $arg2;
  }
  else {
    $default = date('m-d');
  }
  $parts = explode('-', $default);
  if (user_access('create room reservations extended')) {
    $advancedays = variable_get('room_reservations_advance_extended', 180);
  }
  else {
    $advancedays = variable_get('room_reservations_advance_standard', 14);
  }
  $yearnow = date('Y');

  $absdaynow = date('z');
  $absdaydefault = date('z', mktime(0, 0, 0, $parts[0], $parts[1], $yearnow));
  if ($absdaynow > $absdaydefault) {
    $yeardefault = $yearnow + 1;
  }
  else {
    $yeardefault = $yearnow;
  }

  $form['date'] = array(
    '#type' => 'date_popup',
    '#default_value' => $yeardefault . '-' . $default . ' 00:00:00',
    '#date_type' => DATE_DATETIME,
    '#date_timezone' => date_default_timezone(),
    '#date_format' => 'Y/m/d',
    '#date_increment' => 1,
    '#date_year_range' => '-0:+1',
    '#datepicker_options' => array(
      'minDate' => '+0d',
      'maxDate' => $advancedays . 'D',
    ),
  );

  $form['#after_build'][] = '_room_reservations_admin_date_picker_afterbuild';
  return $form;
}

/**
 * Description.
 *
 * @param array $form
 *   Standard $form array.
 * @param array $form_state
 *   Standard $form_state array.
 *
 * @return string
 *   The html for displaying the page.
 */
function _room_reservations_admin_date_picker_afterbuild($form, &$form_state) {
  $form['date']['date']['#title'] = '';
  $form['date']['date']['#description'] = t('click in box to select date');
  return $form;
}

/**
 * Build hours of day column.
 *
 * @param array $hours
 *   Array of hours.
 *
 * @return string
 *   The HTML for displaying the hours of day column.
 */
function room_reservations_build_hours_column($hours) {
  $output = "<div class='gcolumns'>
              <div class='grid-column hours-column'><ul><li class='room-info'>" . t('Room') . "</li>
                <li class='room-info room-info-heading'>" . t('Capacity') . '</li>';

  foreach ($hours as $time_slot) {
    $time_display = ($time_slot['class'] == 'odd') ? $time_slot['display'] : "";
    $output .= "<li class='" . $time_slot['class'] . " timeslot' time='" . $time_slot['time']
      . "' " . '>' . $time_display . '</li>';
  }

  $output .= "<li class='room-info room-info-footer'>" . t('Room') . "</li>
              <li class='room-info'>" . t('Capacity') . "</li></ul></div>";




  /*
        <div class='gcolumns'>
          <div class='grid-column hours-column'><ul><li class='room-info'>" . t('Room') . '</li>';
        $output .= "<li class='room-info room-info-heading'>" . t('Capacity') . '</li>';
        // Available hours column.
        foreach ($hours as $time_slot) {
          $time_display = ($time_slot['class'] == 'odd') ? $time_slot['display'] : "";
          $output .= "<li class='" . $time_slot['class'] . " timeslot' time='" . $time_slot['time'] . "' " . '>' . $time_display . '</li>';
        }
        $output .= "<li class='room-info room-info-footer'>" . t('Room') . '</li>';
        $output .= "<li class='room-info'>" . t('Capacity') . '</li>';
        $output .= '</ul></div>';
  */


  return $output;
}
