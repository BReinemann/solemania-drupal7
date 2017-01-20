<?php

/**
 * @file
 * Administrative page callbacks for the AJAX chat module.
 */

/**
 * Page callback: Returns a form for creating a new chat channel or container.
 *
 * @param $type
 *   What is being added. Possible values are 'channel' and 'container'.
 * @param $edit
 *   (optional) Associative array containing a term to be edited.
 *   Defaults to an empty array.
 *
 * @return
 *   A form for creating a new channel or container.
 *
 * @see ajaxchat_menu()
 */
function ajaxchat_form_main($type, $edit = array()) {
  $edit = (array) $edit;
  if ((isset($_POST['op']) && $_POST['op'] == t('Delete')) || !empty($_POST['confirm'])) {
    return drupal_get_form('ajaxchat_confirm_delete', $edit['tid']);
  }
  switch ($type) {
    case 'channel':
      return drupal_get_form('ajaxchat_form_channel', $edit);
      break;
    case 'container':
      return drupal_get_form('ajaxchat_form_container', $edit);
      break;
  }
}

/**
 * Form constructor for adding and editing a channel term.
 *
 * @param $edit
 *   (optional) Associative array containing a term to be added or edited.
 *   Defaults to an empty array.
 *
 * @see ajaxchat_form_submit()
 * @ingroup forms
 */
function ajaxchat_form_channel($form, &$form_state, $edit = array()) {
  $edit += array(
    'name' => '',
    'description' => '',
    'tid' => NULL,
    'weight' => 0,
  );
  $form['name'] = array('#type' => 'textfield',
    '#title' => t('Channel name'),
    '#default_value' => $edit['name'],
    '#maxlength' => 255,
    '#description' => t('Short name for this chat channel.'),
    '#required' => TRUE,
  );
  $form['description'] = array('#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => $edit['description'],
    '#description' => t('Description and guidelines for discussions within this channel.'),
  );
  $form['parent']['#tree'] = TRUE;
  $form['parent'][0] = _ajaxchat_parent_select($edit['tid'], t('Parent'), 'channel');
  $form['weight'] = array('#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => $edit['weight'],
    '#description' => t('Channels are displayed in ascending order by weight (channels with equal weights are displayed alphabetically).'),
  );

  $form['vid'] = array('#type' => 'hidden', '#value' => variable_get('ajaxchat_vocabulary', ''));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  if ($edit['tid']) {
    $form['actions']['delete'] = array('#type' => 'submit', '#value' => t('Delete'));
    $form['tid'] = array('#type' => 'hidden', '#value' => $edit['tid']);
  }
  $form['#submit'][] = 'ajaxchat_form_submit';
  $form['#theme'] = 'ajaxchat_form';

  return $form;
}

/**
 * Form submission handler for ajaxchat_form_channel() and ajaxchat_form_container().
 */
function ajaxchat_form_submit($form, &$form_state) {
  if ($form['form_id']['#value'] == 'ajaxchat_form_container') {
    $container = TRUE;
    $type = t('container');
  }
  else {
    $container = FALSE;
    $type = t('channel');
  }

  $term = (object) $form_state['values'];
  $status = taxonomy_term_save($term);
  switch ($status) {
    case SAVED_NEW:
      if ($container) {
        $containers = variable_get('ajaxchat_containers', array());
        $containers[] = $term->tid;
        variable_set('ajaxchat_containers', $containers);
      }
      $form_state['values']['tid'] = $term->tid;
      drupal_set_message(t('Created new @type %term.', array('%term' => $form_state['values']['name'], '@type' => $type)));
      break;
    case SAVED_UPDATED:
      drupal_set_message(t('The @type %term has been updated.', array('%term' => $form_state['values']['name'], '@type' => $type)));
      // Clear the page and block caches to avoid stale data.
      cache_clear_all();
      break;
  }
  $form_state['redirect'] = 'admin/structure/chat';
  return;
}

/**
 * Returns HTML for a chat form.
 *
 * By default this does not alter the appearance of a form at all, but is
 * provided as a convenience for themers.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_ajaxchat_form($variables) {
  return drupal_render_children($variables['form']);
}

/**
 * Form constructor for adding and editing containers.
 *
 * @param $edit
 *   (optional) Associative array containing a container term to be added or edited.
 *   Defaults to an empty array.
 *
 * @see ajaxchat_form_submit()
 * @ingroup forms
 */
function ajaxchat_form_container($form, &$form_state, $edit = array()) {
  $edit += array(
    'name' => '',
    'description' => '',
    'tid' => NULL,
    'weight' => 0,
  );
  // Handle a delete operation.
  $form['name'] = array(
    '#title' => t('Container name'),
    '#type' => 'textfield',
    '#default_value' => $edit['name'],
    '#maxlength' => 255,
    '#description' => t('Short but meaningful name for this collection of related chat channels.'),
    '#required' => TRUE
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => $edit['description'],
    '#description' => t('Description and guidelines for chat channels within this container.')
  );
  $form['parent']['#tree'] = TRUE;
  $form['parent'][0] = _ajaxchat_parent_select($edit['tid'], t('Parent'), 'container');
  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => $edit['weight'],
    '#description' => t('Containers are displayed in ascending order by weight (containers with equal weights are displayed alphabetically).')
  );

  $form['vid'] = array(
    '#type' => 'hidden',
    '#value' => variable_get('ajaxchat_vocabulary', ''),
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );
  if ($edit['tid']) {
    $form['actions']['delete'] = array('#type' => 'submit', '#value' => t('Delete'));
    $form['tid'] = array('#type' => 'value', '#value' => $edit['tid']);
  }
  $form['#submit'][] = 'ajaxchat_form_submit';
  $form['#theme'] = 'forum_form';

  return $form;
}

/**
 * Form constructor for confirming deletion of a taxonomy term.
 *
 * @param $tid
 *   ID of the term to be deleted.
 *
 * @see ajaxchat_confirm_delete_submit()
 * @ingroup forms
 */
function ajaxchat_confirm_delete($form, &$form_state, $tid) {
  $term = taxonomy_term_load($tid);

  $form['tid'] = array('#type' => 'value', '#value' => $tid);
  $form['name'] = array('#type' => 'value', '#value' => $term->name);

  return confirm_form($form, t('Are you sure you want to delete %name?', array('%name' => $term->name)), 'admin/structure/chat', t('Deleting a channel or container will also delete its sub-channels, if any. This action cannot be undone.', array('@content' => url('admin/content'))), t('Delete'), t('Cancel'));
}

/**
 * Form submission handler for ajaxchat_confirm_delete().
 */
function ajaxchat_confirm_delete_submit($form, &$form_state) {
  taxonomy_term_delete($form_state['values']['tid']);
  drupal_set_message(t('The container/channel %term and any children have been deleted.', array('%term' => $form_state['values']['name'])));
  watchdog('content', 'channel: deleted %term and any children.', array('%term' => $form_state['values']['name']));

  $form_state['redirect'] = 'admin/structure/chat';
  return;
}

/**
 * Form constructor for the ajax chat settings page.
 *
 * @see ajaxchat_menu()
 * @see system_settings_form()
 * @ingroup forms
 */
function ajaxchat_admin_settings($form) {
  $form = array();

  // General settings.
  $form['ajaxchat_general'] = array(
    '#type' => 'fieldset',
    '#title' => t('General'),
    '#collapsible' => FALSE,
  );
  $form['ajaxchat_general']['ajaxchat_install_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Ajax Chat path'),
    '#description' => t('This is the path to your Ajax Chat rooms, relative to the Drupal base path.'),
    '#default_value' => variable_get('ajaxchat_install_path', 'chat'),
    '#size' => 14,
  );
  $form['ajaxchat_general']['ajaxchat_closed'] = array(
    '#type' => 'select',
    '#title' => t('Chat closed'),
    '#description' => t('If yes, the chat will only be accessible for the admin.'),
    '#options' => array(1 => 'Yes', 2 => 'No'),
    '#default_value' => variable_get('ajaxchat_closed', 2),
  );
  $hours = drupal_map_assoc(range(0, 24));
  $form['ajaxchat_general']['ajaxchat_opening_hour'] = array(
    '#type' => 'select',
    '#title' => t('Opening hour'),
    '#description' => t('Defines the hour of the day the chat is opened.'),
    '#options' => $hours,
    '#default_value' => variable_get('ajaxchat_opening_hour', 0),
  );
  $form['ajaxchat_general']['ajaxchat_closing_hour'] = array(
    '#type' => 'select',
    '#title' => t('Closing hour'),
    '#description' => t('Defines the hour of the day the chat is closed.'),
    '#options' => $hours,
    '#default_value' => variable_get('ajaxchat_closing_hour', 24),
  );
  $form['ajaxchat_general']['ajaxchat_opening_week_days'] = array(
    '#type' => 'textfield',
    '#title' => t('Open days'),
    '#description' => t('Comma separated list of the days the chat is opened. (0=Sunday & 6=Saturday)'),
    '#default_value' => variable_get('ajaxchat_opening_week_days', '0,1,2,3,4,5,6'),
    '#size' => 14,
  );

  // Channel settings.
  $form['ajaxchat_channel'] = array(
    '#type' => 'fieldset',
    '#title' => t('Channel'),
    '#collapsible' => FALSE,
  );
  $form['ajaxchat_channel']['ajaxchat_allow_private_channels'] = array(
    '#type' => 'select',
    '#title' => t('Allow private channels'),
    '#options' => array(1 => 'Yes', 2 => 'No'),
    '#default_value' => variable_get('ajaxchat_allow_private_channels', 1),
  );
  $form['ajaxchat_channel']['ajaxchat_private_channel_prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Private channel prefix'),
    '#description' => t('Private channels should be distinguished by either a prefix or a suffix or both. (no whitespace)'),
    '#default_value' => variable_get('ajaxchat_private_channel_prefix', '['),
    '#size' => 4,
  );
  $form['ajaxchat_channel']['ajaxchat_private_channel_suffix'] = array(
    '#type' => 'textfield',
    '#title' => t('Private channel suffix'),
    '#default_value' => variable_get('ajaxchat_private_channel_suffix', ']'),
    '#size' => 4,
  );
  $form['ajaxchat_channel']['ajaxchat_show_channel_messages'] = array(
    '#type' => 'select',
    '#title' => t('Show automated channel messages with chat bot'),
    '#description' => t('Defines if login/logout and channel enter/leave messages are displayed.'),
    '#options' => array(1 => 'Yes', 2 => 'No'),
    '#default_value' => variable_get('ajaxchat_show_channel_messages', 1),
  );
  $form['ajaxchat_channel']['ajaxchat_chat_bot_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Channel bot name'),
    '#description' => t('Username displayed for automated bot messages.'),
    '#default_value' => variable_get('ajaxchat_chat_bot_name', 'ChatBot'),
    '#size' => 14,
  );

  // User settings.
  $form['ajaxchat_user'] = array(
    '#type' => 'fieldset',
    '#title' => t('User'),
    '#collapsible' => FALSE,
  );
  $form['ajaxchat_user']['ajaxchat_max_users'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum simultaneous users'),
    '#default_value' => variable_get('ajaxchat_max_users', 100),
    '#size' => 4,
  );
  $form['ajaxchat_user']['ajaxchat_allow_user_message_delete'] = array(
    '#type' => 'select',
    '#title' => t('Allow users to delete messages'),
    '#options' => array(1 => 'Yes', 2 => 'No'),
    '#default_value' => variable_get('ajaxchat_allow_user_message_delete', 2),
  );
  $form['ajaxchat_user']['ajaxchat_inactive_timeout'] = array(
    '#type' => 'textfield',
    '#title' => t('Inactivity timeout'),
    '#description' => t('Minutes until a user is declared inactive (2 minimum).'),
    '#default_value' => variable_get('ajaxchat_inactive_timeout', 2),
    '#size' => 4,
  );
  $form['ajaxchat_user']['ajaxchat_inactive_check_interval'] = array(
    '#type' => 'textfield',
    '#title' => t('Inactivity check interval'),
    '#description' => t('Interval in minutes to check for inactive users.'),
    '#default_value' => variable_get('ajaxchat_inactive_check_interval', 5),
    '#size' => 4,
  );
  $form['ajaxchat_user']['ajaxchat_default_ban_time'] = array(
    '#type' => 'textfield',
    '#title' => t('Default ban time'),
    '#description' => t('Defines the default time in minutes a user gets banned if kicked from a moderator without ban minutes parameter.'),
    '#default_value' => variable_get('ajaxchat_default_ban_time', 5),
    '#size' => 4,
  );
  $form['ajaxchat_user']['ajaxchat_verify_ip'] = array(
    '#type' => 'select',
    '#title' => t('Verify IP address'),
    '#description' => t('If true, checks if the user IP is the same when logged in.'),
    '#options' => array(1 => 'Yes', 2 => 'No'),
    '#default_value' => variable_get('ajaxchat_verify_ip', 1),
  );

  // Message settings.
  $form['ajaxchat_message'] = array(
    '#type' => 'fieldset',
    '#title' => t('Message'),
    '#collapsible' => FALSE,
  );
  $form['ajaxchat_message']['ajaxchat_allow_private_messages'] = array(
    '#type' => 'select',
    '#title' => t('Allow private messages'),
    '#options' => array(1 => 'Yes', 2 => 'No'),
    '#default_value' => variable_get('ajaxchat_allow_private_messages', 1),
  );
  $form['ajaxchat_message']['ajaxchat_show_prior_messages'] = array(
    '#type' => 'select',
    '#title' => t('Show messages sent prior to entering channel'),
    '#description' => t('Defines if messages are shown which have been sent before the user entered the channel.'),
    '#options' => array(1 => 'Yes', 2 => 'No'),
    '#default_value' => variable_get('ajaxchat_show_prior_messages', 1),
  );
  $form['ajaxchat_message']['ajaxchat_show_prior_channel_list'] = array(
    '#type' => 'textfield',
    '#title' => t('Always show previous messages in these channels'),
    '#description' => t('Defines comma-separated list of channel IDs for which the previous setting is always true (will be ignored if set to null).'),
    '#default_value' => variable_get('ajaxchat_show_prior_channel_list', ''),
    '#size' => 40,
  );
  $form['ajaxchat_message']['ajaxchat_prior_message_time_difference'] = array(
    '#type' => 'textfield',
    '#title' => t('Show previous messages not older than'),
    '#description' => t('Max time difference in hours for messages to display on each request.'),
    '#default_value' => variable_get('ajaxchat_prior_message_time_difference', 24),
    '#size' => 4,
  );
  $form['ajaxchat_message']['ajaxchat_prior_message_max'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum previous messages to display'),
    '#description' => t('Max number of messages to display on each request.'),
    '#default_value' => variable_get('ajaxchat_prior_message_max', 10),
    '#size' => 4,
  );
  $form['ajaxchat_message']['ajaxchat_max_message_length'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum message text length'),
    '#default_value' => variable_get('ajaxchat_max_message_length', 4096),
    '#size' => 4,
  );
  $form['ajaxchat_message']['ajaxchat_max_messages_per_minute'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum messages per minute'),
    '#description' => t('Defines the max number of messages a user may send per minute.'),
    '#default_value' => variable_get('ajaxchat_max_messages_per_minute', 20),
    '#size' => 4,
  );

  // Network settings.
  $form['ajaxchat_network'] = array(
    '#type' => 'fieldset',
    '#title' => t('Network'),
    '#collapsible' => FALSE,
  );
  $form['ajaxchat_network']['ajaxchat_timer_rate'] = array(
    '#type' => 'textfield',
    '#title' => t('Time period between polls'),
    '#description' => t('The number of milliseconds to delay before subsequent polls.'),
    '#default_value' => variable_get('ajaxchat_timer_rate', 2000),
    '#size' => 4,
  );
  $form['ajaxchat_network']['ajaxchat_socket_server_enabled'] = array(
    '#type' => 'select',
    '#title' => t('Enable socket server'),
    '#description' => t('The chat function uses a JavaScript-to-Flash brigde to establish a permanent socket connection from client side. The JavaScript-to-Flash brigde requires a Flash plugin >= 9 installed on the user browser. Clients without this requirement will fall back to poll the server for updates.'),
    '#options' => array(1 => 'Yes', 2 => 'No'),
    '#default_value' => variable_get('ajaxchat_socket_server_enabled', 2),
  );
  $form['ajaxchat_network']['ajaxchat_socket_server_host'] = array(
    '#type' => 'textfield',
    '#title' => t('Socket server host'),
    '#description' => t('Defines the hostname of the socket server used to connect from client side (the server hostname is used if set to null).'),
    '#default_value' => variable_get('ajaxchat_socket_server_host', ''),
    '#size' => 24,
  );
  $form['ajaxchat_network']['ajaxchat_socket_server_ip'] = array(
    '#type' => 'textfield',
    '#title' => t('Socket server IP address'),
    '#description' => t('Defines the IP of the socket server used to connect from server-side to broadcast update messages.'),
    '#default_value' => variable_get('ajaxchat_socket_server_ip', '127.0.0.1'),
    '#size' => 24,
  );
  $form['ajaxchat_network']['ajaxchat_socket_server_port'] = array(
    '#type' => 'textfield',
    '#title' => t('Socket server port number'),
    '#description' => t('The listening port number of the socket server.'),
    '#default_value' => variable_get('ajaxchat_socket_server_port', 1935),
    '#size' => 4,
  );
  $form['ajaxchat_network']['ajaxchat_socket_server_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Socket server ID'),
    '#default_value' => variable_get('ajaxchat_socket_server_id', 1),
    '#size' => 4,
  );

  // Logging settings.
  $form['ajaxchat_logging'] = array(
    '#type' => 'fieldset',
    '#title' => t('Logging'),
    '#collapsible' => FALSE,
  );
  $form['ajaxchat_logging']['ajaxchat_log_time_difference'] = array(
    '#type' => 'textfield',
    '#title' => t('Display logged entries no older than'),
    '#description' => t('Defines the max time difference in hours for logs when no period or search condition is given.'),
    '#default_value' => variable_get('ajaxchat_log_time_difference', 1),
    '#size' => 4,
  );
  $form['ajaxchat_logging']['ajaxchat_log_request_limit'] = array(
    '#type' => 'textfield',
    '#title' => t('Logged entries returned per request'),
    '#description' => t('Defines the limit for the number of logs returned per  request.'),
    '#default_value' => variable_get('ajaxchat_log_request_limit', 1),
    '#size' => 4,
  );
  $form['ajaxchat_logging']['ajaxchat_log_first_year'] = array(
    '#type' => 'textfield',
    '#title' => t('Earliest year for log selection'),
    '#description' => t('Defines the earliest year used for the logs selection.'),
    '#default_value' => variable_get('ajaxchat_log_first_year', date('Y')),
    '#size' => 4,
  );
  $form['ajaxchat_logging']['ajaxchat_log_purge_logs'] = array(
    '#type' => 'select',
    '#title' => t('Purge logged messages from database'),
    '#description' => t('Defines if old messages are purged from the database.'),
    '#options' => array(1 => 'Yes', 2 => 'No'),
    '#default_value' => variable_get('ajaxchat_log_purge_logs', 2),
  );
  $form['ajaxchat_logging']['ajaxchat_log_purge_time_difference'] = array(
    '#type' => 'textfield',
    '#title' => t('Purge logged messages older than'),
    '#description' => t('Max time difference in days for old messages before they are purged from the database.'),
    '#default_value' => variable_get('ajaxchat_log_purge_time_difference', 365),
    '#size' => 4,
  );
  $form['ajaxchat_logging']['ajaxchat_log_user_access'] = array(
    '#type' => 'select',
    '#title' => t('Grant log access to all registered users'),
    '#description' => t('Allow registered users (including moderators) access to message logs (admins are always granted access).'),
    '#options' => array(1 => 'Yes', 2 => 'No'),
    '#default_value' => variable_get('ajaxchat_log_user_access', 2),
  );
  $form['ajaxchat_logging']['ajaxchat_log_user_access_channel_list'] = array(
    '#type' => 'textfield',
    '#title' => t('Always allow log access for following channels'),
    '#description' => t('Defines a comma-separate list of channels IDs to limit the logs access for registered users, includes all channels the user has access to if set to null.'),
    '#default_value' => variable_get('ajaxchat_log_user_access_channel_list', ''),
    '#size' => 40,
  );

  // Style settings.
  $form['ajaxchat_style'] = array(
    '#type' => 'fieldset',
    '#title' => t('Style'),
    '#collapsible' => FALSE,
  );
  $defaults = array('b','i','u','quote','code','color','url');
  $form['ajaxchat_style']['ajaxchat_bbc'] = array(
    '#type' => 'textarea',
    '#title' => t('Bulletin Board Code (BBC) tags accepted'),
    '#default_value' => variable_get('ajaxchat_bbc', implode("\n", $defaults)),
  );
  $defaults = array(':)',':(',';)',':P',':D',':|',':O',':?','8)','8o','B)',':-)',':-(',':-*','O:-D','>:-D','>:o','x-|',':o)',':idea:',':important:',':help:',':error:',':warning:',':favorite:');
  $form['ajaxchat_style']['ajaxchat_emoticon_codes'] = array(
    '#type' => 'textarea',
    '#title' => t('Emoticon codes'),
    '#default_value' => variable_get('ajaxchat_emoticon_codes', implode("\n", $defaults)),
  );
  $defaults = array('smile.png','sad.png','wink.png','razz.png','grin.png','plain.png','surprise.png','confused.png','glasses.png','eek.png','cool.png','smile-big.png','crying.png','kiss.png','angel.png','devilish.png','angry.png','sick.png','monkey.png','idea.png','important.png','help.png','error.png','warning.png','favorite.png');
  $form['ajaxchat_style']['ajaxchat_emoticon_image_files'] = array(
    '#type' => 'textarea',
    '#title' => t('Emoticon image files'),
    '#default_value' => variable_get('ajaxchat_emoticon_image_files', implode("\n", $defaults)),
  );
  $defaults = array('#131313','#c0c0c0','#ffffff','#f1683d','#9b7853','#ffa500','#f4ed85','#ffff00','#808000','#8acf06','#88e588','#1c9e1c','#89fedc','#00ffff','#17a7a7','#74b4fb','#9aa2be','#C9ABD6','#C470E0');
  $form['ajaxchat_style']['ajaxchat_text_color_codes'] = array(
    '#type' => 'textarea',
    '#title' => t('Text color codes'),
    '#default_value' => variable_get('ajaxchat_text_color_codes', implode("\n", $defaults)),
  );
  $defaults = array('Black','Gray','White','Dark Orange','Tan','Orange','Pastel Yellow','Yellow','Olive','Lime Green','Mint Green','Green','Blue Green','Cyan','Teal','Pastel Blue','Slate Blue','Purple','Deep Purple');
  $form['ajaxchat_style']['ajaxchat_text_color_names'] = array(
    '#type' => 'textarea',
    '#title' => t('Text color names'),
    '#default_value' => variable_get('ajaxchat_text_color_names', implode("\n", $defaults)),
  );

  return system_settings_form($form);
}

/**
 * Form constructor for the chat overview form.
 *
 * Returns a form for controlling the hierarchy of existing channels and
 * containers.
 *
 * @see ajaxchat_menu()
 * @ingroup forms
 */
function ajaxchat_overview($form, &$form_state) {
  module_load_include('inc', 'taxonomy', 'taxonomy.admin');

  $vid = variable_get('ajaxchat_vocabulary', '');
  $vocabulary = taxonomy_vocabulary_load($vid);
  $form = taxonomy_overview_terms($form, $form_state, $vocabulary);

  foreach (element_children($form) as $key) {
    if (isset($form[$key]['#term'])) {
      $term = $form[$key]['#term'];
      $form[$key]['view']['#href'] = 'chat/' . $term['tid'];
      if (in_array($form[$key]['#term']['tid'], variable_get('ajaxchat_containers', array()))) {
        $form[$key]['edit']['#title'] = t('edit container');
        $form[$key]['edit']['#href'] = 'admin/structure/chat/edit/container/' . $term['tid'];
      }
      else {
        $form[$key]['edit']['#title'] = t('edit channel');
        $form[$key]['edit']['#href'] = 'admin/structure/chat/edit/channel/' . $term['tid'];
      }
    }
  }

  // Remove the alphabetical reset.
  unset($form['actions']['reset_alphabetical']);

  // The form needs to have submit and validate handlers set explicitly.
  $form['#theme'] = 'taxonomy_overview_terms';
  $form['#submit'] = array('taxonomy_overview_terms_submit'); // Use the existing taxonomy overview submit handler.
  $form['#empty_text'] = t('No containers or channels available. <a href="@container">Add container</a> or <a href="@channel">Add channel</a>.', array('@container' => url('admin/structure/chat/add/container'), '@channel' => url('admin/structure/chat/add/channel')));
  return $form;
}

/**
 * Returns a select box for available parent terms.
 *
 * @param $tid
 *   ID of the term that is being added or edited.
 * @param $title
 *   Title for the select box.
 * @param $child_type
 *   Whether the child is a  or a container.
 *
 * @return
 *   A select form element.
 */
function _ajaxchat_parent_select($tid, $title, $child_type) {
  $parents = taxonomy_get_parents($tid);
  if ($parents) {
    $parent = array_shift($parents);
    $parent = $parent->tid;
  }
  else {
    $parent = 0;
  }

  $vid = variable_get('ajaxchat_vocabulary', '');
  $children = taxonomy_get_tree($vid, $tid);

  // A term can't be the child of itself, nor of its children.
  foreach ($children as $child) {
    $exclude[] = $child->tid;
  }
  $exclude[] = $tid;

  $tree = taxonomy_get_tree($vid);
  $options[0] = '<' . t('root') . '>';
  if ($tree) {
    foreach ($tree as $term) {
      if (!in_array($term->tid, $exclude)) {
        $options[$term->tid] = str_repeat(' -- ', $term->depth) . $term->name;
      }
    }
  }
  if ($child_type == 'container') {
    $description = t('Containers are usually placed at the top (root) level, but may also be placed inside another container or channel.');
  }
  elseif ($child_type == 'channel') {
    $description = t('Channels may be placed at the top (root) level, or inside another container or channel.');
  }

  return array('#type' => 'select', '#title' => $title, '#default_value' => $parent, '#options' => $options, '#required' => TRUE, '#description' => $description);
}
