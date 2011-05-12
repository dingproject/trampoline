<?php
/**
 * @file
 * Customised version of Drupal core functions for Trampoline usage.
 */

/**
 * Lightweight version of the standard theme function.
 *
 * Supports most simple cases:
 *  - Arguments,
 *  - Preprocess functions within the module or in included files
 *  - Template files
 *
 * @return string Themed output
 */
function theme() {
  global $trampoline_config;
  global $trampoline_route;

  $args = func_get_args();
  $hook = array_shift($args);

  if (isset($trampoline_route['theme'][$hook])) {
    $theme_info = $trampoline_route['theme'][$hook];

    //Convert arguments
    $named_args = array();
    if (isset($theme_info['arguments'])) {
      $theme_args = array_keys($theme_info['arguments']);
      foreach ($args as $i => $value) {
        $named_args[$theme_args[$i]] = $value;
      }
    }

    //Run preprocess functions
    if ($theme_info['preprocess functions']) {
      foreach ($theme_info['preprocess functions'] as $function) {
        if (function_exists($function)) {
          call_user_func($function, $named_args);
        }
      }
    }

    if ($theme_info['template']) {
      //Render templates
      $file_path = trampoline_base_path() . '/' . $theme_info['template'] . '.tpl.php';

      if ($theme_info['template'] &&
          (file_exists($file_path))) {

        ob_start();
        extract($named_args);
        require $file_path;
        $output = ob_get_contents();
        ob_end_clean();

        return $output;
      }
    }
    elseif ($theme_info['function'] && function_exists($theme_info['function'])) {
      return call_user_func_array($theme_info['function'], $named_args);
    }
  }
}

function theme_get_setting($setting_name, $refresh = FALSE) {
  global $trampoline_config;

  return isset($trampoline_config['theme_settings'][$setting_name]) ? $trampoline_config['theme_settings'] : NULL;
}

function module_exists($module) { return true; }

/**
 * Finds all files that match a given mask in a given directory.
 * Directories and files beginning with a period are excluded; this
 * prevents hidden files and directories (such as SVN working directories)
 * from being scanned.
 *
 * @param $dir
 *   The base directory for the scan, without trailing slash.
 * @param $mask
 *   The regular expression of the files to find.
 * @param $nomask
 *   An array of files/directories to ignore.
 * @param $callback
 *   The callback function to call for each match.
 * @param $recurse
 *   When TRUE, the directory scan will recurse the entire tree
 *   starting at the provided directory.
 * @param $key
 *   The key to be used for the returned array of files. Possible
 *   values are "filename", for the path starting with $dir,
 *   "basename", for the basename of the file, and "name" for the name
 *   of the file without an extension.
 * @param $min_depth
 *   Minimum depth of directories to return files from.
 * @param $depth
 *   Current depth of recursion. This parameter is only used internally and should not be passed.
 *
 * @return
 *   An associative array (keyed on the provided key) of objects with
 *   "path", "basename", and "name" members corresponding to the
 *   matching files.
 */
function file_scan_directory($dir, $mask, $nomask = array('.', '..', 'CVS'), $callback = 0, $recurse = TRUE, $key = 'filename', $min_depth = 0, $depth = 0) {
  $key = (in_array($key, array('filename', 'basename', 'name')) ? $key : 'filename');
  $files = array();

  if (is_dir($dir) && $handle = opendir($dir)) {
    while (FALSE !== ($file = readdir($handle))) {
      if (!in_array($file, $nomask) && $file[0] != '.') {
        if (is_dir("$dir/$file") && $recurse) {
          // Give priority to files in this folder by merging them in after any subdirectory files.
          $files = array_merge(file_scan_directory("$dir/$file", $mask, $nomask, $callback, $recurse, $key, $min_depth, $depth + 1), $files);
        }
        elseif ($depth >= $min_depth && ereg($mask, $file)) {
          // Always use this match over anything already set in $files with the same $$key.
          $filename = "$dir/$file";
          $basename = basename($file);
          $name = substr($basename, 0, strrpos($basename, '.'));
          $files[$$key] = new stdClass();
          $files[$$key]->filename = $filename;
          $files[$$key]->basename = $basename;
          $files[$$key]->name = $name;
          if ($callback) {
            $callback($filename);
          }
        }
      }
    }

    closedir($handle);
  }

  return $files;
}
function module_implements($hook, $sort = FALSE, $refresh = FALSE) {
  global $trampoline_config;

  return (array) $trampoline_config['hooks'][$hook];
}

/**
 * Gets the value of a variable.
 *
 * This version only supports variables from memory e.g. settings.php.
 *
 * Uses same arguments as original variable_get.
 *
 * @see variable_get
 */
function variable_get($name, $default) {
  global $drupal_vars;

  if (!isset($drupal_vars)) {
    require trampoline_conf_path().'/settings.php';
    if (isset($conf)) {
      $drupal_vars = $conf;
    }
    else {
      $drupal_vars = array();
    }
  }

  return (isset($drupal_vars[$name])) ? $drupal_vars[$name] : $default;
}

/**
 * Sets the value of a variable.
 *
 * This version only supports variables from memory e.g. settings.php.
 *
 * Uses same arguments as original variable_set.
 *
 * @see variable_set
 */
function variable_set($name, $value) {
  global $drupal_vars;
  $drupal_vars[$name] = $value;
}

/**
 * Returns and optionally sets the filename for a system item (module,
 * theme, etc.).
 *
 * Uses same arguments as original drupal_get_filename.
 *
 * @see drupal_get_filename
 */
function drupal_get_filename($type, $name, $filename = NULL) {
  static $files = array();

  if (!isset($files[$type])) {
    $files[$type] = array();
  }

  if (!empty($filename) && file_exists($filename)) {
    $files[$type][$name] = $filename;
  }
  elseif (isset($files[$type][$name])) {
    // nothing
  }
  else {
    $paths = array(conf_path(), trampoline_profile_path());
    foreach ($paths as $config) {
      $dir = (($type == 'theme_engine') ? 'themes/engines' : "${type}s");
      $file = (($type == 'theme_engine') ? "$name.engine" : "$name.$type");
      foreach (array("$config/$dir/$file", "$config/$dir/$name/$file", "$dir/$file", "$Dir/$name/$file", "$config/$dir/contrib/$name/$file") as $file) {
        if (file_exists($file)) {
          $files[$type][$name] = $file;
          break(2);
        }
      }
    }
  }

  if (isset($files[$type][$name])) {
    return './'. str_replace($_SERVER['DOCUMENT_ROOT'], '', $files[$type][$name]);
    //return $files[$type][$name];
  }
}
/**
 * Invoke a hook in all enabled modules that implement it.
 *
 * @param $hook
 *   The name of the hook to invoke.
 * @param ...
 *   Arguments to pass to the hook.
 * @return
 *   An array of return values of the hook implementations. If modules return
 *   arrays from their implementations, those are merged into one array.
 */
function module_invoke_all() {
  $args = func_get_args();
  $hook = $args[0];
  unset($args[0]);
  $return = array();
  foreach (module_implements($hook) as $module) {
    $function = $module .'_'. $hook;
    $result = call_user_func_array($function, $args);
    if (isset($result) && is_array($result)) {
      $return = array_merge_recursive($return, $result);
    }
    else if (isset($result)) {
      $return[] = $result;
    }
  }

  return $return;
}

/**
 * Returns whether the database is active.
 *
 * This version always returns false. Trampoline is never bootstrapped
 * to access the database.
 *
 * @see db_is_active
 */
function db_is_active() {
  return false;
}

/**
 * Returns the path to the configuration.
 *
 * @see conf_path
 */
function conf_path($require_settings = TRUE, $reset = FALSE) {
  return trampoline_conf_path();
}

/**
 * Determine the default 'files' directory.
 *
 * @see file_directory_path
 */
function file_directory_path() {
  return trampoline_files_path();
}


/**
 * Processes an HTML attribute value and ensures it does not contain an URL
 * with a disallowed protocol (e.g. javascript:)
 *
 * To keep this simple this version does not perform any filtering and
 * simply returns the input string.
 *
 * @see filter_xss_bad_protocol
 */
function filter_xss_bad_protocol($string, $decode = TRUE) {
  return $string;
}

/**
 * Log a system message.
 *
 * This version uses the default PHP trigger_error error reporting as
 * trampoline does not support module invoking.
 *
 * @see watchdog
 */
function watchdog($type, $message, $variables = array(), $severity = WATCHDOG_NOTICE, $link = NULL) {
  $severities = array(
    WATCHDOG_ALERT => E_USER_WARNING,
    WATCHDOG_CRITICAL => E_USER_ERROR,
    WATCHDOG_DEBUG => E_USER_NOTICE,
    WATCHDOG_EMERG => E_USER_ERROR,
    WATCHDOG_ERROR => E_USER_ERROR,
    WATCHDOG_INFO => E_USER_NOTICE,
    WATCHDOG_NOTICE => E_USER_WARNING,
    WATCHDOG_WARNING => E_USER_WARNING,
  );
  //Clean up arguments to avoid warnings
  $variables = (!$variables) ? array() : $variables;

  trigger_error(sprintf('%s: %s', $type, str_replace(array_keys($variables), array_values($variables), $message)), $severities[$severity]);
}

/**
 * Given an internal Drupal path, return the alias set by the administrator.
 *
 * Trampoline does not support aliases and returns the input path.
 *
 * @see drupal_get_path_alias
 */
function drupal_get_path_alias($path, $path_language = '') {
  return $path;
}

/**
 * Make sure the destination is a complete path and resides in the file system
 * directory, if it is not prepend the file system directory.
 *
 * This version always returns FALSE indicating file not found.
 *
 * @see file_create_path
 */
function file_create_path($dest = 0) {
  // File not found.
  return FALSE;
}

/**
 * Pressflow moves drupal_set_header to bootstrap.inc. If drupal_set_header
 * doesn't exist, we use these pressflow versions instead, as we can't
 * included bootstrap.inc.
 */
if (!function_exists('drupal_set_header')) {
  /**
   * Set an HTTP response header for the current page.
   *
   * Note: When sending a Content-Type header, always include a 'charset' type,
   * too. This is necessary to avoid security bugs (e.g. UTF-7 XSS).
   *
   * @param $name
   *   The HTTP header name, or a status code followed by a reason phrase, e.g.
   *   "404 Not Found".
   * @param $value
   *   The HTTP header value; if omitted, the specified header is unset.
   * @param $append
   *   Whether to append the value to an existing header or to replace it.
   */
  function drupal_set_header($name = NULL, $value = NULL, $append = FALSE) {
    // The headers as name/value pairs.
    static $headers = array();

    if (!isset($name)) {
      return $headers;
    }

    // Support the Drupal 6 header API
    if (!isset($value)) {
      if (strpos($name, ':') !== FALSE) {
        $parts = explode(':', $name, 2);
        $name = trim($parts[0]);
        $value = trim($parts[1]);
      }
    }
    if (substr($name, 0, 7) == 'HTTP/1.') {
      $name = substr($name, 9);
    }

    // Save status codes using the special key ":status".
    if (preg_match('/^\d{3} /', $name)) {
      $value = $name;
      $name = $name_lower = ':status';
    }
    else {
      $name_lower = strtolower($name);
    }
    _drupal_set_preferred_header_name($name);

    if (!isset($value)) {
      $headers[$name_lower] = FALSE;
    }
    elseif (isset($headers[$name_lower]) && $append) {
      // Multiple headers with identical names may be combined using comma (RFC
      // 2616, section 4.2).
      $headers[$name_lower] .= ',' . $value;
    }
    else {
      $headers[$name_lower] = $value;
    }
    drupal_send_headers(array($name => $headers[$name_lower]), TRUE);
  }

  /**
   * Get the HTTP response headers for the current page.
   *
   * @param $name
   *   An HTTP header name. If omitted, all headers are returned as name/value
   *   pairs. If an array value is FALSE, the header has been unset.
   * @return
   *   A string containing the header value, or FALSE if the header has been set,
   *   or NULL if the header has not been set.
   */
  function drupal_get_header($name = NULL) {
    $headers = drupal_set_header();
    if (isset($name)) {
      $name = strtolower($name);
      return isset($headers[$name]) ? $headers[$name] : NULL;
    }
    else {
      return $headers;
    }
  }

  /**
   * Header names are case-insensitive, but for maximum compatibility they should
   * follow "common form" (see RFC 2617, section 4.2).
   */
  function _drupal_set_preferred_header_name($name = NULL) {
    static $header_names = array();

    if (!isset($name)) {
      return $header_names;
    }
    $header_names[strtolower($name)] = $name;
  }

  /**
   * Send the HTTP response headers previously set using drupal_set_header().
   * Add default headers, unless they have been replaced or unset using
   * drupal_set_header().
   *
   * @param $default_headers
   *   An array of headers as name/value pairs.
   * @param $single
   *   If TRUE and headers have already be sent, send only the specified header.
   */
  function drupal_send_headers($default_headers = array(), $only_default = FALSE) {
    static $headers_sent = FALSE;
    $headers = drupal_get_header();
    if ($only_default && $headers_sent) {
      $headers = array();
    }
    $headers_sent = TRUE;

    $header_names = _drupal_set_preferred_header_name();
    foreach ($default_headers as $name => $value) {
      $name_lower = strtolower($name);
      if (!isset($headers[$name_lower])) {
        $headers[$name_lower] = $value;
        $header_names[$name_lower] = $name;
      }
    }
    foreach ($headers as $name_lower => $value) {
      if ($name_lower == ':status') {
        header($_SERVER['SERVER_PROTOCOL'] . ' ' . $value);
      }
      // Skip headers that have been unset.
      elseif ($value) {
        header($header_names[$name_lower] . ': ' . $value);
      }
    }
  }
}
