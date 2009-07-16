//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
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
// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_MeshInfosDlg.cxx
// Author : Nicolas BARBEROU
// SMESH includes
//
#include "SMESHGUI_MeshInfosDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>

#include <LightApp_SelectionMgr.h>
#include <LightApp_Application.h>
#include <SALOME_ListIO.hxx>

// SALOME KERNEL includes
#include <SALOMEDSClient_Study.hxx>

// Qt includes
#include <QGroupBox>
#include <QLabel>
#include <QFrame>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QKeyEvent>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)

#define COLONIZE(str)   (QString(str).contains(":") > 0 ? QString(str) : QString(str) + " :" )
#define SPACING 6
#define MARGIN  11

//=================================================================================
// function : SMESHGUI_MeshInfosDlg()
// purpose  : Constructor
//=================================================================================
SMESHGUI_MeshInfosDlg::SMESHGUI_MeshInfosDlg(SMESHGUI* theModule): 
  QDialog(SMESH::GetDesktop(theModule)),
  mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
  mySMESHGUI(theModule)
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle(tr("SMESH_MESHINFO_TITLE"));
  setSizeGripEnabled(true);

  myStartSelection = true;
  myIsActiveWindow = true;

  QVBoxLayout* aTopLayout = new QVBoxLayout(this);
  aTopLayout->setSpacing(SPACING);  aTopLayout->setMargin(MARGIN);

  // select button & label
  QPixmap image0(SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH",tr("ICON_SELECT")));
  mySelectBtn = new QPushButton(this);
  mySelectBtn->setIcon(image0);
  mySelectBtn->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

  mySelectLab = new QLabel(this);
  mySelectLab->setAlignment(Qt::AlignCenter);
  QFont fnt = mySelectLab->font(); fnt.setBold(true);
  mySelectLab->setFont(fnt);

  QHBoxLayout* aSelectLayout = new QHBoxLayout;
  aSelectLayout->setMargin(0); aSelectLayout->setSpacing(0);
  aSelectLayout->addWidget(mySelectBtn);
  aSelectLayout->addWidget(mySelectLab);

  // top widget stack
  myWGStack = new QStackedWidget(this);

  // no valid selection
  QWidget* myBadWidget = new QWidget(myWGStack);
  QVBoxLayout* aBadLayout = new QVBoxLayout(myBadWidget);
  QLabel* myBadLab = new QLabel(tr("SMESH_BAD_SELECTION"), myBadWidget);
  myBadLab->setAlignment(Qt::AlignCenter);
  myBadLab->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  aBadLayout->addWidget(myBadLab);
  myWGStack->addWidget(myBadWidget);

  // mesh
  myMeshWidget = new QWidget(myWGStack);
  QGridLayout* aMeshLayout = new QGridLayout(myMeshWidget);
  aMeshLayout->setSpacing(SPACING);  aMeshLayout->setMargin(0);
  myWGStack->addWidget(myMeshWidget);

  // --> name
  QLabel* myMeshNameLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NAME")), myMeshWidget);
  myMeshName    = new QLabel(myMeshWidget);
  myMeshName->setMinimumWidth(100);
  QFrame* line1 = new QFrame(myMeshWidget);
  line1->setFrameStyle(QFrame::HLine | QFrame::Sunken);

  // --> nodes
  QLabel* myMeshNbNodesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NODES")), myMeshWidget);
  myMeshNbNodes  = new QLabel(myMeshWidget);
  myMeshNbNodes->setMinimumWidth(100);
  QFrame* line12 = new QFrame(myMeshWidget);
  line12->setFrameStyle(QFrame::HLine | QFrame::Sunken);

  // --> 0D elements
  QLabel* myMeshNb0DElemsLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_0DELEMS")), myMeshWidget);
  myMeshNb0DElems = new QLabel(myMeshWidget);
  myMeshNb0DElems->setMinimumWidth(100);

  // --> header with orders
  QLabel* myMeshOrder0Lab = new QLabel(tr("SMESH_MESHINFO_ORDER0"), myMeshWidget);
  QLabel* myMeshOrder1Lab = new QLabel(tr("SMESH_MESHINFO_ORDER1"), myMeshWidget);
  QLabel* myMeshOrder2Lab = new QLabel(tr("SMESH_MESHINFO_ORDER2"), myMeshWidget);
  QFont fnti = myMeshOrder0Lab->font(); fnti.setItalic(true);
  myMeshOrder0Lab->setFont(fnti);
  myMeshOrder1Lab->setFont(fnti);
  myMeshOrder2Lab->setFont(fnti);

  // --> edges
  QLabel* myMeshNbEdgesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_EDGES")), myMeshWidget);
  myMeshNbEdges    = new QLabel(myMeshWidget);
  myMeshNbEdges->setMinimumWidth(100);
  myMeshNbEdges1   = new QLabel(myMeshWidget);
  myMeshNbEdges1->setMinimumWidth(100);
  myMeshNbEdges2   = new QLabel(myMeshWidget);
  myMeshNbEdges2->setMinimumWidth(100);

  // --> faces
  myMeshFacesGroup = new QGroupBox(tr("SMESH_MESHINFO_FACES"), myMeshWidget);
  QGridLayout* myMeshFacesGroupLayout = new QGridLayout(myMeshFacesGroup);
  myMeshFacesGroupLayout->setSpacing(SPACING);  myMeshFacesGroupLayout->setMargin(MARGIN);

  // --> faces --> total
  QLabel* myMeshNbFacesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TOTAL")), myMeshFacesGroup);
  myMeshNbFacesLab->setFont(fnt);
  myMeshNbFaces    = new QLabel(myMeshFacesGroup);
  myMeshNbFaces->setMinimumWidth(100);
  myMeshNbFaces->setFont(fnt);
  myMeshNbFaces1   = new QLabel(myMeshFacesGroup);
  myMeshNbFaces1->setMinimumWidth(100);
  myMeshNbFaces1->setFont(fnt);
  myMeshNbFaces2   = new QLabel(myMeshFacesGroup);
  myMeshNbFaces2->setMinimumWidth(100);
  myMeshNbFaces2->setFont(fnt);

  // --> faces --> triangles
  QLabel* myMeshNbTrianglesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TRIANGLES")), myMeshFacesGroup);
  myMeshNbTriangles    = new QLabel(myMeshFacesGroup);
  myMeshNbTriangles->setMinimumWidth(100);
  myMeshNbTriangles1   = new QLabel(myMeshFacesGroup);
  myMeshNbTriangles1->setMinimumWidth(100);
  myMeshNbTriangles2   = new QLabel(myMeshFacesGroup);
  myMeshNbTriangles2->setMinimumWidth(100);

  // --> faces --> quadrangles
  QLabel* myMeshNbQuadranglesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_QUADRANGLES")), myMeshFacesGroup);
  myMeshNbQuadrangles    = new QLabel(myMeshFacesGroup);
  myMeshNbQuadrangles->setMinimumWidth(100);
  myMeshNbQuadrangles1   = new QLabel(myMeshFacesGroup);
  myMeshNbQuadrangles1->setMinimumWidth(100);
  myMeshNbQuadrangles2   = new QLabel(myMeshFacesGroup);
  myMeshNbQuadrangles2->setMinimumWidth(100);

  // --> faces --> polygons
  QLabel* myMeshNbPolygonesLab = new QLabel( COLONIZE( tr( "SMESH_MESHINFO_POLYGONES" ) ), myMeshFacesGroup );
  myMeshNbPolygones      = new QLabel( myMeshFacesGroup );
  myMeshNbPolygones->setMinimumWidth( 100 );

  myMeshFacesGroupLayout->addWidget(myMeshNbFacesLab,       0, 0);
  myMeshFacesGroupLayout->addWidget(myMeshNbFaces,          0, 1);
  myMeshFacesGroupLayout->addWidget(myMeshNbFaces1,         0, 2);
  myMeshFacesGroupLayout->addWidget(myMeshNbFaces2,         0, 3);
  myMeshFacesGroupLayout->addWidget(myMeshNbTrianglesLab,   1, 0);
  myMeshFacesGroupLayout->addWidget(myMeshNbTriangles,      1, 1);
  myMeshFacesGroupLayout->addWidget(myMeshNbTriangles1,     1, 2);
  myMeshFacesGroupLayout->addWidget(myMeshNbTriangles2,     1, 3);
  myMeshFacesGroupLayout->addWidget(myMeshNbQuadranglesLab, 2, 0);
  myMeshFacesGroupLayout->addWidget(myMeshNbQuadrangles,    2, 1);
  myMeshFacesGroupLayout->addWidget(myMeshNbQuadrangles1,   2, 2);
  myMeshFacesGroupLayout->addWidget(myMeshNbQuadrangles2,   2, 3);
  myMeshFacesGroupLayout->addWidget(myMeshNbPolygonesLab,   3, 0);
  myMeshFacesGroupLayout->addWidget(myMeshNbPolygones,      3, 1);
  
  // --> volumes
  myMeshVolumesGroup = new QGroupBox(tr("SMESH_MESHINFO_VOLUMES"), myMeshWidget);
  QGridLayout* myMeshVolumesGroupLayout = new QGridLayout(myMeshVolumesGroup);
  myMeshVolumesGroupLayout->setSpacing(SPACING);  myMeshVolumesGroupLayout->setMargin(MARGIN);

  // --> volumes --> total
  QLabel* myMeshNbVolumesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TOTAL")), myMeshVolumesGroup);
  myMeshNbVolumesLab->setFont(fnt);
  myMeshNbVolumes    = new QLabel(myMeshVolumesGroup);
  myMeshNbVolumes->setMinimumWidth(100);
  myMeshNbVolumes->setFont(fnt);
  myMeshNbVolumes1   = new QLabel(myMeshVolumesGroup);
  myMeshNbVolumes1->setMinimumWidth(100);
  myMeshNbVolumes1->setFont(fnt);
  myMeshNbVolumes2   = new QLabel(myMeshVolumesGroup);
  myMeshNbVolumes2->setMinimumWidth(100);
  myMeshNbVolumes2->setFont(fnt);

  // --> volumes --> tetrahedrons
  QLabel* myMeshNbTetraLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TETRAS")), myMeshVolumesGroup);
  myMeshNbTetra    = new QLabel(myMeshVolumesGroup);
  myMeshNbTetra->setMinimumWidth(100);
  myMeshNbTetra1   = new QLabel(myMeshVolumesGroup);
  myMeshNbTetra1->setMinimumWidth(100);
  myMeshNbTetra2   = new QLabel(myMeshVolumesGroup);
  myMeshNbTetra2->setMinimumWidth(100);

  // --> volumes --> hexahedrons
  QLabel* myMeshNbHexaLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_HEXAS")), myMeshVolumesGroup);
  myMeshNbHexa    = new QLabel(myMeshVolumesGroup);
  myMeshNbHexa->setMinimumWidth(100);
  myMeshNbHexa1   = new QLabel(myMeshVolumesGroup);
  myMeshNbHexa1->setMinimumWidth(100);
  myMeshNbHexa2   = new QLabel(myMeshVolumesGroup);
  myMeshNbHexa2->setMinimumWidth(100);

  // --> volumes --> prisms
  QLabel* myMeshNbPrismLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_PRISMS")), myMeshVolumesGroup);
  myMeshNbPrism    = new QLabel(myMeshVolumesGroup);
  myMeshNbPrism->setMinimumWidth(100);
  myMeshNbPrism1   = new QLabel(myMeshVolumesGroup);
  myMeshNbPrism1->setMinimumWidth(100);
  myMeshNbPrism2   = new QLabel(myMeshVolumesGroup);
  myMeshNbPrism2->setMinimumWidth(100);

  // --> volumes --> pyramids
  QLabel* myMeshNbPyraLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_PYRAS")), myMeshVolumesGroup);
  myMeshNbPyra    = new QLabel(myMeshVolumesGroup);
  myMeshNbPyra->setMinimumWidth(100);
  myMeshNbPyra1   = new QLabel(myMeshVolumesGroup);
  myMeshNbPyra1->setMinimumWidth(100);
  myMeshNbPyra2   = new QLabel(myMeshVolumesGroup);
  myMeshNbPyra2->setMinimumWidth(100);

  // --> volumes --> polyherones
  QLabel* myMeshNbPolyhedronesLab = new QLabel( COLONIZE( tr( "SMESH_MESHINFO_POLYEDRES" ) ), myMeshVolumesGroup );
  myMeshNbPolyhedrones = new QLabel( myMeshVolumesGroup );
  myMeshNbPolyhedrones->setMinimumWidth( 100 );

  myMeshVolumesGroupLayout->addWidget(myMeshNbVolumesLab,      0, 0);
  myMeshVolumesGroupLayout->addWidget(myMeshNbVolumes,         0, 1);
  myMeshVolumesGroupLayout->addWidget(myMeshNbVolumes1,        0, 2);
  myMeshVolumesGroupLayout->addWidget(myMeshNbVolumes2,        0, 3);
  myMeshVolumesGroupLayout->addWidget(myMeshNbTetraLab,        1, 0);
  myMeshVolumesGroupLayout->addWidget(myMeshNbTetra,           1, 1);
  myMeshVolumesGroupLayout->addWidget(myMeshNbTetra1,          1, 2);
  myMeshVolumesGroupLayout->addWidget(myMeshNbTetra2,          1, 3);
  myMeshVolumesGroupLayout->addWidget(myMeshNbHexaLab,         2, 0);
  myMeshVolumesGroupLayout->addWidget(myMeshNbHexa,            2, 1);
  myMeshVolumesGroupLayout->addWidget(myMeshNbHexa1,           2, 2);
  myMeshVolumesGroupLayout->addWidget(myMeshNbHexa2,           2, 3);
  myMeshVolumesGroupLayout->addWidget(myMeshNbPrismLab,        3, 0);
  myMeshVolumesGroupLayout->addWidget(myMeshNbPrism,           3, 1);
  myMeshVolumesGroupLayout->addWidget(myMeshNbPrism1,          3, 2);
  myMeshVolumesGroupLayout->addWidget(myMeshNbPrism2,          3, 3);
  myMeshVolumesGroupLayout->addWidget(myMeshNbPyraLab,         4, 0);
  myMeshVolumesGroupLayout->addWidget(myMeshNbPyra,            4, 1);
  myMeshVolumesGroupLayout->addWidget(myMeshNbPyra1,           4, 2);
  myMeshVolumesGroupLayout->addWidget(myMeshNbPyra2,           4, 3);
  myMeshVolumesGroupLayout->addWidget(myMeshNbPolyhedronesLab, 5, 0);
  myMeshVolumesGroupLayout->addWidget(myMeshNbPolyhedrones,    5, 1);

  aMeshLayout->addWidget(myMeshNameLab,      0, 0);
  aMeshLayout->addWidget(myMeshName,         0, 1);
  aMeshLayout->addWidget(line1,              1, 0, 1, 2);
  aMeshLayout->addWidget(myMeshNbNodesLab,   2, 0);
  aMeshLayout->addWidget(myMeshNbNodes,      2, 1);
  aMeshLayout->addWidget(line12,             3, 0, 1, 2);
  aMeshLayout->addWidget(myMeshNb0DElemsLab, 4, 0);
  aMeshLayout->addWidget(myMeshNb0DElems,    4, 1);
  aMeshLayout->addWidget(myMeshOrder0Lab,    5, 1);
  aMeshLayout->addWidget(myMeshOrder1Lab,    5, 2);
  aMeshLayout->addWidget(myMeshOrder2Lab,    5, 3);
  aMeshLayout->addWidget(myMeshNbEdgesLab,   6, 0);
  aMeshLayout->addWidget(myMeshNbEdges,      6, 1);
  aMeshLayout->addWidget(myMeshNbEdges1,     6, 2);
  aMeshLayout->addWidget(myMeshNbEdges2,     6, 3);
  aMeshLayout->addWidget(myMeshFacesGroup,   7, 0, 1, 4);
  aMeshLayout->addWidget(myMeshVolumesGroup, 8, 0, 1, 4);
  aMeshLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 9, 0);

  // submesh
  mySubMeshWidget = new QWidget(myWGStack);
  QGridLayout* aSubMeshLayout = new QGridLayout(mySubMeshWidget);
  aSubMeshLayout->setSpacing(SPACING);  aSubMeshLayout->setMargin(0);
  myWGStack->addWidget(mySubMeshWidget);

  // --> name
  QLabel* mySubMeshNameLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NAME")), mySubMeshWidget);
  mySubMeshName    = new QLabel(mySubMeshWidget);
  mySubMeshName->setMinimumWidth(100);
  QFrame* line2 = new QFrame(mySubMeshWidget);
  line2->setFrameStyle(QFrame::HLine | QFrame::Sunken);

  // --> nodes
  QLabel* mySubMeshNbNodesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NODES")), mySubMeshWidget);
  mySubMeshNbNodes    = new QLabel(mySubMeshWidget);
  mySubMeshNbNodes->setMinimumWidth(100);

  // --> elements
  mySubMeshElementsGroup = new QGroupBox(tr("SMESH_MESHINFO_ELEMENTS"), mySubMeshWidget);
  QGridLayout* mySubMeshElementsGroupLayout = new QGridLayout(mySubMeshElementsGroup);
  mySubMeshElementsGroupLayout->setSpacing(SPACING);  mySubMeshElementsGroupLayout->setMargin(MARGIN);

  // --> elements --> total
  QLabel* mySubMeshNbElementsLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TOTAL")), mySubMeshElementsGroup);
  mySubMeshNbElementsLab->setFont(fnt);
  mySubMeshNbElements    = new QLabel(mySubMeshElementsGroup);
  mySubMeshNbElements->setMinimumWidth(100);
  mySubMeshNbElements->setFont(fnt);

  // --> 0D elements
  QLabel* mySubMeshNb0DElemsLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_0DELEMS")), mySubMeshElementsGroup);
  mySubMeshNb0DElems = new QLabel(mySubMeshElementsGroup);
  mySubMeshNb0DElems->setMinimumWidth(100);

  // --> elements --> edges
  QLabel* mySubMeshNbEdgesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_EDGES")), mySubMeshElementsGroup);
  mySubMeshNbEdges    = new QLabel(mySubMeshElementsGroup);
  mySubMeshNbEdges->setMinimumWidth(100);

  // --> elements --> faces
  QLabel* mySubMeshNbFacesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_FACES")), mySubMeshElementsGroup);
  mySubMeshNbFaces    = new QLabel(mySubMeshElementsGroup);
  mySubMeshNbFaces->setMinimumWidth(100);

  // --> elements --> volumes
  QLabel* mySubMeshNbVolumesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_VOLUMES")), mySubMeshElementsGroup);
  mySubMeshNbVolumes    = new QLabel(mySubMeshElementsGroup);
  mySubMeshNbVolumes->setMinimumWidth(100);

  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbElementsLab, 0, 0);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbElements,    0, 1);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNb0DElemsLab,  1, 0);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNb0DElems,     1, 1);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbEdgesLab,    2, 0);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbEdges,       2, 1);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbFacesLab,    3, 0);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbFaces,       3, 1);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbVolumesLab,  4, 0);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbVolumes,     4, 1);

  aSubMeshLayout->addWidget(mySubMeshNameLab,       0, 0);
  aSubMeshLayout->addWidget(mySubMeshName,          0, 1);
  aSubMeshLayout->addWidget(line2,                  1, 0, 1, 2);
  aSubMeshLayout->addWidget(mySubMeshNbNodesLab,    2, 0);
  aSubMeshLayout->addWidget(mySubMeshNbNodes,       2, 1);
  aSubMeshLayout->addWidget(mySubMeshElementsGroup, 3, 0, 1, 2);
  aSubMeshLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 4, 0);

  // group
  myGroupWidget = new QWidget(myWGStack);
  QGridLayout* myGroupWidgetLayout = new QGridLayout(myGroupWidget);
  myGroupWidgetLayout->setSpacing(SPACING);  myGroupWidgetLayout->setMargin(0);
  myWGStack->addWidget(myGroupWidget);

  // --> name
  QLabel* myGroupNameLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NAME")), myGroupWidget);
  myGroupName = new QLabel(myGroupWidget);
  myGroupName->setMinimumWidth(100);
  QFrame* line3 = new QFrame(myGroupWidget);
  line3->setFrameStyle(QFrame::HLine | QFrame::Sunken);

  // --> type
  QLabel* myGroupTypeLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TYPE")), myGroupWidget);
  myGroupType = new QLabel(myGroupWidget);
  myGroupType->setMinimumWidth(100);

  // --> number of entities
  QLabel* myGroupNbLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_ENTITIES")), myGroupWidget);
  myGroupNb = new QLabel(myGroupWidget);
  myGroupNb->setMinimumWidth(100);

  myGroupWidgetLayout->addWidget(myGroupNameLab, 0, 0);
  myGroupWidgetLayout->addWidget(myGroupName,    0, 1);
  myGroupWidgetLayout->addWidget(line3,          1, 0, 1, 2);
  myGroupWidgetLayout->addWidget(myGroupTypeLab, 2, 0);
  myGroupWidgetLayout->addWidget(myGroupType,    2, 1);
  myGroupWidgetLayout->addWidget(myGroupNbLab,   3, 0);
  myGroupWidgetLayout->addWidget(myGroupNb,      3, 1);
  myGroupWidgetLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 4, 0);

  // buttons
  myButtonsGroup = new QGroupBox(this);
  QHBoxLayout* myButtonsGroupLayout = new QHBoxLayout(myButtonsGroup);
  myButtonsGroupLayout->setSpacing(SPACING); myButtonsGroupLayout->setMargin(MARGIN);

  // buttons --> OK and Help buttons
  myOkBtn = new QPushButton(tr("SMESH_BUT_OK" ), myButtonsGroup);
  myOkBtn->setAutoDefault(true); myOkBtn->setDefault(true);
  myHelpBtn = new QPushButton(tr("SMESH_BUT_HELP" ), myButtonsGroup);
  myHelpBtn->setAutoDefault(true);

  myButtonsGroupLayout->addWidget(myOkBtn);
  myButtonsGroupLayout->addSpacing(10);
  myButtonsGroupLayout->addStretch();
  myButtonsGroupLayout->addWidget(myHelpBtn);

  aTopLayout->addLayout(aSelectLayout);
  aTopLayout->addWidget(myWGStack);
  aTopLayout->addWidget(myButtonsGroup);

  mySMESHGUI->SetActiveDialogBox(this);

  // connect signals
  connect(myOkBtn,                 SIGNAL(clicked()),                      this, SLOT(close()));
  connect( myHelpBtn,              SIGNAL(clicked()),                      this, SLOT(onHelp()));
  connect(mySelectBtn,             SIGNAL(clicked()),                      this, SLOT(onStartSelection()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()),        this, SLOT(close()));
  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr,          SIGNAL(currentSelectionChanged()),      this, SLOT(onSelectionChanged()));

  // init dialog with current selection
  onSelectionChanged();

  myHelpFileName = "mesh_infos_page.html#advanced_mesh_infos_anchor";
}

//=================================================================================
// function : ~SMESHGUI_MeshInfosDlg()
// purpose  : Destructor
//=================================================================================
SMESHGUI_MeshInfosDlg::~SMESHGUI_MeshInfosDlg()
{
}

//=================================================================================
// function : DumpMeshInfos()
// purpose  : 
//=================================================================================
void SMESHGUI_MeshInfosDlg::DumpMeshInfos()
{
  SUIT_OverrideCursor wc;

  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);

  int nbSel = aList.Extent();
  if (nbSel == 1) {
    myStartSelection = false;
    mySelectLab->setText("");
    Handle(SALOME_InteractiveObject) IObject = aList.First();
    _PTR(SObject) aSO = SMESH::GetActiveStudyDocument()->FindObjectID(IObject->getEntry());
    if (aSO) {
      //CORBA::Object_var anObject = aSO->GetObject();
      CORBA::Object_var anObject = SMESH::SObjectToObject(aSO);
      if (!CORBA::is_nil(anObject)) {
	SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow(anObject);
	if (!aMesh->_is_nil()) {
	  myWGStack->setCurrentWidget(myMeshWidget);
	  setWindowTitle(tr("SMESH_MESHINFO_TITLE") + " [" + tr("SMESH_OBJECT_MESH") + "]");
	  myMeshName->setText(aSO->GetName().c_str());
	  myMeshNbNodes->setNum((int)aMesh->NbNodes());
          myMeshNb0DElems->setNum((int)aMesh->Nb0DElements());
	  myMeshNbEdges->setNum((int)aMesh->NbEdges());
	  myMeshNbEdges1->setNum((int)aMesh->NbEdgesOfOrder(SMESH::ORDER_LINEAR));
	  myMeshNbEdges2->setNum((int)aMesh->NbEdgesOfOrder(SMESH::ORDER_QUADRATIC));
	  myMeshNbFaces->setNum((int)aMesh->NbFaces());
	  myMeshNbFaces1->setNum((int)aMesh->NbFacesOfOrder(SMESH::ORDER_LINEAR));
	  myMeshNbFaces2->setNum((int)aMesh->NbFacesOfOrder(SMESH::ORDER_QUADRATIC));
	  myMeshNbTriangles->setNum((int)aMesh->NbTriangles());
	  myMeshNbTriangles1->setNum((int)aMesh->NbTrianglesOfOrder(SMESH::ORDER_LINEAR));
	  myMeshNbTriangles2->setNum((int)aMesh->NbTrianglesOfOrder(SMESH::ORDER_QUADRATIC));
	  myMeshNbQuadrangles->setNum((int)aMesh->NbQuadrangles());
	  myMeshNbQuadrangles1->setNum((int)aMesh->NbQuadranglesOfOrder(SMESH::ORDER_LINEAR));
	  myMeshNbQuadrangles2->setNum((int)aMesh->NbQuadranglesOfOrder(SMESH::ORDER_QUADRATIC));
	  myMeshNbPolygones->setNum( (int)aMesh->NbPolygons() );
	  myMeshNbVolumes->setNum((int)aMesh->NbVolumes());
	  myMeshNbVolumes1->setNum((int)aMesh->NbVolumesOfOrder(SMESH::ORDER_LINEAR));
	  myMeshNbVolumes2->setNum((int)aMesh->NbVolumesOfOrder(SMESH::ORDER_QUADRATIC));
	  myMeshNbTetra->setNum((int)aMesh->NbTetras());
	  myMeshNbTetra1->setNum((int)aMesh->NbTetrasOfOrder(SMESH::ORDER_LINEAR));
	  myMeshNbTetra2->setNum((int)aMesh->NbTetrasOfOrder(SMESH::ORDER_QUADRATIC));
	  myMeshNbHexa->setNum((int)aMesh->NbHexas());
	  myMeshNbHexa1->setNum((int)aMesh->NbHexasOfOrder(SMESH::ORDER_LINEAR));
	  myMeshNbHexa2->setNum((int)aMesh->NbHexasOfOrder(SMESH::ORDER_QUADRATIC));
	  myMeshNbPrism->setNum((int)aMesh->NbPrisms());
	  myMeshNbPrism1->setNum((int)aMesh->NbPrismsOfOrder(SMESH::ORDER_LINEAR));
	  myMeshNbPrism2->setNum((int)aMesh->NbPrismsOfOrder(SMESH::ORDER_QUADRATIC));
	  myMeshNbPyra->setNum((int)aMesh->NbPyramids());
	  myMeshNbPyra1->setNum((int)aMesh->NbPyramidsOfOrder(SMESH::ORDER_LINEAR));
	  myMeshNbPyra2->setNum((int)aMesh->NbPyramidsOfOrder(SMESH::ORDER_QUADRATIC));
	  myMeshNbPolyhedrones->setNum( (int)aMesh->NbPolyhedrons() );
	  return;
	}
	SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow(anObject);
	if (!aSubMesh->_is_nil()) {
	  myWGStack->setCurrentWidget(mySubMeshWidget);
	  setWindowTitle(tr("SMESH_MESHINFO_TITLE") + " [" + tr("SMESH_SUBMESH") + "]");
	  mySubMeshName->setText(aSO->GetName().c_str());
	  mySubMeshNbNodes->setNum((int)aSubMesh->GetNumberOfNodes(true));
	  mySubMeshNbElements->setNum((int)aSubMesh->GetNumberOfElements());
	  mySubMeshNb0DElems->setNum((int)(aSubMesh->GetElementsByType(SMESH::ELEM0D)->length()));
	  mySubMeshNbEdges->setNum((int)(aSubMesh->GetElementsByType(SMESH::EDGE)->length()));
	  mySubMeshNbFaces->setNum((int)(aSubMesh->GetElementsByType(SMESH::FACE)->length()));
	  mySubMeshNbVolumes->setNum((int)(aSubMesh->GetElementsByType(SMESH::VOLUME)->length()));
	  return;
	}
	SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(anObject);
	if (!aGroup->_is_nil()) {
	  myWGStack->setCurrentWidget(myGroupWidget);
	  setWindowTitle(tr("SMESH_MESHINFO_TITLE") + " [" + tr("SMESH_GROUP") + "]");
	  myGroupName->setText(aSO->GetName().c_str());
	  int aType = aGroup->GetType();
	  QString strType;
	  switch (aType) {
	  case SMESH::NODE:
	    strType = "SMESH_MESHINFO_NODES"; break;
	  case SMESH::ELEM0D:
	    strType = "SMESH_MESHINFO_0DELEMS"; break;
	  case SMESH::EDGE:
	    strType = "SMESH_MESHINFO_EDGES"; break;
	  case SMESH::FACE:
	    strType = "SMESH_MESHINFO_FACES"; break;
	  case SMESH::VOLUME:
	    strType = "SMESH_MESHINFO_VOLUMES"; break;
	  default:
	    strType = "SMESH_MESHINFO_ALL_TYPES"; break;
	  }

	  myGroupType->setText(tr(strType.toLatin1().data()));
	  myGroupNb->setNum((int)aGroup->Size());
	  return;
	}
      }
    }
  }
  myWGStack->setCurrentIndex(0);
  setWindowTitle(tr("SMESH_MESHINFO_TITLE"));
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection has changed
//=================================================================================
void SMESHGUI_MeshInfosDlg::onSelectionChanged()
{
  if (myStartSelection)
    DumpMeshInfos();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::closeEvent(QCloseEvent* e)
{
  mySMESHGUI->ResetState();
  QDialog::closeEvent(e);
}

//=================================================================================
// function : windowActivationChange()
// purpose  : called when window is activated/deactivated
//=================================================================================
void SMESHGUI_MeshInfosDlg::windowActivationChange(bool oldActive)
{
  QDialog::windowActivationChange(oldActive);
  if (isActiveWindow() && myIsActiveWindow != isActiveWindow())
    ActivateThisDialog();
  myIsActiveWindow = isActiveWindow();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::DeactivateActiveDialog()
{
  disconnect(mySelectionMgr, 0, this, 0);
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate any active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(onSelectionChanged()));
}

//=================================================================================
// function : onStartSelection()
// purpose  : starts selection
//=================================================================================
void SMESHGUI_MeshInfosDlg::onStartSelection()
{
  myStartSelection = true;
  onSelectionChanged();
  myStartSelection = true;
  mySelectLab->setText(tr("INF_SELECT_OBJECT"));
}

//=================================================================================
// function : onHelp()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::onHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app) 
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
  else {
    QString platform;
#ifdef WIN32
    platform = "winapplication";
#else
    platform = "application";
#endif
    SUIT_MessageBox::warning(this, tr("WRN_WARNING"),
			     tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			     arg(app->resourceMgr()->stringValue("ExternalBrowser", 
								 platform)).
			     arg(myHelpFileName));
  }
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    onHelp();
  }
}
