# Ims
The IMS-module enriches the placement part of holdings to provide users with more accurate and detailed placement information. The placements are pulled from Lyngsøes Intelligent Material Management System. The module connects to a SOAP Webservice endpoint. Lyngsøes webservice documentation can be found here: https://lyngsoe-imms.com/docs/webservices/ME-Ims4Ils-GetItemsOfBibliographicRecord.html
 
The module registers a new availability provider: IMS availability provider. It plugs into the Ding Provider framework.
  
## Installation
No special installation steps are required, simply enable the module and configure the IMS connection settings (see `Configuration` section of this file).
 
A technical note: There are two competing availability providers in DDB CMS.
One is registered by the FBS module and the other is registered by the IMS module.
DDB CMS chooses one of them based on alphabetical order. 
The IMS module wins the game because "I" comes after "F" in the alphabet. So the name of the module is important.
 
## Dependencies
This module requires the FBS-module from ding install profile version 7.x-6.5.0 or later to function.
* ding version < 7.x-6.5.0: No support for ims-placements
* ding-version = 7.x-6.5.0: Support for ims-placements (monographies only)
* ding-version > 7.x-6.5.0: Support for ims-placements (monographies and periodicals)
 
The module dependencies are:
* FBS
* ding_provider

## Prerequisite configuration in the IMS webclient
Some necessary configuration changes must be made in the IMS Webclient before enabling the IMS-module.
Documentation can be found here: https://platform.dandigbib.org/projects/ddb-cms/wiki/DDB_CMS_IMS_manual
 
## Configuration
- Go to Settings > Ding > Ding provider > Ims Provider. Path: `admin/config/ding/provider/ims`.
- Specify your library's unique SOAP Webservice endpoint URL and credentials. Any user will do. There are no role requirements. The pattern of the URL looks like this:
 https://[library-short-name].lyngsoe-imms.com/ImsWs/soap/Ims4Ils?wsdl
 
 
- The option "Filter away holdings with 0 available items" lets you choose whether you want to have holding with zero available items displayed or have them filtered away.
