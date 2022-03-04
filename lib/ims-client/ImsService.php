<?php
/**
 * @file
 * ImsService class.
 */

class ImsService {
  protected $wsdlUrl;
  protected $username;
  protected $password;

  /**
   * Instantiate the ims client.
   */
  public function __construct($wsdl_url, $username, $password) {
    $this->wsdlUrl = $wsdl_url;
    $this->username = $username;
    $this->password = $password;
  }

  /**
   * Get information by FAUST number.
   *
   * @param mixed $faust_numbers
   *   Expects either a single FAUST number, or an array of them, for looking
   *   up multiple titles at a time.
   *
   * @return array
   *   Array of placements.
   */
  public function getByFaustNumber($faust_numbers) {
    // Cast to array to ensure the variable is an array and not a string
    $faust_numbers = (array) $faust_numbers;

    // Loop the faust numbers since the ims service only accepts one faust 
    // pr. request 
    $ims_placements = array();
    foreach($faust_numbers as $faust_number) {
      $response = $this->sendRequest($faust_number);
      $ims_placements[$faust_number] = $this->extractPlacements($response);
    }

    return $ims_placements;
  }

  /**
   * Send request to the ims server, returning the data response.
   */
  protected function sendRequest($faust_number) {
    $auth_info = array(
      'Username' => $this->username,
      'Password' => $this->password,
    );

    // New ims service.
    $client = new SoapClient($this->wsdlUrl);

    // Record the start time, so we can calculate the difference, once
    // the ims service responds.
    $start_time = explode(' ', microtime());


    // Fetch placements from IMS SOAP Webservice
    try {    
      $response = $client->GetItemsOfBibliographicRecord(array(
        'Credentials' => $auth_info,
        'BibliographicRecordId' => $faust_number,
      ));
    }
    catch (Exception $e) {
      // Re-throw Ims specific exception.
      throw new ImsServiceException($e->getMessage());
    }

    $stop_time = explode(' ', microtime());
    $time = floatval(($stop_time[1] + $stop_time[0]) - ($start_time[1] + $start_time[0]));

    // Write entry to watchdog if logging is enabled for the ims-module
    if (variable_get('ims_enable_logging', FALSE)) {
      watchdog('ims', 'IMS WS Response: %response, Duration: %time s.', array('%response' => print_r($response, true), '%time' => round($time, 3)), WATCHDOG_DEBUG, $link = NULL);
    }

    return $response;
  }

  /**
   * Extract the ims placement data from the data response.
   */
  protected function extractPlacements($response) {
    $ims_placements = array();
    
    // Handle situation where ims response contains no materials 
    if (!isset($response->Item)) {
      $items = array();
    }
    // If response contains only one material it's not wrapped in an array. Let's wrap it. 
    elseif (!is_array($response->Item)) {
      $items = array($response->Item);
    }
    //If response contains multiple materials it's ready to loop
    else {
      $items = $response->Item;
    }
   

    // Add location array to array keyed by material number (itemid)
    foreach ($items as $item) {
      // A string - not an array - is returned if the placement of a material is only one level deep.
      // It's a rare case observed when a material belongs to an "opstillingsgruppe" with no attached location.
      // We need to make sure the placement is of type array.
      $ims_placements[$item->ItemId] = (array)$item->Placement->LocationName;
    }

    return $ims_placements;    
  }
}