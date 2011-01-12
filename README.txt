
DESCRIPTION
===========

This module makes it possible for modules to avoid a full Drupal
bootstrap in AJAX callbacks, if they don't use the Drupal database.


USAGE
-----

Your module should implement hook_trampoline like so:

function ting_availability_trampoline() {
  $items = array();
  $items['ting/availability/item/%/details'] = array(
    'includes' => array(
      drupal_get_path('module', 'ding_provider') . '/ding_provider.module',
      drupal_get_path('module', 'ctools') . '/ctools.module'),
    'variables' => array(
      'ting_agency',
      'ting_search_url',
      'ting_scan_url',
      'ting_spell_url',
      'ting_recommendation_server',
    ),
  );
  return $items;
}

The $items key is the same as the menu router path from
hook_menu(). The page callback, page callback arguments, module and
file to be included will be copied from the menu settings. The array
here can contain the following keys:

  'includes':
    An array of extra files that will be loaded before calling the
    callback. Can be include files or modules.

  'variables':
    An array of variables that should be saved so they'll be available
    via variable_get.

  'theme_hooks':
    An array of theme hooks that will be made available.

When generating the path to a trampolined callback, use the trampoline_url() function.

FILES
-----

index.php                       # The main dispatch file.
trampoline.inc                  # Main trampoline functions.
trampoline.cache.inc            # Replacement cache (noop) functions.
trampoline.drupal-default.inc   # Copy of some important Drupal functions
                                # that cannot be loaded through their
                                # original include file.
trampoline.drupal-mod.inc       # Modified Drupal functions.
