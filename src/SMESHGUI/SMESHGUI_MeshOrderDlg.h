// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File   : SMESHGUI_MeshOrderDlg.h
// Author : Pavel TELKOV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_MeshOrderDlg_H
#define SMESHGUI_MeshOrderDlg_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_Dialog.h"

// Qt includes
#include <QGroupBox>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QToolButton;
class QListWidget;

typedef QList<QStringList> ListListName;
typedef QList<int>         ListId;
typedef QList<ListId>  ListListId;

/*!
 * \brief Reusable widget that shows and allows modify meshing order
 */

class SMESHGUI_EXPORT SMESHGUI_MeshOrderBox : public QGroupBox
{
  Q_OBJECT
    
 public:
  /*! Public methods */
  SMESHGUI_MeshOrderBox( QWidget* );
  ~SMESHGUI_MeshOrderBox();

  //! Clear mesh box
  void         Clear();

  //! Set mesh (submesh) names and indeces
  void         SetMeshes(const ListListName& theMeshNames,
                         const ListListId&   theMeshIds);

  //! returns status is order changed by user
  bool         IsOrderChanged() const;

  //! Returns result (ordered by user) mesh names
  ListListId   GetMeshIds() const;
  //! Returns result (ordered by user) mesh indeces
  ListListName GetMeshNames() const;
 
 private slots:
  /*! Private slots */
  //! update state of arrow buttons according to selection
  void         onSelectionChanged();
  //! move item according to clicked arrow button
  void         onMoveItem();

 private:
  /*! Privatemethods */
  //! move mesh in order up or down
  void         moveItem(const bool theIsUp);

 private:
  /*! Private fields */
  bool         myIsChanged;
  QToolButton* myUpBtn;
  QToolButton* myDownBtn;
  QListWidget* myMeshNames;
};

/*!
 * \brief Dialog contains mesh order box for modification operation
 */

class SMESHGUI_EXPORT SMESHGUI_MeshOrderDlg : public SMESHGUI_Dialog
{
  Q_OBJECT
    
 public:
  /*! Public methods */
  SMESHGUI_MeshOrderDlg( QWidget* );
  ~SMESHGUI_MeshOrderDlg();

  SMESHGUI_MeshOrderBox* GetMeshOrderBox() const;

 private:
  /*! Private fields */
  SMESHGUI_MeshOrderBox* myBox;
};

#endif // SMESHGUI_MeshOrderDlg_H
