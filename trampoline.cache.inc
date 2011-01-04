<?php

/**
 * Default implementation of a static value caching.
 */

/**
 * Gets a value from the cache.
 *
 * Uses same arguments as original cache_get.
 *
 * @see cache_get
 */
function cache_get($cid, $table = 'cache') {
  global $trampoline_cache;

  //Use a simple variable based cache.
  //Cache will automatically expire after request completion
  if (is_array($trampoline_cache) && array_key_exists($table . '_' . $cid, $trampoline_cache) && $trampoline_cache[$table.'_'.$cid]) {
    return array('data' => $trampoline_cache[$table.'_'.$cid]);
  } else {
    return FALSE;
  }
}

/**
 * Caches a value.
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