// Copyright (C) 2011-2021  CEA/DEN, EDF R&D
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

#ifndef SMESH_HOMARD_H_UTILS
#define SMESH_HOMARD_H_UTILS

#include "SMESH_SMESHGUI.hxx"

#include <SALOMEconfig.h>

#include <omniORB4/CORBA.h>
//#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(SMESH_Homard)

#include <SALOMEDSClient_definitions.hxx>
#include <SALOME_InteractiveObject.hxx>
#include <LightApp_DataOwner.h>
#include <SalomeApp_Application.h>
#include <SALOME_ListIO.hxx>
//#include <SALOME_Selection.h>
#include <SalomeApp_Module.h>

#include <med.h>

#include <qstring.h>
#include <vector>
#include <set>

class QString;
class QStringList;
class QComboBox;

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

namespace SMESH_HOMARD_UTILS {

  SMESHGUI_EXPORT SUIT_Study* GetActiveStudy();
  SMESHGUI_EXPORT _PTR(Study) getStudy();

  SMESHGUI_EXPORT void updateObjBrowser();

  // Function returns a list of SALOME_InteractiveObject's from
  // selection manager in GUI
  SMESHGUI_EXPORT const SALOME_ListIO& selectedIO();

  // Function returns the number of selected objects
  SMESHGUI_EXPORT int   IObjectCount();

  // Function returns the first selected object in the list
  // of selected objects
  SMESHGUI_EXPORT Handle(SALOME_InteractiveObject) firstIObject() ;

  // Function returns the last selected object in the list
  // of selected objects
  SMESHGUI_EXPORT Handle(SALOME_InteractiveObject) lastIObject() ;

  SMESHGUI_EXPORT bool isBoundaryCAO(_PTR(SObject)  MonObj);
  SMESHGUI_EXPORT bool isBoundaryAn(_PTR(SObject)  MonObj);
  SMESHGUI_EXPORT bool isBoundaryDi(_PTR(SObject)  MonObj);
  SMESHGUI_EXPORT bool isCase(_PTR(SObject)  MonObj);
  SMESHGUI_EXPORT bool isHypo(_PTR(SObject)  MonObj);
  SMESHGUI_EXPORT bool isIter(_PTR(SObject)  MonObj);
  SMESHGUI_EXPORT bool isFileType(_PTR(SObject)  MonObj, QString TypeFile);
  SMESHGUI_EXPORT bool isObject(_PTR(SObject)  MonObj, QString TypeObject, int option );

  extern SALOME_ListIO mySelected;
}

namespace SMESH_HOMARD_QT_COMMUN
{
    SMESHGUI_EXPORT QString PushNomFichier(bool avertir, QString TypeFichier="");
    SMESHGUI_EXPORT QString LireNomMaillage(QString aFile);
    SMESHGUI_EXPORT QString LireNomMaillage2(med_idt Medidt,int MeshId);

    SMESHGUI_EXPORT med_idt OuvrirFichier(QString aFile);

    SMESHGUI_EXPORT std::list<QString> GetListeChamps(QString aFile);
    SMESHGUI_EXPORT std::list<QString> GetListeComposants(QString aFile, QString aChamp);

    SMESHGUI_EXPORT QString SelectionArbreEtude(QString commentaire, int grave );
    SMESHGUI_EXPORT QString SelectionCasEtude();
};

#endif // ifndef SMESH_HOMARD_H_UTILS
