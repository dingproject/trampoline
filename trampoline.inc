<?php

/**
 * Main Trampoline request handler.
 * 
 * @param array $config The trampoline configuration
 */
function trampoline_run($config) {
	// Globals - ugggh
  global $trampoline_config; //Internal configuration
  global $trampoline_route; //Active route
	global $language; //Language global used by Drupal
	global $profile; // Profile global used by Drupal
	global $base_path;

	$trampoline_config = $config;
	
	//Set profile
	$profile = $config['profile'];
	
	//Set language
	$language->language = $config['language'];
	
	//Set base path
	$base_path = '/';
	
	//Do not display errors as output will prevent us from setting headers
	ini_set('display_errors', FALSE);
	
	// We (re)use the q variable to represent the original path
	$path = $_REQUEST['q'];
	if (isset($config['routes']) && is_array($config['routes'])) {
  	foreach ($config['routes'] as $route => $config) {
  		// Check if the path matches the route
  		$args = trampoline_match_route($route, $path);
  		if ($args !== FALSE) {
  			//We've found our active route!
  			$trampoline_route = $config;
  			
  			// Include required files
  			if (isset($config['includes']) && is_array($config['includes'])) {
  				$base_path = trampoline_base_path();
  				foreach ($config['includes'] as $file) {
  					require $base_path.'/'.$file;
  				}
  			}
  			// Execute callback with arguments
  			if (isset($config['callback'])) {
    			print call_user_func_array($config['callback'], $args);
  			}
  		}
  	}
	}
}

/**
 * Return Drupal base path if possible.
 */
function trampoline_base_path() {
	static $doc_root;
	
	if (!isset($doc_root)) {
		$doc_root = realpath($_SERVER['DOCUMENT_ROOT']);
		
		if (!is_dir($doc_root.'/sites')) {
			trigger_error('Trampoline unable to determine Drupal base path', E_ERROR);
		}
	}
	
	return $doc_root;
}

/**
 * Return Drupal files folder if possible.
 */
function trampoline_files_path() {
	static $file_path;
	
	if (!isset($file_path)) {
		$base_path = trampoline_base_path();
		//Potential locations for the files directory.
		$files_paths = array('files', 'sites/default/files');
		foreach ($files_paths as $fp) {
			if (is_dir($base_path.'/'.$fp)) {
				$file_path = $base_path.'/'.$fp;
				break;
			}
		}
		
		if (!isset($file_path)) {
      trigger_error('Trampoline unable to determine Drupal files path', E_ERROR);
		}
	}

	return $file_path;
}

/**
 * Return Drupal configuration folder if possible.
 */
function trampoline_conf_path() {
  static $conf_path;
  
  if (!isset($conf_path)) {
    $base_path = trampoline_base_path();
    // Potential locations for the configuration folder
    $conf_paths = array('sites/default');
    foreach ($conf_paths as $path) {
    	// We only have the correct location if it contains the settings.php file
      if (is_dir($base_path.'/'.$path) && file_exists($base_path.'/'.$path.'/settings.php')) {
        $conf_path = $base_path.'/'.$path;
        break;
      }
    }
    
    if (!isset($conf_path)) {
      trigger_error('Trampoline unable to determine Drupal configuration path', E_ERROR);
    }
  }
  return $conf_path;
}

/**
 * Return Drupal profile paths.
 */
function trampoline_profile_path() {
	global $profile;
	return trampoline_base_path().'/profiles/'.$profile;  
}

/**
 * Determine if a path matches a route.
 * 
 * @param string $route 
 *  The route to test against. Uses the format from keys in the hook_menu array. % indicates an argument.
 * @param string $path The actual path to test.
 * @return  array of matched arguments. 
 *  The array is empty if the path matches but there are no arguments.
 *  Returns false if the path does not match the route.
 */
function trampoline_match_route($route, $path) {
	if ($route == $path) {
	 return array();
	} elseif (strpos($route, '%') !== FALSE) {
		// We only need regular expressions if we use wildcards
  	$pattern = '|'.str_replace('%', '(.+)', $route).'|';
		$matches = array();
		if (preg_match_all($pattern, $path, $matches)) {
			return (isset($matches[1])) ? $matches[1] : array();
		} 
	}
	return false;
}