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

// Execute trampoline
trampoline_run($trampoline_config);
