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

// File   : SMESHGUI_MeshInfosBox.h
// Author : Edward AGAPOV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_MeshInfosBox_H
#define SMESHGUI_MeshInfosBox_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QGroupBox>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QLabel;

/*!
 * \brief Box showing mesh info
 */

class SMESHGUI_EXPORT SMESHGUI_MeshInfosBox : public QGroupBox
{
  Q_OBJECT

public:
  SMESHGUI_MeshInfosBox( const bool, QWidget* );

  void    SetMeshInfo( const SMESH::long_array& theInfo );

private:
  bool    myFull;
  QLabel* myNbNode;
  QLabel* my0DElem;
  QLabel* myBall;
  QLabel* myNbEdge;
  QLabel* myNbLinEdge;
  QLabel* myNbQuadEdge;
  QLabel* myNbTrai;
  QLabel* myNbLinTrai;
  QLabel* myNbQuadTrai;
  QLabel* myNbBiQuadTrai;
  QLabel* myNbQuad;
  QLabel* myNbLinQuad;
  QLabel* myNbQuadQuad;
  QLabel* myNbBiQuadQuad;
  QLabel* myNbFace;
  QLabel* myNbLinFace;
  QLabel* myNbQuadFace;
  QLabel* myNbBiQuadFace;
  QLabel* myNbPolyg;
  QLabel* myNbLinPolyg;
  QLabel* myNbQuadPolyg;
  QLabel* myNbHexa;
  QLabel* myNbLinHexa;
  QLabel* myNbQuadHexa;
  QLabel* myNbBiQuadHexa;
  QLabel* myNbTetra;
  QLabel* myNbLinTetra;
  QLabel* myNbQuadTetra;
  QLabel* myNbPyra;
  QLabel* myNbLinPyra;
  QLabel* myNbQuadPyra;
  QLabel* myNbPrism;
  QLabel* myNbLinPrism;
  QLabel* myNbQuadPrism;
  QLabel* myNbVolum;
  QLabel* myNbLinVolum;
  QLabel* myNbQuadVolum;
  QLabel* myNbBiQuadVolum;
  QLabel* myNbHexaPrism;
  QLabel* myNbPolyh;
};

#endif // SMESHGUI_MeshInfosBox_H
