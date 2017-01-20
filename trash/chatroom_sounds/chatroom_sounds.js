(function ($) {
    Drupal.Nodejs.callbacks.chatroomSoundsPlaySound = {
        callback: function (message) {
            if(Drupal.settings.chatroom.uid != message.data.uid)
                document.getElementById('notification_sound').play();
        }
    };
    
    $(document).ready(function(){
        $('#chatroom-sound-on-off-button').click(function (e) {           
            e.preventDefault();
            e.stopPropagation();
            Drupal.settings.chatroom.sound = !Drupal.settings.chatroom.sound;
            if(Drupal.settings.chatroom.sound)
                $('#chatroom-sound-on-off-button').attr('value', 'Turn Sound Off');
            else
                $('#chatroom-sound-on-off-button').attr('value', 'Turn On Sound');    
        });
    })
    
})(jQuery);