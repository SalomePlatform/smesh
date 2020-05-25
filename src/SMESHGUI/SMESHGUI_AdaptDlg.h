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

// SMESH SMESHGUI : GUI for the adaptation in the SMESH component
// File   : SMESHGUI_AdaptDlg.h
// Author : Gérald NICOLAS, EDF
//
#ifndef SMESHGUI_ADAPTDLG_H
#define SMESHGUI_ADAPTDLG_H

#include "HOMARDGUI_Exports.hxx"

#include <SalomeApp_Module.h>
#include "SalomeApp_Study.h"

#include <SALOMEconfig.h>
#include <SALOME_LifeCycleCORBA.hxx>
#include CORBA_CLIENT_HEADER(HOMARD_Gen)
#include CORBA_CLIENT_HEADER(HOMARD_Cas)
#include CORBA_CLIENT_HEADER(HOMARD_Hypothesis)

#include <QString>

//=================================================================================
// class    : SMESHGUI_AdaptDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_AdaptDlg: public QDialog
{
  Q_OBJECT

public:
   SMESHGUI_AdaptDlg( SMESHGUI*,
                      int theCommandID);
  ~SMESHGUI_AdaptDlg();

  virtual bool OnGUIEvent (int theCommandID);
  void    initialize( CAM_Application* );
  QString engineIOR() const;
  void    windows( QMap<int, int>& ) const;

  static HOMARD::HOMARD_Gen_var InitHOMARDGen(SalomeApp_Application* );


public slots:
    bool    deactivateModule( SUIT_Study* );
    bool    activateModule( SUIT_Study* );
    static void setOrb();

private slots:
    void OnGUIEvent();
    void EditAsciiFile();
    void LanceCalcul0();
    void LanceCalcul1();
    void IterInfo();
    void NextIter();
    void MeshPublish0();
    void MeshPublish1();
    void YACSCreate();
    void YACSWrite();
    void Edit();
    void Delete();

public:
   virtual void  contextMenuPopup( const QString&, QMenu*, QString& );


private:
   void createHOMARDAction( const int id, const QString& po_id, const QString& icon_id = QString(""),
                            const int key = 0, const bool toggle = false );
   void createActions();
   void createMenus();
   void recupPreferences();
   void createPopupMenus();
   _PTR(SObject)  chercheMonObjet();

   virtual void createPreferences();
   HOMARD::HOMARD_Gen_var myComponentHomard;

   int anId;
   QString _ObjectName;
   QString _LanguageShort ;
   int _PublisMeshIN ;
   int _PublisMeshOUT ;
   int _YACSMaxIter ;
   int _YACSMaxNode ;
   int _YACSMaxElem ;
   int _YACSTypeTest ;
};

#endif // SMESHGUI_ADAPTDLG_H
