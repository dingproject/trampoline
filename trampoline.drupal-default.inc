<?php

/**
 * This file contains code which has been directly copy/pasted from
 * elsewhere to be used by Trampoline.
 */

/**
 * Indicates that the item should never be removed unless explicitly told to
 * using cache_clear_all() with a cache ID.
 */
define('CACHE_PERMANENT', 0);

/**
 * Indicates that the item should be removed at the next general cache wipe.
 */
define('CACHE_TEMPORARY', -1);

/**
 * Indicates that page caching is disabled.
 */
define('CACHE_DISABLED', 0);

/**
 * Indicates that page caching is enabled, using "normal" mode.
 */
define('CACHE_NORMAL', 1);

/**
 * Indicates that page caching is using "aggressive" mode. This bypasses
 * loading any modules for additional speed, which may break functionality in
 * modules that expect to be run on each page load.
 */
define('CACHE_AGGRESSIVE', 2);

/**
 * Log message severity -- Emergency: system is unusable.
 *
 * The WATCHDOG_* constant definitions correspond to the logging severity levels
 * defined in RFC 3164, section 4.1.1: http://www.faqs.org/rfcs/rfc3164.html
 *
 * @see watchdog()
 * @see watchdog_severity_levels()
 */
define('WATCHDOG_EMERG', 0);

/**
 * Log message severity -- Alert: action must be taken immediately.
 *
 * The WATCHDOG_* constant definitions correspond to the logging severity levels
 * defined in RFC 3164, section 4.1.1: http://www.faqs.org/rfcs/rfc3164.html
 *
 * @see watchdog()
 * @see watchdog_severity_levels()
 */
define('WATCHDOG_ALERT', 1);

/**
 * Log message severity -- Critical: critical conditions.
 *
 * The WATCHDOG_* constant definitions correspond to the logging severity levels
 * defined in RFC 3164, section 4.1.1: http://www.faqs.org/rfcs/rfc3164.html
 *
 * @see watchdog()
 * @see watchdog_severity_levels()
 */
define('WATCHDOG_CRITICAL', 2);

/**
 * Log message severity -- Error: error conditions.
 *
 * The WATCHDOG_* constant definitions correspond to the logging severity levels
 * defined in RFC 3164, section 4.1.1: http://www.faqs.org/rfcs/rfc3164.html
 *
 * @see watchdog()
 * @see watchdog_severity_levels()
 */
define('WATCHDOG_ERROR', 3);

/**
 * Log message severity -- Warning: warning conditions.
 *
 * The WATCHDOG_* constant definitions correspond to the logging severity levels
 * defined in RFC 3164, section 4.1.1: http://www.faqs.org/rfcs/rfc3164.html
 *
 * @see watchdog()
 * @see watchdog_severity_levels()
 */
define('WATCHDOG_WARNING', 4);

/**
 * Log message severity -- Notice: normal but significant condition.
 *
 * The WATCHDOG_* constant definitions correspond to the logging severity levels
 * defined in RFC 3164, section 4.1.1: http://www.faqs.org/rfcs/rfc3164.html
 *
 * @see watchdog()
 * @see watchdog_severity_levels()
 */
define('WATCHDOG_NOTICE', 5);

/**
 * Log message severity -- Informational: informational messages.
 *
 * The WATCHDOG_* constant definitions correspond to the logging severity levels
 * defined in RFC 3164, section 4.1.1: http://www.faqs.org/rfcs/rfc3164.html
 *
 * @see watchdog()
 * @see watchdog_severity_levels()
 */
define('WATCHDOG_INFO', 6);

/**
 * Log message severity -- Debug: debug-level messages.
 *
 * The WATCHDOG_* constant definitions correspond to the logging severity levels
 * defined in RFC 3164, section 4.1.1: http://www.faqs.org/rfcs/rfc3164.html
 *
 * @see watchdog()
 * @see watchdog_severity_levels()
 */
define('WATCHDOG_DEBUG', 7);

function check_plain($text) {
  static $php525;

  if (!isset($php525)) {
    $php525 = version_compare(PHP_VERSION, '5.2.5', '>=');
  }
  // We duplicate the preg_match() to validate strings as UTF-8 from
  // drupal_validate_utf8() here. This avoids the overhead of an additional
  // function call, since check_plain() may be called hundreds of times during
  // a request. For PHP 5.2.5+, this check for valid UTF-8 should be handled
  // internally by PHP in htmlspecialchars().
  // @see http://www.php.net/releases/5_2_5.php
  // @todo remove this when support for either IE6 or PHP < 5.2.5 is dropped.

  if ($php525) {
    return htmlspecialchars($text, ENT_QUOTES, 'UTF-8');
  }
  return (preg_match('/^./us', $text) == 1) ? htmlspecialchars($text, ENT_QUOTES, 'UTF-8') : '';
}

function theme_placeholder($text) {
  return '<em>' . check_plain($text) . '</em>';
}

function theme_image($path, $alt = '', $title = '', $attributes = NULL, $getsize = TRUE) {
  if (!$getsize || (is_file($path) && (list($width, $height, $type, $image_attributes) = @getimagesize($path)))) {
    $attributes = drupal_attributes($attributes);
    $url = (url($path) == $path) ? $path : (base_path() . $path);
    return '<img src="' . check_url($url) . '" alt="' . check_plain($alt) . '" title="' . check_plain($title) . '" ' . (isset($image_attributes) ? $image_attributes : '') . $attributes . ' />';
  }
}
