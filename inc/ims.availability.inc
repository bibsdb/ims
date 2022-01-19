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
      // Fbs holdings for current faust
      $fbs_holdings = $fbs_result['holdings'];
      // Create and ajust holdings to acommodate ims_placements
      $adjusted_holdings = _fbs_merge_ims_placements($fbs_holdings, $ims_result);
      $fbs_results[$faust]['holdings'] = $adjusted_holdings;
    }
  }
}

/**
 *  Merge ims placements into the holdingsdata from FBS.
 */
function _ims_merge_ims_placements(array $holdings, array $ims_placements) {

  // Fbs holdings arrays and ims holdings arrays are identical in structure.
  $holding_structure = array(
    'available_count' => 0,
    'total_count' => 0,
    'reference_count' => 0,
    'placement' => array(),
    'materials' => array(),
  );
  
  // Create an array to store the decremented fbs holdings
  $adjusted_holdings = $holdings;

  // Create array to store new holdings items created based on ims placement
  $ims_holdings = array();
  // Loop fbs holdings for current faust
  foreach ($holdings as $holding_key => $holding) {

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
        foreach ($ims_holdings as $ims_holding) {
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
          $ims_holdings[] = $ims_holding;
        }

        // Adjust available and total of the fbs holding the material belongs to
        $adjusted_holdings[$holding_key]['total_count']--;
        // Decrease available count in holding if material is available
        if ($material['available']) {
          $adjusted_holdings[$holding_key]['available_count']--;
        }
        // Remove material
        $adjusted_holdings[$holding_key]['materials'] = array_udiff($materials, [$material], '_fbs_compare_materials');
      }
    }
  }
  // Collect all holdings in one array
  return array_merge($adjusted_holdings, $ims_holdings);
}

function _fbs_compare_materials($m1, $m2)
{
    return $m1['material_id'] - $m2['material_id'];
}