<?php

/**
 * Trampoline dispatcher
 */

// Load trampoline
require './trampoline.inc';
require './trampoline.drupal-default.inc';
require './trampoline.drupal-mod.inc';

require trampoline_files_path().'/trampoline.config.inc';

// Load Drupal utility functions
require trampoline_base_path().'/includes/common.inc';

// Execute trampoline
trampoline_run($trampoline_config);