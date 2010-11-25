//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  File   : SMESHGUI_MeshInfo.cxx
//  Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

#include "SMESHGUI_MeshInfo.h"

#include "SMESH_Actor.h"
#include "SMESHGUI.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMDSAbs_ElementType.hxx"
#include "SMDS_Mesh.hxx"

#include <LightApp_SelectionMgr.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SVTK_ViewWindow.h>

#include <SALOMEDSClient_Study.hxx>

#include <QApplication>
#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

const int SPACING = 6;
const int MARGIN  = 9;

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

  // object
  QLabel* aNameLab     = new QLabel( tr( "NAME_LAB" ), this );
  QLabel* aName        = createField();
  aName->setMinimumWidth( 150 );
  QLabel* aObjLab      = new QLabel( tr( "OBJECT_LAB" ), this );
  QLabel* aObj         = createField();
  aObj->setMinimumWidth( 150 );
  myWidgets[0] << aNameLab << aName;
  myWidgets[1] << aObjLab  << aObj;

  // nodes
  QWidget* aNodesLine  = createLine();
  QLabel*  aNodesLab   = new QLabel( tr( "NODES_LAB" ), this );
  QLabel*  aNodes      = createField();
  myWidgets[2] << aNodesLine;
  myWidgets[3] << aNodesLab << aNodes;

  // elements
  QWidget* aElemLine   = createLine();
  QLabel*  aElemLab    = new QLabel( tr( "ELEMENTS_LAB" ),  this );
  QLabel*  aElemTotal  = new QLabel( tr( "TOTAL_LAB" ),     this );
  QLabel*  aElemLin    = new QLabel( tr( "LINEAR_LAB" ),    this );
  QLabel*  aElemQuad   = new QLabel( tr( "QUADRATIC_LAB" ), this );
  myWidgets[4] << aElemLine;
  myWidgets[5] << aElemLab << aElemTotal << aElemLin << aElemQuad;

  // ... 0D elements
  QWidget* a0DLine     = createLine();
  QLabel*  a0DLab      = new QLabel( tr( "0D_LAB" ), this );
  QLabel*  a0DTotal    = createField();
  myWidgets[6] << a0DLine;
  myWidgets[7] << a0DLab << a0DTotal;

  // ... 1D elements
  QWidget* a1DLine     = createLine();
  QLabel*  a1DLab      = new QLabel( tr( "1D_LAB" ), this );
  QLabel*  a1DTotal    = createField();
  QLabel*  a1DLin      = createField();
  QLabel*  a1DQuad     = createField();
  myWidgets[8] << a1DLine;
  myWidgets[9] << a1DLab << a1DTotal << a1DLin << a1DQuad;

  // ... 2D elements
  QWidget* a2DLine     = createLine();
  QLabel*  a2DLab      = new QLabel( tr( "2D_LAB" ), this );
  QLabel*  a2DTotal    = createField();
  QLabel*  a2DLin      = createField();
  QLabel*  a2DQuad     = createField();
  QLabel*  a2DTriLab   = new QLabel( tr( "TRIANGLES_LAB" ), this );
  QLabel*  a2DTriTotal = createField();
  QLabel*  a2DTriLin   = createField();
  QLabel*  a2DTriQuad  = createField();
  QLabel*  a2DQuaLab   = new QLabel( tr( "QUADRANGLES_LAB" ), this );
  QLabel*  a2DQuaTotal = createField();
  QLabel*  a2DQuaLin   = createField();
  QLabel*  a2DQuaQuad  = createField();
  QLabel*  a2DPolLab   = new QLabel( tr( "POLYGONS_LAB" ), this );
  QLabel*  a2DPolTotal = createField();
  myWidgets[10] << a2DLine;
  myWidgets[11] << a2DLab    << a2DTotal    << a2DLin    << a2DQuad;
  myWidgets[12] << a2DTriLab << a2DTriTotal << a2DTriLin << a2DTriQuad;
  myWidgets[13] << a2DQuaLab << a2DQuaTotal << a2DQuaLin << a2DQuaQuad;
  myWidgets[14] << a2DPolLab << a2DPolTotal;

  // ... 3D elements
  QWidget* a3DLine     = createLine();
  QLabel*  a3DLab      = new QLabel( tr( "3D_LAB" ), this );
  QLabel*  a3DTotal    = createField();
  QLabel*  a3DLin      = createField();
  QLabel*  a3DQuad     = createField();
  QLabel*  a3DTetLab   = new QLabel( tr( "TETRAHEDRONS_LAB" ), this );
  QLabel*  a3DTetTotal = createField();
  QLabel*  a3DTetLin   = createField();
  QLabel*  a3DTetQuad  = createField();
  QLabel*  a3DHexLab   = new QLabel( tr( "HEXAHEDONRS_LAB" ), this );
  QLabel*  a3DHexTotal = createField();
  QLabel*  a3DHexLin   = createField();
  QLabel*  a3DHexQuad  = createField();
  QLabel*  a3DPyrLab   = new QLabel( tr( "PYRAMIDS_LAB" ), this );
  QLabel*  a3DPyrTotal = createField();
  QLabel*  a3DPyrLin   = createField();
  QLabel*  a3DPyrQuad  = createField();
  QLabel*  a3DPriLab   = new QLabel( tr( "PRISMS_LAB" ), this );
  QLabel*  a3DPriTotal = createField();
  QLabel*  a3DPriLin   = createField();
  QLabel*  a3DPriQuad  = createField();
  QLabel*  a3DPolLab   = new QLabel( tr( "POLYHEDRONS_LAB" ), this );
  QLabel*  a3DPolTotal = createField();
  myWidgets[15] << a3DLine;
  myWidgets[16] << a3DLab    << a3DTotal    << a3DLin    << a3DQuad;
  myWidgets[17] << a3DTetLab << a3DTetTotal << a3DTetLin << a3DTetQuad;
  myWidgets[18] << a3DHexLab << a3DHexTotal << a3DHexLin << a3DHexQuad;
  myWidgets[19] << a3DPyrLab << a3DPyrTotal << a3DPyrLin << a3DPyrQuad;
  myWidgets[20] << a3DPriLab << a3DPriTotal << a3DPriLin << a3DPriQuad;
  myWidgets[21] << a3DPolLab << a3DPolTotal;

  setFontAttributes( aNameLab,   Bold );
  setFontAttributes( aObjLab,    Bold );
  setFontAttributes( aNodesLab,  Bold );
  setFontAttributes( aElemLab,   Bold );
  setFontAttributes( aElemTotal, Italic );
  setFontAttributes( aElemLin,   Italic );
  setFontAttributes( aElemQuad,  Italic );
  setFontAttributes( a0DLab,     Bold );
  setFontAttributes( a1DLab,     Bold );
  setFontAttributes( a2DLab,     Bold );
  setFontAttributes( a3DLab,     Bold );

  l->addWidget( aNameLab,     0, 0 );
  l->addWidget( aName,        0, 1, 1, 3 );
  l->addWidget( aObjLab,      1, 0 );
  l->addWidget( aObj,         1, 1, 1, 3 );
  l->addWidget( aNodesLine,   2, 0, 1, 4 );
  l->addWidget( aNodesLab,    3, 0 );
  l->addWidget( aNodes,       3, 1 );
  l->addWidget( aElemLine,    4, 0, 1, 4 );
  l->addWidget( aElemLab,     5, 0 );
  l->addWidget( aElemTotal,   5, 1 );
  l->addWidget( aElemLin,     5, 2 );
  l->addWidget( aElemQuad,    5, 3 );
  l->addWidget( a0DLine,      6, 1, 1, 3 );
  l->addWidget( a0DLab,       7, 0 );
  l->addWidget( a0DTotal,     7, 1 );
  l->addWidget( a1DLine,      8, 1, 1, 3 );
  l->addWidget( a1DLab,       9, 0 );
  l->addWidget( a1DTotal,     9, 1 );
  l->addWidget( a1DLin,       9, 2 );
  l->addWidget( a1DQuad,      9, 3 );
  l->addWidget( a2DLine,     10, 1, 1, 3 );
  l->addWidget( a2DLab,      11, 0 );
  l->addWidget( a2DTotal,    11, 1 );
  l->addWidget( a2DLin,      11, 2 );
  l->addWidget( a2DQuad,     11, 3 );
  l->addWidget( a2DTriLab,   12, 0 );
  l->addWidget( a2DTriTotal, 12, 1 );
  l->addWidget( a2DTriLin,   12, 2 );
  l->addWidget( a2DTriQuad,  12, 3 );
  l->addWidget( a2DQuaLab,   13, 0 );
  l->addWidget( a2DQuaTotal, 13, 1 );
  l->addWidget( a2DQuaLin,   13, 2 );
  l->addWidget( a2DQuaQuad,  13, 3 );
  l->addWidget( a2DPolLab,   14, 0 );
  l->addWidget( a2DPolTotal, 14, 1 );
  l->addWidget( a3DLine,     15, 1, 1, 3 );
  l->addWidget( a3DLab,      16, 0 );
  l->addWidget( a3DTotal,    16, 1 );
  l->addWidget( a3DLin,      16, 2 );
  l->addWidget( a3DQuad,     16, 3 );
  l->addWidget( a3DTetLab,   17, 0 );
  l->addWidget( a3DTetTotal, 17, 1 );
  l->addWidget( a3DTetLin,   17, 2 );
  l->addWidget( a3DTetQuad,  17, 3 );
  l->addWidget( a3DHexLab,   18, 0 );
  l->addWidget( a3DHexTotal, 18, 1 );
  l->addWidget( a3DHexLin,   18, 2 );
  l->addWidget( a3DHexQuad,  18, 3 );
  l->addWidget( a3DPyrLab,   19, 0 );
  l->addWidget( a3DPyrTotal, 19, 1 );
  l->addWidget( a3DPyrLin,   19, 2 );
  l->addWidget( a3DPyrQuad,  19, 3 );
  l->addWidget( a3DPriLab,   20, 0 );
  l->addWidget( a3DPriTotal, 20, 1 );
  l->addWidget( a3DPriLin,   20, 2 );
  l->addWidget( a3DPriQuad,  20, 3 );
  l->addWidget( a3DPolLab,   21, 0 );
  l->addWidget( a3DPolTotal, 21, 1 );
  l->setColumnStretch( 0, 0 );
  l->setColumnStretch( 1, 5 );
  l->setColumnStretch( 2, 5 );
  l->setColumnStretch( 3, 5 );
  l->setRowStretch( 22, 5 );

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
    _PTR(SObject) sobj = ObjectToSObject( obj );
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
      case SMESH::NODE:
	objType = tr( "OBJECT_GROUP_NODES" );
	break;
      case SMESH::EDGE:
	objType = tr( "OBJECT_GROUP_EDGES" );
	break;
      case SMESH::FACE:
	objType = tr( "OBJECT_GROUP_FACES" );
	break;
      case SMESH::VOLUME:
	objType = tr( "OBJECT_GROUP_VOLUMES" );
	break;
      case SMESH::ELEM0D:
	objType = tr( "OBJECT_GROUP_0DELEMS" );
	break;
      default:
	objType = tr( "OBJECT_GROUP" );
	break;
      }
      myWidgets[iObject][iSingle]->setProperty( "text", objType );
    }
    SMESH::long_array_var info = obj->GetMeshInfo();
    myWidgets[iNodes][iTotal]->setProperty( "text", QString::number( info[SMDSEntity_Node] ) );
    myWidgets[i0D][iTotal]->setProperty( "text", QString::number( info[SMDSEntity_0D] ) );
    long nbEdges = info[SMDSEntity_Edge] + info[SMDSEntity_Quad_Edge];
    myWidgets[i1D][iTotal]->setProperty( "text", QString::number( nbEdges ) );
    myWidgets[i1D][iLinear]->setProperty( "text", QString::number( info[SMDSEntity_Edge] ) );
    myWidgets[i1D][iQuadratic]->setProperty( "text", QString::number( info[SMDSEntity_Quad_Edge] ) );
    long nbTriangles   = info[SMDSEntity_Triangle]   + info[SMDSEntity_Quad_Triangle];
    long nbQuadrangles = info[SMDSEntity_Quadrangle] + info[SMDSEntity_Quad_Quadrangle];
    long nb2DLinear    = info[SMDSEntity_Triangle] + info[SMDSEntity_Quadrangle] + info[SMDSEntity_Polygon];
    long nb2DQuadratic = info[SMDSEntity_Quad_Triangle] + info[SMDSEntity_Quad_Quadrangle];
    myWidgets[i2D][iTotal]->setProperty( "text", QString::number( nb2DLinear + nb2DQuadratic ) );
    myWidgets[i2D][iLinear]->setProperty( "text", QString::number( nb2DLinear ) );
    myWidgets[i2D][iQuadratic]->setProperty( "text", QString::number( nb2DQuadratic ) );
    myWidgets[i2DTriangles][iTotal]->setProperty( "text", QString::number( nbTriangles ) );
    myWidgets[i2DTriangles][iLinear]->setProperty( "text", QString::number( info[SMDSEntity_Triangle] ) );
    myWidgets[i2DTriangles][iQuadratic]->setProperty( "text", QString::number( info[SMDSEntity_Quad_Triangle] ) );
    myWidgets[i2DQuadrangles][iTotal]->setProperty( "text", QString::number( nbQuadrangles ) );
    myWidgets[i2DQuadrangles][iLinear]->setProperty( "text", QString::number( info[SMDSEntity_Quadrangle] ) );
    myWidgets[i2DQuadrangles][iQuadratic]->setProperty( "text", QString::number( info[SMDSEntity_Quad_Quadrangle] ) );
    myWidgets[i2DPolygons][iTotal]->setProperty( "text", QString::number( info[SMDSEntity_Polygon] ) );
    long nbTetrahedrons = info[SMDSEntity_Tetra]   + info[SMDSEntity_Quad_Tetra];
    long nbHexahedrons  = info[SMDSEntity_Hexa]    + info[SMDSEntity_Quad_Hexa];
    long nbPyramids     = info[SMDSEntity_Pyramid] + info[SMDSEntity_Quad_Pyramid];
    long nbPrisms       = info[SMDSEntity_Penta]   + info[SMDSEntity_Quad_Penta];
    long nb3DLinear     = info[SMDSEntity_Tetra] + info[SMDSEntity_Hexa] + info[SMDSEntity_Pyramid] + info[SMDSEntity_Penta] + info[SMDSEntity_Polyhedra];
    long nb3DQuadratic  = info[SMDSEntity_Quad_Tetra] + info[SMDSEntity_Quad_Hexa] + info[SMDSEntity_Quad_Pyramid] + info[SMDSEntity_Quad_Penta];
    myWidgets[i3D][iTotal]->setProperty( "text", QString::number( nb3DLinear + nb3DQuadratic ) );
    myWidgets[i3D][iLinear]->setProperty( "text", QString::number( nb3DLinear ) );
    myWidgets[i3D][iQuadratic]->setProperty( "text", QString::number( nb3DQuadratic ) );
    myWidgets[i3DTetrahedrons][iTotal]->setProperty( "text", QString::number( nbTetrahedrons ) );
    myWidgets[i3DTetrahedrons][iLinear]->setProperty( "text", QString::number( info[SMDSEntity_Tetra] ) );
    myWidgets[i3DTetrahedrons][iQuadratic]->setProperty( "text", QString::number( info[SMDSEntity_Quad_Tetra] ) );
    myWidgets[i3DHexahedrons][iTotal]->setProperty( "text", QString::number( nbHexahedrons ) );
    myWidgets[i3DHexahedrons][iLinear]->setProperty( "text", QString::number( info[SMDSEntity_Hexa] ) );
    myWidgets[i3DHexahedrons][iQuadratic]->setProperty( "text", QString::number( info[SMDSEntity_Quad_Hexa] ) );
    myWidgets[i3DPyramids][iTotal]->setProperty( "text", QString::number( nbPyramids ) );
    myWidgets[i3DPyramids][iLinear]->setProperty( "text", QString::number( info[SMDSEntity_Pyramid] ) );
    myWidgets[i3DPyramids][iQuadratic]->setProperty( "text", QString::number( info[SMDSEntity_Quad_Pyramid] ) );
    myWidgets[i3DPrisms][iTotal]->setProperty( "text", QString::number( nbPrisms ) );
    myWidgets[i3DPrisms][iLinear]->setProperty( "text", QString::number( info[SMDSEntity_Penta] ) );
    myWidgets[i3DPrisms][iQuadratic]->setProperty( "text", QString::number( info[SMDSEntity_Quad_Penta] ) );
    myWidgets[i3DPolyhedrons][iTotal]->setProperty( "text", QString::number( info[SMDSEntity_Polyhedra] ) );
  }
}

/*!
  \brief Reset the widget to the initial state (nullify all fields).
*/
void SMESHGUI_MeshInfo::clear()
{
  myWidgets[iName][iSingle]->setProperty( "text", QString() );
  myWidgets[iObject][iSingle]->setProperty( "text", QString() );
  myWidgets[iNodes][iTotal]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i0D][iTotal]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i1D][iTotal]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i1D][iLinear]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i1D][iQuadratic]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2D][iTotal]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2D][iLinear]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2D][iQuadratic]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DTriangles][iTotal]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DTriangles][iLinear]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DTriangles][iQuadratic]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DQuadrangles][iTotal]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DQuadrangles][iLinear]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DQuadrangles][iQuadratic]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i2DPolygons][iTotal]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3D][iTotal]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3D][iLinear]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3D][iQuadratic]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DTetrahedrons][iTotal]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DTetrahedrons][iLinear]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DTetrahedrons][iQuadratic]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DHexahedrons][iTotal]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DHexahedrons][iLinear]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DHexahedrons][iQuadratic]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DPyramids][iTotal]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DPyramids][iLinear]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DPyramids][iQuadratic]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DPrisms][iTotal]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DPrisms][iLinear]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DPrisms][iQuadratic]->setProperty( "text", QString::number( 0 ) );
  myWidgets[i3DPolyhedrons][iTotal]->setProperty( "text", QString::number( 0 ) );
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

/*!
  \class SMESHGUI_ElemInfo
  \brief Base class for the mesh element information widget.
*/

/*!
  \brief Constructor
  \param parent parent widget
*/
SMESHGUI_ElemInfo::SMESHGUI_ElemInfo( QWidget* parent )
: QWidget( parent ), myActor( 0 ), myID( 0 ), myIsElement( -1 )
{
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
    myID = 0;
    myIsElement = -1;
    clear();
  }
}

/*!
  \brief Show mesh element information
  \param long id mesh node / element ID
  \param isElem show mesh element information if \c true or mesh node information if \c false
*/
void SMESHGUI_ElemInfo::showInfo( long id, bool isElem )
{
  myID = id;
  myIsElement = isElem;
}

/*!
  \fn void SMESHGUI_ElemInfo::clear()
  \brief Clear mesh element information widget
*/

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
  myInfo = new QTextBrowser( this );
  QVBoxLayout* l = new QVBoxLayout( this );
  l->setMargin( 0 );
  l->addWidget( myInfo );
}

/*!
  \brief Show mesh element information
  \param long id mesh node / element ID
  \param isElem show mesh element information if \c true or mesh node information if \c false
*/
void SMESHGUI_SimpleElemInfo::showInfo( long id, bool isElem )
{
  if ( myID == id && myIsElement == isElem ) return;

  SMESHGUI_ElemInfo::showInfo( id, isElem );

  clear();
  
  if ( myActor ) {
    int precision = SMESHGUI::resourceMgr()->integerValue( "SMESH", "length_precision", 6 );
    if ( !isElem ) {
      //
      // show node info
      //
      const SMDS_MeshElement* e = myActor->GetObject()->GetMesh()->FindNode( id );
      if ( !e ) return;
      const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( e );
      
      // node ID
      myInfo->append( QString( "<b>%1 #%2</b>" ).arg( tr( "NODE" ) ).arg( id ) );
      // separator
      myInfo->append( "" );
      // coordinates
      myInfo->append( QString( "<b>%1:</b> (%2, %3, %4)" ).arg( tr( "COORDINATES" ) ).
		      arg( node->X(), 0, precision > 0 ? 'f' : 'g', qAbs( precision ) ).
		      arg( node->Y(), 0, precision > 0 ? 'f' : 'g', qAbs( precision ) ).
		      arg( node->Z(), 0, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
      // separator
      myInfo->append( "" );
      // connectivity
      Connectivity connectivity = nodeConnectivity( node );
      if ( !connectivity.isEmpty() ) {
	myInfo->append( QString( "<b>%1:</b>" ).arg( tr( "CONNECTIVITY" ) ) );
	QString con = formatConnectivity( connectivity, SMDSAbs_0DElement );
	if ( !con.isEmpty() )
	  myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "0D_ELEMENTS" ) ).arg( con ) );
	con = formatConnectivity( connectivity, SMDSAbs_Edge );
	if ( !con.isEmpty() )
	  myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "EDGES" ) ).arg( con ) );
	con = formatConnectivity( connectivity, SMDSAbs_Face );
	if ( !con.isEmpty() )
	  myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "FACES" ) ).arg( con ) );
	con = formatConnectivity( connectivity, SMDSAbs_Volume );
	if ( !con.isEmpty() )
	  myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "VOLUMES" ) ).arg( con ) );
      }
      else {
	myInfo->append( QString( "<b>%1</b>" ).arg( tr( "FREE_NODE" ) ).arg( id ) );
      }
    }
    else {
      //
      // show element info
      // 
      const SMDS_MeshElement* e = myActor->GetObject()->GetMesh()->FindElement( id );
      if ( !e ) return;

      // element ID && type
      QString stype;
      switch( e->GetType() ) {
      case SMDSAbs_0DElement:
	stype = tr( "0D ELEMENT" ); break;
      case SMDSAbs_Edge:
	stype = tr( "EDGE" ); break;
      case SMDSAbs_Face:
	stype = tr( "FACE" ); break;
      case SMDSAbs_Volume:
	stype = tr( "VOLUME" ); break;
      default: 
	break;
      }
      if ( stype.isEmpty() ) return;
      myInfo->append( QString( "<b>%1 #%2</b>" ).arg( stype ).arg( id ) );
      // separator
      myInfo->append( "" );
      // geometry type
      QString gtype;
      switch( e->GetEntityType() ) {
      case SMDSEntity_Triangle:
      case SMDSEntity_Quad_Triangle:
	gtype = tr( "TRIANGLE" ); break;
      case SMDSEntity_Quadrangle:
      case SMDSEntity_Quad_Quadrangle:
	gtype = tr( "QUADRANGLE" ); break;
      case SMDSEntity_Polygon:
      case SMDSEntity_Quad_Polygon:
	gtype = tr( "QUADRANGLE" ); break;
      case SMDSEntity_Tetra:
      case SMDSEntity_Quad_Tetra:
	gtype = tr( "TETRAHEDRON" ); break;
      case SMDSEntity_Pyramid:
      case SMDSEntity_Quad_Pyramid:
	gtype = tr( "PYRAMID" ); break;
      case SMDSEntity_Hexa:
      case SMDSEntity_Quad_Hexa:
	gtype = tr( "HEXAHEDRON" ); break;
      case SMDSEntity_Penta:
      case SMDSEntity_Quad_Penta:
	gtype = tr( "PRISM" ); break;
      case SMDSEntity_Polyhedra:
      case SMDSEntity_Quad_Polyhedra:
	gtype = tr( "POLYHEDRON" ); break;
      default: 
	break;
      }
      if ( !gtype.isEmpty() )
	myInfo->append( QString( "<b>%1:</b> %2" ).arg( tr( "TYPE" ) ).arg( gtype ) );
      // quadratic flag and gravity center (any element except 0D)
      if ( e->GetEntityType() > SMDSEntity_0D && e->GetEntityType() < SMDSEntity_Last ) {
	// quadratic flag
	myInfo->append( QString( "<b>%1?</b> %2" ).arg( tr( "QUADRATIC" ) ).arg( e->IsQuadratic() ? tr( "YES" ) : tr( "NO" ) ) );
	// separator
	myInfo->append( "" );
	// gravity center
	XYZ gc = gravityCenter( e );
	myInfo->append( QString( "<b>%1:</b> (%2, %3, %4)" ).arg( tr( "GRAVITY_CENTER" ) ).arg( gc.x() ).arg( gc.y() ).arg( gc.z() ) );
      }
      // separator
      myInfo->append( "" );
      // connectivity
      SMDS_ElemIteratorPtr nodeIt = e->nodesIterator();
      for ( int idx = 1; nodeIt->more(); idx++ ) {
	const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( nodeIt->next() );
	// node number and ID
	myInfo->append( QString( "<b>%1 %2/%3</b> - #%4" ).arg( tr( "NODE" ) ).arg( idx ).arg( e->NbNodes() ).arg( node->GetID() ) );
	// node coordinates
	myInfo->append( QString( "<b>%1:</b> (%2, %3, %4)" ).arg( tr( "COORDINATES" ) ).
			arg( node->X(), 0, precision > 0 ? 'f' : 'g', qAbs( precision ) ).
			arg( node->Y(), 0, precision > 0 ? 'f' : 'g', qAbs( precision ) ).
			arg( node->Z(), 0, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
	// node connectivity
	Connectivity connectivity = nodeConnectivity( node );
	if ( !connectivity.isEmpty() ) {
	  myInfo->append( QString( "<b>%1:</b>" ).arg( tr( "CONNECTIVITY" ) ) );
	  QString con = formatConnectivity( connectivity, SMDSAbs_0DElement );
	  if ( !con.isEmpty() )
	    myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "0D_ELEMENTS" ) ).arg( con ) );
	  con = formatConnectivity( connectivity, SMDSAbs_Edge );
	  if ( !con.isEmpty() )
	    myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "EDGES" ) ).arg( con ) );
	  con = formatConnectivity( connectivity, SMDSAbs_Face );
	  if ( !con.isEmpty() )
	    myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "FACES" ) ).arg( con ) );
	  con = formatConnectivity( connectivity, SMDSAbs_Volume );
	  if ( !con.isEmpty() )
	    myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "VOLUMES" ) ).arg( con ) );
	}
	else {
	  myInfo->append( QString( "<b>%1</b>" ).arg( tr( "FREE_NODE" ) ).arg( id ) );
	}
	// separator
	myInfo->append( "" );
      }
    }
  }
}

/*!
  \brief Clear mesh element information widget
*/
void SMESHGUI_SimpleElemInfo::clear()
{
  myInfo->clear();
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
  myInfo = new QTreeWidget( this );
  myInfo->setColumnCount( 2 );
  myInfo->setHeaderLabels( QStringList() << tr( "PROPERTY" ) << tr( "VALUE" ) );
  myInfo->header()->setStretchLastSection( true );
  myInfo->header()->setResizeMode( 0, QHeaderView::ResizeToContents );
  myInfo->setItemDelegate( new ItemDelegate( myInfo ) );
  QVBoxLayout* l = new QVBoxLayout( this );
  l->setMargin( 0 );
  l->addWidget( myInfo );
}

/*!
  \brief Show mesh element information
  \param long id mesh node / element ID
  \param isElem show mesh element information if \c true or mesh node information if \c false
*/
void SMESHGUI_TreeElemInfo::showInfo( long id, bool isElem )
{
  if ( myID == id && myIsElement == isElem ) return;

  SMESHGUI_ElemInfo::showInfo( id, isElem );

  clear();
  
  if ( myActor ) {
    int precision = SMESHGUI::resourceMgr()->integerValue( "SMESH", "length_precision", 6 );
    if ( !isElem ) {
      //
      // show node info
      //
      const SMDS_MeshElement* e = myActor->GetObject()->GetMesh()->FindNode( id );
      if ( !e ) return;
      const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( e );
      
      // node ID
      QTreeWidgetItem* nodeItem = createItem( 0, -1 );
      nodeItem->setText( 0, tr( "NODE" ) );
      nodeItem->setText( 1, QString( "#%1" ).arg( id ) );
      nodeItem->setExpanded( true );
      // coordinates
      QTreeWidgetItem* coordItem = createItem( nodeItem, 0 );
      coordItem->setText( 0, tr( "COORDINATES" ) );
      coordItem->setExpanded( true );
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
      QTreeWidgetItem* conItem = createItem( nodeItem, 0 );
      conItem->setText( 0, tr( "CONNECTIVITY" ) );
      conItem->setExpanded( true );
      Connectivity connectivity = nodeConnectivity( node );
      if ( !connectivity.isEmpty() ) {
	QString con = formatConnectivity( connectivity, SMDSAbs_0DElement );
	if ( !con.isEmpty() ) {
	  QTreeWidgetItem* i = createItem( conItem );
	  i->setText( 0, tr( "0D_ELEMENTS" ) );
	  i->setText( 1, con );
	}
	con = formatConnectivity( connectivity, SMDSAbs_Edge );
	if ( !con.isEmpty() ) {
	  QTreeWidgetItem* i = createItem( conItem );
	  i->setText( 0, tr( "EDGES" ) );
	  i->setText( 1, con );
	}
	con = formatConnectivity( connectivity, SMDSAbs_Face );
	if ( !con.isEmpty() ) {
	  QTreeWidgetItem* i = createItem( conItem );
	  i->setText( 0, tr( "FACES" ) );
	  i->setText( 1, con );
	}
	con = formatConnectivity( connectivity, SMDSAbs_Volume );
	if ( !con.isEmpty() ) {
	  QTreeWidgetItem* i = createItem( conItem );
	  i->setText( 0, tr( "VOLUMES" ) );
	  i->setText( 1, con );
	}
      }
      else {
	conItem->setText( 1, tr( "FREE_NODE" ) );
      }
    }
    else {
      //
      // show element info
      // 
      const SMDS_MeshElement* e = myActor->GetObject()->GetMesh()->FindElement( id );
      if ( !e ) return;

      // element ID && type
      QString stype;
      switch( e->GetType() ) {
      case SMDSAbs_0DElement:
	stype = tr( "0D ELEMENT" ); break;
      case SMDSAbs_Edge:
	stype = tr( "EDGE" ); break;
      case SMDSAbs_Face:
	stype = tr( "FACE" ); break;
      case SMDSAbs_Volume:
	stype = tr( "VOLUME" ); break;
      default: 
	break;
      }
      if ( stype.isEmpty() ) return;
      QTreeWidgetItem* elemItem = createItem( 0, -1 );
      elemItem->setText( 0, stype );
      elemItem->setText( 1, QString( "#%1" ).arg( id ) );
      elemItem->setExpanded( true );
      // geometry type
      QString gtype;
      switch( e->GetEntityType() ) {
      case SMDSEntity_Triangle:
      case SMDSEntity_Quad_Triangle:
	gtype = tr( "TRIANGLE" ); break;
      case SMDSEntity_Quadrangle:
      case SMDSEntity_Quad_Quadrangle:
	gtype = tr( "QUADRANGLE" ); break;
      case SMDSEntity_Polygon:
      case SMDSEntity_Quad_Polygon:
	gtype = tr( "QUADRANGLE" ); break;
      case SMDSEntity_Tetra:
      case SMDSEntity_Quad_Tetra:
	gtype = tr( "TETRAHEDRON" ); break;
      case SMDSEntity_Pyramid:
      case SMDSEntity_Quad_Pyramid:
	gtype = tr( "PYRAMID" ); break;
      case SMDSEntity_Hexa:
      case SMDSEntity_Quad_Hexa:
	gtype = tr( "HEXAHEDRON" ); break;
      case SMDSEntity_Penta:
      case SMDSEntity_Quad_Penta:
	gtype = tr( "PRISM" ); break;
      case SMDSEntity_Polyhedra:
      case SMDSEntity_Quad_Polyhedra:
	gtype = tr( "POLYHEDRON" ); break;
      default: 
	break;
      }
      if ( !gtype.isEmpty() ) {
	QTreeWidgetItem* typeItem = createItem( elemItem, 0 );
	typeItem->setText( 0, tr( "TYPE" ) );
	typeItem->setText( 1, gtype );
      }
      // quadratic flag and gravity center (any element except 0D)
      if ( e->GetEntityType() > SMDSEntity_0D && e->GetEntityType() < SMDSEntity_Last ) {
	// quadratic flag
	QTreeWidgetItem* quadItem = createItem( elemItem, 0 );
	quadItem->setText( 0, tr( "QUADRATIC" ) );
	quadItem->setText( 1, e->IsQuadratic() ? tr( "YES" ) : tr( "NO" ) );
	// gravity center
	XYZ gc = gravityCenter( e );
	QTreeWidgetItem* gcItem = createItem( elemItem, 0 );
	gcItem->setText( 0, tr( "GRAVITY_CENTER" ) );
	gcItem->setExpanded( true );
	QTreeWidgetItem* xItem = createItem( gcItem );
	xItem->setText( 0, "X" );
	xItem->setText( 1, QString::number( gc.x(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
	QTreeWidgetItem* yItem = createItem( gcItem );
	yItem->setText( 0, "Y" );
	yItem->setText( 1, QString::number( gc.y(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
	QTreeWidgetItem* zItem = createItem( gcItem );
	zItem->setText( 0, "Z" );
	zItem->setText( 1, QString::number( gc.z(), precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
      }
      // connectivity
      QTreeWidgetItem* conItem = createItem( elemItem, 0 );
      conItem->setText( 0, tr( "CONNECTIVITY" ) );
      conItem->setExpanded( true );
      SMDS_ElemIteratorPtr nodeIt = e->nodesIterator();
      for ( int idx = 1; nodeIt->more(); idx++ ) {
	const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( nodeIt->next() );
	// node number and ID
	QTreeWidgetItem* nodeItem = createItem( conItem, 0 );
	nodeItem->setText( 0, QString( "%1 %2/%3" ).arg( tr( "NODE" ) ).arg( idx ).arg( e->NbNodes() ) );
	nodeItem->setText( 1, QString( "#%1" ).arg( node->GetID() ) );
	//nodeItem->setExpanded( true );
	// node coordinates
	QTreeWidgetItem* coordItem = createItem( nodeItem );
	coordItem->setText( 0, tr( "COORDINATES" ) );
	coordItem->setExpanded( true );
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
	nconItem->setText( 0, tr( "CONNECTIVITY" ) );
	nconItem->setExpanded( true );
	Connectivity connectivity = nodeConnectivity( node );
	if ( !connectivity.isEmpty() ) {
	  QString con = formatConnectivity( connectivity, SMDSAbs_0DElement );
	  if ( !con.isEmpty() ) {
	    QTreeWidgetItem* i = createItem( nconItem );
	    i->setText( 0, tr( "0D_ELEMENTS" ) );
	    i->setText( 1, con );
	  }
	  con = formatConnectivity( connectivity, SMDSAbs_Edge );
	  if ( !con.isEmpty() ) {
	    QTreeWidgetItem* i = createItem( nconItem );
	    i->setText( 0, tr( "EDGES" ) );
	    i->setText( 1, con );
	  }
	  con = formatConnectivity( connectivity, SMDSAbs_Face );
	  if ( !con.isEmpty() ) {
	    QTreeWidgetItem* i = createItem( nconItem );
	    i->setText( 0, tr( "FACES" ) );
	    i->setText( 1, con );
	  }
	  con = formatConnectivity( connectivity, SMDSAbs_Volume );
	  if ( !con.isEmpty() ) {
	    QTreeWidgetItem* i = createItem( nconItem );
	    i->setText( 0, tr( "VOLUMES" ) );
	    i->setText( 1, con );
	  }
	}
      }
    }
  }
}

/*!
  \brief Clear mesh element information widget
*/
void SMESHGUI_TreeElemInfo::clear()
{
  myInfo->clear();
  myInfo->repaint();
}

/*!
  \brief Create new tree item.
  \param parnt parent tree widget item
  \param column item column to be set bold, if it is -1, bold font will be set for all columns
  \return new tree widget item
*/
QTreeWidgetItem* SMESHGUI_TreeElemInfo::createItem( QTreeWidgetItem* parent, int column )
{
  QTreeWidgetItem* item;
  if ( parent )
    item = new QTreeWidgetItem( parent );
  else
    item = new QTreeWidgetItem( myInfo );

  item->setFlags( item->flags() | Qt::ItemIsEditable );

  QFont f = item->font( 0 );
  f.setBold( true );
  if ( column >= 0 && column < myInfo->columnCount() ) {
    item->setFont( column, f );
  }
  else if ( column == -1 ) {
    for ( int i = 0; i < myInfo->columnCount(); i++ )
      item->setFont( i, f );
  }
  return item;
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
  myID->setValidator( new SMESHGUI_IdValidator( this, 1 ) );

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

  QPushButton* okBtn = new QPushButton( tr( "SMESH_BUT_OK" ), this );
  okBtn->setAutoDefault( true );
  okBtn->setDefault( true );
  okBtn->setFocus();
  QPushButton* helpBtn = new QPushButton( tr( "SMESH_BUT_HELP" ), this );
  helpBtn->setAutoDefault( true );

  QHBoxLayout* btnLayout = new QHBoxLayout;
  btnLayout->setSpacing( SPACING );
  btnLayout->setMargin( 0 );

  btnLayout->addWidget( okBtn );
  btnLayout->addStretch( 10 );
  btnLayout->addWidget( helpBtn );

  QVBoxLayout* l = new QVBoxLayout ( this );
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );
  l->addWidget( myTabWidget );
  l->addStretch();
  l->addLayout( btnLayout );

  myTabWidget->setCurrentIndex( qMax( (int)BaseInfo, qMin( (int)ElemInfo, page ) ) );

  connect( okBtn,       SIGNAL( clicked() ),              this, SLOT( reject() ) );
  connect( helpBtn,     SIGNAL( clicked() ),              this, SLOT( help() ) );
  connect( myTabWidget, SIGNAL( currentChanged( int  ) ), this, SLOT( updateSelection() ) );
  connect( myMode,      SIGNAL( buttonClicked( int  ) ),  this, SLOT( modeChanged() ) );
  connect( myID,        SIGNAL( textEdited( QString  ) ), this, SLOT( idChanged() ) );
  connect( SMESHGUI::GetSMESHGUI(),  SIGNAL( SignalDeactivateActiveDialog() ), this, SLOT( deactivate() ) );
  connect( SMESHGUI::GetSMESHGUI(),  SIGNAL( SignalCloseAllDialogs() ),        this, SLOT( reject() ) );

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
  SMESH::SMESH_IDSource_var obj = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( IO );
  if ( !CORBA::is_nil( obj ) ) {
    myBaseInfo->showInfo( obj );
    
    myActor = SMESH::FindActorByEntry( IO->getEntry() );
    SVTK_Selector* selector = SMESH::GetViewWindow()->GetSelector();
    QString ID;
    int nb = 0;
    if ( myActor && selector ) {
      nb = myMode->checkedId() == NodeMode ? 
	SMESH::GetNameOfSelectedElements( selector, IO, ID ) :
	SMESH::GetNameOfSelectedNodes( selector, IO, ID );
    }
    if ( nb == 1 ) {
      myID->setText( ID.trimmed() );
      myElemInfo->setSource( myActor ) ;
      myElemInfo->showInfo( ID.toLong(), myMode->checkedId() == ElemMode );
    }
    else {
      myID->clear();
      myElemInfo->clear();
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
  activate();
}

/*!
  \brief Setup selection mode depending on the current dialog box state.
*/
void SMESHGUI_MeshInfoDlg::updateSelection()
{
  LightApp_SelectionMgr* selMgr = SMESHGUI::selectionMgr();

  disconnect( selMgr, 0, this, 0 );
  selMgr->clearFilters();

  if ( myTabWidget->currentIndex() == BaseInfo ) {
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

  int oldID = myID->text().toLong();
  SMESH_Actor* oldActor = myActor;
  myID->clear();
  
  connect( selMgr, SIGNAL( currentSelectionChanged() ), this, SLOT( updateInfo() ) );
  updateInfo();
  
  if ( oldActor == myActor && myActor && oldID ) {
    myID->setText( QString::number( oldID ) );
    idChanged();
  }
}

/*!
  \brief Show help page
*/
void SMESHGUI_MeshInfoDlg::help()
{
  SMESH::ShowHelpFile( myTabWidget->currentIndex() == BaseInfo ?
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
//   else {
//     myBaseInfo->clear();
//     myElemInfo->clear();
//   }
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
  SVTK_Selector* selector = SMESH::GetViewWindow()->GetSelector();
  if ( myActor && selector ) {
    Handle(SALOME_InteractiveObject) IO = myActor->getIO();
    TColStd_MapOfInteger ID;
    ID.Add( myID->text().toLong() );
    selector->AddOrRemoveIndex( IO, ID, false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
      aViewWindow->highlight( IO, true, true );
    myElemInfo->showInfo( myID->text().toLong(), myMode->checkedId() == ElemMode );
  }
}
