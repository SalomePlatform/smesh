// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_MeshDlg.h
// Author : Sergey LITONIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_MESHDLG_H
#define SMESHGUI_MESHDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_Dialog.h"

// Qt includes
#include <QFrame>
#include <QStringList>
#include <QMap>

class SMESHGUI_MeshTab;
class QAction;
class QComboBox;
class QListWidget;
class QMenu;
class QTabWidget;
class QToolButton;

/*!
 * \brief Dialog for mech creation or editing
 *
 *  This dialog is used for mech creation or editing. 
*/
class SMESHGUI_EXPORT SMESHGUI_MeshDlg : public SMESHGUI_Dialog
{ 
  Q_OBJECT       
      
public:
  
  enum Controls { Obj, Mesh, Geom };
  
  /*! Describes dimensions */
  enum Dimensions { Dim0D = 0, Dim1D, Dim2D, Dim3D };      
  
public:
  SMESHGUI_MeshDlg( const bool, const bool );
  virtual ~SMESHGUI_MeshDlg();
  
  SMESHGUI_MeshTab*            tab( const int ) const;
  void                         reset();
  void                         setCurrentTab( const int );
  void                         setMaxHypoDim( const int );
  void                         setHypoSets( const QStringList& );
  void                         setGeomPopupEnabled( const bool );
  void                         disableTab(const int);
  void                         enableTab(const int);
  bool                         isTabEnabled(const int) const;
  int                          getActiveObject();
  void                         setAvailableMeshType(const QStringList& );
  int                          currentMeshType();
  void                         setCurrentMeshType( const int );
  void                         setTitile( const bool, const bool );

signals:
  void                         hypoSet( const QString& );
  void                         geomSelectionByMesh( bool );
  void                         selectMeshType( const int, const int );

private slots:  
  void                         onHypoSetPopup( QAction* );
  void                         onGeomPopup( QAction* );
  void                         onGeomSelectionButton( bool );
  void                         onChangedMeshType( const int );

 private:
  QMap<int, SMESHGUI_MeshTab*> myTabs;
  QTabWidget*                  myTabWg;
  QToolButton*                 myHypoSetButton;
  QMenu*                       myGeomPopup;
  QComboBox*                   myMeshType;
};

/*!
 * \brief Tab for tab widget containing controls for definition of
 * algorithms and hypotheses
 */

class SMESHGUI_EXPORT SMESHGUI_MeshTab : public QFrame
{
  Q_OBJECT

    public:
  /*! To differ main algorithms, hypotheses and additional ones*/
  enum HypType
  {
    Algo = 0, //!< algorithms
    MainHyp,  //!< main hypothesis
    AddHyp,    //!< additional hypothesis
    MoreAddHyp //! since several additional hypothesis are possible, the 2-nd, 3-d etc
    // additional hypotheses are coded as being of HypType (AddHyp + 1), (AddHyp + 2) etc.
    // Nb of HypType's after MainHyp is returned by SMESHGUI_MeshTab::nbAddHypTypes()
  };

public:      
  SMESHGUI_MeshTab( QWidget* );
  virtual ~SMESHGUI_MeshTab();
  
  void                         setAvailableHyps( const int, const QStringList& );
  void                         setExistingHyps( const int, const QStringList&, bool=false);
  void                         addHyp( const int, const QString& );
  //void                         renameHyp( const int, const int, const QString& );
  void                         setCurrentHyp( const int, const int );
  int                          currentHyp( const int ) const;
  int                          nbAddHypTypes() const;
  void                         reset();

signals:  
  void                         createHyp( const int, const int );
  //!< Emited when "Create hypothesis" button clicked
  void                         editHyp( const int, const int );
  //!< Emited when "Edit hypothesis" button clicked
  void                         selectAlgo( const int );
  //!< Emited when an algorithm is selected
  
private slots:  
  void                         onCreateHyp();  
  void                         onEditHyp();
  void                         onHyp( int );
  void                         onMoreAddHyp();
  void                         onLessAddHyp();

private:

  void                         addItem( const QString& txt, const int type, const int index, const bool isGroup=false );
  int                          getCurrentIndex( const int type, const bool curByType=false) const;

  QMap<int, QStringList>       myAvailableHypTypes;

  QMap<int, QComboBox*>        myHypCombo;
  QMap<int, QToolButton*>      myCreateHypBtn;
  QMap<int, QToolButton*>      myEditHypBtn;

  QToolButton*                 myMoreAddHypBtn;
  QToolButton*                 myLessAddHypBtn;
  QListWidget*                 myAddHypList; // 2-nd, etc. additional hyps

};

#endif // SMESHGUI_MESHDLG_H
