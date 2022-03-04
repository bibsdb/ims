<?php
/**
 * @file
 * Combines availability and placement data from the library system
 * (FBS) with placement data from IMS.
 */

/**
 * Implements provider availability, items.
 *
 * @param array $provider_ids
 *   Array of ting object ids (faust).
 *
 * @return array
 *   Availability info.
 */
function ims_availability_items($provider_ids) {
    // Just call fbs availability items provider. Ims cannot contribute.
    module_load_include('inc', 'fbs', 'includes/fbs.availability');
    return fbs_availability_items($provider_ids);
}

/**
 * Implements provider availability, holdings.
 *
 * @param array $provider_ids
 *   Array of ting object ids (faust).
 *
 * @return array
 *   Holdings info.
 */
function ims_availability_holdings($provider_ids) {
    // Call fbs availability holdings provider.
    // $fbs_results is plural because it holds results for 
    // more that one faust-number ($provider_ids)
    module_load_include('inc', 'fbs', 'includes/fbs.availability');
    $fbs_results = fbs_availability_holdings($provider_ids);

    // If the ims module is enabled we try to fetch ims placements
    if (module_exists('ims')) {
      try {
        // Fetch ims placements for all $provider_ids
        $ims_results = ims_placements($provider_ids);
      }
      catch (Exception $e) {
        watchdog_exception('ims', $e);
      }

    // @todo: Does it work for periodicals?

    // Loop each faust-number
    foreach ($fbs_results as $faust => $fbs_result) {
      // Ims placements for current faust
      $ims_result = $ims_results[$faust];
      // Adjust fbs holdings for a periodical
      // They have holdings pr. volume for each issue. They must be iterated.
      if ($fbs_result['is_periodical']) {
        $issues = $fbs_result['issues'];
        // We need to adjust holdings for each volume in each issue
        foreach ($issues as $issue_key => $volumes)  {
          foreach ($volumes as $volume_key => $volume) {
            $fbs_adjusted_holdings = _ims_merge_ims_placements($volume['placement'], $ims_result);
            $fbs_results[$faust]['issues'][$issue_key][$volume_key]['placement'] = $fbs_adjusted_holdings;
          }
        }
      }
      else {
        // Adjust Fbs holdings for a monography
        $fbs_holdings = $fbs_result['holdings'];
        // Create and ajust holdings to acommodate ims_placements
        $fbs_adjusted_holdings = _ims_merge_ims_placements($fbs_holdings, $ims_result);
        $fbs_results[$faust]['holdings'] = $fbs_adjusted_holdings;
      }
    }

    return $fbs_results;
  }
}

/**
 *  Merge ims placements into the holdingsdata from FBS.
 */
function _ims_merge_ims_placements(array $holdings, array $ims_placements) {

  // Skeleton used to create new holdings arrays.
  $holding_structure = array(
    'available_count' => 0,
    'total_count' => 0,
    'reference_count' => 0,
    'placement' => array(),
    'materials' => array(),
    'reservable' => FALSE, // Filled for periodicals only 
    'location' => "", // Filled for periodicals only
  );
  
  // Create an array to store the decremented fbs holdings
  $fbs_adjusted_holdings = $holdings;

  // Create array to store new holdings items created based on ims placement
  $ims_holdings = array();
  // Loop fbs holdings for current faust
  foreach ($holdings as $holding_key => $holding) {

    // Only recent versions of the FBS provider returns material numbers.
    // Without them we can do nothing. README.md explains dependencies.
    // Handle situation where ims module is installed in incompatible environment
    if (!array_key_exists('materials', $holding)) {
      return $holdings;
    }
    
    // TODO: When a shelf mark service becomes available it should be used.
    // Alternatively the shelf mark code could be factored out of the fbs.availability code 
    // so that it could be called independently. Right now our only choice is to assume 
    // that the last element of fbs placement array is the shelf mark. 
    $shelf_mark = end($holding['placement']);

    // Loop materials
    $materials = $holding['materials'];
    foreach ($materials as $material) {
      // Check if material_id is present in ims result, meaning that
      // this material should have it's ims placement displayed.
      if (isset($ims_placements[$material['material_id']])) {

        $ims_placement = $ims_placements[$material['material_id']];
        // Check if an ims holding with a matching placement array, 
        // was created earlier. If so increment holding.
        // Note that if the ims location tree is build in a way so 
        // that the ims placement array competely matches the 
        // fbs placement array then two identical placementstrings 
        // are displayed. There is lttle chance of that though, and 
        // the outcome is acceptable in these rare cases.
        $found = false;
        foreach ($ims_holdings as $key => $ims_holding) {
          $placement_without_shelf_mark = array_slice($ims_holding['placement'], 0, -1);
          if ($placement_without_shelf_mark == $ims_placement) {
            $found = true;
            // Increment total_count
            $ims_holding['total_count']++;
            // Increment available count if material is available
            if ($material['available']) {
              $ims_holding['available_count']++;
            }
            // Add material
            $ims_holding['materials'][] = $material; 
            // Save the modified ims_holding to the ims_holdings array
            $ims_holdings[$key] = $ims_holding;
            break;
          }
        }
        
        // There is no holding with matching placement.
        // We have to create one from scratch.
        if (!$found) {
          $ims_holding = $holding_structure;
          $ims_holding['total_count']++;
          // Increment available count if material is available
          if ($material['available']) {
            $ims_holding['available_count']++;
          }
          // Add material
          $ims_holding['materials'][] = $material; 
          // Add placement
          $ims_holding['placement'] = $ims_placement;
          // Add shelf mark as the last element
          $ims_holding['placement'][] = $shelf_mark;
          // Periodicals have the reservable flag also set on this level - lets just set it for all materials
          $ims_holding['reservable'] = $holding['reservable']; //DO THE LOGIG
          // Periodicals need come with a location string - lets just create it for all materials
          $ims_holding['location'] = implode(' > ', $ims_holding['placement']);

          $ims_holdings[] = $ims_holding;
        }

        // Adjust available and total of the fbs holding the material belongs to
        $fbs_adjusted_holdings[$holding_key]['total_count']--;
        // Remove material id from fbs holding - it now sits in the ims holding
        $fbs_adjusted_holdings[$holding_key]['materials'] = array_udiff($materials, [$material], '_ims_compare_materials');
        // Decrease available count in holding if material is available
        if ($material['available']) {
          $fbs_adjusted_holdings[$holding_key]['available_count']--;
          // If available count reaches zero we are creating a "fake zero holding"
          // placement. Lets remove it.
          if ($fbs_adjusted_holdings[$holding_key]['available_count'] == 0) {
            // unset() doesn't change array keys
            unset($fbs_adjusted_holdings[$holding_key]);
          }
        }

      }
    }
  }

  // Filter away holdings with zero available items if the ims-setting says so  
  if (variable_get('ims_hide_zero_holdings', FALSE)) {
    foreach($fbs_adjusted_holdings as $holding_key => $holding) {
      if ($fbs_adjusted_holdings[$holding_key]['available_count'] == 0) {
        unset($fbs_adjusted_holdings[$holding_key]);
      }  
    }
  }

  // Collect all holdings in one array
  return array_merge($fbs_adjusted_holdings, $ims_holdings);
}

function _ims_compare_materials($m1, $m2)
{
    return $m1['material_id'] - $m2['material_id'];
}