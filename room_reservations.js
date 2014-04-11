Drupal.behaviors.room_reservations = {
    attach: function(context, settings) {
        jQuery('#rooms ul li.reservable, #rooms ul li.open').mouseover(function() {
            jQuery(this).css({
                'background-color': '#999999'
            });
        });
        jQuery('#rooms ul li.reservable, #rooms ul li.open').mouseout(function() {
            jQuery(this).css({
                'background-color': '#FFFFFF'
            });
        });

        jQuery('.room-tabs a').click(function() {
            jQuery('.panel').hide();
            jQuery('.room-tabs a.active').removeClass('active');
          jQuery(this).addClass('active').blur();
            var panel = jQuery(this).attr('href');
            jQuery(panel).fadeIn(250);
//alert(jQuery(this).attr('href'));
            return false;
        });

        jQuery('#date-selectbox').change(function() {
            var val = jQuery(this).find('option:selected').attr('value');
            var path = window.location.href;
            var loc = path.lastIndexOf('room_reservations');
            var end = loc + 17;
            var newpath = path.substring(0, end).concat('/');
            window.location.href = newpath.concat(val);
        });

        jQuery('#edit-textmsg').each(function() {
            if ($(this).attr('checked')) {
                jQuery('#txtmsg-fields').slideDown('fast');
            }
            else {
                jQuery('#txtmsg-fields').slideUp('fast');
            }
        });
        jQuery('#edit-textmsg').click(function() {
            if (jQuery(this).attr('checked')) {
                jQuery('#txtmsg-fields').slideDown('fast');
            }
            else {
                jQuery('#txtmsg-fields').slideUp('fast');
            }
        });
    }
}