<?php


/**
 * This file contains versions of existing Drupal functions customized
 * for Trampoline usage
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
          call_user_func($function, &$named_args);	  			
	  		}
	  	}
	  }
    	  
	  //Render templates
	  $file_path = trampoline_base_path().'/'.$theme_info['template'].'.tpl.php';
	  if ($theme_info['template'] && 
	      (file_exists($file_path))) {
	   
	    ob_start();
	    extract($named_args);
	  	require_once $file_path;
	  	$output = ob_get_contents();
	  	ob_end_clean();
	  	
	  	return $output;
	  }
  }
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
  	$drupal_vars = $conf;
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
 * Gets a value from the cache. 
 * 
 * This version only supports memory based cache and is reset between request.
 * 
 * Uses same arguments as original cache_get.
 *
 * @see cache_get
 */
function cache_get($cid, $table = 'cache') {
	global $trampoline_cache;
	
  //Use a simple variable based cache. 
  //Cache will automatically expire after request completion
	if ($trampoline_cache[$table.'_'.$cid]) {
		return array('data' => $trampoline_cache[$table.'_'.$cid]);
	} else {
		return FALSE;
	}
}

/**
 * Caches a value. 
 * 
 * This version only supports memory based cache and is reset between request.
 * 
 * Uses same arguments as original cache_set.
 *
 * @see cache_set
 */
function cache_set($cid, $data, $table = 'cache', $expire = CACHE_PERMANENT, $headers = NULL) {
	global $trampoline_cache;
	
	//Use a simple variable based cache. 
	//Cache will automatically expire after request completion
	if (!isset($trampoline_cache)) {
		$trampoline_cache = array();
	}
  $trampoline_cache[$table.'_'.$cid] = $data;
}

/**
 * Caches a value. 
 * 
 * This version only supports memory based cache and is reset between request.
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
	    foreach (array("$config/$dir/$file", "$config/$dir/$name/$file", "$dir/$file", "$dir/$name/$file") as $file) {
	      if (file_exists($file)) {
	        $files[$type][$name] = $file;
	        break(2);
	      }
	    }
    }
  }

  if (isset($files[$type][$name])) {
    return $files[$type][$name];
  }
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