// Copyright (C) 2011-2020  CEA/DEN, EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

#ifndef HOMARD_H_UTILS
#define HOMARD_H_UTILS

#include "HOMARDGUI_Exports.hxx"

#include <SALOMEconfig.h>

#include <omniORB4/CORBA.h>
#include CORBA_CLIENT_HEADER(HOMARD_Cas)
#include CORBA_CLIENT_HEADER(HOMARD_Gen)

#include "SALOMEDSClient_definitions.hxx"
#include "SALOME_InteractiveObject.hxx"
#include "LightApp_DataOwner.h"
#include "SalomeApp_Application.h"
#include <SALOME_ListIO.hxx>

class QString;
class QStringList;

class SUIT_ViewWindow;
class SUIT_Desktop;
class SUIT_Study;
class SUIT_ResourceMgr;

class CAM_Module;

class SALOMEDSClient_Study;
class SALOMEDSClient_SObject;

class SalomeApp_Study;
class SalomeApp_Module;
class LightApp_SelectionMgr;

namespace HOMARD_UTILS {

  HOMARD_EXPORT SUIT_Study* GetActiveStudy();
  HOMARD_EXPORT _PTR(Study) getStudy();

  HOMARD_EXPORT void updateObjBrowser();

  HOMARD_EXPORT const SALOME_ListIO& selectedIO();    // Function returns a list of SALOME_InteractiveObject's from
                                        // selection manager in GUI

  HOMARD_EXPORT int   IObjectCount() ;                // Function returns the number of selected objects

  HOMARD_EXPORT Handle(SALOME_InteractiveObject) firstIObject() ;
                                        // Function returns the first selected object in the list
                                        // of selected objects

  HOMARD_EXPORT Handle(SALOME_InteractiveObject) lastIObject() ;
                                      // Function returns the last selected object in the list
                                       // of selected objects

  HOMARD_EXPORT bool isBoundaryCAO(_PTR(SObject)  MonObj);
  HOMARD_EXPORT bool isBoundaryAn(_PTR(SObject)  MonObj);
  HOMARD_EXPORT bool isBoundaryDi(_PTR(SObject)  MonObj);
  HOMARD_EXPORT bool isCase(_PTR(SObject)  MonObj);
  HOMARD_EXPORT bool isHypo(_PTR(SObject)  MonObj);
  HOMARD_EXPORT bool isIter(_PTR(SObject)  MonObj);
  HOMARD_EXPORT bool isYACS(_PTR(SObject)  MonObj);
  HOMARD_EXPORT bool isZone(_PTR(SObject)  MonObj);
  HOMARD_EXPORT bool isFileType(_PTR(SObject)  MonObj, QString TypeFile);
  HOMARD_EXPORT bool isObject(_PTR(SObject)  MonObj, QString TypeObject, int option );

  HOMARD_EXPORT void PushOnHelp(QString monFichierAide, QString contexte, QString LanguageShort);

  extern SALOME_ListIO mySelected;
}

#endif // ifndef HOMARD_H_UTILS
