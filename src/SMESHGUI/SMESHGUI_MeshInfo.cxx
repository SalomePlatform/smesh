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
//  File   : SMESHGUI_MeshInfo.cxx
//  Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

#include "SMESHGUI_MeshInfo.h"

#include "SMDSAbs_ElementType.hxx"
#include "SMDS_BallElement.hxx"
#include "SMDS_EdgePosition.hxx"
#include "SMDS_FacePosition.hxx"
#include "SMDS_Mesh.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESHGUI.h"
#include "SMESHGUI_FilterUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESH_Actor.h"

#include <LightApp_SelectionMgr.h>
#include <SUIT_FileDlg.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SVTK_ViewWindow.h>

#include <SALOMEDSClient_Study.hxx>
#include <SalomeApp_Study.h>

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QContextMenuEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QRadioButton>
#include <QTextStream>
#include <QTabWidget>
#include <QTextBrowser>
#include <QVBoxLayout>

#include "utilities.h"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GEOM_Gen)

namespace {

const int SPACING      = 6;
const int MARGIN       = 9;
const int MAXITEMS     = 10;
const int GROUPS_ID    = 100;
const int SUBMESHES_ID = 200;
const int SPACING_INFO = 2;

enum InfoRole {
  TypeRole = Qt::UserRole + 10,
  IdRole,
};

enum InfoType {
  NodeConnectivity = 100,
  ElemConnectivity,
};
} // namesapce

/*!
  \class ExtraWidget
  \internal
*/
class ExtraWidget : public QWidget
{
public:
  ExtraWidget( QWidget*, bool = false );
  ~ExtraWidget();

  void updateControls( int, int, int = MAXITEMS );

public:
  QLabel*      current;
  QPushButton* prev;
  QPushButton* next;
  bool         brief;
};

ExtraWidget::ExtraWidget( QWidget* parent, bool b ) : QWidget( parent ), brief( b )
{
  current = new QLabel( this );
  current->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  prev = new QPushButton( tr( "<<" ), this );
  next = new QPushButton( tr( ">>" ), this );
  QHBoxLayout* hbl = new QHBoxLayout( this );
  hbl->setContentsMargins( 0, SPACING, 0, 0 );
  hbl->setSpacing( SPACING );
  hbl->addStretch();
  hbl->addWidget( current );
  hbl->addWidget( prev );
  hbl->addWidget( next );
}

ExtraWidget::~ExtraWidget()
{
}

void ExtraWidget::updateControls( int total, int index, int blockSize )
{
  setVisible( total > blockSize );
  QString format = brief ? QString( "%1-%2 / %3" ) : SMESHGUI_MeshInfoDlg::tr( "X_FROM_Y_ITEMS_SHOWN" );
  current->setText( format.arg( index*blockSize+1 ).arg( qMin( index*blockSize+blockSize, total ) ).arg( total ) );
  prev->setEnabled( index > 0 );
  next->setEnabled( (index+1)*blockSize < total );
}

/*!
  \class DumpFileDlg
  \brief Customization of standard "Save file" dialog box for dump info operation
  \internal
*/

class DumpFileDlg : public SUIT_FileDlg
{
public:
  DumpFileDlg( QWidget* parent );

  QCheckBox* myBaseChk;
  QCheckBox* myElemChk;
  QCheckBox* myAddChk;
  QCheckBox* myCtrlChk;
};

/*!
  \brief Constructor
  \internal
*/
DumpFileDlg::DumpFileDlg( QWidget* parent ) : SUIT_FileDlg( parent, false, true, true )
{
  QGridLayout* grid = ::qobject_cast<QGridLayout *>( layout() );
  if ( grid ) {
    QWidget* hB = new QWidget( this );
    myBaseChk = new QCheckBox( SMESHGUI::tr( "PREF_DUMP_BASE_INFO" ), hB );
    myElemChk = new QCheckBox( SMESHGUI::tr( "PREF_DUMP_ELEM_INFO" ), hB );
    myAddChk  = new QCheckBox( SMESHGUI::tr( "PREF_DUMP_ADD_INFO" ),  hB );
    myCtrlChk = new QCheckBox( SMESHGUI::tr( "PREF_DUMP_CTRL_INFO" ), hB );

    QGridLayout* layout = new QGridLayout( hB );
    layout->addWidget( myBaseChk, 0, 0 );
    layout->addWidget( myElemChk, 0, 1 );
    layout->addWidget( myAddChk, 1, 0 );
    layout->addWidget( myCtrlChk, 1, 1 );

    QPushButton* pb = new QPushButton( this );

    int row = grid->rowCount();
    grid->addWidget( new QLabel( "", this ), row, 0 );
    grid->addWidget( hB, row, 1, 1, 3 );
    grid->addWidget( pb, row, 5 );

    pb->hide();
  }
}

/*!
  \brief Get depth of the tree item
  \internal
  \param theItem tree widget item
  \return item's depth in tree widget (where top-level items have zero depth)
*/
static int itemDepth( QTreeWidgetItem* item )
{
  int d = 0;
  QTreeWidgetItem* p = item->parent();
  while ( p ) {
    d++;
    p = p->parent();
  }
  return d;
}

/*!
  \class SMESHGUI_MeshInfo
  \brief Base mesh information widget
  
  Displays the base information about mesh object: mesh, sub-mesh, group or arbitrary ID source.
*/

/*!
  \brief Constructor.
  \param parent parent widget
*/
SMESHGUI_MeshInfo::SMESHGUI_MeshInfo( QWidget* parent )
  : QFrame( parent ), myWidgets( iElementsEnd )
{
  setFrameStyle( StyledPanel | Sunken );

  QGridLayout* l = new QGridLayout( this );
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );

  int index = 0;

  // object
  QLabel* aNameLab     = new QLabel( tr( "NAME_LAB" ), this );
  QLabel* aName        = createField();
  aName->setObjectName("meshName");
  aName->setMinimumWidth( 150 );
  QLabel* aObjLab      = new QLabel( tr( "OBJECT_LAB" ), this );
  QLabel* aObj         = createField();
  aObj->setObjectName("meshType");
  aObj->setMinimumWidth( 150 );
  myWidgets[ index++ ] << aNameLab << aName;
  myWidgets[ index++ ] << aObjLab  << aObj;

  // nodes
  QWidget* aNodesLine  = createLine();
  QLabel*  aNodesLab   = new QLabel( tr( "NODES_LAB" ), this );
  QLabel*  aNodes      = createField();
  aNodes->setObjectName("nbNodes");
  myWidgets[ index++ ] << aNodesLine;
  myWidgets[ index++ ] << aNodesLab << aNodes;

  // elements
  QWidget* aElemLine   = createLine();
  QLabel*  aElemLab    = new QLabel( tr( "ELEMENTS_LAB" ),     this );
  QLabel*  aElemTotal  = new QLabel( tr( "TOTAL_LAB" ),        this );
  QLabel*  aElemLin    = new QLabel( tr( "LINEAR_LAB" ),       this );
  QLabel*  aElemQuad   = new QLabel( tr( "QUADRATIC_LAB" ),    this );
  QLabel*  aElemBiQuad = new QLabel( tr( "BI_QUADRATIC_LAB" ), this );
  myWidgets[ index++ ] << aElemLine;
  myWidgets[ index++ ] << aElemLab << aElemTotal << aElemLin << aElemQuad << aElemBiQuad;

  // ... Number elements
  QWidget* aNbLine     = createLine(); 
  QLabel*  aNbTotal    = createField();
  aNbTotal->setObjectName("totalNbElems");
  QLabel*  aNbLin      = createField();
  aNbLin->setObjectName("totalNbLinearElems");
  QLabel*  aNbQuad     = createField();
  aNbQuad->setObjectName("totalNbQuadraticElems");
  QLabel*  aNbBiQuad   = createField();
  aNbBiQuad->setObjectName("totalNbBiQuadraticElems");
  myWidgets[ index++ ] << aNbLine;
  myWidgets[ index++ ] << new QLabel( "", this ) << aNbTotal << aNbLin << aNbQuad << aNbBiQuad;

  // ... 0D elements
  QWidget* a0DLine     = createLine();
  QLabel*  a0DLab      = new QLabel( tr( "0D_LAB" ), this );
  QLabel*  a0DTotal    = createField();
  a0DTotal->setObjectName("nb0D");

  myWidgets[ index++ ] << a0DLine;
  myWidgets[ index++ ] << a0DLab << a0DTotal;

  // ... Ball elements
  QWidget* aBallLine     = createLine();
  QLabel*  aBallLab      = new QLabel( tr( "BALL_LAB" ), this );
  QLabel*  aBallTotal    = createField();
  aBallTotal->setObjectName("nbBall");
  myWidgets[ index++ ] << aBallLine;
  myWidgets[ index++ ] << aBallLab << aBallTotal;

  // ... 1D elements
  QWidget* a1DLine     = createLine();
  QLabel*  a1DLab      = new QLabel( tr( "1D_LAB" ), this );
  QLabel*  a1DTotal    = createField();
  a1DTotal->setObjectName("nb1D");
  QLabel*  a1DLin      = createField();
  a1DLin->setObjectName("nbLinear1D");
  QLabel*  a1DQuad     = createField();
  a1DQuad->setObjectName("nbQuadratic1D");
  myWidgets[ index++ ] << a1DLine;
  myWidgets[ index++ ] << a1DLab << a1DTotal << a1DLin << a1DQuad;

  // ... 2D elements
  QWidget* a2DLine      = createLine();
  QLabel*  a2DLab       = new QLabel( tr( "2D_LAB" ), this );
  QLabel*  a2DTotal     = createField();
  a2DTotal->setObjectName("nb2D");
  QLabel*  a2DLin       = createField();
  a2DLin->setObjectName("nbLinear2D");
  QLabel*  a2DQuad      = createField();
  a2DQuad->setObjectName("nbQuadratic2D");
  QLabel*  a2DBiQuad    = createField();
  a2DBiQuad->setObjectName("nbBiQuadratic2D");
  QLabel*  a2DTriLab    = new QLabel( tr( "TRIANGLES_LAB" ), this );
  QLabel*  a2DTriTotal  = createField();
  a2DTriTotal->setObjectName("nbTriangle");
  QLabel*  a2DTriLin    = createField();
  a2DTriLin->setObjectName("nbLinearTriangle");
  QLabel*  a2DTriQuad   = createField();
  a2DTriQuad->setObjectName("nbQuadraticTriangle");
  QLabel*  a2DTriBiQuad = createField();
    a2DTriBiQuad->setObjectName("nbBiQuadraticTriangle");
  QLabel*  a2DQuaLab    = new QLabel( tr( "QUADRANGLES_LAB" ), this );
  QLabel*  a2DQuaTotal  = createField();
  a2DQuaTotal->setObjectName("nbQuadrangle");
  QLabel*  a2DQuaLin    = createField();
  a2DQuaLin->setObjectName("nbLinearQuadrangle");
  QLabel*  a2DQuaQuad   = createField();
  a2DQuaQuad->setObjectName("nbQuadraticQuadrangle");
  QLabel*  a2DQuaBiQuad = createField();
  a2DQuaBiQuad->setObjectName("nbBiQuadraticQuadrangle");
  QLabel*  a2DPolLab    = new QLabel( tr( "POLYGONS_LAB" ), this );
  QLabel*  a2DPolTotal  = createField();
  a2DPolTotal->setObjectName("nbPolygon");
  QLabel*  a2DPolLin    = createField();
  a2DPolLin->setObjectName("nbLinearPolygon");
  QLabel*  a2DPolQuad   = createField();
  a2DPolQuad->setObjectName("nbQuadraticPolygon");
  myWidgets[ index++ ] << a2DLine;
  myWidgets[ index++ ] << a2DLab    << a2DTotal    << a2DLin    << a2DQuad    << a2DBiQuad;
  myWidgets[ index++ ] << a2DTriLab << a2DTriTotal << a2DTriLin << a2DTriQuad << a2DTriBiQuad;
  myWidgets[ index++ ] << a2DQuaLab << a2DQuaTotal << a2DQuaLin << a2DQuaQuad << a2DQuaBiQuad;
  myWidgets[ index++ ] << a2DPolLab << a2DPolTotal << a2DPolLin << a2DPolQuad;

  // ... 3D elements
  QWidget* a3DLine      = createLine();
  QLabel*  a3DLab       = new QLabel( tr( "3D_LAB" ), this );
  QLabel*  a3DTotal     = createField();
  a3DTotal->setObjectName("nb3D");
  QLabel*  a3DLin       = createField();
  a3DLin->setObjectName("nbLinear3D");
  QLabel*  a3DQuad      = createField();
  a3DQuad->setObjectName("nbQuadratic3D");
  QLabel*  a3DBiQuad    = createField();
  a3DBiQuad->setObjectName("nbBiQuadratic3D");
  QLabel*  a3DTetLab    = new QLabel( tr( "TETRAHEDRONS_LAB" ), this );
  QLabel*  a3DTetTotal  = createField();
  a3DTetTotal->setObjectName("nbTetrahedron");
  QLabel*  a3DTetLin    = createField();
  a3DTetLin->setObjectName("nbLinearTetrahedron");
  QLabel*  a3DTetQuad   = createField();
  a3DTetQuad->setObjectName("nbQudraticTetrahedron");
  QLabel*  a3DHexLab    = new QLabel( tr( "HEXAHEDONRS_LAB" ), this );
  QLabel*  a3DHexTotal  = createField();
  a3DHexTotal->setObjectName("nbHexahedron");
  QLabel*  a3DHexLin    = createField();
  a3DHexLin->setObjectName("nbLinearHexahedron");
  QLabel*  a3DHexQuad   = createField();
  a3DHexQuad->setObjectName("nbQuadraticHexahedron");
  QLabel*  a3DHexBiQuad = createField();
  a3DHexBiQuad->setObjectName("nbBiQuadraticHexahedron");
  QLabel*  a3DPyrLab    = new QLabel( tr( "PYRAMIDS_LAB" ), this );
  QLabel*  a3DPyrTotal  = createField();
  a3DPyrTotal->setObjectName("nbPyramid");
  QLabel*  a3DPyrLin    = createField();
  a3DPyrLin->setObjectName("nbLinearPyramid");
  QLabel*  a3DPyrQuad   = createField();
  a3DPyrQuad->setObjectName("nbQuadraticPyramid");
  QLabel*  a3DPriLab    = new QLabel( tr( "PRISMS_LAB" ), this );
  QLabel*  a3DPriTotal  = createField();
  a3DPriTotal->setObjectName("nbPrism");
  QLabel*  a3DPriLin    = createField();
  a3DPriLin->setObjectName("nbLinearPrism");
  QLabel*  a3DPriQuad   = createField();
  a3DPriQuad->setObjectName("nbQuadraticPrism");
  QLabel*  a3DHexPriLab   = new QLabel( tr( "HEX_PRISMS_LAB" ), this );
  QLabel*  a3DHexPriTotal = createField();
  a3DHexPriTotal->setObjectName("nbHexagonalPrism");
  QLabel*  a3DPolLab    = new QLabel( tr( "POLYHEDRONS_LAB" ), this );
  QLabel*  a3DPolTotal  = createField();
  a3DPolTotal->setObjectName("nbPolyhedron");
  myWidgets[ index++ ] << a3DLine;
  myWidgets[ index++ ] << a3DLab    << a3DTotal    << a3DLin    << a3DQuad    << a3DBiQuad;
  myWidgets[ index++ ] << a3DTetLab << a3DTetTotal << a3DTetLin << a3DTetQuad;
  myWidgets[ index++ ] << a3DHexLab << a3DHexTotal << a3DHexLin << a3DHexQuad << a3DHexBiQuad;
  myWidgets[ index++ ] << a3DPyrLab << a3DPyrTotal << a3DPyrLin << a3DPyrQuad;
  myWidgets[ index++ ] << a3DPriLab << a3DPriTotal << a3DPriLin << a3DPriQuad;
  myWidgets[ index++ ] << a3DHexPriLab << a3DHexPriTotal;
  myWidgets[ index++ ] << a3DPolLab << a3DPolTotal;

  myLoadBtn = new QPushButton( tr( "BUT_LOAD_MESH" ), this );
  myLoadBtn->setAutoDefault( true );
  connect( myLoadBtn, SIGNAL( clicked() ), this, SLOT( loadMesh() ) );
  
  setFontAttributes( aNameLab,    Bold );
  setFontAttributes( aObjLab,     Bold );
  setFontAttributes( aNodesLab,   Bold );
  setFontAttributes( aElemLab,    Bold );
  setFontAttributes( aElemTotal,  Italic );
  setFontAttributes( aElemLin,    Italic );
  setFontAttributes( aElemQuad,   Italic );
  setFontAttributes( aElemBiQuad, Italic );
  setFontAttributes( a0DLab,      Bold );
  setFontAttributes( aBallLab,    Bold );
  setFontAttributes( a1DLab,      Bold );
  setFontAttributes( a2DLab,      Bold );
  setFontAttributes( a3DLab,      Bold );

  l->addWidget( aNameLab,     0, 0 );
  l->addWidget( aName,        0, 1, 1, 4 );
  l->addWidget( aObjLab,      1, 0 );
  l->addWidget( aObj,         1, 1, 1, 4 );
  l->addWidget( aNodesLine,   2, 0, 1, 5 );
  l->addWidget( aNodesLab,    3, 0 );
  l->addWidget( aNodes,       3, 1 );
  l->addWidget( aElemLine,    4, 0, 1, 5 );
  l->addWidget( aElemLab,     5, 0 );
  l->addWidget( aElemTotal,   5, 1 );
  l->addWidget( aElemLin,     5, 2 );
  l->addWidget( aElemQuad,    5, 3 );
  l->addWidget( aElemBiQuad,  5, 4 );
  l->addWidget( aNbLine,      6, 1, 1, 4 );
  l->addWidget( aNbTotal,     7, 1 );
  l->addWidget( aNbLin,       7, 2 );
  l->addWidget( aNbQuad,      7, 3 );
  l->addWidget( aNbBiQuad,    7, 4 );
  l->addWidget( a0DLine,      8, 1, 1, 4 );
  l->addWidget( a0DLab,       9, 0 );
  l->addWidget( a0DTotal,     9, 1 );
  l->addWidget( aBallLine,    10, 1, 1, 4 );
  l->addWidget( aBallLab,     11, 0 );
  l->addWidget( aBallTotal,   11, 1 );
  l->addWidget( a1DLine,      12, 1, 1, 4 );
  l->addWidget( a1DLab,       13, 0 );
  l->addWidget( a1DTotal,     13, 1 );
  l->addWidget( a1DLin,       13, 2 );
  l->addWidget( a1DQuad,      13, 3 );
  l->addWidget( a2DLine,      14, 1, 1, 4 );
  l->addWidget( a2DLab,       15, 0 );
  l->addWidget( a2DTotal,     15, 1 );
  l->addWidget( a2DLin,       15, 2 );
  l->addWidget( a2DQuad,      15, 3 );
  l->addWidget( a2DBiQuad,    15, 4 );
  l->addWidget( a2DTriLab,    16, 0 );
  l->addWidget( a2DTriTotal,  16, 1 );
  l->addWidget( a2DTriLin,    16, 2 );
  l->addWidget( a2DTriQuad,   16, 3 );
  l->addWidget( a2DTriBiQuad, 16, 4 );
  l->addWidget( a2DQuaLab,    17, 0 );
  l->addWidget( a2DQuaTotal,  17, 1 );
  l->addWidget( a2DQuaLin,    17, 2 );
  l->addWidget( a2DQuaQuad,   17, 3 );
  l->addWidget( a2DQuaBiQuad, 17, 4 );
  l->addWidget( a2DPolLab,    18, 0 );
  l->addWidget( a2DPolTotal,  18, 1 );
  l->addWidget( a2DPolLin,    18, 2 );
  l->addWidget( a2DPolQuad,   18, 3 );
  l->addWidget( a3DLine,      19, 1, 1, 4 );
  l->addWidget( a3DLab,       20, 0 );
  l->addWidget( a3DTotal,     20, 1 );
  l->addWidget( a3DLin,       20, 2 );
  l->addWidget( a3DQuad,      20, 3 );
  l->addWidget( a3DBiQuad,    20, 4 );
  l->addWidget( a3DTetLab,    21, 0 );
  l->addWidget( a3DTetTotal,  21, 1 );
  l->addWidget( a3DTetLin,    21, 2 );
  l->addWidget( a3DTetQuad,   21, 3 );
  l->addWidget( a3DHexLab,    22, 0 );
  l->addWidget( a3DHexTotal,  22, 1 );
  l->addWidget( a3DHexLin,    22, 2 );
  l->addWidget( a3DHexQuad,   22, 3 );
  l->addWidget( a3DHexBiQuad, 22, 4 );
  l->addWidget( a3DPyrLab,    23, 0 );
  l->addWidget( a3DPyrTotal,  23, 1 );
  l->addWidget( a3DPyrLin,    23, 2 );
  l->addWidget( a3DPyrQuad,   23, 3 );
  l->addWidget( a3DPriLab,    24, 0 );
  l->addWidget( a3DPriTotal,  24, 1 );
  l->addWidget( a3DPriLin,    24, 2 );
  l->addWidget( a3DPriQuad,   24, 3 );
  l->addWidget( a3DHexPriLab,   25, 0 );
  l->addWidget( a3DHexPriTotal, 25, 1 );
  l->addWidget( a3DPolLab,    26, 0 );
  l->addWidget( a3DPolTotal,  26, 1 );
  l->addWidget( myLoadBtn,    28, 1, 1, 4 );

  l->setColumnStretch( 0, 0 );
  l->setColumnStretch( 1, 5 );
  l->setColumnStretch( 2, 5 );
  l->setColumnStretch( 3, 5 );
  l->setColumnStretch( 4, 5 );
  l->setRowStretch( 27, 5 );

  clear();
}

/*!
  \brief Destructor
*/
SMESHGUI_MeshInfo::~SMESHGUI_MeshInfo()
{
}

/*!
  \brief Show information on the mesh object.
  \param obj object being processed (mesh, sub-mesh, group, ID source)
*/
void SMESHGUI_MeshInfo::showInfo( SMESH::SMESH_IDSource_ptr obj )
{
  clear();
  if ( !CORBA::is_nil( obj ) ) {
    _PTR(SObject) sobj = SMESH::ObjectToSObject( obj );
    if ( sobj ) 
      myWidgets[iName][iSingle]->setProperty( "text", sobj->GetName().c_str() );
    SMESH::SMESH_Mesh_var      aMesh    = SMESH::SMESH_Mesh::_narrow( obj );
    SMESH::SMESH_subMesh_var   aSubMesh = SMESH::SMESH_subMesh::_narrow( obj );
    SMESH::SMESH_GroupBase_var aGroup   = SMESH::SMESH_GroupBase::_narrow( obj );
    if ( !aMesh->_is_nil() ) {
      myWidgets[iObject][iSingle]->setProperty( "text", tr( "OBJECT_MESH" ) );
    }
    else if ( !aSubMesh->_is_nil() ) {
      myWidgets[iObject][iSingle]->setProperty( "text", tr( "OBJECT_SUBMESH" ) );
    }
    else if ( !aGroup->_is_nil() ) {
      QString objType;
      switch( aGroup->GetType() ) {
      case SMESH::NODE:  objType = tr( "OBJECT_GROUP_NODES"   );break;
      case SMESH::EDGE:  objType = tr( "OBJECT_GROUP_EDGES"   );break;
      case SMESH::FACE:  objType = tr( "OBJECT_GROUP_FACES"   );break;
      case SMESH::VOLUME:objType = tr( "OBJECT_GROUP_VOLUMES" );break;
      case SMESH::ELEM0D:objType = tr( "OBJECT_GROUP_0DELEMS" );break;
      case SMESH::BALL:  objType = tr( "OBJECT_GROUP_BALLS"   );break;
      default:           objType = tr( "OBJECT_GROUP"         );break;
      }
      myWidgets[iObject][iSingle]->setProperty( "text", objType );
    }
    SMESH::long_array_var info = obj->GetMeshInfo();
    myWidgets[iNodes][iTotal] ->setProperty( "text", QString::number( info[SMDSEntity_Node] ) );
    myWidgets[i0D][iTotal]    ->setProperty( "text", QString::number( info[SMDSEntity_0D] ) );
    myWidgets[iBalls][iTotal] ->setProperty( "text", QString::number( info[SMDSEntity_Ball] ) );
    long nbEdges = info[SMDSEntity_Edge] + info[SMDSEntity_Quad_Edge];
    myWidgets[i1D][iTotal]    ->setProperty( "text", QString::number( nbEdges ) );
    myWidgets[i1D][iLinear]   ->setProperty( "text", QString::number( info[SMDSEntity_Edge] ) );
    myWidgets[i1D][iQuadratic]->setProperty( "text", QString::number( info[SMDSEntity_Quad_Edge] ) );
    long nbTriangles     = info[SMDSEntity_Triangle]   + info[SMDSEntity_Quad_Triangle]   + info[SMDSEntity_BiQuad_Triangle];
    long nbQuadrangles   = info[SMDSEntity_Quadrangle] + info[SMDSEntity_Quad_Quadrangle] + info[SMDSEntity_BiQuad_Quadrangle];
    long nb2DPolygons    = info[SMDSEntity_Polygon]    + info[SMDSEntity_Quad_Polygon];
    long nb2DLinear      = info[SMDSEntity_Triangle]        + info[SMDSEntity_Quadrangle] + info[SMDSEntity_Polygon];
    long nb2DQuadratic   = info[SMDSEntity_Quad_Triangle]   + info[SMDSEntity_Quad_Quadrangle] + info[SMDSEntity_Quad_Polygon];
    long nb2DBiQuadratic = info[SMDSEntity_BiQuad_Triangle] + info[SMDSEntity_BiQuad_Quadrangle];
    long nb2DTotal       = nb2DLinear + nb2DQuadratic + nb2DBiQuadratic;

    myWidgets[i2D][iTotal]                  ->setProperty( "text", QString::number( nb2DTotal ));
    myWidgets[i2D][iLinear]                 ->setProperty( "text", QString::number( nb2DLinear ) );
    myWidgets[i2D][iQuadratic]              ->setProperty( "text", QString::number( nb2DQuadratic ) );
    myWidgets[i2D][iBiQuadratic]            ->setProperty( "text", QString::number( nb2DBiQuadratic ) );
    myWidgets[i2DTriangles][iTotal]         ->setProperty( "text", QString::number( nbTriangles ) );
    myWidgets[i2DTriangles][iLinear]        ->setProperty( "text", QString::number( info[SMDSEntity_Triangle] ) );
    myWidgets[i2DTriangles][iQuadratic]     ->setProperty( "text", QString::number( info[SMDSEntity_Quad_Triangle] ) );
    myWidgets[i2DTriangles][iBiQuadratic]   ->setProperty( "text", QString::number( info[SMDSEntity_BiQuad_Triangle] ) );
    myWidgets[i2DQuadrangles][iTotal]       ->setProperty( "text", QString::number( nbQuadrangles ) );
    myWidgets[i2DQuadrangles][iLinear]      ->setProperty( "text", QString::number( info[SMDSEntity_Quadrangle] ) );
    myWidgets[i2DQuadrangles][iQuadratic]   ->setProperty( "text", QString::number( info[SMDSEntity_Quad_Quadrangle] ) );
    myWidgets[i2DQuadrangles][iBiQuadratic] ->setProperty( "text", QString::number( info[SMDSEntity_BiQuad_Quadrangle] ) );
    myWidgets[i2DPolygons][iTotal]          ->setProperty( "text", QString::number( nb2DPolygons ));
    myWidgets[i2DPolygons][iLinear]         ->setProperty( "text", QString::number( info[SMDSEntity_Polygon] ) );
    myWidgets[i2DPolygons][iQuadratic]      ->setProperty( "text", QString::number( info[SMDSEntity_Quad_Polygon] ) );
    long nbTetrahedrons  = info[SMDSEntity_Tetra]   + info[SMDSEntity_Quad_Tetra];
    long nbHexahedrons   = info[SMDSEntity_Hexa]    + info[SMDSEntity_Quad_Hexa] + info[SMDSEntity_TriQuad_Hexa];
    long nbPyramids      = info[SMDSEntity_Pyramid] + info[SMDSEntity_Quad_Pyramid];
    long nbPrisms        = info[SMDSEntity_Penta]   + info[SMDSEntity_Quad_Penta];
    long nb3DLinear      = info[SMDSEntity_Tetra]   + info[SMDSEntity_Hexa] + info[SMDSEntity_Pyramid] + info[SMDSEntity_Penta] + info[SMDSEntity_Polyhedra] + info[SMDSEntity_Hexagonal_Prism];
    long nb3DQuadratic   = info[SMDSEntity_Quad_Tetra] + info[SMDSEntity_Quad_Hexa] + info[SMDSEntity_Quad_Pyramid] + info[SMDSEntity_Quad_Penta];
    long nb3DBiQuadratic = info[SMDSEntity_TriQuad_Hexa];
    long nb3DTotal       = nb3DLinear + nb3DQuadratic + nb3DBiQuadratic;
    myWidgets[i3D][iTotal]                  ->setProperty( "text", QString::number( nb3DTotal ) );
    myWidgets[i3D][iLinear]                 ->setProperty( "text", QString::number( nb3DLinear ) );
    myWidgets[i3D][iQuadratic]              ->setProperty( "text", QString::number( nb3DQuadratic ) );
    myWidgets[i3D][iBiQuadratic]            ->setProperty( "text", QString::number( nb3DBiQuadratic ) );
    myWidgets[i3DTetrahedrons][iTotal]      ->setProperty( "text", QString::number( nbTetrahedrons ) );
    myWidgets[i3DTetrahedrons][iLinear]     ->setProperty( "text", QString::number( info[SMDSEntity_Tetra] ) );
    myWidgets[i3DTetrahedrons][iQuadratic]  ->setProperty( "text", QString::number( info[SMDSEntity_Quad_Tetra] ) );
    myWidgets[i3DHexahedrons][iTotal]       ->setProperty( "text", QString::number( nbHexahedrons ) );
    myWidgets[i3DHexahedrons][iLinear]      ->setProperty( "text", QString::number( info[SMDSEntity_Hexa] ) );
    myWidgets[i3DHexahedrons][iQuadratic]   ->setProperty( "text", QString::number( info[SMDSEntity_Quad_Hexa] ) );
    myWidgets[i3DHexahedrons][iBiQuadratic] ->setProperty( "text", QString::number( info[SMDSEntity_TriQuad_Hexa] ) );
    myWidgets[i3DPyramids][iTotal]          ->setProperty( "text", QString::number( nbPyramids ) );
    myWidgets[i3DPyramids][iLinear]         ->setProperty( "text", QString::number( info[SMDSEntity_Pyramid] ) );
    myWidgets[i3DPyramids][iQuadratic]      ->setProperty( "text", QString::number( info[SMDSEntity_Quad_Pyramid] ) );
    myWidgets[i3DPrisms][iTotal]            ->setProperty( "text", QString::number( nbPrisms ) );
    myWidgets[i3DPrisms][iLinear]           ->setProperty( "text", QString::number( info[SMDSEntity_Penta] ) );
    myWidgets[i3DPrisms][iQuadratic]        ->setProperty( "text", QString::number( info[SMDSEntity_Quad_Penta] ) );
    myWidgets[i3DHexaPrisms][iTotal]        ->setProperty( "text", QString::number( info[SMDSEntity_Hexagonal_Prism] ) );
    myWidgets[i3DPolyhedrons][iTotal]       ->setProperty( "text", QString::number( info[SMDSEntity_Polyhedra] ) );
    long nbElemTotal       = info[SMDSEntity_0D] + info[SMDSEntity_Ball] + nbEdges + nb2DTotal + nb3DTotal;
    long nbElemLinerial    = info[SMDSEntity_Edge] + nb2DLinear + nb3DLinear;
    long nbElemQuadratic   = info[SMDSEntity_Quad_Edge] + nb2DQuadratic + nb3DQuadratic;
    long nbElemBiQuadratic = nb2DBiQuadratic + nb3DBiQuadratic;
    myWidgets[iNb][iTotal]      ->setProperty( "text", QString::number( nbElemTotal ) );
    myWidgets[iNb][iLinear]     ->setProperty( "text", QString::number( nbElemLinerial ) );
    myWidgets[iNb][iQuadratic]  ->setProperty( "text", QString::number( nbElemQuadratic ) );
    myWidgets[iNb][iBiQuadratic]->setProperty( "text", QString::number( nbElemBiQuadratic ) );
    // before full loading from study file, type of elements in a sub-mesh can't be defined
    // in some cases
    bool infoOK = obj->IsMeshInfoCorrect();
    myLoadBtn->setVisible( !infoOK );
    if ( !infoOK )
    {
      // two options:
      // 1. Type of 2D or 3D elements is unknown but their nb is OK (for a sub-mesh)
      // 2. No info at all (for a group on geom or filter)
      bool hasAnyInfo = false;
      for ( size_t i = 0; i < info->length() && !hasAnyInfo; ++i )
        hasAnyInfo = info[i];
      if ( hasAnyInfo ) // believe it is a sub-mesh
      {
        if ( nb2DLinear + nb2DQuadratic + nb2DBiQuadratic > 0 )
        {
          myWidgets[i2D][iLinear]                 ->setProperty( "text", "?" );
          myWidgets[i2D][iQuadratic]              ->setProperty( "text", "?" );
          myWidgets[i2D][iBiQuadratic]            ->setProperty( "text", "?" );
          myWidgets[i2DTriangles][iTotal]         ->setProperty( "text", "?" );
          myWidgets[i2DTriangles][iLinear]        ->setProperty( "text", "?" );
          myWidgets[i2DTriangles][iQuadratic]     ->setProperty( "text", "?" );
          myWidgets[i2DTriangles][iBiQuadratic]   ->setProperty( "text", "?" );
          myWidgets[i2DQuadrangles][iTotal]       ->setProperty( "text", "?" );
          myWidgets[i2DQuadrangles][iLinear]      ->setProperty( "text", "?" );
          myWidgets[i2DQuadrangles][iQuadratic]   ->setProperty( "text", "?" );
          myWidgets[i2DQuadrangles][iBiQuadratic] ->setProperty( "text", "?" );
          myWidgets[i2DPolygons][iLinear]         ->setProperty( "text", "?" );
          myWidgets[i2DPolygons][iQuadratic]      ->setProperty( "text", "?" );
          myWidgets[i2DPolygons][iTotal]          ->setProperty( "text", "?" );
          myWidgets[iNb][iTotal]                  ->setProperty( "text", "?" );
          myWidgets[iNb][iLinear]                 ->setProperty( "text", "?" );
          myWidgets[iNb][iQuadratic]              ->setProperty( "text", "?" );
          myWidgets[iNb][iBiQuadratic]            ->setProperty( "text", "?" );
        }
        else if ( nb3DLinear + nb3DQuadratic + nb3DBiQuadratic > 0 )
        {
          myWidgets[i3D][iLinear]                 ->setProperty( "text", "?" );
          myWidgets[i3D][iQuadratic]              ->setProperty( "text", "?" );
          myWidgets[i3D][iBiQuadratic]            ->setProperty( "text", "?" );
          myWidgets[i3DTetrahedrons][iTotal]      ->setProperty( "text", "?" );
          myWidgets[i3DTetrahedrons][iLinear]     ->setProperty( "text", "?" );
          myWidgets[i3DTetrahedrons][iQuadratic]  ->setProperty( "text", "?" );
          myWidgets[i3DHexahedrons][iTotal]       ->setProperty( "text", "?" );
          myWidgets[i3DHexahedrons][iLinear]      ->setProperty( "text", "?" );
          myWidgets[i3DHexahedrons][iQuadratic]   ->setProperty( "text", "?" );
          myWidgets[i3DHexahedrons][iBiQuadratic] ->setProperty( "text", "?" );
          myWidgets[i3DPyramids][iTotal]          ->setProperty( "text", "?" );
          myWidgets[i3DPyramids][iLinear]         ->setProperty( "text", "?" );
          myWidgets[i3DPyramids][iQuadratic]      ->setProperty( "text", "?" );
          myWidgets[i3DPrisms][iTotal]            ->setProperty( "text", "?" );
          myWidgets[i3DPrisms][iLinear]           ->setProperty( "text", "?" );
          myWidgets[i3DPrisms][iQuadratic]        ->setProperty( "text", "?" );
          myWidgets[i3DHexaPrisms][iTotal]        ->setProperty( "text", "?" );
          myWidgets[i3DPolyhedrons][iTotal]       ->setProperty( "text", "?" );
          myWidgets[iNb][iTotal]                  ->setProperty( "text", "?" );
          myWidgets[iNb][iLinear]                 ->setProperty( "text", "?" );
          myWidgets[iNb][iQuadratic]              ->setProperty( "text", "?" );
          myWidgets[iNb][iBiQuadratic]            ->setProperty( "text", "?" );
        }
      }
      else
      {
        myWidgets[iNodes][iTotal]               ->setProperty( "text", "?" );
        myWidgets[i0D][iTotal]                  ->setProperty( "text", "?" );
        myWidgets[iBalls][iTotal]               ->setProperty( "text", "?" );
        myWidgets[i1D][iTotal]                  ->setProperty( "text", "?" );
        myWidgets[i1D][iLinear]                 ->setProperty( "text", "?" );
        myWidgets[i1D][iQuadratic]              ->setProperty( "text", "?" );
        myWidgets[i2D][iTotal]                  ->setProperty( "text", "?" );
        myWidgets[i2D][iLinear]                 ->setProperty( "text", "?" );
        myWidgets[i2D][iQuadratic]              ->setProperty( "text", "?" );
        myWidgets[i2D][iBiQuadratic]            ->setProperty( "text", "?" );
        myWidgets[i2DTriangles][iTotal]         ->setProperty( "text", "?" );
        myWidgets[i2DTriangles][iLinear]        ->setProperty( "text", "?" );
        myWidgets[i2DTriangles][iQuadratic]     ->setProperty( "text", "?" );
        myWidgets[i2DTriangles][iBiQuadratic]   ->setProperty( "text", "?" );
        myWidgets[i2DQuadrangles][iTotal]       ->setProperty( "text", "?" );
        myWidgets[i2DQuadrangles][iLinear]      ->setProperty( "text", "?" );
        myWidgets[i2DQuadrangles][iQuadratic]   ->setProperty( "text", "?" );
        myWidgets[i2DQuadrangles][iBiQuadratic] ->setProperty( "text", "?" );
        myWidgets[i2DPolygons][iTotal]          ->setProperty( "text", "?" );
        myWidgets[i3D][iTotal]                  ->setProperty( "text", "?" );
        myWidgets[i3D][iLinear]                 ->setProperty( "text", "?" );
        myWidgets[i3D][iQuadratic]              ->setProperty( "text", "?" );
        myWidgets[i3DTetrahedrons][iTotal]      ->setProperty( "text", "?" );
        myWidgets[i3DTetrahedrons][iLinear]     ->setProperty( "text", "?" );
        myWidgets[i3DTetrahedrons][iQuadratic]  ->setProperty( "text", "?" );
        myWidgets[i3DHexahedrons][iTotal]       ->setProperty( "text", "?" );
        myWidgets[i3DHexahedrons][iLinear]      ->setProperty( "text", "?" );
        myWidgets[i3DHexahedrons][iQuadratic]   ->setProperty( "text", "?" );
        myWidgets[i3DHexahedrons][iBiQuadratic] ->setProperty( "text", "?" );
        myWidgets[i3DPyramids][iTotal]          ->setProperty( "text", "?" );
        myWidgets[i3DPyramids][iLinear]         ->setProperty( "text", "?" );
        myWidgets[i3DPyramids][iQuadratic]      ->setProperty( "text", "?" );
        myWidgets[i3DPrisms][iTotal]            ->setProperty( "text", "?" );
        myWidgets[i3DPrisms][iLinear]           ->setProperty( "text", "?" );
        myWidgets[i3DPrisms][iQuadratic]        ->setProperty( "text", "?" );
        myWidgets[i3DHexaPrisms][iTotal]        ->setProperty( "text", "?" );
        myWidgets[i3DPolyhedrons][iTotal]       ->setProperty( "text", "?" );
        myWidgets[iNb][iTotal]                  ->setProperty( "text", "?" );
        myWidgets[iNb][iLinear]                 ->setProperty( "text", "?" );
        myWidgets[iNb][iQuadratic]              ->setProperty( "text", "?" );
        myWidgets[iNb][iBiQuadratic]            ->setProperty( "text", "?" );
      }
    }
  }
}

/*!
  \brief Load mesh from a study file
*/
void SMESHGUI_MeshInfo::loadMesh()
{
  SUIT_OverrideCursor wc;

  SALOME_ListIO selected;
  SMESHGUI::selectionMgr()->selectedObjects( selected );

  if ( selected.Extent() == 1 ) {
    Handle(SALOME_InteractiveObject) IO = selected.First();
    SMESH::SMESH_IDSource_var obj = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( IO );
    if ( !CORBA::is_nil( obj ) ) {
      SMESH::SMESH_Mesh_var mesh = obj->GetMesh();
      if ( !mesh->_is_nil() )
      {
        mesh->Load();
        showInfo( obj );
      }
    }
  }
}

/*!
  \brief Reset the widget to the initial state (nullify all fields).
*/
void SMESHGUI_MeshInfo::clear()
{
  myWidgets[iName][iSingle]               ->setProperty( "text", QString() );
  myWidgets[iObject][iSingle]             ->setProperty( "text", QString() );
  myWidgets[iNodes][iTotal]               ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i0D][iTotal]                  ->setProperty( "text", QString::number( 0 ) );
  myWidgets[iBalls][iTotal]               ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i1D][iTotal]                  ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i1D][iLinear]                 ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i1D][iQuadratic]              ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2D][iTotal]                  ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2D][iLinear]                 ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2D][iQuadratic]              ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2D][iBiQuadratic]            ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DTriangles][iTotal]         ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DTriangles][iLinear]        ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DTriangles][iQuadratic]     ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DTriangles][iBiQuadratic]   ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DQuadrangles][iTotal]       ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DQuadrangles][iLinear]      ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DQuadrangles][iQuadratic]   ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DQuadrangles][iBiQuadratic] ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DPolygons][iLinear]         ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DPolygons][iQuadratic]      ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DPolygons][iTotal]          ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3D][iTotal]                  ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3D][iLinear]                 ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3D][iQuadratic]              ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3D][iBiQuadratic]            ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DTetrahedrons][iTotal]      ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DTetrahedrons][iLinear]     ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DTetrahedrons][iQuadratic]  ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DHexahedrons][iTotal]       ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DHexahedrons][iLinear]      ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DHexahedrons][iQuadratic]   ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DHexahedrons][iBiQuadratic] ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DPyramids][iTotal]          ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DPyramids][iLinear]         ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DPyramids][iQuadratic]      ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DPrisms][iTotal]            ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DPrisms][iLinear]           ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DPrisms][iQuadratic]        ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DHexaPrisms][iTotal]        ->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DPolyhedrons][iTotal]       ->setProperty( "text", QString::number( 0 ) );
  myWidgets[iNb][iTotal]                  ->setProperty( "text", QString::number( 0 ) );
  myWidgets[iNb][iLinear]                 ->setProperty( "text", QString::number( 0 ) );
  myWidgets[iNb][iQuadratic]              ->setProperty( "text", QString::number( 0 ) );
  myWidgets[iNb][iBiQuadratic]            ->setProperty( "text", QString::number( 0 ) );
}

/*!
  \brief Create info field
  \return new info field
*/
QLabel* SMESHGUI_MeshInfo::createField()
{
  QLabel* lab = new QLabel( this );
  lab->setFrameStyle( StyledPanel | Sunken );
  lab->setAlignment( Qt::AlignCenter );
  lab->setAutoFillBackground( true );
  QPalette pal = lab->palette();
  pal.setColor( QPalette::Window, QApplication::palette().color( QPalette::Active, QPalette::Base ) );
  lab->setPalette( pal );
  lab->setMinimumWidth( 70 );
  return lab;
}

/*!
  \brief Create horizontal rule.
  \return new line object
*/
QWidget* SMESHGUI_MeshInfo::createLine()
{
  QFrame* line = new QFrame( this );
  line->setFrameStyle( HLine | Sunken );
  return line;
}

/*!
  \brief Change widget font attributes (bold, italic, ...).
  \param w widget
  \param attr font attributes (XORed flags)
  \param val value to be set to attributes
*/
void SMESHGUI_MeshInfo::setFontAttributes( QWidget* w, int attr, bool val )
{
  if ( w && attr ) {
    QFont f = w->font();
    if ( attr & Bold   ) f.setBold( val );
    if ( attr & Italic ) f.setItalic( val );
    w->setFont( f );
  }
}

/*!
  \brief Show/hide group(s) of fields.
  \param start beginning of the block
  \param end end of the block
  \param on visibility flag
*/
void SMESHGUI_MeshInfo::setFieldsVisible( int start, int end, bool on )
{
  start = qMax( 0, start );
  end   = qMin( end, (int)iElementsEnd );
  for ( int i = start; i < end; i++ ) {
    wlist wl = myWidgets[i];
    foreach ( QWidget* w, wl ) w->setVisible( on );
  }
}

void SMESHGUI_MeshInfo::saveInfo( QTextStream &out )
{
  out << QString( 9, '-' ) << "\n";
  out << tr( "BASE_INFO" ) << "\n";
  out << QString( 9, '-' ) << "\n";
  out <<                                   tr( "NAME_LAB" )         << "  " << ( myWidgets[iName][iSingle]->property( "text" ) ).toString() << "\n";
  out <<                                   tr( "OBJECT_LAB" )       << "  " << ( myWidgets[iObject][iSingle]->property( "text" ) ).toString() << "\n";
  out <<                                   tr( "NODES_LAB" )        << "  " << ( myWidgets[iNodes][iTotal]->property( "text" ) ).toString() << "\n";
  out <<                                   tr( "ELEMENTS_LAB" )     << "\n";
  out << QString( SPACING_INFO,   ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[iNb][iTotal]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO,   ' ' ) << tr( "LINEAR_LAB" )       << ": " << ( myWidgets[iNb][iLinear]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO,   ' ' ) << tr( "QUADRATIC_LAB" )    << ": " << ( myWidgets[iNb][iQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO,   ' ' ) << tr( "BI_QUADRATIC_LAB" ) << ": " << ( myWidgets[iNb][iBiQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO,   ' ' ) << tr( "0D_LAB" )           << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[i0D][iTotal]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO,   ' ' ) << tr( "BALL_LAB" )         << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[iBalls][iTotal]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO,   ' ' ) << tr( "1D_LAB" )           << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[i1D][iTotal]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "LINEAR_LAB" )       << ": " << ( myWidgets[i1D][iLinear]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "QUADRATIC_LAB" )    << ": " << ( myWidgets[i1D][iQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO,   ' ' ) << tr( "2D_LAB" )           << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[i2D][iTotal]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "LINEAR_LAB" )       << ": " << ( myWidgets[i2D][iLinear]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "QUADRATIC_LAB" )    << ": " << ( myWidgets[i2D][iQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "BI_QUADRATIC_LAB" ) << ": " << ( myWidgets[i2D][iBiQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "TRIANGLES_LAB" )    << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[i2DTriangles][iTotal]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "LINEAR_LAB" )       << ": " << ( myWidgets[i2DTriangles][iLinear]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "QUADRATIC_LAB" )    << ": " << ( myWidgets[i2DTriangles][iQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "BI_QUADRATIC_LAB" ) << ": " << ( myWidgets[i2DTriangles][iBiQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "QUADRANGLES_LAB" )  << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[i2DQuadrangles][iTotal]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "LINEAR_LAB" )       << ": " << ( myWidgets[i2DQuadrangles][iLinear]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "QUADRATIC_LAB" )    << ": " << ( myWidgets[i2DQuadrangles][iQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "BI_QUADRATIC_LAB" ) << ": " << ( myWidgets[i2DQuadrangles][iBiQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "POLYGONS_LAB" )     << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[i2DPolygons][iTotal]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "LINEAR_LAB" )       << ": " << ( myWidgets[i2DPolygons][iLinear]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "QUADRATIC_LAB" )    << ": " << ( myWidgets[i2DPolygons][iQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO,   ' ' ) << tr( "3D_LAB" )           << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[i3D][iTotal]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "LINEAR_LAB" )       << ": " << ( myWidgets[i3D][iLinear]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "QUADRATIC_LAB" )    << ": " << ( myWidgets[i3D][iQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "BI_QUADRATIC_LAB" ) << ": " << ( myWidgets[i3D][iBiQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "TETRAHEDRONS_LAB" ) << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[i3DTetrahedrons][iTotal]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "LINEAR_LAB" )       << ": " << ( myWidgets[i3DTetrahedrons][iLinear]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "QUADRATIC_LAB" )    << ": " << ( myWidgets[i3DTetrahedrons][iQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "HEXAHEDONRS_LAB" )  << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[i3DHexahedrons][iTotal]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "LINEAR_LAB" )       << ": " << ( myWidgets[i3DHexahedrons][iLinear]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "QUADRATIC_LAB" )    << ": " << ( myWidgets[i3DHexahedrons][iQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "BI_QUADRATIC_LAB" ) << ": " << ( myWidgets[i3DHexahedrons][iBiQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "PYRAMIDS_LAB" )     << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[i3DPyramids][iTotal]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "LINEAR_LAB" )       << ": " << ( myWidgets[i3DPyramids][iLinear]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "QUADRATIC_LAB" )    << ": " << ( myWidgets[i3DPyramids][iQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "PRISMS_LAB" )       << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[i3DPrisms][iTotal]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "LINEAR_LAB" )       << ": " << ( myWidgets[i3DPrisms][iLinear]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "QUADRATIC_LAB" )    << ": " << ( myWidgets[i3DPrisms][iQuadratic]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "HEX_PRISMS_LAB" )   << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[i3DHexaPrisms][iTotal]->property( "text" ) ).toString() << "\n";
  out << QString( SPACING_INFO*2, ' ' ) << tr( "POLYHEDRONS_LAB" )  << "\n";
  out << QString( SPACING_INFO*3, ' ' ) << tr( "TOTAL_LAB" )        << ": " << ( myWidgets[i3DPolyhedrons][iTotal]->property( "text" ) ).toString() << "\n" << "\n";
}

/*!
  \class SMESHGUI_ElemInfo
  \brief Base class for the mesh element information widget.
*/

/*!
  \brief Constructor
  \param parent parent widget
*/
SMESHGUI_ElemInfo::SMESHGUI_ElemInfo( QWidget* parent )
: QWidget( parent ), myActor( 0 ), myIsElement( -1 )
{
  myFrame = new QWidget( this );
  myExtra = new ExtraWidget( this );
  QVBoxLayout* vbl = new QVBoxLayout( this );
  vbl->setMargin( 0 );
  vbl->setSpacing( 0 );
  vbl->addWidget( myFrame );
  vbl->addWidget( myExtra );
  connect( myExtra->prev, SIGNAL( clicked() ), this, SLOT( showPrevious() ) );
  connect( myExtra->next, SIGNAL( clicked() ), this, SLOT( showNext() ) );
  clear();
}

/*!
  \brief Destructor
*/
SMESHGUI_ElemInfo::~SMESHGUI_ElemInfo()
{
}

/*!
  \brief Set mesh data source (actor)
  \param actor mesh object actor
*/
void SMESHGUI_ElemInfo::setSource( SMESH_Actor* actor )
{
  if ( myActor != actor ) {
    myActor = actor;
    myIsElement = -1;
    clear();
  }
}

/*!
  \brief Show mesh element information
  \param id mesh node / element ID
  \param isElem show mesh element information if \c true or mesh node information if \c false
*/
void SMESHGUI_ElemInfo::showInfo( long id, bool isElem )
{
  QSet<long> ids;
  ids << id;
  showInfo( ids, isElem );
}

/*!
  \brief Show mesh element information
  \param ids mesh nodes / elements identifiers
  \param isElem show mesh element information if \c true or mesh node information if \c false
*/
void SMESHGUI_ElemInfo::showInfo( QSet<long> ids, bool isElem )
{
  QList<long> newIds = ids.toList();
  qSort( newIds );
  if ( myIDs == newIds && myIsElement == isElem ) return;

  myIDs = newIds;
  myIsElement = isElem;
  myIndex = 0;
  updateControls();
  information( myIDs.mid( myIndex*MAXITEMS, MAXITEMS ) );
}

/*!
  \brief Clear mesh element information widget
*/
void SMESHGUI_ElemInfo::clear()
{
  myIDs.clear();
  myIndex = 0;
  clearInternal();
  updateControls();
}

/*!
  \brief Get central area widget
  \return central widget
*/
QWidget* SMESHGUI_ElemInfo::frame() const
{
  return myFrame;
}

/*!
  \brief Get actor
  \return actor being used
*/
SMESH_Actor* SMESHGUI_ElemInfo::actor() const
{
  return myActor;
}

/*!
  \brief Get current info mode.
  \return \c true if mesh element information is shown or \c false if node information is shown
*/
bool SMESHGUI_ElemInfo::isElements() const
{
  return myIsElement;
}

/*!
  \fn void SMESHGUI_ElemInfo::information( const QList<long>& ids )
  \brief Show information on the specified nodes / elements

  This function is to be redefined in sub-classes.

  \param ids nodes / elements identifiers information is to be shown on
*/

/*!
  \brief Internal clean-up (reset widget)
*/
void SMESHGUI_ElemInfo::clearInternal()
{
}

/*!
  \brief Get node connectivity
  \param node mesh node
  \return node connectivity map
*/
SMESHGUI_ElemInfo::Connectivity SMESHGUI_ElemInfo::nodeConnectivity( const SMDS_MeshNode* node )
{
  Connectivity elmap;
  if ( node ) {
    SMDS_ElemIteratorPtr it = node->GetInverseElementIterator();
    while ( it && it->more() ) {
      const SMDS_MeshElement* ne = it->next();
      elmap[ ne->GetType() ] << ne->GetID();
    }
  }
  return elmap;
}

/*!
  \brief Format connectivity data to string representation
  \param connectivity connetivity map
  \param type element type
  \return string representation of the connectivity
*/
QString SMESHGUI_ElemInfo::formatConnectivity( Connectivity connectivity, int type )
{
  QStringList str;
  if ( connectivity.contains( type ) ) {
    QList<int> elements = connectivity[ type ];
    qSort( elements );
    foreach( int id, elements )
      str << QString::number( id );
  }
  return str.join( " " );
}

/*!
  \brief Calculate gravity center of the mesh element
  \param element mesh element
*/
SMESHGUI_ElemInfo::XYZ SMESHGUI_ElemInfo::gravityCenter( const SMDS_MeshElement* element )
{
  XYZ xyz;
  if ( element ) {
    SMDS_ElemIteratorPtr nodeIt = element->nodesIterator();
    while ( nodeIt->more() ) {
      const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( nodeIt->next() );
      xyz.add( node->X(), node->Y(), node->Z() );
    }
    xyz.divide( element->NbNodes() );
  }
  return xyz;
}

/*!
  \brief Calculate normal vector to the mesh face
  \param element mesh face
*/
SMESHGUI_ElemInfo::XYZ SMESHGUI_ElemInfo::normal( const SMDS_MeshElement* element )
{
  gp_XYZ n = SMESH::getNormale( dynamic_cast<const SMDS_MeshFace*>( element ) );
  return XYZ(n.X(), n.Y(), n.Z());
}

/*!
  \brief This slot is called from "Show Previous" button click.
  Shows information on the previous group of the items.
*/
void SMESHGUI_ElemInfo::showPrevious()
{
  myIndex = qMax( 0, myIndex-1 );
  updateControls();
  information( myIDs.mid( myIndex*MAXITEMS, MAXITEMS ) );
}

/*!
  \brief This slot is called from "Show Next" button click.
  Shows information on the next group of the items.
*/
void SMESHGUI_ElemInfo::showNext()
{
  myIndex = qMin( myIndex+1, myIDs.count() / MAXITEMS );
  updateControls();
  information( myIDs.mid( myIndex*MAXITEMS, MAXITEMS ) );
}

/*!
  \brief Update widgets state
*/
void SMESHGUI_ElemInfo::updateControls()
{
  myExtra->updateControls( myIDs.count(), myIndex );
}

/*!
  \class SMESHGUI_SimpleElemInfo
  \brief Represents mesh element information in the simple text area.
*/

/*!
  \brief Constructor
  \param parent parent widget
*/
SMESHGUI_SimpleElemInfo::SMESHGUI_SimpleElemInfo( QWidget* parent )
: SMESHGUI_ElemInfo( parent )
{
  myInfo = new QTextBrowser( frame() );
  QVBoxLayout* l = new QVBoxLayout( frame() );
  l->setMargin( 0 );
  l->addWidget( myInfo );
}

/*!
  \brief Show mesh element information
  \param ids mesh nodes / elements identifiers
*/
void SMESHGUI_SimpleElemInfo::information( const QList<long>& ids )
{
  clearInternal();
  
  if ( actor() ) {
    int grp_details = SMESHGUI::resourceMgr()->booleanValue( "SMESH", "elem_info_grp_details", false );
    int precision   = SMESHGUI::resourceMgr()->integerValue( "SMESH", "length_precision", 6 );
    int cprecision = -1;
    if ( SMESHGUI::resourceMgr()->booleanValue( "SMESH", "use_precision", false ) ) 
      cprecision = SMESHGUI::resourceMgr()->integerValue( "SMESH", "controls_precision", -1 );
    foreach ( long id, ids ) {
      if ( !isElements() ) {
        //
        // show node info
        //
        const SMDS_MeshNode* node = actor()->GetObject()->GetMesh()->FindNode( id );
        if ( !node ) return;

        // node ID
        myInfo->append( QString( "<b>%1 #%2</b>" ).arg( SMESHGUI_ElemInfo::tr( "NODE" ) ).arg( id ) );
        // separator
        myInfo->append( "" );
        // coordinates
        myInfo->append( QString( "<b>%1:</b> (%2, %3, %4)" ).arg( SMESHGUI_ElemInfo::tr( "COORDINATES" ) ).
                        arg( node->X(), 0, precision > 0 ? 'f' : 'g', qAbs( precision ) ).
                        arg( node->Y(), 0, precision > 0 ? 'f' : 'g', qAbs( precision ) ).
                        arg( node->Z(), 0, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
        // separator
        myInfo->append( "" );
        // connectivity
        Connectivity connectivity = nodeConnectivity( node );
        if ( !connectivity.isEmpty() ) {
          myInfo->append( QString( "<b>%1:</b>" ).arg( SMESHGUI_ElemInfo::tr( "CONNECTIVITY" ) ) );
          QString con = formatConnectivity( connectivity, SMDSAbs_0DElement );
          if ( !con.isEmpty() )
            myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "0D_ELEMENTS" ) ).arg( con ) );
          con = formatConnectivity( connectivity, SMDSAbs_Edge );
          if ( !con.isEmpty() )
            myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "EDGES" ) ).arg( con ) );
          con = formatConnectivity( connectivity, SMDSAbs_Ball );
          if ( !con.isEmpty() )
            myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "BALL_ELEMENTS" ) ).arg( con ) );
          con = formatConnectivity( connectivity, SMDSAbs_Face );
          if ( !con.isEmpty() )
            myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "FACES" ) ).arg( con ) );
          con = formatConnectivity( connectivity, SMDSAbs_Volume );
          if ( !con.isEmpty() )
            myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "VOLUMES" ) ).arg( con ) );
        }
        else {
          myInfo->append( QString( "<b>%1</b>" ).arg( SMESHGUI_ElemInfo::tr( "FREE_NODE" ) ).arg( id ) );
        }
        // node position
        SMESH::SMESH_Mesh_ptr aMeshPtr = actor()->GetObject()->GetMeshServer();   
        if ( !CORBA::is_nil( aMeshPtr ) ) {
          SMESH::NodePosition_var pos = aMeshPtr->GetNodePosition( id );
          int shapeID = pos->shapeID;
          if ( shapeID > 0 ) {
            QString shapeType;
            double u = 0, v = 0;
            switch ( pos->shapeType ) {
            case GEOM::EDGE:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_EDGE" );
              if ( pos->params.length() == 1 )
                u = pos->params[0];
              break;
            case GEOM::FACE:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_FACE" );
              if ( pos->params.length() == 2 ) {
               u = pos->params[0];
               v = pos->params[1];
              }
              break;
            case GEOM::VERTEX:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_VERTEX" );
              break;
            default:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SOLID" );
              break;
            }
            // separator
            myInfo->append( "" );
            myInfo->append( QString( "<b>%1:" ).arg( SMESHGUI_ElemInfo::tr( "POSITION" ) ) );
            myInfo->append( QString( "- <b>%1: #%2</b>" ).arg( shapeType ).arg( shapeID ) );
            if ( pos->shapeType == GEOM::EDGE || pos->shapeType == GEOM::FACE ) {
              myInfo->append( QString( "- <b>%1: #%2</b>" ).arg( SMESHGUI_ElemInfo::tr( "U_POSITION" ) ).
                              arg( QString::number( u, precision > 0 ? 'f' : 'g', qAbs( precision )) ) );
              if ( pos->shapeType == GEOM::FACE ) {
                myInfo->append( QString( "- <b>%1: #%2</b>" ).arg( SMESHGUI_ElemInfo::tr( "V_POSITION" ) ).
                                arg( QString::number( v, precision > 0 ? 'f' : 'g', qAbs( precision )) ) );
              }
            }
          }
        }
        // groups node belongs to
        SMESH::SMESH_Mesh_ptr aMesh = actor()->GetObject()->GetMeshServer();
        if ( !CORBA::is_nil( aMesh ) ) {
          SMESH::ListOfGroups_var groups = aMesh->GetGroups();
          myInfo->append( "" ); // separator
          bool top_created = false;
          for ( CORBA::ULong i = 0; i < groups->length(); i++ ) {
            SMESH::SMESH_GroupBase_var aGrp = groups[i];
            if ( CORBA::is_nil( aGrp ) ) continue;
            QString aName = aGrp->GetName();
            if ( aGrp->GetType() == SMESH::NODE && !aName.isEmpty() && aGrp->Contains( id ) ) {
              if ( !top_created ) {
                myInfo->append( QString( "<b>%1:</b>" ).arg( SMESHGUI_AddInfo::tr( "GROUPS" ) ) );
                top_created = true;
              }
              myInfo->append( QString( "+ <b>%1:</b>" ).arg( aName.trimmed() ) );
              if ( grp_details ) {
                SMESH::SMESH_Group_var         aStdGroup  = SMESH::SMESH_Group::_narrow( aGrp );
                SMESH::SMESH_GroupOnGeom_var   aGeomGroup = SMESH::SMESH_GroupOnGeom::_narrow( aGrp );
                SMESH::SMESH_GroupOnFilter_var aFltGroup  = SMESH::SMESH_GroupOnFilter::_narrow( aGrp );
                
                // type : group on geometry, standalone group, group on filter
                if ( !CORBA::is_nil( aStdGroup ) ) {
                  myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "TYPE" ) ).
                                  arg( SMESHGUI_AddInfo::tr( "STANDALONE_GROUP" ) ) );
                }
                else if ( !CORBA::is_nil( aGeomGroup ) ) {
                  myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "TYPE" ) ).
                                  arg( SMESHGUI_AddInfo::tr( "GROUP_ON_GEOMETRY" ) ) );
                  GEOM::GEOM_Object_var gobj = aGeomGroup->GetShape();
                  _PTR(SObject) sobj = SMESH::ObjectToSObject( gobj );
                  if ( sobj ) {
                    myInfo->append( QString( "  - <b>%1:</b> %2: %3" ).arg( SMESHGUI_AddInfo::tr( "TYPE" ) ).
                                    arg( SMESHGUI_AddInfo::tr( "GEOM_OBJECT" ) ).arg( sobj->GetName().c_str() ) );
                  }
                }
                else if ( !CORBA::is_nil( aFltGroup ) ) {
                  myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "TYPE" ) ).
                                  arg( SMESHGUI_AddInfo::tr( "GROUP_ON_FILTER" ) ) );
                }
                
                // size
                myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "SIZE" ) ).
                                arg( QString::number( aGrp->Size() ) ) );
                
                // color
                SALOMEDS::Color color = aGrp->GetColor();
                myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "COLOR" ) ).
                                arg( QColor( color.R*255., color.G*255., color.B*255. ).name() ) );
              }
            }
          }
        }
      }
      else {
        //
        // show element info
        // 
        const SMDS_MeshElement* e = actor()->GetObject()->GetMesh()->FindElement( id );
        SMESH::Controls::NumericalFunctorPtr afunctor;
        if ( !e ) return;
        
        // Element ID && Type
        QString stype;
        switch( e->GetType() ) {
        case SMDSAbs_0DElement:
          stype = SMESHGUI_ElemInfo::tr( "0D_ELEMENT" ); break;
        case SMDSAbs_Ball:
          stype = SMESHGUI_ElemInfo::tr( "BALL" ); break;
        case SMDSAbs_Edge:
          stype = SMESHGUI_ElemInfo::tr( "EDGE" ); break;
        case SMDSAbs_Face:
          stype = SMESHGUI_ElemInfo::tr( "FACE" ); break;
        case SMDSAbs_Volume:
          stype = SMESHGUI_ElemInfo::tr( "VOLUME" ); break;
        default: 
          break;
        }
        if ( stype.isEmpty() ) return;
        myInfo->append( QString( "<b>%1 #%2</b>" ).arg( stype ).arg( id ) );
        // separator
        myInfo->append( "" );

        // Geometry type
        QString gtype;
        switch( e->GetEntityType() ) {
        case SMDSEntity_Triangle:
        case SMDSEntity_Quad_Triangle:
        case SMDSEntity_BiQuad_Triangle:
          gtype = SMESHGUI_ElemInfo::tr( "TRIANGLE" ); break;
        case SMDSEntity_Quadrangle:
        case SMDSEntity_Quad_Quadrangle:
        case SMDSEntity_BiQuad_Quadrangle:
          gtype = SMESHGUI_ElemInfo::tr( "QUADRANGLE" ); break;
        case SMDSEntity_Polygon:
        case SMDSEntity_Quad_Polygon:
          gtype = SMESHGUI_ElemInfo::tr( "POLYGON" ); break;
        case SMDSEntity_Tetra:
        case SMDSEntity_Quad_Tetra:
          gtype = SMESHGUI_ElemInfo::tr( "TETRAHEDRON" ); break;
        case SMDSEntity_Pyramid:
        case SMDSEntity_Quad_Pyramid:
          gtype = SMESHGUI_ElemInfo::tr( "PYRAMID" ); break;
        case SMDSEntity_Hexa:
        case SMDSEntity_Quad_Hexa:
        case SMDSEntity_TriQuad_Hexa:
          gtype = SMESHGUI_ElemInfo::tr( "HEXAHEDRON" ); break;
        case SMDSEntity_Penta:
        case SMDSEntity_Quad_Penta:
          gtype = SMESHGUI_ElemInfo::tr( "PRISM" ); break;
        case SMDSEntity_Hexagonal_Prism:
          gtype = SMESHGUI_ElemInfo::tr( "HEX_PRISM" ); break;
        case SMDSEntity_Polyhedra:
        case SMDSEntity_Quad_Polyhedra:
          gtype = SMESHGUI_ElemInfo::tr( "POLYHEDRON" ); break;
        default: 
          break;
        }
        if ( !gtype.isEmpty() )
          myInfo->append( QString( "<b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "TYPE" ) ).arg( gtype ) );

        // Quadratic flag (any element except 0D)
        if ( e->GetEntityType() > SMDSEntity_0D && e->GetEntityType() < SMDSEntity_Ball ) {
          myInfo->append( QString( "<b>%1?</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "QUADRATIC" ) ).arg( e->IsQuadratic() ? SMESHGUI_ElemInfo::tr( "YES" ) : SMESHGUI_ElemInfo::tr( "NO" ) ) );
        }
        if ( const SMDS_BallElement* ball = dynamic_cast<const SMDS_BallElement*>( e )) {
          // Ball diameter
          myInfo->append( QString( "<b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "BALL_DIAMETER" ) ).arg( ball->GetDiameter() ));
        }
        // separator
        myInfo->append( "" );

        // Connectivity
        SMDS_ElemIteratorPtr nodeIt = e->nodesIterator();
        for ( int idx = 1; nodeIt->more(); idx++ ) {
          const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( nodeIt->next() );
          // node number and ID
          myInfo->append( QString( "<b>%1 %2/%3</b> - #%4" ).arg( SMESHGUI_ElemInfo::tr( "NODE" ) ).arg( idx ).arg( e->NbNodes() ).arg( node->GetID() ) );
          // node coordinates
          myInfo->append( QString( "<b>%1:</b> (%2, %3, %4)" ).arg( SMESHGUI_ElemInfo::tr( "COORDINATES" ) ).
                          arg( node->X(), 0, precision > 0 ? 'f' : 'g', qAbs( precision ) ).
                          arg( node->Y(), 0, precision > 0 ? 'f' : 'g', qAbs( precision ) ).
                          arg( node->Z(), 0, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
          // node connectivity
          Connectivity connectivity = nodeConnectivity( node );
          if ( !connectivity.isEmpty() ) {
            myInfo->append( QString( "<b>%1:</b>" ).arg( SMESHGUI_ElemInfo::tr( "CONNECTIVITY" ) ) );
            QString con = formatConnectivity( connectivity, SMDSAbs_0DElement );
            if ( !con.isEmpty() )
              myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "0D_ELEMENTS" ) ).arg( con ) );
            con = formatConnectivity( connectivity, SMDSAbs_Edge );
            if ( !con.isEmpty() )
              myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "EDGES" ) ).arg( con ) );
            con = formatConnectivity( connectivity, SMDSAbs_Face );
            if ( !con.isEmpty() )
              myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "FACES" ) ).arg( con ) );
            con = formatConnectivity( connectivity, SMDSAbs_Volume );
            if ( !con.isEmpty() )
              myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "VOLUMES" ) ).arg( con ) );
          }
          else {
            myInfo->append( QString( "<b>%1</b>" ).arg( SMESHGUI_ElemInfo::tr( "FREE_NODE" ) ).arg( id ) );
          }
        }
        // separator
        myInfo->append( "" );

        // Controls
        myInfo->append( QString( "<b>%1:</b>" ).arg( SMESHGUI_ElemInfo::tr( "CONTROLS" ) ) );
        //Length
        if ( e->GetType() == SMDSAbs_Edge ) {
          afunctor.reset( new SMESH::Controls::Length() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "LENGTH_EDGES" ) ).arg( afunctor->GetValue( id ) ) );  
        }
        if( e->GetType() == SMDSAbs_Face ) {
          //Area
          afunctor.reset(  new SMESH::Controls::Area() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );  
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "AREA_ELEMENTS" ) ).arg( afunctor->GetValue( id ) ) );
          //Taper
          afunctor.reset( new SMESH::Controls::Taper() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );  
          afunctor->SetPrecision( cprecision );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "TAPER_ELEMENTS" ) ).arg( afunctor->GetValue( id ) ) );
          //AspectRatio2D
          afunctor.reset( new SMESH::Controls::AspectRatio() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "ASPECTRATIO_ELEMENTS" ) ).arg( afunctor->GetValue( id ) ) );
          //Minimum angle         
          afunctor.reset( new SMESH::Controls::MinimumAngle() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "MINIMUMANGLE_ELEMENTS" ) ).arg( afunctor->GetValue( id ) ) );
          //Wraping angle        
          afunctor.reset( new SMESH::Controls::Warping() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "WARP_ELEMENTS" ) ).arg( afunctor->GetValue( id ) ) );
          //Skew         
          afunctor.reset( new SMESH::Controls::Skew() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "SKEW_ELEMENTS" ) ).arg( afunctor->GetValue( id ) ) );
          //ElemDiam2D   
          afunctor.reset( new SMESH::Controls::MaxElementLength2D() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "MAX_ELEMENT_LENGTH_2D" ) ).arg( afunctor->GetValue( id ) ) );
        }
        if( e->GetType() == SMDSAbs_Volume ) {
          //AspectRatio3D
          afunctor.reset(  new SMESH::Controls::AspectRatio3D() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "ASPECTRATIO_3D_ELEMENTS" ) ).arg( afunctor->GetValue( id ) ) );
          //Volume      
          afunctor.reset(  new SMESH::Controls::Volume() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "VOLUME_3D_ELEMENTS" ) ).arg( afunctor->GetValue( id ) ) );
          //ElementDiameter3D    
          afunctor.reset(  new SMESH::Controls::Volume() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "MAX_ELEMENT_LENGTH_3D" ) ).arg( afunctor->GetValue( id ) ) );
        }
        // separator
        myInfo->append( "" );

        // Gravity center
        XYZ gc = gravityCenter( e );
        myInfo->append( QString( "<b>%1:</b> (%2, %3, %4)" ).arg( SMESHGUI_ElemInfo::tr( "GRAVITY_CENTER" ) ).arg( gc.x() ).arg( gc.y() ).arg( gc.z() ) );
        
        // Normal vector
        if( e->GetType() == SMDSAbs_Face ) {
          XYZ gc = normal( e );
          myInfo->append( QString( "<b>%1:</b> (%2, %3, %4)" ).arg( SMESHGUI_ElemInfo::tr( "NORMAL_VECTOR" ) ).arg( gc.x() ).arg( gc.y() ).arg( gc.z() ) );
        }

        // Element position
        if ( e->GetType() >= SMDSAbs_Edge && e->GetType() <= SMDSAbs_Volume ) {
          SMESH::SMESH_Mesh_ptr aMesh = actor()->GetObject()->GetMeshServer();    
          if ( !CORBA::is_nil( aMesh ) ) {
            SMESH::ElementPosition pos = aMesh->GetElementPosition( id );
            int shapeID = pos.shapeID;
            if ( shapeID > 0 ) {
              myInfo->append( "" ); // separator
              QString shapeType;
              switch ( pos.shapeType ) {
              case GEOM::EDGE:   shapeType = SMESHGUI_ElemInfo::tr( "GEOM_EDGE" );   break;
              case GEOM::FACE:   shapeType = SMESHGUI_ElemInfo::tr( "GEOM_FACE" );   break;
              case GEOM::VERTEX: shapeType = SMESHGUI_ElemInfo::tr( "GEOM_VERTEX" ); break;
              case GEOM::SOLID:  shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SOLID" );  break;
              case GEOM::SHELL:  shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SHELL" );  break;
              default:           shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SHAPE" );  break;
              }
              myInfo->append( QString( "<b>%1:</b> %2 #%3" ).arg( SMESHGUI_ElemInfo::tr( "POSITION" ) ).arg( shapeType ).arg( shapeID ) );
            }
          }
        }

        // Groups the element belongs to
        SMESH::SMESH_Mesh_ptr aMesh = actor()->GetObject()->GetMeshServer();
        if ( !CORBA::is_nil( aMesh ) ) {
          SMESH::ListOfGroups_var  groups = aMesh->GetGroups();
          myInfo->append( "" ); // separator
          bool top_created = false;
          for ( CORBA::ULong i = 0; i < groups->length(); i++ ) {
            SMESH::SMESH_GroupBase_var aGrp = groups[i];
            if ( CORBA::is_nil( aGrp ) ) continue;
            QString aName = aGrp->GetName();
            if ( aGrp->GetType() != SMESH::NODE && !aName.isEmpty() && aGrp->Contains( id ) ) {
              if ( !top_created ) {
                myInfo->append( QString( "<b>%1:</b>" ).arg( SMESHGUI_AddInfo::tr( "GROUPS" ) ) );
                top_created = true;
              }
              myInfo->append( QString( "+ <b>%1:</b>" ).arg( aName.trimmed() ) );
              if ( grp_details ) {
                SMESH::SMESH_Group_var         aStdGroup  = SMESH::SMESH_Group::_narrow( aGrp );
                SMESH::SMESH_GroupOnGeom_var   aGeomGroup = SMESH::SMESH_GroupOnGeom::_narrow( aGrp );
                SMESH::SMESH_GroupOnFilter_var aFltGroup  = SMESH::SMESH_GroupOnFilter::_narrow( aGrp );
                
                // type : group on geometry, standalone group, group on filter
                if ( !CORBA::is_nil( aStdGroup ) ) {
                  myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "TYPE" ) ).
                                  arg( SMESHGUI_AddInfo::tr( "STANDALONE_GROUP" ) ) );
                }
                else if ( !CORBA::is_nil( aGeomGroup ) ) {
                  myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "TYPE" ) ).
                                  arg( SMESHGUI_AddInfo::tr( "GROUP_ON_GEOMETRY" ) ) );
                  GEOM::GEOM_Object_var gobj = aGeomGroup->GetShape();
                  _PTR(SObject) sobj = SMESH::ObjectToSObject( gobj );
                  if ( sobj ) {
                    myInfo->append( QString( "  - <b>%1:</b> %2: %3" ).arg( SMESHGUI_AddInfo::tr( "TYPE" ) ).
                                    arg( SMESHGUI_AddInfo::tr( "GEOM_OBJECT" ) ).arg( sobj->GetName().c_str() ) );
                  }
                }
                else if ( !CORBA::is_nil( aFltGroup ) ) {
                  myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "TYPE" ) ).
                                  arg( SMESHGUI_AddInfo::tr( "GROUP_ON_FILTER" ) ) );
                }
                
                myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "SIZE" ) ).
                                arg( QString::number( aGrp->Size() ) ) );
                
                // color
                SALOMEDS::Color color = aGrp->GetColor();
                myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "COLOR" ) ).
                                arg( QColor( color.R*255., color.G*255., color.B*255. ).name() ) );
              }
            }
          }
        }
      }
      // separator
      if ( ids.count() > 1 ) {
        myInfo->append( "" );
        myInfo->append( "------" );
        myInfo->append( "" );
      }
    }
  }
}

/*!
  \brief Internal clean-up (reset widget)
*/
void SMESHGUI_SimpleElemInfo::clearInternal()
{
  myInfo->clear();
}

void SMESHGUI_SimpleElemInfo::saveInfo( QTextStream &out )
{
  out << QString( 12, '-' ) << "\n";
  out << SMESHGUI_ElemInfo::tr( "ELEM_INFO" ) << "\n";
  out << QString( 12, '-' ) << "\n";
  out << myInfo->toPlainText();
  out << "\n";
}


/*!
  \class SMESHGUI_TreeElemInfo::ItemDelegate
  \brief Item delegate for tree mesh info widget
  \internal
*/
class SMESHGUI_TreeElemInfo::ItemDelegate : public QItemDelegate
{
public:
  ItemDelegate( QObject* );
  QWidget* createEditor( QWidget*, const QStyleOptionViewItem&, const QModelIndex& ) const;
};

/*!
  \brief Constructor
  \internal
*/
SMESHGUI_TreeElemInfo::ItemDelegate::ItemDelegate( QObject* parent ) : QItemDelegate( parent )
{
}

/*!
  \brief Create item editor widget
  \internal
*/
QWidget* SMESHGUI_TreeElemInfo::ItemDelegate::createEditor( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  QWidget* w = index.column() == 0 ? 0: QItemDelegate::createEditor( parent, option, index );
  if ( qobject_cast<QLineEdit*>( w ) ) qobject_cast<QLineEdit*>( w )->setReadOnly(  true );
  return w;
}

/*!
  \class SMESHGUI_TreeElemInfo
  \brief Represents mesh element information in the tree-like form.
*/

/*!
  \brief Constructor
  \param parent parent widget
*/
SMESHGUI_TreeElemInfo::SMESHGUI_TreeElemInfo( QWidget* parent )
: SMESHGUI_ElemInfo( parent )
{
  myInfo = new QTreeWidget( frame() );
  myInfo->setColumnCount( 2 );
  myInfo->setHeaderLabels( QStringList() << tr( "PROPERTY" ) << tr( "VALUE" ) );
  myInfo->header()->setStretchLastSection( true );
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  myInfo->header()->setResizeMode( 0, QHeaderView::ResizeToContents );
#else
  myInfo->header()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
#endif
  myInfo->setItemDelegate( new ItemDelegate( myInfo ) );
  QVBoxLayout* l = new QVBoxLayout( frame() );
  l->setMargin( 0 );
  l->addWidget( myInfo );
  connect( myInfo, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( itemDoubleClicked( QTreeWidgetItem*, int ) ) );
}

/*!
  \brief Show mesh element information
  \param ids mesh nodes / elements identifiers
*/
void SMESHGUI_TreeElemInfo::information( const QList<long>& ids )
{
  clearInternal();

  if ( actor() ) {
    int grp_details = SMESHGUI::resourceMgr()->booleanValue( "SMESH", "elem_info_grp_details", false );
    int precision   = SMESHGUI::resourceMgr()->integerValue( "SMESH", "length_precision", 6 );
    int cprecision = -1;
    if ( SMESHGUI::resourceMgr()->booleanValue( "SMESH", "use_precision", false ) ) 
      cprecision = SMESHGUI::resourceMgr()->integerValue( "SMESH", "controls_precision", -1 );
    foreach ( long id, ids ) {
      if ( !isElements() ) {
        //
        // show node info
        //
        const SMDS_MeshElement* e = actor()->GetObject()->GetMesh()->FindNode( id );
        if ( !e ) return;
        const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( e );
      
        // node ID
        QTreeWidgetItem* nodeItem = createItem( 0, Bold | All );
        nodeItem->setText( 0, SMESHGUI_ElemInfo::tr( "NODE" ) );
        nodeItem->setText( 1, QString( "#%1" ).arg( id ) );
        // coordinates
        QTreeWidgetItem* coordItem = createItem( nodeItem, Bold );
        coordItem->setText( 0, SMESHGUI_ElemInfo::tr( "COORDINATES" ) );
        QTreeWidgetItem* xItem = createItem( coordItem );
        xItem->setText( 0, "X" );
        xItem->setText( 1, QString::number( node->X(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
        QTreeWidgetItem* yItem = createItem( coordItem );
        yItem->setText( 0, "Y" );
        yItem->setText( 1, QString::number( node->Y(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
        QTreeWidgetItem* zItem = createItem( coordItem );
        zItem->setText( 0, "Z" );
        zItem->setText( 1, QString::number( node->Z(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
        // connectivity
        QTreeWidgetItem* conItem = createItem( nodeItem, Bold );
        conItem->setText( 0, SMESHGUI_ElemInfo::tr( "CONNECTIVITY" ) );
        Connectivity connectivity = nodeConnectivity( node );
        if ( !connectivity.isEmpty() ) {
          QString con = formatConnectivity( connectivity, SMDSAbs_0DElement );
          if ( !con.isEmpty() ) {
            QTreeWidgetItem* i = createItem( conItem );
            i->setText( 0, SMESHGUI_ElemInfo::tr( "0D_ELEMENTS" ) );
            i->setText( 1, con );
          }
          con = formatConnectivity( connectivity, SMDSAbs_Ball );
          if ( !con.isEmpty() ) {
            QTreeWidgetItem* i = createItem( conItem );
            i->setText( 0, SMESHGUI_ElemInfo::tr( "BALL_ELEMENTS" ) );
            i->setText( 1, con );
            i->setData( 1, TypeRole, NodeConnectivity );
          }
          con = formatConnectivity( connectivity, SMDSAbs_Edge );
          if ( !con.isEmpty() ) {
            QTreeWidgetItem* i = createItem( conItem );
            i->setText( 0, SMESHGUI_ElemInfo::tr( "EDGES" ) );
            i->setText( 1, con );
            i->setData( 1, TypeRole, NodeConnectivity );
          }
          con = formatConnectivity( connectivity, SMDSAbs_Face );
          if ( !con.isEmpty() ) {
            QTreeWidgetItem* i = createItem( conItem );
            i->setText( 0, SMESHGUI_ElemInfo::tr( "FACES" ) );
            i->setText( 1, con );
            i->setData( 1, TypeRole, NodeConnectivity );
          }
          con = formatConnectivity( connectivity, SMDSAbs_Volume );
          if ( !con.isEmpty() ) {
            QTreeWidgetItem* i = createItem( conItem );
            i->setText( 0, SMESHGUI_ElemInfo::tr( "VOLUMES" ) );
            i->setText( 1, con );
            i->setData( 1, TypeRole, NodeConnectivity );
          }
        }
        else {
          conItem->setText( 1, SMESHGUI_ElemInfo::tr( "FREE_NODE" ) );
        }
        // node position
        SMESH::SMESH_Mesh_ptr aMeshPtr = actor()->GetObject()->GetMeshServer();   
        if ( !CORBA::is_nil( aMeshPtr ) ) {
          SMESH::NodePosition_var pos = aMeshPtr->GetNodePosition( id );
          int shapeID = pos->shapeID;
          if ( shapeID > 0 ) {
            QString shapeType;
            double u = 0, v = 0;
            switch ( pos->shapeType ) {
            case GEOM::EDGE:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_EDGE" );
              if ( pos->params.length() == 1 )
                u = pos->params[0];
              break;
            case GEOM::FACE:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_FACE" );
              if ( pos->params.length() == 2 ) {
                u = pos->params[0];
                v = pos->params[1];
              }
              break;
            case GEOM::VERTEX:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_VERTEX" );
              break;
            default:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SOLID" );
              break;
            }
            QTreeWidgetItem* posItem = createItem( nodeItem, Bold );
            posItem->setText( 0, SMESHGUI_ElemInfo::tr("POSITION") );
            posItem->setText( 1, (shapeType + " #%1").arg( shapeID ));
            if ( pos->shapeType == GEOM::EDGE || pos->shapeType == GEOM::FACE ) {
              QTreeWidgetItem* uItem = createItem( posItem );
              uItem->setText( 0, SMESHGUI_ElemInfo::tr("U_POSITION") );
              uItem->setText( 1, QString::number( u, precision > 0 ? 'f' : 'g', qAbs( precision )));
              if ( pos->shapeType == GEOM::FACE ) {
                QTreeWidgetItem* vItem = createItem( posItem );
                vItem->setText( 0, SMESHGUI_ElemInfo::tr("V_POSITION") );
                vItem->setText( 1, QString::number( v, precision > 0 ? 'f' : 'g', qAbs( precision )));
              }
            }
          }
        }
        // groups node belongs to
        SMESH::SMESH_Mesh_ptr aMesh = actor()->GetObject()->GetMeshServer();
        if ( !CORBA::is_nil( aMesh ) ) {
          SMESH::ListOfGroups_var groups = aMesh->GetGroups();
          QTreeWidgetItem* groupsItem = 0;
          for ( CORBA::ULong i = 0; i < groups->length(); i++ ) {
            SMESH::SMESH_GroupBase_var aGrp = groups[i];
            if ( CORBA::is_nil( aGrp ) ) continue;
            QString aName = aGrp->GetName();
            if ( aGrp->GetType() == SMESH::NODE && !aName.isEmpty() && aGrp->Contains( id ) ) {
              if ( !groupsItem ) {
                groupsItem = createItem( nodeItem, Bold );
                groupsItem->setText( 0, SMESHGUI_AddInfo::tr( "GROUPS" ) );
              }
              QTreeWidgetItem* it = createItem( groupsItem, Bold );
              it->setText( 0, aName.trimmed() );
              if ( grp_details ) {
                SMESH::SMESH_Group_var         aStdGroup  = SMESH::SMESH_Group::_narrow( aGrp );
                SMESH::SMESH_GroupOnGeom_var   aGeomGroup = SMESH::SMESH_GroupOnGeom::_narrow( aGrp );
                SMESH::SMESH_GroupOnFilter_var aFltGroup  = SMESH::SMESH_GroupOnFilter::_narrow( aGrp );
                
                // type : group on geometry, standalone group, group on filter
                QTreeWidgetItem* typeItem = createItem( it );
                typeItem->setText( 0, SMESHGUI_AddInfo::tr( "TYPE" ) );
                if ( !CORBA::is_nil( aStdGroup ) ) {
                  typeItem->setText( 1, SMESHGUI_AddInfo::tr( "STANDALONE_GROUP" ) );
                }
                else if ( !CORBA::is_nil( aGeomGroup ) ) {
                  typeItem->setText( 1, SMESHGUI_AddInfo::tr( "GROUP_ON_GEOMETRY" ) );
                  GEOM::GEOM_Object_var gobj = aGeomGroup->GetShape();
                  _PTR(SObject) sobj = SMESH::ObjectToSObject( gobj );
                  if ( sobj ) {
                    QTreeWidgetItem* gobjItem = createItem( typeItem );
                    gobjItem->setText( 0, SMESHGUI_AddInfo::tr( "GEOM_OBJECT" ) );
                    gobjItem->setText( 1, sobj->GetName().c_str() );
                  }
                }
                else if ( !CORBA::is_nil( aFltGroup ) ) {
                  typeItem->setText( 1, SMESHGUI_AddInfo::tr( "GROUP_ON_FILTER" ) );
                }
                
                // size
                QTreeWidgetItem* sizeItem = createItem( it );
                sizeItem->setText( 0, SMESHGUI_AddInfo::tr( "SIZE" ) );
                sizeItem->setText( 1, QString::number( aGrp->Size() ) );
                
                // color
                SALOMEDS::Color color = aGrp->GetColor();
                QTreeWidgetItem* colorItem = createItem( it );
                colorItem->setText( 0, SMESHGUI_AddInfo::tr( "COLOR" ) );
                colorItem->setBackground( 1, QBrush( QColor( color.R*255., color.G*255., color.B*255.) ) );
              }
            }
          }
        }
      }
      else {
        //
        // show element info
        // 
        const SMDS_MeshElement* e = actor()->GetObject()->GetMesh()->FindElement( id );
        SMESH::Controls::NumericalFunctorPtr afunctor;
        if ( !e ) return;
        
        // element ID && type
        QString stype;
        switch( e->GetType() ) {
        case SMDSAbs_0DElement: stype = SMESHGUI_ElemInfo::tr( "0D_ELEMENT" ); break;
        case SMDSAbs_Ball:      stype = SMESHGUI_ElemInfo::tr( "BALL" ); break;
        case SMDSAbs_Edge:      stype = SMESHGUI_ElemInfo::tr( "EDGE" ); break;
        case SMDSAbs_Face:      stype = SMESHGUI_ElemInfo::tr( "FACE" ); break;
        case SMDSAbs_Volume:    stype = SMESHGUI_ElemInfo::tr( "VOLUME" ); break;
        default:;
        }
        if ( stype.isEmpty() ) return;
        QTreeWidgetItem* elemItem = createItem( 0, Bold | All );
        elemItem->setText( 0, stype );
        elemItem->setText( 1, QString( "#%1" ).arg( id ) );
        // geometry type
        QString gtype;
        switch( e->GetEntityType() ) {
        case SMDSEntity_Triangle:
        case SMDSEntity_Quad_Triangle:
        case SMDSEntity_BiQuad_Triangle:
          gtype = SMESHGUI_ElemInfo::tr( "TRIANGLE" ); break;
        case SMDSEntity_Quadrangle:
        case SMDSEntity_Quad_Quadrangle:
        case SMDSEntity_BiQuad_Quadrangle:
          gtype = SMESHGUI_ElemInfo::tr( "QUADRANGLE" ); break;
        case SMDSEntity_Polygon:
        case SMDSEntity_Quad_Polygon:
          gtype = SMESHGUI_ElemInfo::tr( "POLYGON" ); break;
        case SMDSEntity_Tetra:
        case SMDSEntity_Quad_Tetra:
          gtype = SMESHGUI_ElemInfo::tr( "TETRAHEDRON" ); break;
        case SMDSEntity_Pyramid:
        case SMDSEntity_Quad_Pyramid:
          gtype = SMESHGUI_ElemInfo::tr( "PYRAMID" ); break;
        case SMDSEntity_Hexa:
        case SMDSEntity_Quad_Hexa:
        case SMDSEntity_TriQuad_Hexa:
          gtype = SMESHGUI_ElemInfo::tr( "HEXAHEDRON" ); break;
        case SMDSEntity_Penta:
        case SMDSEntity_Quad_Penta:
          gtype = SMESHGUI_ElemInfo::tr( "PRISM" ); break;
        case SMDSEntity_Hexagonal_Prism:
          gtype = SMESHGUI_ElemInfo::tr( "HEX_PRISM" ); break;
        case SMDSEntity_Polyhedra:
        case SMDSEntity_Quad_Polyhedra:
          gtype = SMESHGUI_ElemInfo::tr( "POLYHEDRON" ); break;
        default: 
          break;
        }
        if ( !gtype.isEmpty() ) {
          QTreeWidgetItem* typeItem = createItem( elemItem, Bold );
          typeItem->setText( 0, SMESHGUI_ElemInfo::tr( "TYPE" ) );
          typeItem->setText( 1, gtype );
        }
        // quadratic flag (for edges, faces and volumes)
        if ( e->GetType() >= SMDSAbs_Edge && e->GetType() <= SMDSAbs_Volume ) {
          // quadratic flag
          QTreeWidgetItem* quadItem = createItem( elemItem, Bold );
          quadItem->setText( 0, SMESHGUI_ElemInfo::tr( "QUADRATIC" ) );
          quadItem->setText( 1, e->IsQuadratic() ? SMESHGUI_ElemInfo::tr( "YES" ) : SMESHGUI_ElemInfo::tr( "NO" ) );
        }
        if ( const SMDS_BallElement* ball = dynamic_cast<const SMDS_BallElement*>( e )) {
          // ball diameter
          QTreeWidgetItem* diamItem = createItem( elemItem, Bold );
          diamItem->setText( 0, SMESHGUI_ElemInfo::tr( "BALL_DIAMETER" ) );
          diamItem->setText( 1, QString( "%1" ).arg( ball->GetDiameter() ));
        }
        // connectivity
        QTreeWidgetItem* conItem = createItem( elemItem, Bold );
        conItem->setText( 0, SMESHGUI_ElemInfo::tr( "CONNECTIVITY" ) );


        if( e->GetGeomType() != SMDSGeom_POLYHEDRA ) {
          SMDS_ElemIteratorPtr nodeIt = e->nodesIterator();
          for ( int idx = 1; nodeIt->more(); idx++ ) {
            const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( nodeIt->next() );
            nodeInfo( node, idx, e->NbNodes(), conItem );
          }
        }
        else {
          const SMDS_VtkVolume* aVtkVolume = dynamic_cast<const SMDS_VtkVolume*>(e);
          SMDS_ElemIteratorPtr nodeIt = aVtkVolume->uniqueNodesIterator();
          QList<const SMDS_MeshElement*> uniqueNodes;
          while ( nodeIt->more() )
            uniqueNodes.append( nodeIt->next() );

          SMDS_VolumeTool vtool( e );
          const int nbFaces = vtool.NbFaces();
          for( int face_id = 0; face_id < nbFaces; face_id++ ) {
            QTreeWidgetItem* faceItem = createItem( conItem, Bold );
            faceItem->setText( 0, QString( "%1 %2 / %3" ).arg( SMESHGUI_ElemInfo::tr( "FACE" ) ).arg( face_id + 1 ).arg( nbFaces ) );
            faceItem->setExpanded( true );

            const SMDS_MeshNode** aNodeIds = vtool.GetFaceNodes( face_id );
            const int nbNodes = vtool.NbFaceNodes( face_id );
            for( int node_id = 0; node_id < nbNodes; node_id++ ) {
              const SMDS_MeshNode* node = aNodeIds[node_id];
              nodeInfo( node, uniqueNodes.indexOf(node) + 1, aVtkVolume->NbUniqueNodes(), faceItem );
            }
          }
        }
        //Controls
        QTreeWidgetItem* cntrItem = createItem( elemItem, Bold );
        cntrItem->setText( 0, SMESHGUI_ElemInfo::tr( "CONTROLS" ) );
        //Length
        if( e->GetType()==SMDSAbs_Edge){         
          afunctor.reset( new SMESH::Controls::Length() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          QTreeWidgetItem* lenItem = createItem( cntrItem, Bold );
          lenItem->setText( 0, tr( "LENGTH_EDGES" ) );
          lenItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id ) ) );         
        }
        if( e->GetType() == SMDSAbs_Face ) {
          //Area         
          afunctor.reset( new SMESH::Controls::Area() );        
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          QTreeWidgetItem* areaItem = createItem( cntrItem, Bold );
          areaItem->setText( 0, tr( "AREA_ELEMENTS" ) );
          areaItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue(id) ) );         
          //Taper
          afunctor.reset( new SMESH::Controls::Taper() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          QTreeWidgetItem* taperlItem = createItem( cntrItem, Bold );
          taperlItem->setText( 0, tr( "TAPER_ELEMENTS" ) );
          taperlItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id ) ) );    
          //AspectRatio2D
          afunctor.reset( new SMESH::Controls::AspectRatio() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );  
          QTreeWidgetItem* ratlItem = createItem( cntrItem, Bold );
          ratlItem->setText( 0, tr( "ASPECTRATIO_ELEMENTS" ));
          ratlItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id ) ) );      
          //Minimum angle
          afunctor.reset( new SMESH::Controls::MinimumAngle() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          QTreeWidgetItem* minanglItem = createItem( cntrItem, Bold );
          minanglItem->setText( 0, tr( "MINIMUMANGLE_ELEMENTS" ) );
          minanglItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id ) ) );    
          //Wraping angle       
          afunctor.reset( new SMESH::Controls::Warping() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          QTreeWidgetItem* warpItem = createItem( cntrItem, Bold );
          warpItem->setText( 0, tr( "WARP_ELEMENTS" ));
          warpItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id ) ) );        
          //Skew          
          afunctor.reset( new SMESH::Controls::Skew() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          QTreeWidgetItem* skewItem = createItem( cntrItem, Bold );
          skewItem->setText( 0, tr( "SKEW_ELEMENTS" ) );
          skewItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id ) ) );       
          //ElemDiam2D    
          afunctor.reset( new SMESH::Controls::MaxElementLength2D() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          QTreeWidgetItem* diamItem = createItem( cntrItem, Bold );
          diamItem->setText( 0, tr( "MAX_ELEMENT_LENGTH_2D" ));
          diamItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id ) ) );       
        }
        if( e->GetType() == SMDSAbs_Volume ) {
          //AspectRatio3D
          afunctor.reset( new SMESH::Controls::AspectRatio3D() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          QTreeWidgetItem* ratlItem3 = createItem( cntrItem, Bold );
          ratlItem3->setText( 0, tr( "ASPECTRATIO_3D_ELEMENTS" ) );
          ratlItem3->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id ) ) );      
          //Volume
          afunctor.reset( new SMESH::Controls::Volume() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          QTreeWidgetItem* volItem = createItem( cntrItem, Bold );
          volItem->setText( 0, tr( "VOLUME_3D_ELEMENTS" ) );
          volItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id ) ) );
          //ElementDiameter3D
          afunctor.reset( new SMESH::Controls::MaxElementLength3D() );
          afunctor->SetMesh( actor()->GetObject()->GetMesh() );
          QTreeWidgetItem* diam3Item = createItem( cntrItem, Bold );
          diam3Item->setText( 0, tr( "MAX_ELEMENT_LENGTH_3D" ) );
          diam3Item->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id ) ) );     
        }

        // gravity center
        XYZ gc = gravityCenter( e );
        QTreeWidgetItem* gcItem = createItem( elemItem, Bold );
        gcItem->setText( 0, SMESHGUI_ElemInfo::tr( "GRAVITY_CENTER" ) );
        QTreeWidgetItem* xItem = createItem( gcItem );
        xItem->setText( 0, "X" );
        xItem->setText( 1, QString::number( gc.x(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
        QTreeWidgetItem* yItem = createItem( gcItem );
        yItem->setText( 0, "Y" );
        yItem->setText( 1, QString::number( gc.y(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
        QTreeWidgetItem* zItem = createItem( gcItem );
        zItem->setText( 0, "Z" );
        zItem->setText( 1, QString::number( gc.z(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );

        // normal vector
        if( e->GetType() == SMDSAbs_Face ) {
          XYZ gc = normal( e );
          QTreeWidgetItem* nItem = createItem( elemItem, Bold );
          nItem->setText( 0, SMESHGUI_ElemInfo::tr( "NORMAL_VECTOR" ) );
          QTreeWidgetItem* xItem = createItem( nItem );
          xItem->setText( 0, "X" );
          xItem->setText( 1, QString::number( gc.x(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
          QTreeWidgetItem* yItem = createItem( nItem );
          yItem->setText( 0, "Y" );
          yItem->setText( 1, QString::number( gc.y(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
          QTreeWidgetItem* zItem = createItem( nItem );
          zItem->setText( 0, "Z" );
          zItem->setText( 1, QString::number( gc.z(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
        }

        // element position
        SMESH::SMESH_Mesh_ptr aMesh = actor()->GetObject()->GetMeshServer();
        if ( e->GetType() >= SMDSAbs_Edge && e->GetType() <= SMDSAbs_Volume ) {
          if ( !CORBA::is_nil( aMesh ) ) {
            SMESH::ElementPosition pos = aMesh->GetElementPosition( id );
            int shapeID = pos.shapeID;
            if ( shapeID > 0 ) {
              QTreeWidgetItem* shItem = createItem( elemItem, Bold );
              QString shapeType;
              switch ( pos.shapeType ) {
              case GEOM::EDGE:   shapeType = SMESHGUI_ElemInfo::tr( "GEOM_EDGE" );   break;
              case GEOM::FACE:   shapeType = SMESHGUI_ElemInfo::tr( "GEOM_FACE" );   break;
              case GEOM::VERTEX: shapeType = SMESHGUI_ElemInfo::tr( "GEOM_VERTEX" ); break;
              case GEOM::SOLID:  shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SOLID" );  break;
              case GEOM::SHELL:  shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SHELL" );  break;
              default:           shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SHAPE" );  break;
              }
              shItem->setText( 0, SMESHGUI_ElemInfo::tr( "POSITION" ) );
              shItem->setText( 1, QString( "%1 #%2" ).arg( shapeType ).arg( shapeID ) );
            }
          }
        }
        // groups element belongs to
        if ( !CORBA::is_nil( aMesh ) ) {
          SMESH::ListOfGroups_var  groups = aMesh->GetGroups();
          QTreeWidgetItem* groupsItem = 0;
          for ( CORBA::ULong i = 0; i < groups->length(); i++ ) {
            SMESH::SMESH_GroupBase_var aGrp = groups[i];
            if ( CORBA::is_nil( aGrp ) ) continue;
            QString aName = aGrp->GetName();
            if ( aGrp->GetType() != SMESH::NODE && !aName.isEmpty() && aGrp->Contains( id ) ) {
              if ( !groupsItem ) {
                groupsItem = createItem( elemItem, Bold );
                groupsItem->setText( 0, SMESHGUI_AddInfo::tr( "GROUPS" ) );
              }
              QTreeWidgetItem* it = createItem( groupsItem, Bold );
              it->setText( 0, aName.trimmed() );
              if ( grp_details ) {
                SMESH::SMESH_Group_var         aStdGroup  = SMESH::SMESH_Group::_narrow( aGrp );
                SMESH::SMESH_GroupOnGeom_var   aGeomGroup = SMESH::SMESH_GroupOnGeom::_narrow( aGrp );
                SMESH::SMESH_GroupOnFilter_var aFltGroup  = SMESH::SMESH_GroupOnFilter::_narrow( aGrp );
                
                // type : group on geometry, standalone group, group on filter
                QTreeWidgetItem* typeItem = createItem( it );
                typeItem->setText( 0, SMESHGUI_AddInfo::tr( "TYPE" ) );
                if ( !CORBA::is_nil( aStdGroup ) ) {
                  typeItem->setText( 1, SMESHGUI_AddInfo::tr( "STANDALONE_GROUP" ) );
                }
                else if ( !CORBA::is_nil( aGeomGroup ) ) {
                  typeItem->setText( 1, SMESHGUI_AddInfo::tr( "GROUP_ON_GEOMETRY" ) );
                  GEOM::GEOM_Object_var gobj = aGeomGroup->GetShape();
                  _PTR(SObject) sobj = SMESH::ObjectToSObject( gobj );
                  if ( sobj ) {
                    QTreeWidgetItem* gobjItem = createItem( typeItem );
                    gobjItem->setText( 0, SMESHGUI_AddInfo::tr( "GEOM_OBJECT" ) );
                    gobjItem->setText( 1, sobj->GetName().c_str() );
                  }
                }
                else if ( !CORBA::is_nil( aFltGroup ) ) {
                  typeItem->setText( 1, SMESHGUI_AddInfo::tr( "GROUP_ON_FILTER" ) );
                }
                
                // size
                QTreeWidgetItem* sizeItem = createItem( it );
                sizeItem->setText( 0, SMESHGUI_AddInfo::tr( "SIZE" ) );
                sizeItem->setText( 1, QString::number( aGrp->Size() ) );
                
                // color
                SALOMEDS::Color color = aGrp->GetColor();
                QTreeWidgetItem* colorItem = createItem( it );
                colorItem->setText( 0, SMESHGUI_AddInfo::tr( "COLOR" ) );
                colorItem->setBackground( 1, QBrush( QColor( color.R*255., color.G*255., color.B*255.) ) );
              }
            }
          }
        }
      }
    }
  }
}

/*!
  \brief Show node information
  \param node mesh node for showing
  \param index index of current node
  \param nbNodes number of unique nodes in element
  \param parentItem parent item of tree
*/
void SMESHGUI_TreeElemInfo::nodeInfo( const SMDS_MeshNode* node, int index,
                                      int nbNodes, QTreeWidgetItem* parentItem )
{
  int precision   = SMESHGUI::resourceMgr()->integerValue( "SMESH", "length_precision", 6 );
  // node number and ID
  QTreeWidgetItem* nodeItem = createItem( parentItem, Bold );
  nodeItem->setText( 0, QString( "%1 %2 / %3" ).arg( SMESHGUI_ElemInfo::tr( "NODE" ) ).arg( index ).arg( nbNodes ) );
  nodeItem->setText( 1, QString( "#%1" ).arg( node->GetID() ) );
  nodeItem->setData( 1, TypeRole, ElemConnectivity );
  nodeItem->setData( 1, IdRole, node->GetID() );
  nodeItem->setExpanded( false );
  // node coordinates
  QTreeWidgetItem* coordItem = createItem( nodeItem );
  coordItem->setText( 0, SMESHGUI_ElemInfo::tr( "COORDINATES" ) );
  QTreeWidgetItem* xItem = createItem( coordItem );
  xItem->setText( 0, "X" );
  xItem->setText( 1, QString::number( node->X(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
  QTreeWidgetItem* yItem = createItem( coordItem );
  yItem->setText( 0, "Y" );
  yItem->setText( 1, QString::number( node->Y(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
  QTreeWidgetItem* zItem = createItem( coordItem );
  zItem->setText( 0, "Z" );
  zItem->setText( 1, QString::number( node->Z(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
  // node connectivity
  QTreeWidgetItem* nconItem = createItem( nodeItem );
  nconItem->setText( 0, SMESHGUI_ElemInfo::tr( "CONNECTIVITY" ) );
  Connectivity connectivity = nodeConnectivity( node );
  if ( !connectivity.isEmpty() ) {
    QString con = formatConnectivity( connectivity, SMDSAbs_0DElement );
    if ( !con.isEmpty() ) {
      QTreeWidgetItem* i = createItem( nconItem );
      i->setText( 0, SMESHGUI_ElemInfo::tr( "0D_ELEMENTS" ) );
      i->setText( 1, con );
    }
    con = formatConnectivity( connectivity, SMDSAbs_Edge );
    if ( !con.isEmpty() ) {
      QTreeWidgetItem* i = createItem( nconItem );
      i->setText( 0, SMESHGUI_ElemInfo::tr( "EDGES" ) );
      i->setText( 1, con );
      i->setData( 1, TypeRole, NodeConnectivity );
    }
    con = formatConnectivity( connectivity, SMDSAbs_Ball );
    if ( !con.isEmpty() ) {
      QTreeWidgetItem* i = createItem( nconItem );
      i->setText( 0, SMESHGUI_ElemInfo::tr( "BALL_ELEMENTS" ) );
      i->setText( 1, con );
      i->setData( 1, TypeRole, NodeConnectivity );
    }
    con = formatConnectivity( connectivity, SMDSAbs_Face );
    if ( !con.isEmpty() ) {
      QTreeWidgetItem* i = createItem( nconItem );
      i->setText( 0, SMESHGUI_ElemInfo::tr( "FACES" ) );
      i->setText( 1, con );
      i->setData( 1, TypeRole, NodeConnectivity );
    }
    con = formatConnectivity( connectivity, SMDSAbs_Volume );
    if ( !con.isEmpty() ) {
      QTreeWidgetItem* i = createItem( nconItem );
      i->setText( 0, SMESHGUI_ElemInfo::tr( "VOLUMES" ) );
      i->setText( 1, con );
      i->setData( 1, TypeRole, NodeConnectivity );
    }
  }
}
/*!
  \brief Internal clean-up (reset widget)
*/
void SMESHGUI_TreeElemInfo::clearInternal()
{
  myInfo->clear();
  myInfo->repaint();
}

/*!
  \brief Create new tree item.
  \param parent parent tree widget item
  \param flags item flag
  \return new tree widget item
*/
QTreeWidgetItem* SMESHGUI_TreeElemInfo::createItem( QTreeWidgetItem* parent, int flags )
{
  QTreeWidgetItem* item;
  if ( parent )
    item = new QTreeWidgetItem( parent );
  else
    item = new QTreeWidgetItem( myInfo );

  item->setFlags( item->flags() | Qt::ItemIsEditable );

  QFont f = item->font( 0 );
  f.setBold( true );
  for ( int i = 0; i < myInfo->columnCount(); i++ ) {
    if ( ( flags & Bold ) && ( i == 0 || flags & All ) )
      item->setFont( i, f );
  }

  item->setExpanded( true );
  return item;
}

void SMESHGUI_TreeElemInfo::contextMenuEvent( QContextMenuEvent* e )
{
  QList< QTreeWidgetItem* > widgets = myInfo->selectedItems();
  if ( widgets.isEmpty() ) return;
  QTreeWidgetItem* aTreeItem = widgets.first();
  int type = aTreeItem->data( 1, TypeRole ).toInt();
  int id   = aTreeItem->data( 1, IdRole ).toInt();
  QMenu menu;
  QAction* a = menu.addAction( tr( "SHOW_ITEM_INFO" ) );
  if ( type == ElemConnectivity && id > 0 && menu.exec( e->globalPos() ) == a )
    emit( itemInfo( id ) );
  else if ( type == NodeConnectivity && menu.exec( e->globalPos() ) == a )
    emit( itemInfo( aTreeItem->text( 1 ) ) );
}

void  SMESHGUI_TreeElemInfo::itemDoubleClicked( QTreeWidgetItem* theItem, int theColumn )
{
  if ( theItem ) {
    int type = theItem->data( 1, TypeRole ).toInt();
    int id   = theItem->data( 1, IdRole ).toInt();
    if ( type == ElemConnectivity && id > 0 )
      emit( itemInfo( id ) );
    else if ( type == NodeConnectivity )
      emit( itemInfo( theItem->text( 1 ) ) );
  }
}

void SMESHGUI_TreeElemInfo::saveInfo( QTextStream &out )
{
  out << QString( 12, '-' ) << "\n";
  out << SMESHGUI_ElemInfo::tr( "ELEM_INFO" ) << "\n";
  out << QString( 12, '-' ) << "\n";

  QTreeWidgetItemIterator it( myInfo );
  while ( *it ) {
    if ( !( *it )->text(0).isEmpty() ) {
      out << QString( SPACING_INFO * itemDepth( *it ), ' ' ) << ( *it )->text(0);
      if ( !( *it )->text(1).isEmpty() ) out << ": " << ( *it )->text(1);
      out << "\n";
    }
    ++it;
  }
  out << "\n";
}

/*!
  \class GrpComputor
  \brief Mesh information computer
  \internal
  
  The class is created for different computation operation. Currently it is used
  to compute number of underlying nodes for the groups.
*/

/*!
  \brief Contructor
*/
GrpComputor::GrpComputor( SMESH::SMESH_GroupBase_ptr grp,
                          QTreeWidgetItem*           item,
                          QObject*                   parent,
                          bool                       toComputeSize)
  : QObject( parent ), myItem( item ), myToComputeSize( toComputeSize )
{
  myGroup = SMESH::SMESH_GroupBase::_narrow( grp );
}

/*!
  \brief Compute function
*/
void GrpComputor::compute()
{
  if ( !CORBA::is_nil( myGroup ) && myItem ) {
    SUIT_OverrideCursor wc;
    QTreeWidgetItem* item = myItem;
    myItem = 0;
    int nb = myToComputeSize ? myGroup->Size() : myGroup->GetNumberOfNodes();
    item->treeWidget()->removeItemWidget( item, 1 );
    item->setText( 1, QString::number( nb ));
  }
}

/*!
  \class SMESHGUI_AddInfo
  \brief The wigdet shows additional information on the mesh object.
*/

/*!
  \brief Constructor
  \param parent parent widget
*/
SMESHGUI_AddInfo::SMESHGUI_AddInfo( QWidget* parent )
: QTreeWidget( parent )
{
  setColumnCount( 2 );
  header()->setStretchLastSection( true );
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  header()->setResizeMode( 0, QHeaderView::ResizeToContents );
#else
  header()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
#endif
  header()->hide();
}

/*!
  \brief Destructor
*/
SMESHGUI_AddInfo::~SMESHGUI_AddInfo()
{
}

/*!
  \brief Show additional information on the selected object
  \param obj object being processed (mesh, sub-mesh, group, ID source)
*/
void SMESHGUI_AddInfo::showInfo( SMESH::SMESH_IDSource_ptr obj )
{
  setProperty( "group_index", 0 );
  setProperty( "submesh_index",  0 );
  myComputors.clear();
  clear();

  if ( CORBA::is_nil( obj ) ) return;

  _PTR(SObject) sobj = SMESH::ObjectToSObject( obj );
  if ( !sobj ) return;

  // name
  QTreeWidgetItem* nameItem = createItem( 0, Bold | All );
  nameItem->setText( 0, tr( "NAME" ) );
  nameItem->setText( 1, sobj->GetName().c_str() );
  
  SMESH::SMESH_Mesh_var      aMesh    = SMESH::SMESH_Mesh::_narrow( obj );
  SMESH::SMESH_subMesh_var   aSubMesh = SMESH::SMESH_subMesh::_narrow( obj );
  SMESH::SMESH_GroupBase_var aGroup   = SMESH::SMESH_GroupBase::_narrow( obj );
  
  if ( !aMesh->_is_nil() )
    meshInfo( aMesh, nameItem );
  else if ( !aSubMesh->_is_nil() )
    subMeshInfo( aSubMesh, nameItem );
  else if ( !aGroup->_is_nil() )
    groupInfo( aGroup.in(), nameItem );
}

/*!
  \brief Create new tree item.
  \param parent parent tree widget item
  \param flags item flag
  \return new tree widget item
*/
QTreeWidgetItem* SMESHGUI_AddInfo::createItem( QTreeWidgetItem* parent, int flags )
{
  QTreeWidgetItem* item;

  if ( parent )
    item = new QTreeWidgetItem( parent );
  else
    item = new QTreeWidgetItem( this );

  //item->setFlags( item->flags() | Qt::ItemIsEditable );

  QFont f = item->font( 0 );
  f.setBold( true );
  for ( int i = 0; i < columnCount(); i++ ) {
    if ( ( flags & Bold ) && ( i == 0 || flags & All ) )
      item->setFont( i, f );
  }

  item->setExpanded( true );
  return item;
}

/*!
  \brief Show mesh info
  \param mesh mesh object
  \param parent parent tree item
*/
void SMESHGUI_AddInfo::meshInfo( SMESH::SMESH_Mesh_ptr mesh, QTreeWidgetItem* parent )
{
  // type
  GEOM::GEOM_Object_var shape = mesh->GetShapeToMesh();
  SMESH::MedFileInfo_var inf = mesh->GetMEDFileInfo();
  QTreeWidgetItem* typeItem = createItem( parent, Bold );
  typeItem->setText( 0, tr( "TYPE" ) );
  if ( !CORBA::is_nil( shape ) ) {
    typeItem->setText( 1, tr( "MESH_ON_GEOMETRY" ) );
    _PTR(SObject) sobj = SMESH::ObjectToSObject( shape );
    if ( sobj ) {
      QTreeWidgetItem* gobjItem = createItem( typeItem );
      gobjItem->setText( 0, tr( "GEOM_OBJECT" ) );
      gobjItem->setText( 1, sobj->GetName().c_str() );
    }
  }
  else if ( strlen( (char*)inf->fileName ) > 0 ) {
    typeItem->setText( 1, tr( "MESH_FROM_FILE" ) );
    QTreeWidgetItem* fileItem = createItem( typeItem );
    fileItem->setText( 0, tr( "FILE_NAME" ) );
    fileItem->setText( 1, (char*)inf->fileName );
  }
  else {
    typeItem->setText( 1, tr( "STANDALONE_MESH" ) );
  }
  
  // groups
  myGroups = mesh->GetGroups();
  showGroups();

  // sub-meshes
  mySubMeshes = mesh->GetSubMeshes();
  showSubMeshes();
}

/*!
  \brief Show sub-mesh info
  \param subMesh sub-mesh object
  \param parent parent tree item
*/
void SMESHGUI_AddInfo::subMeshInfo( SMESH::SMESH_subMesh_ptr subMesh, QTreeWidgetItem* parent )
{
  bool isShort = parent->parent() != 0;

  if ( !isShort ) {
    // parent mesh
    _PTR(SObject) sobj = SMESH::ObjectToSObject( subMesh->GetFather() );
    if ( sobj ) {
      QTreeWidgetItem* nameItem = createItem( parent, Bold );
      nameItem->setText( 0, tr( "PARENT_MESH" ) );
      nameItem->setText( 1, sobj->GetName().c_str() );
    }
  }
  
  // shape
  GEOM::GEOM_Object_var gobj = subMesh->GetSubShape();
  _PTR(SObject) sobj = SMESH::ObjectToSObject( gobj );
  if ( sobj ) {
    QTreeWidgetItem* gobjItem = createItem( parent, Bold );
    gobjItem->setText( 0, tr( "GEOM_OBJECT" ) );
    gobjItem->setText( 1, sobj->GetName().c_str() );
  }
}

/*!
  \brief Show group info
  \param grp mesh group object
  \param parent parent tree item
*/
void SMESHGUI_AddInfo::groupInfo( SMESH::SMESH_GroupBase_ptr grp, QTreeWidgetItem* parent )
{
  bool isShort = parent->parent() != 0;

  SMESH::SMESH_Group_var         aStdGroup  = SMESH::SMESH_Group::_narrow( grp );
  SMESH::SMESH_GroupOnGeom_var   aGeomGroup = SMESH::SMESH_GroupOnGeom::_narrow( grp );
  SMESH::SMESH_GroupOnFilter_var aFltGroup  = SMESH::SMESH_GroupOnFilter::_narrow( grp );

  if ( !isShort ) {
    // parent mesh
    _PTR(SObject) sobj = SMESH::ObjectToSObject( grp->GetMesh() );
    if ( sobj ) {
      QTreeWidgetItem* nameItem = createItem( parent, Bold );
      nameItem->setText( 0, tr( "PARENT_MESH" ) );
      nameItem->setText( 1, sobj->GetName().c_str() );
    }
  }

  // type : group on geometry, standalone group, group on filter
  QTreeWidgetItem* typeItem = createItem( parent, Bold );
  typeItem->setText( 0, tr( "TYPE" ) );
  if ( !CORBA::is_nil( aStdGroup ) ) {
    typeItem->setText( 1, tr( "STANDALONE_GROUP" ) );
  }
  else if ( !CORBA::is_nil( aGeomGroup ) ) {
    typeItem->setText( 1, tr( "GROUP_ON_GEOMETRY" ) );
    GEOM::GEOM_Object_var gobj = aGeomGroup->GetShape();
    _PTR(SObject) sobj = SMESH::ObjectToSObject( gobj );
    if ( sobj ) {
      QTreeWidgetItem* gobjItem = createItem( typeItem );
      gobjItem->setText( 0, tr( "GEOM_OBJECT" ) );
      gobjItem->setText( 1, sobj->GetName().c_str() );
    }
  }
  else if ( !CORBA::is_nil( aFltGroup ) ) {
    typeItem->setText( 1, tr( "GROUP_ON_FILTER" ) );
  }

  if ( !isShort ) {
    // entity type
    QString etype = tr( "UNKNOWN" );
    switch( grp->GetType() ) {
    case SMESH::NODE:
      etype = tr( "NODE" );
      break;
    case SMESH::EDGE:
      etype = tr( "EDGE" );
      break;
    case SMESH::FACE:
      etype = tr( "FACE" );
      break;
    case SMESH::VOLUME:
      etype = tr( "VOLUME" );
      break;
    case SMESH::ELEM0D:
      etype = tr( "0DELEM" );
      break;
    case SMESH::BALL:
      etype = tr( "BALL" );
      break;
    default:
      break;
    }
    QTreeWidgetItem* etypeItem = createItem( parent, Bold );
    etypeItem->setText( 0, tr( "ENTITY_TYPE" ) );
    etypeItem->setText( 1, etype );
  }

  SMESH::SMESH_Mesh_var mesh = grp->GetMesh();
  bool            meshLoaded = mesh->IsLoaded();

  // size. Don't call grp->Size() for GroupOnFilter - issue IPAL52831
  int groupSize = -1;
  if ( grp->IsNodeInfoAvailable() || CORBA::is_nil( aFltGroup ))
    groupSize = grp->Size();

  QTreeWidgetItem* sizeItem = createItem( parent, Bold );
  sizeItem->setText( 0, tr( "SIZE" ) );
  if ( groupSize > -1 ) {
    sizeItem->setText( 1, QString::number( groupSize ) );
  }
  else {
    QPushButton* btn = new QPushButton( tr( meshLoaded ? "COMPUTE" : "LOAD"), this );
    setItemWidget( sizeItem, 1, btn );
    GrpComputor* comp = new GrpComputor( grp, sizeItem, this, /*size=*/true );
    connect( btn, SIGNAL( clicked() ), comp, SLOT( compute() ) );
    myComputors.append( comp );
    if ( !meshLoaded )
      connect( btn, SIGNAL( clicked() ), this, SLOT( changeLoadToCompute() ) );
  }

  // color
  SALOMEDS::Color color = grp->GetColor();
  QTreeWidgetItem* colorItem = createItem( parent, Bold );
  colorItem->setText( 0, tr( "COLOR" ) );
  colorItem->setBackground( 1, QBrush( QColor( color.R*255., color.G*255., color.B*255.) ) );

  // nb of underlying nodes
  if ( grp->GetType() != SMESH::NODE) {
    QTreeWidgetItem* nodesItem = createItem( parent, Bold );
    nodesItem->setText( 0, tr( "NB_NODES" ) );
    int nbNodesLimit = SMESHGUI::resourceMgr()->integerValue( "SMESH", "info_groups_nodes_limit", 100000 );
    bool toShowNodes = groupSize >= 0 ? ( grp->IsNodeInfoAvailable() || nbNodesLimit <= 0 || groupSize <= nbNodesLimit ) : false;
    if ( toShowNodes && meshLoaded ) {
      // already calculated and up-to-date
      nodesItem->setText( 1, QString::number( grp->GetNumberOfNodes() ) );
    }
    else {
      QPushButton* btn = new QPushButton( tr( meshLoaded ? "COMPUTE" : "LOAD"), this );
      setItemWidget( nodesItem, 1, btn );
      GrpComputor* comp = new GrpComputor( grp, nodesItem, this ); 
      connect( btn, SIGNAL( clicked() ), comp, SLOT( compute() ) );
      myComputors.append( comp );
      if ( !meshLoaded )
        connect( btn, SIGNAL( clicked() ), this, SLOT( changeLoadToCompute() ) );
    }
  }
}

void SMESHGUI_AddInfo::showGroups()
{
  myComputors.clear();

  QTreeWidgetItem* parent = topLevelItemCount() > 0 ? topLevelItem( 0 ) : 0; // parent should be first top level item
  if ( !parent ) return;

  int idx = property( "group_index" ).toInt();

  QTreeWidgetItem* itemGroups = 0;
  for ( int i = 0; i < parent->childCount() && !itemGroups; i++ ) {
    if ( parent->child( i )->data( 0, Qt::UserRole ).toInt() == GROUPS_ID ) {
      itemGroups = parent->child( i );
      ExtraWidget* extra = dynamic_cast<ExtraWidget*>( itemWidget( itemGroups, 1 ) );
      if ( extra )
        extra->updateControls( myGroups->length(), idx );
      while ( itemGroups->childCount() ) delete itemGroups->child( 0 ); // clear child items
    }
  }

  QMap<int, QTreeWidgetItem*> grpItems;
  for ( int i = idx*MAXITEMS ; i < qMin( (idx+1)*MAXITEMS, (int)myGroups->length() ); i++ ) {
    SMESH::SMESH_GroupBase_var grp = myGroups[i];
    if ( CORBA::is_nil( grp ) ) continue;
    _PTR(SObject) grpSObj = SMESH::ObjectToSObject( grp );
    if ( !grpSObj ) continue;

    int grpType = grp->GetType();

    if ( !itemGroups ) {
      // create top-level groups container item
      itemGroups = createItem( parent, Bold | All );
      itemGroups->setText( 0, tr( "GROUPS" ) );
      itemGroups->setData( 0, Qt::UserRole, GROUPS_ID );

      // total number of groups > 10, show extra widgets for info browsing
      if ((int) myGroups->length() > MAXITEMS ) {
        ExtraWidget* extra = new ExtraWidget( this, true );
        connect( extra->prev, SIGNAL( clicked() ), this, SLOT( showPreviousGroups() ) );
        connect( extra->next, SIGNAL( clicked() ), this, SLOT( showNextGroups() ) );
        setItemWidget( itemGroups, 1, extra );
        extra->updateControls( myGroups->length(), idx );
      }
    }

    if ( grpItems.find( grpType ) == grpItems.end() ) {
      grpItems[ grpType ] = createItem( itemGroups, Bold | All );
      grpItems[ grpType ]->setText( 0, tr( QString( "GROUPS_%1" ).arg( grpType ).toLatin1().constData() ) );
      itemGroups->insertChild( grpType-1, grpItems[ grpType ] );
    }
  
    // group name
    QTreeWidgetItem* grpNameItem = createItem( grpItems[ grpType ] );
    grpNameItem->setText( 0, QString( grpSObj->GetName().c_str() ).trimmed() ); // name is trimmed

    // group info
    groupInfo( grp.in(), grpNameItem );
  }
}

void SMESHGUI_AddInfo::showSubMeshes()
{
  QTreeWidgetItem* parent = topLevelItemCount() > 0 ? topLevelItem( 0 ) : 0; // parent should be first top level item
  if ( !parent ) return;

  int idx = property( "submesh_index" ).toInt();

  QTreeWidgetItem* itemSubMeshes = 0;
  for ( int i = 0; i < parent->childCount() && !itemSubMeshes; i++ ) {
    if ( parent->child( i )->data( 0, Qt::UserRole ).toInt() == SUBMESHES_ID ) {
      itemSubMeshes = parent->child( i );
      ExtraWidget* extra = dynamic_cast<ExtraWidget*>( itemWidget( itemSubMeshes, 1 ) );
      if ( extra )
        extra->updateControls( mySubMeshes->length(), idx );
      while ( itemSubMeshes->childCount() ) delete itemSubMeshes->child( 0 ); // clear child items
    }
  }

  QMap<int, QTreeWidgetItem*> smItems;
  for ( int i = idx*MAXITEMS ; i < qMin( (idx+1)*MAXITEMS, (int)mySubMeshes->length() ); i++ ) {
    SMESH::SMESH_subMesh_var sm = mySubMeshes[i];
    if ( CORBA::is_nil( sm ) ) continue;
    _PTR(SObject) smSObj = SMESH::ObjectToSObject( sm );
    if ( !smSObj ) continue;
    
    GEOM::GEOM_Object_var gobj = sm->GetSubShape();
    if ( CORBA::is_nil(gobj ) ) continue;
    
    int smType = gobj->GetShapeType();
    if ( smType == GEOM::COMPSOLID ) smType = GEOM::COMPOUND;

    if ( !itemSubMeshes ) {
      itemSubMeshes = createItem( parent, Bold | All );
      itemSubMeshes->setText( 0, tr( "SUBMESHES" ) );
      itemSubMeshes->setData( 0, Qt::UserRole, SUBMESHES_ID );

      // total number of sub-meshes > 10, show extra widgets for info browsing
      if ((int) mySubMeshes->length() > MAXITEMS ) {
        ExtraWidget* extra = new ExtraWidget( this, true );
        connect( extra->prev, SIGNAL( clicked() ), this, SLOT( showPreviousSubMeshes() ) );
        connect( extra->next, SIGNAL( clicked() ), this, SLOT( showNextSubMeshes() ) );
        setItemWidget( itemSubMeshes, 1, extra );
        extra->updateControls( mySubMeshes->length(), idx );
      }
    }
         
    if ( smItems.find( smType ) == smItems.end() ) {
      smItems[ smType ] = createItem( itemSubMeshes, Bold | All );
      smItems[ smType ]->setText( 0, tr( QString( "SUBMESHES_%1" ).arg( smType ).toLatin1().constData() ) );
      itemSubMeshes->insertChild( smType, smItems[ smType ] );
    }
    
    // submesh name
    QTreeWidgetItem* smNameItem = createItem( smItems[ smType ] );
    smNameItem->setText( 0, QString( smSObj->GetName().c_str() ).trimmed() ); // name is trimmed
    
    // submesh info
    subMeshInfo( sm.in(), smNameItem );
  }
}

/*!
 * \brief Change button label of "nb underlying node" group from "Load" to "Compute"
 */
void SMESHGUI_AddInfo::changeLoadToCompute()
{
  for ( int i = 0; i < myComputors.count(); ++i )
  {
    if ( QTreeWidgetItem* item = myComputors[i]->getItem() )
    {
      if ( QPushButton* btn = qobject_cast<QPushButton*>( itemWidget ( item, 1 ) ) )
        btn->setText( tr("COMPUTE") );
    }
  }
}

void SMESHGUI_AddInfo::showPreviousGroups()
{
  int idx = property( "group_index" ).toInt();
  setProperty( "group_index", idx-1 );
  showGroups();
}

void SMESHGUI_AddInfo::showNextGroups()
{
  int idx = property( "group_index" ).toInt();
  setProperty( "group_index", idx+1 );
  showGroups();
}

void SMESHGUI_AddInfo::showPreviousSubMeshes()
{
  int idx = property( "submesh_index" ).toInt();
  setProperty( "submesh_index", idx-1 );
  showSubMeshes();
}

void SMESHGUI_AddInfo::showNextSubMeshes()
{
  int idx = property( "submesh_index" ).toInt();
  setProperty( "submesh_index", idx+1 );
  showSubMeshes();
}

void SMESHGUI_AddInfo::saveInfo( QTextStream &out )
{
  out << QString( 15, '-')       << "\n";
  out << tr( "ADDITIONAL_INFO" ) << "\n";
  out << QString( 15, '-' )      << "\n";
  QTreeWidgetItemIterator it( this );
  while ( *it ) {
    if ( !( ( *it )->text(0) ).isEmpty() ) {
      out << QString( SPACING_INFO * itemDepth( *it ), ' ' ) << ( *it )->text(0);
      if ( ( *it )->text(0)  == tr( "COLOR" ) ) {
        out << ": " << ( ( ( *it )->background(1) ).color() ).name();
      }
      else if ( !( ( *it )->text(1) ).isEmpty() ) out << ": " << ( *it )->text(1);
      out << "\n";
    }
    ++it;
  }
  out << "\n";
}

/*!
  \class SMESHGUI_MeshInfoDlg
  \brief Mesh information dialog box
*/

/*!
  \brief Constructor
  \param parent parent widget
  \param page specifies the dialog page to be shown at the start-up
*/
SMESHGUI_MeshInfoDlg::SMESHGUI_MeshInfoDlg( QWidget* parent, int page )
: QDialog( parent ), myActor( 0 )
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle( tr( "MESH_INFO" ) );
  setSizeGripEnabled( true );

  myTabWidget = new QTabWidget( this );

  // base info 

  myBaseInfo = new SMESHGUI_MeshInfo( myTabWidget );
  myTabWidget->addTab( myBaseInfo, tr( "BASE_INFO" ) );

  // elem info 
  
  QWidget* w = new QWidget( myTabWidget );

  myMode = new QButtonGroup( this );
  myMode->addButton( new QRadioButton( tr( "NODE_MODE" ), w ), NodeMode );
  myMode->addButton( new QRadioButton( tr( "ELEM_MODE" ), w ), ElemMode );
  myMode->button( NodeMode )->setChecked( true );
  myID = new QLineEdit( w );
  myID->setValidator( new SMESHGUI_IdValidator( this ) );

  int mode = SMESHGUI::resourceMgr()->integerValue( "SMESH", "mesh_elem_info", 1 );
  mode = qMin( 1, qMax( 0, mode ) );
  
  if ( mode == 0 ) 
    myElemInfo = new SMESHGUI_SimpleElemInfo( w );
  else
    myElemInfo = new SMESHGUI_TreeElemInfo( w );

  QGridLayout* elemLayout = new QGridLayout( w );
  elemLayout->setMargin( MARGIN );
  elemLayout->setSpacing( SPACING );
  elemLayout->addWidget( myMode->button( NodeMode ), 0, 0 );
  elemLayout->addWidget( myMode->button( ElemMode ), 0, 1 );
  elemLayout->addWidget( myID, 0, 2 );
  elemLayout->addWidget( myElemInfo, 1, 0, 1, 3 );
  
  myTabWidget->addTab( w, tr( "ELEM_INFO" ) );

  // additional info

  myAddInfo = new SMESHGUI_AddInfo( myTabWidget );
  myTabWidget->addTab( myAddInfo, tr( "ADDITIONAL_INFO" ) );

  // controls info

  myCtrlInfo = new SMESHGUI_CtrlInfo( myTabWidget );
  myTabWidget->addTab( myCtrlInfo, tr( "CTRL_INFO" ) );

  // buttons

  QPushButton* okBtn = new QPushButton( tr( "SMESH_BUT_OK" ), this );
  okBtn->setAutoDefault( true );
  okBtn->setDefault( true );
  okBtn->setFocus();
  QPushButton* dumpBtn = new QPushButton( tr( "BUT_DUMP_MESH" ), this );
  dumpBtn->setAutoDefault( true );
  QPushButton* helpBtn = new QPushButton( tr( "SMESH_BUT_HELP" ), this );
  helpBtn->setAutoDefault( true );

  QHBoxLayout* btnLayout = new QHBoxLayout;
  btnLayout->setSpacing( SPACING );
  btnLayout->setMargin( 0 );

  btnLayout->addWidget( okBtn );
  btnLayout->addWidget( dumpBtn );
  btnLayout->addStretch( 10 );
  btnLayout->addWidget( helpBtn );

  QVBoxLayout* l = new QVBoxLayout ( this );
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );
  l->addWidget( myTabWidget );
  l->addLayout( btnLayout );

  myTabWidget->setCurrentIndex( qMax( (int)BaseInfo, qMin( (int)ElemInfo, page ) ) );

  connect( okBtn,       SIGNAL( clicked() ),              this, SLOT( reject() ) );
  connect( dumpBtn,     SIGNAL( clicked() ),              this, SLOT( dump() ) );
  connect( helpBtn,     SIGNAL( clicked() ),              this, SLOT( help() ) );
  connect( myTabWidget, SIGNAL( currentChanged( int  ) ), this, SLOT( updateSelection() ) );
  connect( myMode,      SIGNAL( buttonClicked( int  ) ),  this, SLOT( modeChanged() ) );
  connect( myID,        SIGNAL( textChanged( QString ) ), this, SLOT( idChanged() ) );
  connect( SMESHGUI::GetSMESHGUI(),  SIGNAL( SignalDeactivateActiveDialog() ), this, SLOT( deactivate() ) );
  connect( SMESHGUI::GetSMESHGUI(),  SIGNAL( SignalCloseAllDialogs() ),        this, SLOT( reject() ) );
  connect( myElemInfo,  SIGNAL( itemInfo( int ) ),     this, SLOT( showItemInfo( int ) ) );
  connect( myElemInfo,  SIGNAL( itemInfo( QString ) ), this, SLOT( showItemInfo( QString ) ) );

  updateSelection();
}

/*!
  \brief Destructor
*/
SMESHGUI_MeshInfoDlg::~SMESHGUI_MeshInfoDlg()
{
}

/*!
  \brief Show mesh information
  \param IO interactive object
*/
void SMESHGUI_MeshInfoDlg::showInfo( const Handle(SALOME_InteractiveObject)& IO )
{
  if ( !IO.IsNull() )
    myIO = IO;

  SMESH::SMESH_IDSource_var obj = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( IO );
  if ( !CORBA::is_nil( obj ) )
  {
    myAddInfo->showInfo( obj );  // nb of nodes in a group can be computed by myAddInfo,
    myBaseInfo->showInfo( obj ); // and it will be used by myBaseInfo (IPAL52871)
    if ( myTabWidget->currentIndex() == CtrlInfo )
      myCtrlInfo->showInfo( obj );

    {
      myActor = SMESH::FindActorByEntry( IO->getEntry() );
      SVTK_Selector* selector = SMESH::GetSelector();
      QString ID;
      int nb = 0;
      if ( myActor && selector ) {
        nb = myMode->checkedId() == NodeMode ?
          SMESH::GetNameOfSelectedElements( selector, IO, ID ) :
          SMESH::GetNameOfSelectedNodes( selector, IO, ID );
      }
      myElemInfo->setSource( myActor ) ;
      if ( nb > 0 ) {
        myID->setText( ID.trimmed() );
        QSet<long> ids;
        QStringList idTxt = ID.split( " ", QString::SkipEmptyParts );
        foreach ( ID, idTxt )
          ids << ID.trimmed().toLong();
        myElemInfo->showInfo( ids, myMode->checkedId() == ElemMode );
      }
      else {
        myID->clear();
        myElemInfo->clear();
      }
    }
  }
}

/*!
  \brief Perform clean-up actions on the dialog box closing.
*/
void SMESHGUI_MeshInfoDlg::reject()
{
  LightApp_SelectionMgr* selMgr = SMESHGUI::selectionMgr();
  selMgr->clearFilters();
  SMESH::SetPointRepresentation( false );
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
    aViewWindow->SetSelectionMode( ActorSelection );
  QDialog::reject();
}

/*!
  \brief Process keyboard event
  \param e key press event
*/
void SMESHGUI_MeshInfoDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( !e->isAccepted() && e->key() == Qt::Key_F1 ) {
    e->accept();
    help();
  }
}

/*!
  \brief Reactivate dialog box, when mouse pointer goes into it.
*/
void SMESHGUI_MeshInfoDlg::enterEvent( QEvent* )
{
  //activate();
}

/*!
  \brief Setup selection mode depending on the current dialog box state.
*/
void SMESHGUI_MeshInfoDlg::updateSelection()
{
  LightApp_SelectionMgr* selMgr = SMESHGUI::selectionMgr();

  disconnect( selMgr, 0, this, 0 );
  selMgr->clearFilters();

  if ( myTabWidget->currentIndex() == BaseInfo || myTabWidget->currentIndex() == AddInfo || myTabWidget->currentIndex() == CtrlInfo ) {
    SMESH::SetPointRepresentation( false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
      aViewWindow->SetSelectionMode( ActorSelection );
  }
  else {
    if ( myMode->checkedId() == NodeMode ) {
      SMESH::SetPointRepresentation( true );
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
        aViewWindow->SetSelectionMode( NodeSelection );
    }
    else {
      SMESH::SetPointRepresentation( false );
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
        aViewWindow->SetSelectionMode( CellSelection );
    }
  }

  QString oldID = myID->text().trimmed();
  SMESH_Actor* oldActor = myActor;
  myID->clear();
  
  connect( selMgr, SIGNAL( currentSelectionChanged() ), this, SLOT( updateInfo() ) );
  updateInfo();
  
  if ( oldActor == myActor && myActor && !oldID.isEmpty() ) {
    myID->setText( oldID );
    idChanged();
  }
}

/*!
  \brief Show help page
*/
void SMESHGUI_MeshInfoDlg::help()
{
  SMESH::ShowHelpFile( ( myTabWidget->currentIndex() == BaseInfo || myTabWidget->currentIndex() == AddInfo ) ?
                       "mesh_infos_page.html#advanced_mesh_infos_anchor" : 
                       "mesh_infos_page.html#mesh_element_info_anchor" );
}

/*!
  \brief Show mesh information
*/
void SMESHGUI_MeshInfoDlg::updateInfo()
{
  SUIT_OverrideCursor wc;

  SALOME_ListIO selected;
  SMESHGUI::selectionMgr()->selectedObjects( selected );

  if ( selected.Extent() == 1 ) {
    Handle(SALOME_InteractiveObject) IO = selected.First();
    showInfo( IO );
  }
  else {
    showInfo( myIO );
  }
}

/*!
  \brief Activate dialog box
*/
void SMESHGUI_MeshInfoDlg::activate()
{
  SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();
  SMESHGUI::GetSMESHGUI()->SetActiveDialogBox( this );
  myTabWidget->setEnabled( true );
  updateSelection();
}

/*!
  \brief Deactivate dialog box
*/
void SMESHGUI_MeshInfoDlg::deactivate()
{
  myTabWidget->setEnabled( false );
  disconnect( SMESHGUI::selectionMgr(), SIGNAL( currentSelectionChanged() ), this, SLOT( updateInfo() ) );
}

/*!
  \brief Called when users switches between node / element modes.
*/
void SMESHGUI_MeshInfoDlg::modeChanged()
{
  myID->clear();
  updateSelection();
}

/*!
  \brief Caled when users prints mesh element ID in the corresponding field.
*/
void SMESHGUI_MeshInfoDlg::idChanged()
{
  SVTK_Selector* selector = SMESH::GetSelector();
  if ( myActor && selector ) {
    Handle(SALOME_InteractiveObject) IO = myActor->getIO();
    TColStd_MapOfInteger ID;
    QSet<long> ids;
    QStringList idTxt = myID->text().split( " ", QString::SkipEmptyParts );
    foreach ( QString tid, idTxt ) {
      long id = tid.trimmed().toLong();
      const SMDS_MeshElement* e = myMode->checkedId() == ElemMode ? 
        myActor->GetObject()->GetMesh()->FindElement( id ) :
        myActor->GetObject()->GetMesh()->FindNode( id );
      if ( e ) {
        ID.Add( id );
        ids << id;
      }
    }
    selector->AddOrRemoveIndex( IO, ID, false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() ) {
      aViewWindow->highlight( IO, true, true );
      aViewWindow->Repaint();
    }
    myElemInfo->showInfo( ids, myMode->checkedId() == ElemMode );
  }
}

void SMESHGUI_MeshInfoDlg::showItemInfo( int id )
{
  if ( id > 0 &&  myActor->GetObject()->GetMesh()->FindNode( id ) ) {
    myMode->button( NodeMode )->click();
    myID->setText( QString::number( id ) );
  }
}

void SMESHGUI_MeshInfoDlg::showItemInfo( const QString& theStr )
{
  if ( !theStr.isEmpty() ) {
    myMode->button( ElemMode )->click();
    myID->setText( theStr );
  }
}

void SMESHGUI_MeshInfoDlg::dump()
{
  SUIT_Application* app = SUIT_Session::session()->activeApplication();
  if ( !app ) return;
  SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study *>( app->activeStudy() );
  if ( !appStudy ) return;
  _PTR( Study ) aStudy = appStudy->studyDS();

  QStringList aFilters;
  aFilters.append( tr( "TEXT_FILES" ) );

  bool anIsBase = true;
  bool anIsElem = true;
  bool anIsAdd  = true;
  bool anIsCtrl = true;

  if ( SUIT_ResourceMgr* aResourceMgr = SMESHGUI::resourceMgr() ) {
    anIsBase = aResourceMgr->booleanValue( "SMESH", "info_dump_base", anIsBase );
    anIsElem = aResourceMgr->booleanValue( "SMESH", "info_dump_elem", anIsElem );
    anIsAdd  = aResourceMgr->booleanValue( "SMESH", "info_dump_add",  anIsAdd );
    anIsCtrl = aResourceMgr->booleanValue( "SMESH", "info_dump_ctrl", anIsCtrl );
  }

  DumpFileDlg fd( this );
  fd.setWindowTitle( tr( "SAVE_INFO" ) );
  fd.setNameFilters( aFilters );
  fd.myBaseChk->setChecked( anIsBase );
  fd.myElemChk->setChecked( anIsElem );
  fd.myAddChk ->setChecked( anIsAdd );
  fd.myCtrlChk->setChecked( anIsCtrl );
  if ( fd.exec() == QDialog::Accepted )
  {
    QString aFileName = fd.selectedFile();

    bool toBase = fd.myBaseChk->isChecked();
    bool toElem = fd.myElemChk->isChecked();
    bool toAdd  = fd.myAddChk->isChecked();
    bool toCtrl = fd.myCtrlChk->isChecked();

    if ( !aFileName.isEmpty() ) {
      QFileInfo aFileInfo( aFileName );
      if ( aFileInfo.isDir() )
        return;
 
      QFile aFile( aFileName );
      if ( !aFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
        return;
      
      QTextStream out( &aFile );
      
      if ( toBase ) myBaseInfo->saveInfo( out );
      if ( toElem ) myElemInfo->saveInfo( out );
      if ( toAdd )  myAddInfo ->saveInfo( out );
      if ( toCtrl ) myCtrlInfo->saveInfo( out );
    }
  }
}

/*!
  \class SMESHGUI_CtrlInfo
  \brief Class for the mesh controls information widget.
*/

/*!
  \brief Constructor
  \param parent parent widget
*/
SMESHGUI_CtrlInfo::SMESHGUI_CtrlInfo( QWidget* parent )
  : QFrame( parent ), myPlot( 0 ), myPlot3D( 0 )
{
  setFrameStyle( StyledPanel | Sunken );

  myMainLayout = new QGridLayout( this );
  myMainLayout->setMargin( MARGIN );
  myMainLayout->setSpacing( SPACING );

  // name
  QLabel* aNameLab = new QLabel( tr( "NAME_LAB" ), this );
  QLabel* aName = createField();
  aName->setMinimumWidth( 150 );
  myWidgets << aName;

  SUIT_ResourceMgr* aResMgr = SUIT_Session::session()->resourceMgr();
  QIcon aComputeIcon( aResMgr->loadPixmap( "SMESH", tr( "ICON_COMPUTE" ) ) );

  SMESH::FilterManager_var aFilterMgr = SMESH::GetFilterManager();

  // nodes info
  QLabel* aNodesLab = new QLabel( tr( "NODES_INFO" ), this );
  QLabel* aNodesFreeLab = new QLabel( tr( "NUMBER_OF_THE_FREE_NODES" ), this );
  QLabel* aNodesFree = createField();
  myWidgets << aNodesFree;
  myPredicates << aFilterMgr->CreateFreeNodes();
  //
  QLabel* aNodesNbConnLab = new QLabel( tr( "MAX_NODE_CONNECTIVITY" ), this );
  QLabel* aNodesNbConn = createField();
  myWidgets << aNodesNbConn;
  myNodeConnFunctor = aFilterMgr->CreateNodeConnectivityNumber();
  //
  QLabel* aNodesDoubleLab = new QLabel( tr( "NUMBER_OF_THE_DOUBLE_NODES" ), this );
  QLabel* aNodesDouble = createField();
  myWidgets << aNodesDouble;
  myPredicates << aFilterMgr->CreateEqualNodes();
  QLabel* aToleranceLab = new QLabel( tr( "DOUBLE_NODES_TOLERANCE" ), this );
  myToleranceWidget = new SMESHGUI_SpinBox( this );
  myToleranceWidget->RangeStepAndValidator(0.0000000001, 1000000.0, 0.0000001, "length_precision" );
  myToleranceWidget->setAcceptNames( false );
  myToleranceWidget->SetValue( SMESHGUI::resourceMgr()->doubleValue( "SMESH", "equal_nodes_tolerance", 1e-7 ) );

  // edges info
  QLabel* anEdgesLab = new QLabel( tr( "EDGES_INFO" ),  this );
  QLabel* anEdgesDoubleLab = new QLabel( tr( "NUMBER_OF_THE_DOUBLE_EDGES" ),     this );
  QLabel* anEdgesDouble = createField();
  myWidgets << anEdgesDouble;
  myPredicates << aFilterMgr->CreateEqualEdges();

  // faces info
  QLabel* aFacesLab = new QLabel( tr( "FACES_INFO" ), this );
  QLabel* aFacesDoubleLab = new QLabel( tr( "NUMBER_OF_THE_DOUBLE_FACES" ), this );
  QLabel* aFacesDouble = createField();
  myWidgets << aFacesDouble;
  myPredicates << aFilterMgr->CreateEqualFaces();
  QLabel* aFacesOverLab = new QLabel( tr( "NUMBER_OF_THE_OVER_CONSTRAINED" ), this );
  QLabel* aFacesOver = createField();
  myWidgets << aFacesOver;
  myPredicates << aFilterMgr->CreateOverConstrainedFace();
  QLabel* anAspectRatioLab = new QLabel( tr( "ASPECT_RATIO_HISTOGRAM" ), this );
  myPlot = createPlot( this );
  myAspectRatio = aFilterMgr->CreateAspectRatio();
 
  // volumes info
  QLabel* aVolumesLab = new QLabel( tr( "VOLUMES_INFO" ), this );
  QLabel* aVolumesDoubleLab = new QLabel( tr( "NUMBER_OF_THE_DOUBLE_VOLUMES" ), this );
  QLabel* aVolumesDouble = createField();
  myWidgets << aVolumesDouble;
  myPredicates << aFilterMgr->CreateEqualVolumes();
  QLabel* aVolumesOverLab = new QLabel( tr( "NUMBER_OF_THE_OVER_CONSTRAINED" ), this );
  QLabel* aVolumesOver = createField();
  myWidgets << aVolumesOver;
  myPredicates << aFilterMgr->CreateOverConstrainedVolume();
  QLabel* anAspectRatio3DLab = new QLabel( tr( "ASPECT_RATIO_3D_HISTOGRAM" ), this );
  myPlot3D = createPlot( this );
  myAspectRatio3D = aFilterMgr->CreateAspectRatio3D();

  QToolButton* aFreeNodesBtn = new QToolButton( this );
  aFreeNodesBtn->setIcon(aComputeIcon);
  myButtons << aFreeNodesBtn;       //0

  QToolButton* aNodesNbConnBtn = new QToolButton( this );
  aNodesNbConnBtn->setIcon(aComputeIcon);
  myButtons << aNodesNbConnBtn;     //1

  QToolButton* aDoubleNodesBtn = new QToolButton( this );
  aDoubleNodesBtn->setIcon(aComputeIcon);
  myButtons << aDoubleNodesBtn;     //2

  QToolButton* aDoubleEdgesBtn = new QToolButton( this );
  aDoubleEdgesBtn->setIcon(aComputeIcon);
  myButtons << aDoubleEdgesBtn;     //3

  QToolButton* aDoubleFacesBtn = new QToolButton( this );
  aDoubleFacesBtn->setIcon(aComputeIcon);
  myButtons << aDoubleFacesBtn;     //4

  QToolButton* aOverContFacesBtn = new QToolButton( this );
  aOverContFacesBtn->setIcon(aComputeIcon);
  myButtons << aOverContFacesBtn;   //5

  QToolButton* aComputeFaceBtn = new QToolButton( this );
  aComputeFaceBtn->setIcon(aComputeIcon);
  myButtons << aComputeFaceBtn;     //6

  QToolButton* aDoubleVolumesBtn = new QToolButton( this );
  aDoubleVolumesBtn->setIcon(aComputeIcon);
  myButtons << aDoubleVolumesBtn;   //7

  QToolButton* aOverContVolumesBtn = new QToolButton( this );
  aOverContVolumesBtn->setIcon(aComputeIcon);
  myButtons << aOverContVolumesBtn; //8

  QToolButton* aComputeVolumeBtn = new QToolButton( this );
  aComputeVolumeBtn->setIcon(aComputeIcon);
  myButtons << aComputeVolumeBtn;   //9

  connect( aComputeFaceBtn,   SIGNAL( clicked() ), this, SLOT( computeAspectRatio() ) );
  connect( aComputeVolumeBtn, SIGNAL( clicked() ), this, SLOT( computeAspectRatio3D() ) );
  connect( aFreeNodesBtn,     SIGNAL( clicked() ), this, SLOT( computeFreeNodesInfo() ) );
  connect( aNodesNbConnBtn,   SIGNAL( clicked() ), this, SLOT( computeNodesNbConnInfo() ) );
  connect( aDoubleNodesBtn,   SIGNAL( clicked() ), this, SLOT( computeDoubleNodesInfo() ) );
  connect( aDoubleEdgesBtn,   SIGNAL( clicked() ), this, SLOT( computeDoubleEdgesInfo() ) );
  connect( aDoubleFacesBtn,   SIGNAL( clicked() ), this, SLOT( computeDoubleFacesInfo() ) );
  connect( aOverContFacesBtn, SIGNAL( clicked() ), this, SLOT( computeOverConstrainedFacesInfo() ) );
  connect( aDoubleVolumesBtn, SIGNAL( clicked() ), this, SLOT( computeDoubleVolumesInfo() ) );
  connect( aOverContVolumesBtn,SIGNAL( clicked() ), this, SLOT( computeOverConstrainedVolumesInfo() ) );
  connect( myToleranceWidget, SIGNAL(valueChanged(double)), this, SLOT( setTolerance( double )));

  setFontAttributes( aNameLab );
  setFontAttributes( aNodesLab );
  setFontAttributes( anEdgesLab );
  setFontAttributes( aFacesLab );
  setFontAttributes( aVolumesLab );

  myMainLayout->addWidget( aNameLab,           0, 0 );       //0
  myMainLayout->addWidget( aName,              0, 1, 1, 2 ); //1
  myMainLayout->addWidget( aNodesLab,          1, 0, 1, 3 ); //2
  myMainLayout->addWidget( aNodesFreeLab,      2, 0 );       //3
  myMainLayout->addWidget( aNodesFree,         2, 1 );       //4
  myMainLayout->addWidget( aFreeNodesBtn,      2, 2 );       //5
  myMainLayout->addWidget( aNodesNbConnLab,    3, 0 );       //6
  myMainLayout->addWidget( aNodesNbConn,       3, 1 );       //7
  myMainLayout->addWidget( aNodesNbConnBtn,    3, 2 );       //8
  myMainLayout->addWidget( aNodesDoubleLab,    4, 0 );       //9
  myMainLayout->addWidget( aNodesDouble,       4, 1 );       //10
  myMainLayout->addWidget( aDoubleNodesBtn,    4, 2 );       //11
  myMainLayout->addWidget( aToleranceLab,      5, 0 );       //12
  myMainLayout->addWidget( myToleranceWidget,  5, 1 );       //13
  myMainLayout->addWidget( anEdgesLab,         6, 0, 1, 3 ); //14
  myMainLayout->addWidget( anEdgesDoubleLab,   7, 0 );       //15
  myMainLayout->addWidget( anEdgesDouble,      7, 1 );       //16
  myMainLayout->addWidget( aDoubleEdgesBtn,    7, 2 );       //17
  myMainLayout->addWidget( aFacesLab,          8, 0, 1, 3 ); //18
  myMainLayout->addWidget( aFacesDoubleLab,    9, 0 );       //19
  myMainLayout->addWidget( aFacesDouble,       9, 1 );       //20
  myMainLayout->addWidget( aDoubleFacesBtn,    9, 2 );       //21
  myMainLayout->addWidget( aFacesOverLab,      10, 0 );      //22
  myMainLayout->addWidget( aFacesOver,         10, 1 );      //23
  myMainLayout->addWidget( aOverContFacesBtn,  10, 2 );      //24
  myMainLayout->addWidget( anAspectRatioLab,   11, 0 );      //25
  myMainLayout->addWidget( aComputeFaceBtn,    11, 2 );      //26
  myMainLayout->addWidget( myPlot,             12, 0, 1, 3 );//27
  myMainLayout->addWidget( aVolumesLab,        13, 0, 1, 3 );//28
  myMainLayout->addWidget( aVolumesDoubleLab,  14, 0 );      //29
  myMainLayout->addWidget( aVolumesDouble,     14, 1 );      //30
  myMainLayout->addWidget( aDoubleVolumesBtn,  14, 2 );      //31
  myMainLayout->addWidget( aVolumesOverLab,    15, 0 );      //32
  myMainLayout->addWidget( aVolumesOver,       15, 1 );      //33
  myMainLayout->addWidget( aOverContVolumesBtn,15, 2 );      //34
  myMainLayout->addWidget( anAspectRatio3DLab, 16, 0 );      //35
  myMainLayout->addWidget( aComputeVolumeBtn,  16, 2 );      //36
  myMainLayout->addWidget( myPlot3D,           17, 0, 1, 3 );//37
 
  myMainLayout->setColumnStretch(  0,  0 );
  myMainLayout->setColumnStretch(  1,  5 );
  myMainLayout->setRowStretch   ( 11,  5 );
  myMainLayout->setRowStretch   ( 16,  5 );
  myMainLayout->setRowStretch   ( 17,  1 );

  clearInternal();
}

/*!
  \brief Destructor
*/
SMESHGUI_CtrlInfo::~SMESHGUI_CtrlInfo()
{}

/*!
  \brief Change widget font attributes (bold, ...).
  \param w widget
  \param attr font attributes (XORed flags)
*/
void SMESHGUI_CtrlInfo::setFontAttributes( QWidget* w )
{
  if ( w ) {
    QFont f = w->font();
    f.setBold( true );
    w->setFont( f );
  }
}

/*!
  \brief Create info field
  \return new info field
*/
QLabel* SMESHGUI_CtrlInfo::createField()
{
  QLabel* lab = new QLabel( this );
  lab->setFrameStyle( StyledPanel | Sunken );
  lab->setAlignment( Qt::AlignCenter );
  lab->setAutoFillBackground( true );
  QPalette pal = lab->palette();
  pal.setColor( QPalette::Window, QApplication::palette().color( QPalette::Active, QPalette::Base ) );
  lab->setPalette( pal );
  lab->setMinimumWidth( 60 );
  return lab;
}

/*!
  \brief Create QwtPlot
  \return new QwtPlot
*/
QwtPlot* SMESHGUI_CtrlInfo::createPlot( QWidget* parent )
{
  QwtPlot* aPlot = new QwtPlot( parent );
  aPlot->setMinimumSize( 100, 100 );
  QFont xFont = aPlot->axisFont( QwtPlot::xBottom );
  xFont.setPointSize( 5 );
  QFont yFont = aPlot->axisFont( QwtPlot::yLeft );
  yFont.setPointSize( 5 );
  aPlot->setAxisFont( QwtPlot::xBottom, xFont );
  aPlot->setAxisFont( QwtPlot::yLeft, yFont );
  aPlot->replot();
  return aPlot;
}

/*!
  \brief Show controls information on the selected object
*/
void SMESHGUI_CtrlInfo::showInfo( SMESH::SMESH_IDSource_ptr obj )
{
  clearInternal();

  myObject = SMESH::SMESH_IDSource::_duplicate( obj );
  if ( myObject->_is_nil() ) return;

  if ( _PTR(SObject) aSO = SMESH::FindSObject( obj ))
    myWidgets[0]->setText( aSO->GetName().c_str() );

  SMESH::SMESH_Mesh_var mesh = obj->GetMesh();
  if ( mesh->_is_nil() ) return;

  const bool meshLoaded = mesh->IsLoaded();
  if ( !meshLoaded ) // mesh not yet loaded from the hdf file
    // enable Compute buttons, just in case obj->GetNbElementsByType() fails
    for ( int i = 0; i < myButtons.count(); ++i )
      myButtons[i]->setEnabled( true );

  SMESH::long_array_var nbElemsByType = obj->GetNbElementsByType();
  if ( ! &nbElemsByType.in() ) return;

  const CORBA::Long ctrlLimit =
    meshLoaded ? SMESHGUI::resourceMgr()->integerValue( "SMESH", "info_controls_limit", 3000 ) : -1;

  // nodes info
  const CORBA::Long nbNodes =   nbElemsByType[ SMESH::NODE ];
  const CORBA::Long nbElems = ( nbElemsByType[ SMESH::EDGE ] +
                                nbElemsByType[ SMESH::FACE ] +
                                nbElemsByType[ SMESH::VOLUME ] );
  if ( nbNodes + nbElems > 0 ) {
    if ( Max( (int)nbNodes, (int)nbElems ) <= ctrlLimit ) {
      // free nodes
      computeFreeNodesInfo();
      computeNodesNbConnInfo();
      // double nodes
      if ( Max( (int)mesh->NbNodes(), (int)mesh->NbElements() ) <= ctrlLimit )
        computeDoubleNodesInfo();
    }
    else {
      myButtons[0]->setEnabled( true );
      myButtons[1]->setEnabled( true );
      myButtons[2]->setEnabled( true );
    }
  }
  else {
    for( int i=2; i<=11; i++)
      myMainLayout->itemAt(i)->widget()->setVisible( false );
  }

  // edges info
  if ( nbElemsByType[ SMESH::EDGE ] > 0 ) {
    // double edges
    if( nbElemsByType[ SMESH::EDGE ] <= ctrlLimit )
      computeDoubleEdgesInfo();
    else
      myButtons[3]->setEnabled( true );
  }
  else {
    for( int i=11; i<=14; i++)
      myMainLayout->itemAt(i)->widget()->setVisible( false );
  }
 
  // faces info
  if ( nbElemsByType[ SMESH::FACE ] > 0 ) {
    if ( nbElemsByType[ SMESH::FACE ] <= ctrlLimit ) {
      // double faces
      computeDoubleFacesInfo();
      // over constrained faces
      computeOverConstrainedFacesInfo();
      // aspect Ratio histogram
      computeAspectRatio();
    }
    else {
      myButtons[4]->setEnabled( true );
      myButtons[5]->setEnabled( true );
      myButtons[6]->setEnabled( true );
    }
#ifdef DISABLE_PLOT2DVIEWER
    myMainLayout->setRowStretch(12,0);
    for( int i=25; i<=27; i++)
      myMainLayout->itemAt(i)->widget()->setVisible( false );
#endif
  }
  else {
    myMainLayout->setRowStretch(12,0);
    for( int i=18; i<=27; i++)
      myMainLayout->itemAt(i)->widget()->setVisible( false );
  }

  // volumes info
  if ( nbElemsByType[ SMESH::VOLUME ] > 0 ) {
    if ( nbElemsByType[ SMESH::VOLUME ] <= ctrlLimit ) {
      // double volumes
      computeDoubleVolumesInfo();
      // over constrained volumes
      computeOverConstrainedVolumesInfo();
      // aspect Ratio 3D histogram
      computeAspectRatio3D();
     }
     else {
       myButtons[7]->setEnabled( true );
       myButtons[8]->setEnabled( true );
       myButtons[9]->setEnabled( true );
     }
#ifdef DISABLE_PLOT2DVIEWER
    myMainLayout->setRowStretch(17,0);
    for( int i=35; i<=37; i++)
      myMainLayout->itemAt(i)->widget()->setVisible( false );
#endif
  }
  else {
    myMainLayout->setRowStretch(17,0);
    for( int i=28; i<=37; i++)
      myMainLayout->itemAt(i)->widget()->setVisible( false );
  }
}

//================================================================================
/*!
 * \brief Computes and shows nb of elements satisfying a given predicate
 *  \param [in] ft - a predicate type (SMESH::FunctorType)
 *  \param [in] iBut - index of one of myButtons to disable
 *  \param [in] iWdg - index of one of myWidgets to show the computed number
 */
//================================================================================

void SMESHGUI_CtrlInfo::computeNb( int ft, int iBut, int iWdg )
{
  myButtons[ iBut ]->setEnabled( false );
  myWidgets[ iWdg ]->setText( "" );
  if ( myObject->_is_nil() ) return;

  SUIT_OverrideCursor wc;

  SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
  if ( !mesh->_is_nil() && !mesh->IsLoaded() )
  {
    mesh->Load();
    this->showInfo( myObject ); // try to show all values
    if ( !myWidgets[ iWdg ]->text().isEmpty() )
      return; // <ft> predicate already computed
  }
  // look for a predicate of type <ft>
  for ( int i = 0; i < myPredicates.count(); ++i )
    if ( myPredicates[i]->GetFunctorType() == ft )
    {
      CORBA::Long nb = myPredicates[i]->NbSatisfying( myObject );
      myWidgets[ iWdg ]->setText( QString::number( nb ));
    }
}

void SMESHGUI_CtrlInfo::computeFreeNodesInfo()
{
  computeNb( SMESH::FT_FreeNodes, 0, 1 );
}

void SMESHGUI_CtrlInfo::computeDoubleNodesInfo()
{
  computeNb( SMESH::FT_EqualNodes, 2, 3 );
}

void SMESHGUI_CtrlInfo::computeDoubleEdgesInfo()
{
  computeNb( SMESH::FT_EqualEdges, 3, 4 );
}

void SMESHGUI_CtrlInfo::computeDoubleFacesInfo()
{
  computeNb( SMESH::FT_EqualFaces, 4, 5 );
}

void SMESHGUI_CtrlInfo::computeOverConstrainedFacesInfo()
{
  computeNb( SMESH::FT_OverConstrainedFace, 5, 6 );
}

void SMESHGUI_CtrlInfo::computeDoubleVolumesInfo()
{
  computeNb( SMESH::FT_EqualVolumes, 7, 7 );
}

void SMESHGUI_CtrlInfo::computeOverConstrainedVolumesInfo()
{
  computeNb( SMESH::FT_OverConstrainedVolume, 8, 8 );
}

void SMESHGUI_CtrlInfo::computeNodesNbConnInfo()
{
  myButtons[ 1 ]->setEnabled( false );
  myWidgets[ 2 ]->setText( "" );
  SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
  if ( mesh->_is_nil() ) return;
  if ( !mesh->IsLoaded() )
  {
    mesh->Load();
    this->showInfo( myObject ); // try to show all values
    if ( !myWidgets[ 2 ]->text().isEmpty() )
      return; // already computed
  }
  myNodeConnFunctor->SetMesh( mesh );
  SMESH::Histogram_var histogram =
    myNodeConnFunctor->GetLocalHistogram( 1, /*isLogarithmic=*/false, myObject );

  myWidgets[ 2 ]->setText( QString::number( histogram[0].max ));
}

void SMESHGUI_CtrlInfo::computeAspectRatio()
{
#ifndef DISABLE_PLOT2DVIEWER
  myButtons[6]->setEnabled( false );

  if ( myObject->_is_nil() ) return;

  SUIT_OverrideCursor wc;

  Plot2d_Histogram* aHistogram = getHistogram( myAspectRatio );
  if ( aHistogram && !aHistogram->isEmpty() ) {
    QwtPlotItem* anItem = aHistogram->createPlotItem();
    anItem->attach( myPlot );
    myPlot->replot();
  }
  delete aHistogram;
#endif
}

void SMESHGUI_CtrlInfo::computeAspectRatio3D()
{
#ifndef DISABLE_PLOT2DVIEWER
  myButtons[9]->setEnabled( false );

  if ( myObject->_is_nil() ) return;

  SUIT_OverrideCursor wc;

  Plot2d_Histogram* aHistogram = getHistogram( myAspectRatio3D );
  if ( aHistogram && !aHistogram->isEmpty() ) {
    QwtPlotItem* anItem = aHistogram->createPlotItem();
    anItem->attach( myPlot3D );
    myPlot3D->replot();
  }
  delete aHistogram;
#endif
}

/*!
  \brief Internal clean-up (reset widget)
*/
void SMESHGUI_CtrlInfo::clearInternal()
{
  for( int i=0; i<=35; i++)
    myMainLayout->itemAt(i)->widget()->setVisible( true );
  for( int i=0; i<=9; i++)
    myButtons[i]->setEnabled( false );
  myPlot->detachItems();
  myPlot3D->detachItems();
  myPlot->replot();
  myPlot3D->replot();
  myWidgets[0]->setText( QString() );
  for ( int i = 1; i < myWidgets.count(); i++ )
    myWidgets[i]->setText( "" );
  myMainLayout->setRowStretch(11,5);
  myMainLayout->setRowStretch(16,5);
}

void SMESHGUI_CtrlInfo::setTolerance( double theTolerance )
{
  //SMESH::long_array_var anElems = getElementsByType( SMESH::NODE );
  myButtons[1]->setEnabled( true );
  myWidgets[2]->setText("");
}

#ifndef DISABLE_PLOT2DVIEWER
Plot2d_Histogram* SMESHGUI_CtrlInfo::getHistogram( SMESH::NumericalFunctor_ptr aNumFun )
{
  SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
  if ( mesh->_is_nil() ) return 0;
  if ( !mesh->IsLoaded() )
    mesh->Load();
  aNumFun->SetMesh( mesh );

  CORBA::Long cprecision = 6;
  if ( SMESHGUI::resourceMgr()->booleanValue( "SMESH", "use_precision", false ) ) 
    cprecision = SMESHGUI::resourceMgr()->integerValue( "SMESH", "controls_precision", -1 );
  aNumFun->SetPrecision( cprecision );

  int nbIntervals = SMESHGUI::resourceMgr()->integerValue( "SMESH", "scalar_bar_num_colors", false );

  SMESH::Histogram_var histogramVar = aNumFun->GetLocalHistogram( nbIntervals,
                                                                  /*isLogarithmic=*/false,
                                                                  myObject );
  Plot2d_Histogram* aHistogram = new Plot2d_Histogram();
  aHistogram->setColor( palette().color( QPalette::Highlight ) );
  if ( &histogramVar.in() )
  {
    for ( size_t i = 0, nb = histogramVar->length(); i < nb; i++ )
      aHistogram->addPoint( 0.5 * ( histogramVar[i].min + histogramVar[i].max ), histogramVar[i].nbEvents );
    if ( histogramVar->length() >= 2 )
      aHistogram->setWidth( ( histogramVar[0].max - histogramVar[0].min ) * 0.8 );
  }
  return aHistogram;
}
#endif

void SMESHGUI_CtrlInfo::saveInfo( QTextStream &out ) {
  out << QString( 20, '-' ) << "\n";
  out << tr( "CTRL_INFO"  ) << "\n";
  out << QString( 20, '-' ) << "\n";
  out <<                                 tr( "NAME_LAB" )                       << "  " << myWidgets[0]->text() << "\n";
  out <<                                 tr( "NODES_INFO" )                     << "\n";
  out << QString( SPACING_INFO, ' ' ) << tr( "NUMBER_OF_THE_FREE_NODES" )       << ": " << myWidgets[1]->text() << "\n";
  out << QString( SPACING_INFO, ' ' ) << tr( "NUMBER_OF_THE_DOUBLE_NODES" )     << ": " << myWidgets[2]->text() << "\n";
  out <<                                 tr( "EDGES_INFO" )                     << "\n";
  out << QString( SPACING_INFO, ' ' ) << tr( "NUMBER_OF_THE_DOUBLE_EDGES" )     << ": " << myWidgets[3]->text() << "\n";
  out <<                                 tr( "FACES_INFO" )                     << "\n";
  out << QString( SPACING_INFO, ' ' ) << tr( "NUMBER_OF_THE_DOUBLE_FACES" )     << ": " << myWidgets[4]->text() << "\n";
  out << QString( SPACING_INFO, ' ' ) << tr( "NUMBER_OF_THE_OVER_CONSTRAINED" ) << ": " << myWidgets[5]->text() << "\n";
  out <<                                 tr( "VOLUMES_INFO" )                   << "\n";
  out << QString( SPACING_INFO, ' ' ) << tr( "NUMBER_OF_THE_DOUBLE_VOLUMES" )   << ": " << myWidgets[6]->text() << "\n";
  out << QString( SPACING_INFO, ' ' ) << tr( "NUMBER_OF_THE_OVER_CONSTRAINED" ) << ": " << myWidgets[7]->text() << "\n";
}

/*!
  \class SMESHGUI_CtrlInfoDlg
  \brief Controls information dialog box
*/

/*!
  \brief Constructor
  \param parent parent widget
*/
SMESHGUI_CtrlInfoDlg::SMESHGUI_CtrlInfoDlg( QWidget* parent )
: QDialog( parent )
{
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle( tr( "CTRL_INFO" ) );
  setMinimumSize( 400, 600 );

  myCtrlInfo = new SMESHGUI_CtrlInfo( this );
  
  // buttons
  QPushButton* okBtn = new QPushButton( tr( "SMESH_BUT_OK" ), this );
  okBtn->setAutoDefault( true );
  okBtn->setDefault( true );
  okBtn->setFocus();
  QPushButton* dumpBtn = new QPushButton( tr( "BUT_DUMP_MESH" ), this );
  dumpBtn->setAutoDefault( true );
  QPushButton* helpBtn = new QPushButton( tr( "SMESH_BUT_HELP" ), this );
  helpBtn->setAutoDefault( true );

  QHBoxLayout* btnLayout = new QHBoxLayout;
  btnLayout->setSpacing( SPACING );
  btnLayout->setMargin( 0 );

  btnLayout->addWidget( okBtn );
  btnLayout->addWidget( dumpBtn );
  btnLayout->addStretch( 10 );
  btnLayout->addWidget( helpBtn );

  QVBoxLayout* l = new QVBoxLayout ( this );
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );
  l->addWidget( myCtrlInfo );
  l->addLayout( btnLayout );

  connect( okBtn,   SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( dumpBtn, SIGNAL( clicked() ), this, SLOT( dump() ) );
  connect( helpBtn, SIGNAL( clicked() ), this, SLOT( help() ) );
  connect( SMESHGUI::GetSMESHGUI(), SIGNAL( SignalDeactivateActiveDialog() ), this, SLOT( deactivate() ) );
  connect( SMESHGUI::GetSMESHGUI(), SIGNAL( SignalCloseAllDialogs() ),        this, SLOT( reject() ) );

  updateSelection();
}

/*!
  \brief Destructor
*/
SMESHGUI_CtrlInfoDlg::~SMESHGUI_CtrlInfoDlg()
{
}

/*!
  \brief Show controls information
  \param IO interactive object
*/
void SMESHGUI_CtrlInfoDlg::showInfo( const Handle(SALOME_InteractiveObject)& IO )
{  
  if ( SMESH::SMESH_IDSource_var obj = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( IO ) )
    myCtrlInfo->showInfo( obj );
}

/*!
  \brief Perform clean-up actions on the dialog box closing.
*/
void SMESHGUI_CtrlInfoDlg::reject()
{
  SMESH::SetPointRepresentation( false );
  QDialog::reject();
}

/*!
  \brief Setup selection mode depending on the current dialog box state.
*/
void SMESHGUI_CtrlInfoDlg::updateSelection()
{
  LightApp_SelectionMgr* selMgr = SMESHGUI::selectionMgr();
  disconnect( selMgr, 0, this, 0 );
  SMESH::SetPointRepresentation( false );  
  connect( selMgr, SIGNAL( currentSelectionChanged() ), this, SLOT( updateInfo() ) );
  updateInfo();  
}

/*!
  \brief Show mesh information
*/
void SMESHGUI_CtrlInfoDlg::updateInfo()
{
  SUIT_OverrideCursor wc;

  SALOME_ListIO selected;
  SMESHGUI::selectionMgr()->selectedObjects( selected );

  if ( selected.Extent() == 1 ) {
    Handle(SALOME_InteractiveObject) IO = selected.First();
    showInfo( IO );
  }
}

/*!
  \brief Activate dialog box
*/
void SMESHGUI_CtrlInfoDlg::activate()
{
  SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();
  SMESHGUI::GetSMESHGUI()->SetActiveDialogBox( this );
  updateSelection();
}

/*!
  \brief Deactivate dialog box
*/
void SMESHGUI_CtrlInfoDlg::deactivate()
{
  disconnect( SMESHGUI::selectionMgr(), SIGNAL( currentSelectionChanged() ), this, SLOT( updateInfo() ) );
}

/*!
 * \brief Dump contents into a file
 */
void SMESHGUI_CtrlInfoDlg::dump()
{
  SUIT_Application* app = SUIT_Session::session()->activeApplication();
  if ( !app ) return;
  SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study *>( app->activeStudy() );
  if ( !appStudy ) return;
  _PTR( Study ) aStudy = appStudy->studyDS();

  QStringList aFilters;
  aFilters.append( tr( "TEXT_FILES" ) );

  DumpFileDlg fd( this );
  fd.setWindowTitle( tr( "SAVE_INFO" ) );
  fd.setNameFilters( aFilters );
  fd.myBaseChk->hide();
  fd.myElemChk->hide();
  fd.myAddChk ->hide();
  fd.myCtrlChk->hide();
  if ( fd.exec() == QDialog::Accepted )
  {
    QString aFileName = fd.selectedFile();
    if ( !aFileName.isEmpty() ) {
      QFileInfo aFileInfo( aFileName );
      if ( aFileInfo.isDir() )
        return;
 
      QFile aFile( aFileName );
      if ( !aFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
        return;
      
      QTextStream out( &aFile );
      myCtrlInfo->saveInfo( out );
    }
  }
}

/*!
 * \brief Show help
 */
void SMESHGUI_CtrlInfoDlg::help()
{
  SMESH::ShowHelpFile("mesh_infos_page.html#mesh_quality_info_anchor");
}
