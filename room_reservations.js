
/**
 *  @file
 * JQuery functionality for the Room Reservations module.
 */

/**
 * JQuery function.
 *
 */
(function ($) {

Drupal.behaviors.room_reservations = {};

Drupal.behaviors.room_reservations.attach = function(context) {
    // select room and start time
    $('#rooms ul li.reservable, #rooms ul li.open').mouseover(function() {
      $(this).css({
        'background-color' : '#999999'
      });
    });
    $('#rooms ul li.reservable, #rooms ul li.open').mouseout(function() {
      $(this).css({
        'background-color' : '#FFFFFF'
      });
    });

    // Show the selected category panel.
    $('.room-tabs a').click(function() {
      $this = $(this);
      $('.panel').hide();
      $('.room-tabs a.active').removeClass('active');
      $this.addClass('active').blur();
      var panel = $this.attr('href');
      $(panel).fadeIn(250);
      return false;
    });

    // Change calendar date displayed.
    $('#edit-date-datepicker-popup-0').change(function() {
      var val = $(this).attr('value').substring(5, end);
      var path = window.location.href;
      var loc = path.lastIndexOf('room_reservations');
      var end = loc + 17;
      var newpath = path.substring(0, end).concat('/');
      window.location.href = newpath.concat(val);
    });

    // Show form fields for text message confirmation and reminder.
    $('#edit-textmsg').each(function() {
      if ($(this).attr('checked')) {
        $('#txtmsg-fields').slideDown('fast');
      }
      else {
        $('#txtmsg-fields').slideUp('fast');
      }
    });
    $('#edit-textmsg').click(function() {
      if ($(this).attr('checked')) {
        $('#txtmsg-fields').slideDown('fast');
      }
      else {
        $('#txtmsg-fields').slideUp('fast');
      }
    });
  };
}(jQuery));
