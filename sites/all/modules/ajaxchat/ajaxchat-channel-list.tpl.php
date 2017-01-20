<table id="chat-<?php print $chat_id; ?>">
  <thead>
    <tr>
      <th><?php print t('Channel'); ?></th>
      <th><?php print t('Participants'); ?></th>
    </tr>
  </thead>
  <tbody>
  <?php foreach ($channels as $child_id => $channel): ?>
    <tr id="channel-list-<?php print $child_id; ?>" class="<?php print $channel->zebra; ?>">
      <td <?php print $channel->is_container ? 'colspan="2" class="container"' : 'class="channel"'; ?>>
        <?php /* Enclose the contents of this cell with X divs, where X is the
               * depth this channel resides at. This will allow us to use CSS
               * left-margin for indenting.
               */ ?>
        <?php print str_repeat('<div class="indent">', $channel->depth); ?>
          <?php if ($channel->is_container): ?>
            <div class="name"><?php print $channel->name; ?></div>
          <?php else: ?>
            <div class="name"><a href="<?php print $channel->link; ?>" class="chat-link"><?php print $channel->name; ?></a></div>
            <?php if ($channel->description): ?>
              <div class="description"><?php print $channel->description; ?></div>
            <?php endif; ?>
          <?php endif; ?>
        <?php print str_repeat('</div>', $channel->depth); ?>
      </td>
      <?php if (!$channel->is_container): ?>
        <td class="participants"><?php print isset($channel->participants) ? $channel->participants : 0; ?></td>
      <?php endif; ?>
    </tr>
  <?php endforeach; ?>
  </tbody>
</table>
