<?php

/**
 * Trampoline dispatcher
 */
// Load trampoline
require './trampoline.inc';
require './trampoline.drupal-default.inc';

require trampoline_files_path().'/trampoline.config.inc';

// Load Drupal utility functions
require trampoline_base_path().'/includes/common.inc';

// Load modified Drupal functions
require './trampoline.drupal-mod.inc';

chdir($_SERVER['DOCUMENT_ROOT']);

// Load custom cache handler if defined in $cache_inc from settings.php.
// Otherwise we use a Trampoline implementation using static variables.
//  
// Note that custom cache handlers must handle all communication with the 
// cache engine on it's own. Trampoline does not provide any default Drupal 
// niceties such as a database connection and abstraction layer.
require variable_get('cache_inc', dirname(__FILE__).'/trampoline.cache.inc');

// Execute trampoline
trampoline_run($trampoline_config);
