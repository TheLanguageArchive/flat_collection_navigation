<?php if (isset($variables['islandora_prev_sibling_url'])): ?>
  <a href="<?php print $variables['islandora_prev_sibling_url'];?>">Previous</a> |
<?php endif; ?>
<?php if (isset($variables['islandora_next_sibling_url'])): ?>
  <a href="<?php print $variables['islandora_next_sibling_url'];?>">Next</a>
<?php endif; ?>
<?php if (isset($variables['islandora_prev_sibling_url']) || isset($variables['islandora_next_sibling_url'])) : ?>
  <hr />
<?php endif; ?>
