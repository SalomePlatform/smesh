// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

// File   : SMESHGUI_MeshInfosBox.cxx
// Author : Edward AGAPOV, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_MeshInfosBox.h"

#include "SMDSAbs_ElementType.hxx"

// Qt includes
#include <QFrame>
#include <QLabel>
#include <QGridLayout>

#define SPACING 6
#define MARGIN  11

#define COLONIZE(str)   (QString(str).contains(":") > 0 ? QString(str) : QString(str) + " :" )

static void addSeparator( QWidget* parent )
{
  QGridLayout* l = qobject_cast<QGridLayout*>( parent->layout() );
  int row  = l->rowCount();
  int cols = l->columnCount();
  for ( int i = 0; i < cols; i++ ) {
    QFrame* hline = new QFrame( parent );
    hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    l->addWidget( hline, row, i );
  }
}

enum TCol {
  COL_ALGO = 0, COL_SHAPE, COL_ERROR, COL_SHAPEID, COL_PUBLISHED, COL_BAD_MESH, NB_COLUMNS
};

// =========================================================================================
/*!
 * \brief Box showing mesh info
 */
// =========================================================================================

SMESHGUI_MeshInfosBox::SMESHGUI_MeshInfosBox(const bool full, QWidget* theParent)
: QGroupBox( tr("SMESH_MESHINFO_TITLE"), theParent ), myFull( full ),
  myNbNode(0), my0DElem(0), myNbEdge(0), myNbLinEdge(0), myNbQuadEdge(0),
  myNbTrai(0), myNbLinTrai(0), myNbQuadTrai(0), myNbQuad(0), myNbLinQuad(0),
  myNbQuadQuad(0), myNbFace(0), myNbLinFace(0), myNbQuadFace(0), myNbPolyg(0),
  myNbHexa(0), myNbLinHexa(0), myNbQuadHexa(0), myNbTetra(0), myNbLinTetra(0),
  myNbQuadTetra(0), myNbPyra(0), myNbLinPyra(0), myNbQuadPyra(0), myNbPrism(0),
  myNbLinPrism(0), myNbQuadPrism(0), myNbVolum(0), myNbLinVolum(0), myNbQuadVolum(0),
  myNbPolyh(0)
{
  QGridLayout* l = new QGridLayout(this);
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );

  QFont italic = font(); italic.setItalic(true);
  QFont bold   = font(); bold.setBold(true);

  QLabel* lab;
  int row = 0;

  // title
  lab = new QLabel( this );
  lab->setMinimumWidth(100); lab->setFont( italic );
  l->addWidget( lab, row, 0 );
  // --
  lab = new QLabel(tr("SMESH_MESHINFO_ORDER0"), this );
  lab->setMinimumWidth(100); lab->setFont( italic );
  l->addWidget( lab, row, 1 );
  // --
  lab = new QLabel(tr("SMESH_MESHINFO_ORDER1"), this );
  lab->setMinimumWidth(100); lab->setFont( italic );
  l->addWidget( lab, row, 2 );
  // --
  lab = new QLabel(tr("SMESH_MESHINFO_ORDER2"), this );
  lab->setMinimumWidth(100); lab->setFont( italic );
  l->addWidget( lab, row, 3 );

  if ( myFull )
  {
    // nodes
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NODES")), this );
    lab->setFont( bold );
    l->addWidget( lab,           row, 0 );
    // --
    myNbNode = new QLabel( this );
    l->addWidget( myNbNode,      row, 1 );

    addSeparator(this);          // add separator

    // 0D elements
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_0DELEMS")), this );
    lab->setFont( bold );
    l->addWidget( lab,           row, 0 );
    // --
    my0DElem = new QLabel( this );
    l->addWidget( my0DElem,      row, 1 );

    addSeparator(this);          // add separator

    // edges
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_EDGES")), this );
    lab->setFont( bold );
    l->addWidget( lab,           row, 0 );
    // --
    myNbEdge = new QLabel( this );
    l->addWidget( myNbEdge,      row, 1 );
    // --
    myNbLinEdge = new QLabel( this );
    l->addWidget( myNbLinEdge,   row, 2 );
    // --
    myNbQuadEdge = new QLabel( this );
    l->addWidget( myNbQuadEdge,  row, 3 );

    addSeparator(this);          // add separator

    // faces
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_FACES")), this);
    lab->setFont( bold );
    l->addWidget( lab,           row, 0 );
    // --
    myNbFace     = new QLabel( this );
    l->addWidget( myNbFace,      row, 1 );
    // --
    myNbLinFace  = new QLabel( this );
    l->addWidget( myNbLinFace,   row, 2 );
    // --
    myNbQuadFace = new QLabel( this );
    l->addWidget( myNbQuadFace,  row, 3 );
    // --
    row++;                       // increment row count
    // ... triangles
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TRIANGLES")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbTrai     = new QLabel( this );
    l->addWidget( myNbTrai,      row, 1 );
    // --
    myNbLinTrai  = new QLabel( this );
    l->addWidget( myNbLinTrai,   row, 2 );
    // --
    myNbQuadTrai = new QLabel( this );
    l->addWidget( myNbQuadTrai,  row, 3 );
    // --
    row++;                       // increment row count
    // ... quadrangles
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_QUADRANGLES")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbQuad     = new QLabel( this );
    l->addWidget( myNbQuad,      row, 1 );
    // --
    myNbLinQuad  = new QLabel( this );
    l->addWidget( myNbLinQuad,   row, 2 );
    // --
    myNbQuadQuad = new QLabel( this );
    l->addWidget( myNbQuadQuad,  row, 3 );
    // --
    row++;                       // increment row count
    // ... poligones
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_POLYGONES")), this );
    l->addWidget( lab,           row, 0 );
    myNbPolyg    = new QLabel( this );
    l->addWidget( myNbPolyg,     row, 1 );

    addSeparator(this);          // add separator

    // volumes
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_VOLUMES")), this);
    lab->setFont( bold );
    l->addWidget( lab,           row, 0 );
    // --
    myNbVolum     = new QLabel( this );
    l->addWidget( myNbVolum,     row, 1 );
    // --
    myNbLinVolum  = new QLabel( this );
    l->addWidget( myNbLinVolum,  row, 2 );
    // --
    myNbQuadVolum = new QLabel( this );
    l->addWidget( myNbQuadVolum, row, 3 );
    // --
    row++;                       // increment row count
    // ... tetras
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TETRAS")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbTetra     = new QLabel( this );
    l->addWidget( myNbTetra,     row, 1 );
    // --
    myNbLinTetra  = new QLabel( this );
    l->addWidget( myNbLinTetra,  row, 2 );
    // --
    myNbQuadTetra = new QLabel( this );
    l->addWidget( myNbQuadTetra, row, 3 );
    // --
    row++;                       // increment row count
    // ... hexas
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_HEXAS")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbHexa      = new QLabel( this );
    l->addWidget( myNbHexa,      row, 1 );
    // --
    myNbLinHexa   = new QLabel( this );
    l->addWidget( myNbLinHexa,   row, 2 );
    // --
    myNbQuadHexa  = new QLabel( this );
    l->addWidget( myNbQuadHexa,  row, 3 );
    // --
    row++;                       // increment row count
    // ... pyras
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_PYRAS")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbPyra      = new QLabel( this );
    l->addWidget( myNbPyra,      row, 1 );
    // --
    myNbLinPyra   = new QLabel( this );
    l->addWidget( myNbLinPyra,   row, 2 );
    // --
    myNbQuadPyra  = new QLabel( this );
    l->addWidget( myNbQuadPyra,  row, 3 );
    // --
    row++;                       // increment row count
    // ... prisms
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_PRISMS")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbPrism     = new QLabel( this );
    l->addWidget( myNbPrism,     row, 1 );
    // --
    myNbLinPrism  = new QLabel( this );
    l->addWidget( myNbLinPrism,  row, 2 );
    // --
    myNbQuadPrism = new QLabel( this );
    l->addWidget( myNbQuadPrism, row, 3 );
    // --
    row++;                       // increment row count
    // ... polyedres
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_POLYEDRES")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbPolyh     = new QLabel( this );
    l->addWidget( myNbPolyh,     row, 1 );
  }
  else
  {
    // nodes
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NODES")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbNode      = new QLabel( this );
    l->addWidget( myNbNode,      row, 1 );

    // 0D elements
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_0DELEMS")), this );
    l->addWidget( lab,           row, 0 );
    // --
    my0DElem = new QLabel( this );
    l->addWidget( my0DElem,      row, 1 );

    addSeparator(this);          // add separator

    // edges
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_EDGES")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbEdge      = new QLabel( this );
    l->addWidget( myNbEdge,      row, 1 );
    // --
    myNbLinEdge   = new QLabel( this );
    l->addWidget( myNbLinEdge,   row, 2 );
    // --
    myNbQuadEdge  = new QLabel( this );
    l->addWidget( myNbQuadEdge,  row, 3 );

    // faces
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_FACES")), this);
    l->addWidget( lab,           row, 0 );
    // --
    myNbFace      = new QLabel( this );
    l->addWidget( myNbFace,      row, 1 );
    // --
    myNbLinFace   = new QLabel( this );
    l->addWidget( myNbLinFace,   row, 2 );
    // --
    myNbQuadFace  = new QLabel( this );
    l->addWidget( myNbQuadFace,  row, 3 );

    // volumes
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_VOLUMES")), this);
    l->addWidget( lab,           row, 0 );
    // --
    myNbVolum     = new QLabel( this );
    l->addWidget( myNbVolum,     row, 1 );
    // --
    myNbLinVolum  = new QLabel( this );
    l->addWidget( myNbLinVolum,  row, 2 );
    // --
    myNbQuadVolum = new QLabel( this );
    l->addWidget( myNbQuadVolum, row, 3 );
  }
}

// =========================================================================================
/*!
 * \brief Set mesh info
 */
// =========================================================================================

void SMESHGUI_MeshInfosBox::SetMeshInfo(const SMESH::long_array& theInfo)
{
  // nodes
  myNbNode     ->setText( QString("%1").arg( theInfo[SMDSEntity_Node] ));
  
  //0D elements
  my0DElem     ->setText( QString("%1").arg( theInfo[SMDSEntity_0D] ));

  // edges
  myNbEdge     ->setText( QString("%1").arg( theInfo[SMDSEntity_Edge] +
                                            theInfo[SMDSEntity_Quad_Edge] ));
  myNbLinEdge  ->setText( QString("%1").arg( theInfo[SMDSEntity_Edge] ));
  myNbQuadEdge ->setText( QString("%1").arg( theInfo[SMDSEntity_Quad_Edge] ));

  // faces
  myNbFace     ->setText( QString("%1").arg( theInfo[SMDSEntity_Triangle] +
                                            theInfo[SMDSEntity_Quad_Triangle] +
                                            theInfo[SMDSEntity_Quadrangle] +
                                            theInfo[SMDSEntity_Quad_Quadrangle] +
                                            theInfo[SMDSEntity_Polygon] ));
  myNbLinFace  ->setText( QString("%1").arg( theInfo[SMDSEntity_Triangle] +
                                            theInfo[SMDSEntity_Quadrangle] +
                                            theInfo[SMDSEntity_Polygon] ));
  myNbQuadFace ->setText( QString("%1").arg( theInfo[SMDSEntity_Quad_Triangle] +
                                            theInfo[SMDSEntity_Quad_Quadrangle] ));

  // volumes
  myNbVolum    ->setText( QString("%1").arg( theInfo[SMDSEntity_Tetra] +
                                            theInfo[SMDSEntity_Quad_Tetra] +
                                            theInfo[SMDSEntity_Pyramid] +
                                            theInfo[SMDSEntity_Quad_Pyramid] +
                                            theInfo[SMDSEntity_Hexa] +
                                            theInfo[SMDSEntity_Quad_Hexa] +
                                            theInfo[SMDSEntity_Penta] +
                                            theInfo[SMDSEntity_Quad_Penta] +
                                            theInfo[SMDSEntity_Polyhedra] ));
  myNbLinVolum ->setText( QString("%1").arg( theInfo[SMDSEntity_Tetra] +
                                            theInfo[SMDSEntity_Pyramid] +
                                            theInfo[SMDSEntity_Hexa] +
                                            theInfo[SMDSEntity_Penta] +
                                            theInfo[SMDSEntity_Polyhedra] ));
  myNbQuadVolum->setText( QString("%1").arg( theInfo[SMDSEntity_Quad_Tetra] +
                                            theInfo[SMDSEntity_Quad_Pyramid] +
                                            theInfo[SMDSEntity_Quad_Hexa] +
                                            theInfo[SMDSEntity_Quad_Penta] ));

  if ( myFull )
  {
    // triangles
    myNbTrai     ->setText( QString("%1").arg( theInfo[SMDSEntity_Triangle] +
                                              theInfo[SMDSEntity_Quad_Triangle] ));
    myNbLinTrai  ->setText( QString("%1").arg( theInfo[SMDSEntity_Triangle] ));
    myNbQuadTrai ->setText( QString("%1").arg( theInfo[SMDSEntity_Quad_Triangle] ));
    // quadrangles
    myNbQuad     ->setText( QString("%1").arg( theInfo[SMDSEntity_Quadrangle] +
                                              theInfo[SMDSEntity_Quad_Quadrangle] ));
    myNbLinQuad  ->setText( QString("%1").arg( theInfo[SMDSEntity_Quadrangle] ));
    myNbQuadQuad ->setText( QString("%1").arg( theInfo[SMDSEntity_Quad_Quadrangle] ));
    // poligones
    myNbPolyg    ->setText( QString("%1").arg( theInfo[SMDSEntity_Polygon] ));

    // tetras
    myNbTetra    ->setText( QString("%1").arg( theInfo[SMDSEntity_Tetra] +
                                              theInfo[SMDSEntity_Quad_Tetra] ));
    myNbLinTetra ->setText( QString("%1").arg( theInfo[SMDSEntity_Tetra] ));
    myNbQuadTetra->setText( QString("%1").arg( theInfo[SMDSEntity_Quad_Tetra] ));
    // hexas
    myNbHexa     ->setText( QString("%1").arg( theInfo[SMDSEntity_Hexa] +
                                              theInfo[SMDSEntity_Quad_Hexa] ));
    myNbLinHexa  ->setText( QString("%1").arg( theInfo[SMDSEntity_Hexa] ));
    myNbQuadHexa ->setText( QString("%1").arg( theInfo[SMDSEntity_Quad_Hexa] ));
    // pyras
    myNbPyra     ->setText( QString("%1").arg( theInfo[SMDSEntity_Pyramid] +
                                            theInfo[SMDSEntity_Quad_Pyramid] ));
    myNbLinPyra  ->setText( QString("%1").arg( theInfo[SMDSEntity_Pyramid] ));
    myNbQuadPyra ->setText( QString("%1").arg( theInfo[SMDSEntity_Quad_Pyramid] ));
    // prisms
    myNbPrism    ->setText( QString("%1").arg( theInfo[SMDSEntity_Penta] +
                                              theInfo[SMDSEntity_Quad_Penta] ));
    myNbLinPrism ->setText( QString("%1").arg( theInfo[SMDSEntity_Penta] ));
    myNbQuadPrism->setText( QString("%1").arg( theInfo[SMDSEntity_Quad_Penta] ));
    // polyedres
    myNbPolyh    ->setText( QString("%1").arg( theInfo[SMDSEntity_Polyhedra] ));
  }
}
