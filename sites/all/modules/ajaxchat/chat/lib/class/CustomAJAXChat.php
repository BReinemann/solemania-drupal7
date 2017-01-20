<?php

/*
 * @package AJAX_Chat
 * @author Sebastian Tschan
 * @copyright (c) Sebastian Tschan
 * @license GNU Affero General Public License
 * @link https://blueimp.net/ajax/
 */

class CustomAJAXChat extends AJAXChat {

  // Returns an associative array containing userName, userID and userRole
  // Returns null if login is invalid
  function getValidLoginUserData() {
    global $user;

    // Check if we have a valid registered user:
    if($user->status == 1) {
      $userData = array();
      $userData['userID'] = $user->uid;

      $userData['userName'] = $this->trimUserName($user->name);

      if (in_array('administrator', $user->roles)) {
        $userData['userRole'] = AJAX_CHAT_ADMIN;
      }
      else if (in_array('moderator', $user->roles)) {
        $userData['userRole'] = AJAX_CHAT_MODERATOR;
      }
      else {
        $userData['userRole'] = AJAX_CHAT_USER;
      }

      return $userData;

    } else {
      // Guest users:
      return $this->getGuestUser();
    }
  }

  // Store the channels the current user has access to
  // Make sure channel names don't contain any whitespace
  function &getChannels() {
    if($this->_channels === null) {
      $this->_channels = array();

      $allChannels = $this->getAllChannels();

      drupal_alter('ajaxchat_channels', $this, $allChannels);

      foreach($allChannels as $key=>$value) {
        // Check if we have to limit the available channels:
        if($this->getConfig('limitChannelList') && !in_array($value, $this->getConfig('limitChannelList'))) {
          continue;
        }

        // This would be the point to call a Drupal taxonomy access control function
        // to determine if the current user should be able to access a channel.
        // For now, everyone can access any channel.
        $this->_channels[$key] = $value;
      }
    }
    return $this->_channels;
  }

  // Store all existing channels
  // Make sure channel names don't contain any whitespace
  function &getAllChannels() {
    if($this->_allChannels === null) {

      drupal_alter('ajaxchat_all_channels', $this, $this->_allChannels);

      $defaultChannelFound = false;
      foreach ($this->_allChannels as $channelName => $channelId) {
        if (!$defaultChannelFound && $channelId == $this->getConfig('defaultChannelID')) {
          $defaultChannelFound = true;
        }
      }

      if(!$defaultChannelFound) {
        // Add the default channel as first array element to the channel list:
        $this->_allChannels = array_merge(
          array(
            $this->trimChannelName($this->getConfig('defaultChannelName')) => $this->getConfig('defaultChannelID')
          ),
          $this->_allChannels
        );
      }

    }
    return $this->_allChannels;
  }

  // Initialize custom configuration settings
  function initCustomConfig() {
    $db = Database::getConnection();
    $db_settings = $db->getConnectionOptions();

    // Ideally we'd allow AJAX chat to use the same database connection
    // as Drupal so we'd save the expense of creating a second connection.
    // So far, I've not been able to figure out how to do this.

    // $this->setConfig('dbConnection', 'link', $db->db_connect_id);

    $this->setConfig('dbConnection', 'host', $db_settings['host']);
    $this->setConfig('dbConnection', 'user', $db_settings['username']);
    $this->setConfig('dbConnection', 'pass', $db_settings['password']);
    $this->setConfig('dbConnection', 'name', $db_settings['database']);
    $this->setConfig('dbConnection', 'type', $db_settings['driver']);
    $this->setConfig('dbConnection', 'link', NULL);

    // Translate all the AJAX chat configuration variables from Drupal
    // to native counterparts.

    // Arrays.
    $vars = array(
      'openingWeekDays' => array('ajaxchat_opening_week_days', '0,1,2,3,4,5,6'),
      'requestMessagesPriorChannelEnterList' => array('ajaxchat_show_prior_channel_list', ''),
      'logsUserAccessChannelList' => array('ajaxchat_log_user_access_channel_list', ''),
    );

    foreach ($vars as $key => $var) {
      $value = variable_get($var[0], $var[1]);
      $this->setConfig($key, NULL, !empty($value) ? explode(',', $value) : array());
    }

    // Strings.
    $vars = array(
      'privateChannelPrefix' => array('ajaxchat_private_channel_prefix', '['),
      'privateChannelSuffix' => array('ajaxchat_private_channel_suffix', ']'),
      'openingHour' => array('ajaxchat_opening_hour', 0),
      'closingHour' => array('ajaxchat_closing_hour', 24),
      'chatBotName' => array('ajaxchat_chat_bot_name', 'ChatBot'),
      'inactiveTimeout' => array('ajaxchat_inactive_timeout', 2),
      'inactiveCheckInterval' => array('ajaxchat_inactive_check_interval', 5),
      'requestMessagesTimeDiff' => array('ajaxchat_prior_message_time_difference', 24),
      'requestMessagesLimit' => array('ajaxchat_prior_message_max', 10),
      'maxUsersLoggedIn' => array('ajaxchat_max_users', 100),
      'messageTextMaxLength' => array('ajaxchat_max_message_length', 4096),
      'maxMessageRate' => array('ajaxchat_max_messages_per_minute', 20),
      'defaultBanTime' => array('ajaxchat_default_ban_time', 5),
      'logsRequestMessagesTimeDiff' => array('ajaxchat_log_time_difference', 1),
      'logsRequestMessagesLimit' => array('ajaxchat_log_request_limit', 1),
      'logsFirstYear' => array('ajaxchat_log_first_year', date('Y')),
      'logsPurgeTimeDiff' => array('ajaxchat_log_purge_time_difference', 365),
      'socketServerHost' => array('ajaxchat_socket_server_host', ip_address()),
      'socketServerIP' => array('ajaxchat_socket_server_ip', '127.0.0.1'),
      'socketServerPort' => array('ajaxchat_socket_server_port', 1935),
      'socketServerChatID' => array('ajaxchat_socket_server_id', 1),
    );

    foreach ($vars as $key => $var) {
      $this->setConfig($key, NULL, variable_get($var[0], $var[1]));
    }

    // Booleans.
    $vars = array(
      'chatClosed' => array('ajaxchat_closed', 2),
      'allowPrivateChannels' => array('ajaxchat_allow_private_channels', 1),
      'requestMessagesPriorChannelEnter' => array('ajaxchat_show_prior_messages', 1),
      'allowUserMessageDelete' => array('ajaxchat_allow_user_message_delete', 2),
      'ipCheck' => array('ajaxchat_verify_ip', 1),
      'allowPrivateMessages' => array('ajaxchat_allow_private_messages', 1),
      'showChannelMessages' => array('ajaxchat_show_channel_messages', 1),
      'logsUserAccess' => array('ajaxchat_log_user_access', 2),
      'logsPurgeLogs' => array('ajaxchat_log_purge_logs', 2),
      'socketServerEnabled' => array('ajaxchat_socket_server_enabled', 2),
    );

    foreach ($vars as $key => $var) {
      $value = variable_get($var[0], $var[1]);
      $this->setConfig($key, NULL, $value == 2 ? false : true);
    }

  }

  // Initialize custom request variables:
  function initCustomRequestVars() {
    global $user;
    // Auto-login Drupal users:
    if(!$this->getRequestVar('logout') && ($user->uid > 0)) {
      $this->setRequestVar('login', true);
      if (!empty($_GET['id'])) {
        $this->setRequestVar('channelID', $_GET['channelID']);
      }
    }
  }

  function replaceCustomTemplateTags($tag, $tagContent) {
    switch($tag) {

        case 'DRUPAL_AJAX_CHAT_PATH':
          return AJAX_CHAT_PATH;

        case 'DRUPAL_AJAX_STYLE_SHEETS':
          return $this->getStyleSheetLinkTags();

        case 'DRUPAL_AJAX_CHAT_URL':
          return '/chat';

    }
  }

  function getStyleSheetLinkTags() {

    $httpHeader = new AJAXChatHTTPHeader($this->getConfig('contentEncoding'), $this->getConfig('contentType'));
    $template = new AJAXChatTemplate($this, $this->getTemplateFileName(), $httpHeader->getContentType());

    $styleSheets = '';

    foreach($template->ajaxChat->getConfig('styleAvailable') as $style) {
      $alternate = ($style == $template->ajaxChat->getConfig('styleDefault')) ? '' : 'alternate ';
      $styleSheets .= '<link rel="'.$alternate.'stylesheet" type="text/css" href="' . AJAX_CHAT_PATH . 'css/'.rawurlencode($style).'.css" title="'.$template->ajaxChat->htmlEncode($style).'"/>';
    }
    return $styleSheets;
  }

}
?>
