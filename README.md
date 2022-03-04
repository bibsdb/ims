# Ims
This module enriches the placement part of holdings to provide users with more accurate and detailed placement information. The placements are pulled from Lyngsøes Intelligent Material Management System. The module connects to a SOAP Webservice endpoint. Documentation can be found here: https://lyngsoe-imms.com/docs/webservices/ME-Ims4Ils-GetItemsOfBibliographicRecord.html

The module registers a new availiability provider: The IMS availiability provider. It plugs into the Ding Provider framework. 

## Prereqisite configuration in the IMS webclient
Some nessecary configuration changes must be made in the IMS Webclient before enabeling the IMS-module.
Documentation can be found here:

## Installation
No special installation steps are required, simply enable the module and  IMS connection configuration (see `Configuration` section of this file).

A technical note: There are two competing availiability providers in DDB CMS. 
One is registered by the FBS module and the other is registered by the IMS module. 
DDB CMS chooses one of them based on alphabetical order.  
The IMS module wins the game because "I" comes after "F" in the alphabet. So the name of the module is important.

## Dependencies
This module requires the FBS-module from ding install profile version 7.x-6.5.0 og later to function.
* ding version < 7.x-6.5.0: No support for ims-placements
* ding-version = 7.x-6.5.0: Support for ims-placements (monographies only)
* ding-version > 7.x-6.5.0: Support for ims-placements (monographies and periodicals)

The module dependencies are:
* FBS 
* ding_provider

## Configuration.
- Go to Settings > Ding > Ding provider > Ims Provider. Path: `admin/config/ding/provider/ims`.
- Specify your librarys unique SOAP Webservice endpoint URL and credentials. The pattern of the URL looks like this:
  https://[library-short-name].lyngsoe-imms.com/ImsWs/soap/Ims4Ils?wsdl The credentials provided should belong to a user with XX role.
- A configuration setting has been made so you can choose whether you wan't holding placements with zero available items to be displayed. 


