<?php

/**
 * @file
 * User page callbacks for the AJAX chat module.
 */

/**
 * Page callback: Prints a channel listing.
 *
 * @param $term
 *   A tree of all channel for a given taxonomy term ID. Defaults to NULL. See
 *   the return object of ajaxchat_chatload() for a complete definition.
 *
 * @return
 *   A string containing HTML representing the themed chat channel listing.
 *
 * @see ajaxchat_menu()
 */
function ajaxchat_page($term = NULL) {
  if (!isset($term)) {
    // On the main page, display all the top-level channels.
    $term = ajaxchat_chat_load(0);
  }
  drupal_add_js(drupal_get_path('module', 'ajaxchat') . '/ajaxchat.js');
  return theme('ajaxchat', array('channels' => $term->channels, 'parents' => $term->parents, 'tid' => $term->tid));
}
