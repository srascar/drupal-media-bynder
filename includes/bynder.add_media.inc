<?php
/**
 * @file
 * Functions for Bynder add box.
 */

/**
 * Menu callback function. Display Bynder add media box.
 *
 * @param null|string $js Ajax variable (js|nojs)
 * @param null|string $field_name System file name of widget that ask for Bynder media
 * @param null|int $field_delta Delta number of item of field
 */
function bynder_add_media_callback($js = NULL, $field_name = NULL, $field_delta = NULL) {
  // Bynder box need JavaScript
  if(!$js) {
    return t('Enable JavaScript to add Bynder media.');
  }
  elseif (empty($field_name) || $field_delta === NULL) {
    return t('Bad request to add Bynder media.');
  }
  $path = drupal_get_path('module', 'bynder');
  drupal_add_css($path . '/bynder.css');
  drupal_add_js($path . '/js/spin.min.js');
  drupal_add_js($path . '/js/bynder.js');
  // Store field_name and field_delta as JavaScript settings
  $js_settings['bynder'] = array(
    'field_name' => $field_name,
    'field_delta' => $field_delta,
  );
  drupal_add_js($js_settings, 'setting');


  // Required includes for ctools to work:
  ctools_include('modal');
  ctools_include('ajax');
  $form_state = array(
    'ajax' => TRUE,
  // Form ID not incremented (CSS and JS can use form ID)
    'reset_html_ids' => TRUE,
  );

  // Add Bynder box form in CTools Modal
  $output = ctools_modal_form_wrapper('bynder_add_form', $form_state);

  if (!empty($form_state['executed'])) {
    // We'll just overwrite the form output if it was successful.
    $output = array();
    ctools_add_js('ajax-responder');
    if (isset($_GET['destination'])) {
      $output[] = ctools_ajax_command_redirect($_GET['destination']);
    }
    else {
      $output[] = ctools_ajax_command_reload();
    }
  }
  print ajax_render($output);
}

/**
 * Form to add media from Bynder.
 */
function bynder_add_form($form, &$form_state = array(), $redirect = NULL) {
  global $filter_labels;
  global $meta_property_prefix;
  $meta_property_prefix = 'property_';
  $form['#attached']['css'][] = array(
    'type' => 'external',
    'data' => '//maxcdn.bootstrapcdn.com/font-awesome/4.1.0/css/font-awesome.css',
  );
  $form['#attached']['library'][] = array(
    'bynder', 'bynder_add_form'
  );
  drupal_process_attached($form);

  $form['bynder_search'] = array('#type' => 'fieldset');

  $rest_client = bynder_rest_client();
  $meta_properties = $rest_client->getMetaProperties();

  $filter_labels = array(
    'brandId' => t('Brands'),
    'tags' => t('Tags'),
    'type' => t('Type'),
    'orientation' => t('Orientation')
  );
  foreach ($meta_properties as $meta_property) {
    $filter_labels[$meta_property_prefix . $meta_property['name']] = $meta_property['label'];
    foreach ($meta_property['options'] as $meta_property_option) {
      $filter_labels[$meta_property_option['name']] = $meta_property_option['label'];
    }
  }

  $brands = bynder_brands();
  $search = bynder_search_string($form_state);
  $filters = bynder_filters_json($form_state);

  $selected_facets = bynder_selected_facets($filters);
  $results = bynder_search_results($search, $filters, $meta_properties);
  $available_facets = $results['count'];

  // Create search section
  $form['bynder_search']['search'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#attributes' => array(
      'class' => array(
        'bynder-search-form',
      ),
    ),
  );
  $form['bynder_search']['search']['field'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($search) ? $search : '',
    '#description' => "<p class='alert alert-warning' style='display:none'><span class='text'></span></p>",
  );
  $form['bynder_search']['search']['button'] = array(
    '#type' => 'button',
    '#value' => t('Search'),
    '#submit' => array(
      'bynder_add_form'
    )
  );

  $form['bynder_search']['selected_facets'] = bynder_generate_selected_facets($filters);

  $form['bynder_search']['facets'] = bynder_generate_facets($available_facets, $selected_facets, $brands, $meta_properties);

  $form['bynder_search']['filters'] = array(
    '#type' => 'hidden',
    '#default_value' => (json_encode($filters) != '[]') ? json_encode($filters) : '{"filters": []}'
  );

  // Create the results section
  $form['bynder_search']['results'] = bynder_generate_results($results);

  if (isset($results['totalNrOfResults'])) {
    $form['bynder_search']['total_results'] = array(
      '#type' => 'html_tag',
      '#tag' => 'h3',
      '#attributes' => array(
        'class' => 'total_results'
      ),
      '#value' => t('Total Results: ') . $results['totalNrOfResults']
    );
    $form['bynder_search']['pager'] = bynder_generate_pager(
      $results['totalNrOfResults'], 0, array('search' => $search)
    );
  }

  return $form;
}


  /**
   * Retrieve parameter from container.
   */
  function bynder_parameter($container, $key, $default = NULL) {
    if (isset($container[$key])) {
      return $container[$key];
    }
    return $default;
  }

  /**
   * Callback function to submit search keywords in Bynder add form
   *
   * @see bynder_generate_search()
   */
  function bynder_add_search_submit($form, $form_state) {

  }

  /**
   * Return entered search string.
   */
  function bynder_search_string($form_state) {
    $search = NULL;

    if (isset($form_state['values']['field'])) {
      $search = $form_state['values']['field'];
    }
    elseif (isset($_GET['search'])) {
      $search = check_plain(filter_xss($_GET['search']));
    }
    return $search;
  }

  /**
   * Return entered filters.
   */
  function bynder_filters_json($form_state) {
    $filters = array();

    if (isset($form_state['values']['filters'])) {
      $filters_json = json_decode($form_state['values']['filters']);
      if ($filters_json) {
        $filters = $filters_json->filters;
      }
    }
    return $filters;
  }

  /**
   * Return selected facets.
   */
  function bynder_selected_facets($filters) {
    $selected_facets = array();

    if (isset($form_state['values']['search'])) {
      $selected_facets = array_merge($selected_facets, array(
        "keyword" => (array) $form_state['values']['search']
      ));
    }

    if (isset($form_state['values']['filters'])) {
      $filters_json = json_decode($form_state['values']['filters']);
      $filters = $filters_json->filters;

      foreach ($filters as $filter) {
        $selected_facets = array_merge($selected_facets, array(
          $filter->key => $filter->value
        ));
      }
    }

    return $selected_facets;
  }

  /**
   * Search with selected facets and a search term.
   */
  function bynder_search_facets($selected_facets, $search_term) {
    $full_query = "";
    foreach ($selected_facets as $selected_facet_group_name => $selected_facet_group) {
      $full_query .= "&" . $selected_facet_group_name . '=' . implode(",", $selected_facet_group);
    }

    if (isset($search_term) && strlen($search_term) > 0) {
      if (strlen($full_query) > 0) {
        $full_query .= ' ';
      }
      $full_query .= '&keyword=' . $search_term;
    }

    $rest_client = bynder_rest_client();
    $arr_facets_json = $rest_client->getFacets($full_query);

    if ($arr_facets_json) {
      return $arr_facets_json;
    }
    else {
      drupal_set_message($rest_client->error, 'error');
      return FALSE;
    }
  }

  /**
   * Parse the selected facets.
   */
  function bynder_parse_selected_facets($selected_facets) {
    if (!$selected_facets) {
      return FALSE;
    }

    if (!empty($selected_facets)) {
      $selected_facet_list = array(
        '#theme' => 'item_list',
        '#type' => 'ul',
        '#title' => t('Selected Facets'),
        '#attributes' => array(
          'class' => 'selected_facet_list'
        )
      );

      foreach ($selected_facets as $facet_category => $facets) {
        foreach ($facets as $facet) {
          $selected_facet_list['#items'][] = $facet_category . ' => ' . $facet;
        }
      }

      return $selected_facet_list;
    }

    return FALSE;
  }

  function bynder_generate_selected_facets($selected_facets) {
    global $filter_labels;
    if (empty($selected_facets)) {
      return array();
    }

    $facets_list = array(
      '#theme' => 'item_list',
      '#type' => 'ul',
      '#title' => t('Selected Facets'),
      '#attributes' => array(
        'class' => 'selected_facet_list facet_list'
      )
    );
    foreach ($selected_facets as $selected_facet) {
      $selected_facet_key = $selected_facet->key;
      if (!isset($filter_labels[$selected_facet_key])) {
        $filter_labels[$selected_facet_key] = t($selected_facet_key);
      }

      $facet_category = array(
        'data' => "<span>" . filter_xss($filter_labels[$selected_facet_key]) . "<span class='expand'><i class='fa fa-times'></i></span></span>",
        'class' => array(
          'facet_title'
        ),
        'data-filter-key' => filter_xss($selected_facet->key)
      );
      $facets_list['#items'][] = $facet_category;
    }

    return $facets_list;
  }

  /**
   * Generate facets.
   */
  function bynder_generate_facets($facets, $selected_facets, $brands) {
    global $filter_labels;
    if (empty($facets)) {
      return array();
    }

    $facets_list = array(
      '#theme' => 'item_list',
      '#type' => 'ul',
      '#title' => t('Facets'),
      '#prefix' => '<div class="normal-facet-list-wrapper">',
      '#suffix' => '</div>',
      '#attributes' => array(
        'class' => 'normal_facet_list facet_list'
      )
    );

    foreach ($facets as $facet_category_name => $facet_category) {
      if ($facet_category_name == 'categoryId' ||
        $facet_category_name == 'subBrandId'
      ) {
        continue;
      }
      //Hide single item filters.
      if (count($facet_category) < 2) {
        continue;
      }

      if (!isset($filter_labels[$facet_category_name])) {
        $filter_labels[$facet_category_name] = t($facet_category_name);
      }
      $icons = array(
        'brandId' => 'archive',
        'tags' => 'tags',
        'type' => 'file',
        'orientation' => 'image'
      );
      $children = bynder_generate_facets_link($facet_category, $facet_category_name, $selected_facets, $brands);
      $facet_category = array(
        'data' => "<span><i class='fa fa-" . filter_xss(isset($icons[$facet_category_name]) ? $icons[$facet_category_name] : 'caret-right') . "'></i> " . filter_xss($filter_labels[$facet_category_name]) . "<span class='expand'><i class='fa fa-angle-down'></i></span></span>",
        'id' => preg_replace('/\s+/', '-', $facet_category_name),
        'class' => array(
          'facet_title'
        ),
        'children' => $children
      );
      if (count($children)) {
        $facets_list['#items'][] = $facet_category;
      }
    }
    return $facets_list;
  }

  /**
   * Generate links for facets.
   */
  function bynder_generate_facets_link($facet_category, $facet_category_name, $selected_facets, $brands) {
    global $filter_labels;
    $facet_links = array();

    foreach ($facet_category as $facet_id => $facet_count) {
      if ($facet_category_name == 'brandId') {
        $facet_name = bynder_get_brand_name($brands, $facet_id);
      }
      else {
        $facet_name = $facet_id;
      }
      if (!isset($filter_labels[$facet_name])) {
        $filter_labels[$facet_name] = $facet_name;
      }
      $filter_key = $facet_category_name;
      $filter_value = $facet_id;

      $active = bynder_facet_active($selected_facets, array($facet_category_name => array($facet_id)));
      $active_class = $active ? "active" : "";
      $link = "";
      $link .= '<a class="filter-url ' . filter_xss($active_class) . '" href="#" data-filter-key="' . filter_xss($filter_key) . '" data-filter-value="' . filter_xss($filter_value) . '">' . filter_xss($filter_labels[$facet_name]) . '</a>';
      $link .= '<span class="count">(' . filter_xss($facet_count) . ')</span>';
      $facet_links[] = $link;
    }
    return $facet_links;
  }

  function bynder_get_brand_name($brands, $brand_id) {
    foreach ($brands as $brand) {
      if ($brand['id'] == $brand_id) {
        return $brand['name'];
      }
    }
    return "";
  }

  function bynder_query_facet_link($facet, $facet_value, $selected_facets, $deactivate = FALSE) {
    if (!$deactivate) {
      $selected_facets = array_merge_recursive(
        array($facet => array($facet_value)),
        $selected_facets
      );
    }
    else {
      if (isset($selected_facets[$facet])) {
        $selected_facets[$facet] = array_diff(explode(',', $selected_facets[$facet][0]), array($facet_value));
      }
    }
    $csv_facets = array();
    foreach (array_filter($selected_facets) as $selected_facet_key => $selected_facet) {
      $csv_facets[$selected_facet_key] = implode(',', $selected_facet);
    }
    $value = http_build_query($csv_facets);
    return $value;
  }

  function bynder_facet_active($selected_facets, $current_facet) {
    if (isset($selected_facets[key($current_facet)])) {
      return in_array($current_facet[key($current_facet)][0],
        explode(',', $selected_facets[key($current_facet)][0]));
    }
    else {
      return FALSE;
    }
  }

  /**
   * Parse the facet link.
   */
  function bynder_parse_facet_link($query) {
    $pattern = '/\+\([A-Za-z0-9 ]*\)/';
    preg_match($pattern, $query, $matches);
    $query = urlencode($query);
    if (!empty($matches)) {
      $query = $query . '&search=' . substr(trim($matches[0]), 2, -1);
    }

    return $query;
  }

  function bynder_remove_hidden_facets($available_facets, $selected_facets) {
    foreach ($selected_facets as $selected_facet_key => $selected_facet) {
      $value = $selected_facet[0];
      $isset = isset($available_facets[$selected_facet_key][$value]);
      if (!$isset) {
        unset($selected_facets[$selected_facet_key]);
      }
    }
    return $selected_facets;
  }

  /**
   * Get brands.
   */
  function bynder_brands() {
    $rest_client = bynder_rest_client();
    $result = $rest_client->brands();
    $arr_json = [];

    if ($result) {
      $arr_json = json_decode($result, TRUE);
    }
    else {
      drupal_set_message(t('Could not retrieve search results:') . filter_xss($rest_client->error), 'error');
    }

    return $arr_json;
  }

  /**
   * Perform the search.
   *
   * @param unknown $search
   *   the query entered.
   * @param unknown $selected_facets
   *   the selected facets.
   *
   * @return mixed
   *   the search results.
   */
  function bynder_search_results($search, $selected_facets, $meta_properties) {
    $start_index = 0;
    $nr_of_results = 25;
    $query = '';

    if (isset($_GET['page'])) {
      $current_page = (int) filter_xss($_GET['page']);
      $start_index = $current_page * $nr_of_results;
    }
    $selected_facets_meta_properties = [];
    foreach ($meta_properties as $meta_property) {
      foreach ($meta_property['options'] as $meta_property_option) {
        $selected_facets_meta_properties[$meta_property['name']][$meta_property_option['name']] = $meta_property_option['id'];
      }
    }

    if (isset($selected_facets)) {
      foreach ($selected_facets as $facet) {
        $facets = $facet->value;
        $facet_category = $facet->key;

        if (isset($selected_facets_meta_properties[$facet_category])) {
          $meta_facets = [];
          foreach (explode(',', $facets[0]) as $facet) {
            $meta_facets[] = $selected_facets_meta_properties[$facet_category][$facet];
          }
          $query .= '&propertyOptionId=' . implode(',', $meta_facets);
        }
        else {
          $query .= '&' . $facet_category . '=' . $facets;
        }
      }
    }

    if (isset($search) && strlen($search) > 0) {
      $query .= '&keyword=' . urlencode($search);
    }
    $rest_client = bynder_rest_client();
    $result = $rest_client->search($query, $start_index, $nr_of_results);

    if ($result) {
      $arr_json = json_decode($result, TRUE);

      //Set total count;
      if (isset($arr_json['count']['brandId'])) {
        $total_count = 0;
        foreach ($arr_json['count']['brandId'] as $brand_count) {
          $total_count += $brand_count;
        }
        $arr_json['totalNrOfResults'] = $total_count;
      }
    }
    else {
      drupal_set_message(t('Could not retrieve search results:') . filter_xss($rest_client->error), 'error');
      return FALSE;
    }

    return $arr_json;
  }

  /**
   * Organize results from search engine to visualize them.
   */
  function bynder_generate_results($results) {
    if (isset($results['totalNrOfResults'])) {
      unset($results['totalNrOfResults']);
    }

    if (isset($results['startIndex'])) {
      unset($results['startIndex']);
    }

    $results = $results['media'];

    $output = array(
      '#theme' => 'item_list',
      '#type' => 'ul',
      '#attributes' => array(
        'class' => 'result_list'
      )
    );

    if (!empty($results)) {
      foreach ($results as $media_item) {
        // TODO: replace this check with a link or alternative to show a image that doesn't have a thumbnail
        if (!empty($media_item['thumbnails']['thul'])) {
          $output['#items'][] = bynder_generate_result_current($media_item);
        }
      }
    }
    else {
      return FALSE;
    }

    return $output;
  }

  /**
   * Show search result.
   */
  function bynder_generate_result_current($media_item) {
    if (!isset($media_item['thumbnails']['thul'])) {
      return FALSE;
    }

    $variables = array();

    $variables['path'] = $media_item['thumbnails']['thul'];

    if($media_item['type'] == 'image') {
      $path = $media_item['id'];
    }
    elseif($media_item['type'] == 'video')  {
      $path = $media_item['videoPreviewURLs'][1];
    }
    $holder_link_options = array(
      'external' => TRUE,
      'attributes' => array(
        'class' => 'bynder-image',
        'rel' => $media_item['id'],
        'data-id' => $media_item['id'],
        'data-idHash' => $media_item['idHash'], //fragmentId
        'data-path' => $path,
      ),
      'html' => TRUE,
      'query' => array(
        'id' => $media_item['id'], //fragmentId
        'idHash' => $media_item['idHash'] //fragmentId
      )
    );
    $path = '#';
    $thumbnail = '<div class="image-holder"><img src="' . $media_item['thumbnails']['thul'] . '"/></div>';
    $title = '<span class="media_object_type_' . $media_item['type'] . '">' . t($media_item['name']) . '</span>';

    return l('<div class="image-holder"><img src="' . $media_item['thumbnails']['thul'] . '"/></div>
      <span class="media_object_type_' . $media_item['type'] . '">' . t($media_item['name']) . '</span>', $path, $holder_link_options);
  }

  /**
   * Generate paging functionality.
   */
  function bynder_generate_pager($num_results, $element = 0, $parameters = array()) {
    if (isset($_POST['search'])) {
      $_GET['search'] = filter_xss(check_plain($_POST['search']));
    }

    pager_default_initialize($num_results, 25, $element);
    return array(
      '#markup' => theme('pager', array(
        'parameters' => $parameters,
        'element' => $element
      ))
    );
  }