(function ($) {
  Drupal.behaviors.chatLink = {
    attach: function (context, settings) {
      $('a[class=chat-link]').click( function() {
        var alternateWidth = 660;
        var alternateHeight = 450;
        if ((self.screen.availWidth * 0.8 < alternateWidth) || (self.screen.availHeight * 0.8 < alternateHeight)) {
          alternateWidth = Math.min(alternateWidth, self.screen.availWidth * 0.8);
          alternateHeight = Math.min(alternateHeight, self.screen.availHeight * 0.8);
	}
        window.open(this.href, 'ajaxchat', 'toolbar=no, location=no, status=no, menubar=no, scrollbars=no ,width=' + alternateWidth + ',height=' + alternateHeight + ', resizable=yes');
        return false;
      });
    }
  }
}(jQuery));
