// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//
/**
*  SMESH SMESHGUI
*
*  Copyright (C) 2005  CEA/DEN, EDF R&D
*
*
*
*  File   : SMESHGUI_MeshDlg.h
*  Author : Sergey LITONIN
*  Module : SMESH
*/


#ifndef SMESHGUI_MeshDlg_H
#define SMESHGUI_MeshDlg_H

#include "SMESHGUI_Dialog.h"
#include <qframe.h>
#include <qstringlist.h>
#include <qmap.h>

class SMESHGUI_MeshTab;
class QTabWidget;
class QLineEdit;
class QComboBox;
class QToolButton;
class QString;
class QPopupMenu;

/*!
 * \brief Dialog for mech creation or editing
 *
 *  This dialog is used for mech creation or editing. 
*/
class SMESHGUI_MeshDlg : public SMESHGUI_Dialog
{ 
  Q_OBJECT       
      
public:
  
  enum Controls { Obj, Mesh, Geom };
  
  /*! Describes dimensions */
  enum Dimensions { Dim1D = 0, Dim2D, Dim3D };      
  
public:
  SMESHGUI_MeshDlg( const bool theToCreate, const bool theIsMesh );
  virtual ~SMESHGUI_MeshDlg();
  
  SMESHGUI_MeshTab*            tab( const int ) const;
  void                         reset();
  void                         setCurrentTab( const int );
  void                         setMaxHypoDim( const int );
  void                         setHypoSets( const QStringList& );
  void                         setGeomPopupEnabled( const bool );

signals:

  void                         hypoSet( const QString& );
  void                         geomSelectionByMesh( bool );

private slots:  

  void                         onHypoSetPopup( int );
  void                         onHypoSetButton();
  void                         onGeomPopup( int );
  void                         onGeomSelectionButton( bool );

private:

  QMap< int, SMESHGUI_MeshTab* > myTabs;
  QTabWidget*                    myTabWg;
  QPopupMenu*                    myHypoSetPopup;
  QPopupMenu*                    myGeomPopup;
};

/*!
 * \brief Tab for tab widget containing controls for definition of 
 * algorithms and hypotheses
*/ 

class SMESHGUI_MeshTab : public QFrame
{
  Q_OBJECT
      
public:      
  /*! To differ main algorithms, hypotheses and additional ones*/
  enum HypType
  { 
    Algo = 0, //!< algorithms
    MainHyp,  //!< main hypothesis
    AddHyp    //!< additional hypothesis
  };            
      
public:      
  SMESHGUI_MeshTab( QWidget* );
  virtual ~SMESHGUI_MeshTab();
  
  void                         setAvailableHyps( const int, const QStringList& );
  void                         setExistingHyps( const int, const QStringList& );
  void                         addHyp( const int, const QString& );
  void                         renameHyp( const int, const int, const QString& );
  void                         setCurrentHyp( const int, const int );
  int                          currentHyp( const int ) const;
  void                         reset();

signals:  

  void                         createHyp( const int theHypType, const int theIndex );
  //!< Emited when "Create hypothesis" button clicked
  void                         editHyp( const int theHypType, const int theIndex );
  //!< Emited when "Edit hypothesis" button clicked
  
private slots:  

  void                         onCreateHyp();  
  void                         onEditHyp();
  void                         onHyp( int );
  void                         onPopupItem( int );
  
private:  
  
  QMap< int, QComboBox* >      myHyp;
  QMap< int, QToolButton* >    myCreateHyp;
  QMap< int, QToolButton* >    myEditHyp;
  
  QMap< int, QStringList >     myAvailableHyps;
  QMap< int, QStringList >     myExistingHyps;
  
  QPopupMenu*                  myPopup;
};

#endif




