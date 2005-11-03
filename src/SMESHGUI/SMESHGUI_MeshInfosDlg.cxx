//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org
//
//
//
//  File   : SMESHGUI_MeshInfosDlg.cxx
//  Author : Nicolas BARBEROU
//  Module : SMESH
//  $Header$

#include "SMESHGUI_MeshInfosDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"

#include "SUIT_Desktop.h"
#include "SUIT_ResourceMgr.h"
#include "SUIT_OverrideCursor.h"

#include "LightApp_SelectionMgr.h"
#include "SALOMEDSClient_Study.hxx"
#include "SALOME_ListIO.hxx"

#include "utilities.h"

// QT Includes
#include <qgroupbox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qwidgetstack.h>
#include <qlayout.h>
#include <qmap.h>
#include <qpushbutton.h>

// IDL Headers
#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)

using namespace std;

#define COLONIZE(str)   (QString(str).contains(":") > 0 ? QString(str) : QString(str) + " :" )

//=================================================================================
// function : SMESHGUI_MeshInfosDlg()
// purpose  : Constructor
//=================================================================================
SMESHGUI_MeshInfosDlg::SMESHGUI_MeshInfosDlg (SMESHGUI* theModule, 
					      const char* name, 
					      bool modal, 
					      WFlags fl): 
  QDialog(SMESH::GetDesktop(theModule), 
	  name, 
	  modal, 
	  WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | WDestructiveClose),
  mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
  mySMESHGUI(theModule)
{
  if (!name)
      setName("SMESHGUI_MeshInfosDlg");
  setCaption(tr("SMESH_MESHINFO_TITLE"));
  setSizeGripEnabled(TRUE);

  myStartSelection = true;
  myIsActiveWindow = true;

  QVBoxLayout* aTopLayout = new QVBoxLayout(this);
  aTopLayout->setSpacing(6);  aTopLayout->setMargin(11);

  // select button & label
  QPixmap image0(SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH",tr("ICON_SELECT")));
  mySelectBtn = new QPushButton(this, "mySelectBtn");
  mySelectBtn->setPixmap(image0);
  mySelectBtn->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

  mySelectLab = new QLabel(this, "mySelectLab");
  mySelectLab->setAlignment(AlignCenter);
  QFont fnt = mySelectLab->font(); fnt.setBold(true);
  mySelectLab->setFont(fnt);

  QHBoxLayout* aSelectLayout = new QHBoxLayout;
  aSelectLayout->setMargin(0); aSelectLayout->setSpacing(0);
  aSelectLayout->addWidget(mySelectBtn);
  aSelectLayout->addWidget(mySelectLab);

  // top widget stack
  myWGStack = new QWidgetStack(this);

  // no valid selection
  QWidget* myBadWidget = new QWidget(myWGStack);
  QVBoxLayout* aBadLayout = new QVBoxLayout(myBadWidget);
  QLabel* myBadLab = new QLabel(tr("SMESH_BAD_SELECTION"), myBadWidget, "myBadLab");
  myBadLab->setAlignment(Qt::AlignCenter);
  myBadLab->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  aBadLayout->addWidget(myBadLab);
  myWGStack->addWidget(myBadWidget, 0);

  // mesh
  myMeshWidget = new QWidget(myWGStack);
  QGridLayout* aMeshLayout = new QGridLayout(myMeshWidget);
  aMeshLayout->setSpacing(6);  aMeshLayout->setMargin(0);
  myWGStack->addWidget(myMeshWidget);

  // --> name
  QLabel* myMeshNameLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NAME")), myMeshWidget, "myMeshNameLab");
  myMeshName    = new QLabel(myMeshWidget, "myMeshName");
  myMeshName->setMinimumWidth(100);
  QFrame* line1 = new QFrame(myMeshWidget);
  line1->setFrameStyle(QFrame::HLine | QFrame::Sunken);

  // --> nodes
  QLabel* myMeshNbNodesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NODES")), myMeshWidget, "myMeshNbNodesLab");
  myMeshNbNodes    = new QLabel(myMeshWidget, "myMeshNbNodes");
  myMeshNbNodes->setMinimumWidth(100);

  // --> edges
  QLabel* myMeshNbEdgesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_EDGES")), myMeshWidget, "myMeshNbEdgesLab");
  myMeshNbEdges    = new QLabel(myMeshWidget, "myMeshNbEdges");
  myMeshNbEdges->setMinimumWidth(100);

  // --> faces
  myMeshFacesGroup = new QGroupBox(tr("SMESH_MESHINFO_FACES"), myMeshWidget, "myMeshFacesGroup");
  myMeshFacesGroup->setColumnLayout(0, Qt::Vertical);
  myMeshFacesGroup->layout()->setSpacing(0);  myMeshFacesGroup->layout()->setMargin(0);
  QGridLayout* myMeshFacesGroupLayout = new QGridLayout(myMeshFacesGroup->layout());
  myMeshFacesGroupLayout->setAlignment(Qt::AlignTop);
  myMeshFacesGroupLayout->setSpacing(6);  myMeshFacesGroupLayout->setMargin(11);

  // --> faces --> total
  QLabel* myMeshNbFacesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TOTAL")), myMeshFacesGroup, "myMeshNbFacesLab");
  myMeshNbFacesLab->setFont(fnt);
  myMeshNbFaces    = new QLabel(myMeshFacesGroup, "myMeshNbFaces");
  myMeshNbFaces->setMinimumWidth(100);
  myMeshNbFaces->setFont(fnt);

  // --> faces --> triangles
  QLabel* myMeshNbTrianglesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TRIANGLES")), myMeshFacesGroup, "myMeshNbTrianglesLab");
  myMeshNbTriangles    = new QLabel(myMeshFacesGroup, "myMeshNbTriangles");
  myMeshNbTriangles->setMinimumWidth(100);

  // --> faces --> quadrangles
  QLabel* myMeshNbQuadranglesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_QUADRANGLES")), myMeshFacesGroup, "myMeshNbQuadranglesLab");
  myMeshNbQuadrangles    = new QLabel(myMeshFacesGroup, "myMeshNbQuadrangles");
  myMeshNbQuadrangles->setMinimumWidth(100);

  // --> faces --> polygons
  QLabel* myMeshNbPolygonesLab = new QLabel( COLONIZE( tr( "SMESH_MESHINFO_POLYGONES" ) ), myMeshFacesGroup, "myMeshNbPolygonesLab" );
  myMeshNbPolygones      = new QLabel( myMeshFacesGroup, "myMeshNbPolygones" );
  myMeshNbPolygones->setMinimumWidth( 100 );

  myMeshFacesGroupLayout->addWidget(myMeshNbFacesLab,       0, 0);
  myMeshFacesGroupLayout->addWidget(myMeshNbFaces,          0, 1);
  myMeshFacesGroupLayout->addWidget(myMeshNbTrianglesLab,   1, 0);
  myMeshFacesGroupLayout->addWidget(myMeshNbTriangles,      1, 1);
  myMeshFacesGroupLayout->addWidget(myMeshNbQuadranglesLab, 2, 0);
  myMeshFacesGroupLayout->addWidget(myMeshNbQuadrangles,    2, 1);
  myMeshFacesGroupLayout->addWidget( myMeshNbPolygonesLab,   3, 0 );
  myMeshFacesGroupLayout->addWidget( myMeshNbPolygones,      3, 1 );
  
  // --> volumes
  myMeshVolumesGroup = new QGroupBox(tr("SMESH_MESHINFO_VOLUMES"), myMeshWidget, "myMeshVolumesGroup");
  myMeshVolumesGroup->setColumnLayout(0, Qt::Vertical);
  myMeshVolumesGroup->layout()->setSpacing(0);  myMeshVolumesGroup->layout()->setMargin(0);
  QGridLayout* myMeshVolumesGroupLayout = new QGridLayout(myMeshVolumesGroup->layout());
  myMeshVolumesGroupLayout->setAlignment(Qt::AlignTop);
  myMeshVolumesGroupLayout->setSpacing(6);  myMeshVolumesGroupLayout->setMargin(11);

  // --> volumes --> total
  QLabel* myMeshNbVolumesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TOTAL")), myMeshVolumesGroup, "myMeshNbVolumesLab");
  myMeshNbVolumesLab->setFont(fnt);
  myMeshNbVolumes    = new QLabel(myMeshVolumesGroup, "myMeshNbVolumes");
  myMeshNbVolumes->setMinimumWidth(100);
  myMeshNbVolumes->setFont(fnt);

  // --> volumes --> tetrahedrons
  QLabel* myMeshNbTetraLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TETRAS")), myMeshVolumesGroup, "myMeshNbTetraLab");
  myMeshNbTetra    = new QLabel(myMeshVolumesGroup, "myMeshNbTetra");
  myMeshNbTetra->setMinimumWidth(100);

  // --> volumes --> hexahedrons
  QLabel* myMeshNbHexaLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_HEXAS")), myMeshVolumesGroup, "myMeshNbHexaLab");
  myMeshNbHexa    = new QLabel(myMeshVolumesGroup, "myMeshNbHexa");
  myMeshNbHexaLab->setMinimumWidth(100);

  // --> volumes --> prisms
  QLabel* myMeshNbPrismLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_PRISMS")), myMeshVolumesGroup, "myMeshNbPrismLab");
  myMeshNbPrism    = new QLabel(myMeshVolumesGroup, "myMeshNbPrism");
  myMeshNbPrism->setMinimumWidth(100);

  // --> volumes --> pyramids
  QLabel* myMeshNbPyraLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_PYRAS")), myMeshVolumesGroup, "myMeshNbPyraLab");
  myMeshNbPyra    = new QLabel(myMeshVolumesGroup, "myMeshNbPyra");
  myMeshNbPyra->setMinimumWidth(100);

  // --> volumes --> polyherones
  QLabel* myMeshNbPolyhedronesLab = new QLabel( COLONIZE( tr( "SMESH_MESHINFO_POLYEDRES" ) ), myMeshVolumesGroup, "myMeshNbPolyhedronLab" );
  myMeshNbPolyhedrones = new QLabel( myMeshVolumesGroup, "myMeshNbPolyhedrones" );
  myMeshNbPolyhedrones->setMinimumWidth( 100 );

  myMeshVolumesGroupLayout->addWidget(myMeshNbVolumesLab, 0, 0);
  myMeshVolumesGroupLayout->addWidget(myMeshNbVolumes,    0, 1);
  myMeshVolumesGroupLayout->addWidget(myMeshNbTetraLab,   1, 0);
  myMeshVolumesGroupLayout->addWidget(myMeshNbTetra,      1, 1);
  myMeshVolumesGroupLayout->addWidget(myMeshNbHexaLab,    2, 0);
  myMeshVolumesGroupLayout->addWidget(myMeshNbHexa,       2, 1);
  myMeshVolumesGroupLayout->addWidget(myMeshNbPrismLab,   3, 0);
  myMeshVolumesGroupLayout->addWidget(myMeshNbPrism,      3, 1);
  myMeshVolumesGroupLayout->addWidget(myMeshNbPyraLab,    4, 0);
  myMeshVolumesGroupLayout->addWidget(myMeshNbPyra,       4, 1);
  myMeshVolumesGroupLayout->addWidget( myMeshNbPolyhedronesLab,    5, 0 );
  myMeshVolumesGroupLayout->addWidget( myMeshNbPolyhedrones,       5, 1 );

  aMeshLayout->addWidget(myMeshNameLab,          0, 0);
  aMeshLayout->addWidget(myMeshName,             0, 1);
  aMeshLayout->addMultiCellWidget(line1,   1, 1, 0, 1);
  aMeshLayout->addWidget(myMeshNbNodesLab,       2, 0);
  aMeshLayout->addWidget(myMeshNbNodes,          2, 1);
  aMeshLayout->addWidget(myMeshNbEdgesLab,       3, 0);
  aMeshLayout->addWidget(myMeshNbEdges,          3, 1);
  aMeshLayout->addMultiCellWidget(myMeshFacesGroup,   4, 4, 0, 1);
  aMeshLayout->addMultiCellWidget(myMeshVolumesGroup, 5, 5, 0, 1);
  aMeshLayout->addItem(new QSpacerItem(5, 5, QSizePolicy::Minimum, QSizePolicy::Expanding), 6, 0);

  // submesh
  mySubMeshWidget = new QWidget(myWGStack);
  QGridLayout* aSubMeshLayout = new QGridLayout(mySubMeshWidget);
  aSubMeshLayout->setSpacing(6);  aSubMeshLayout->setMargin(0);
  myWGStack->addWidget(mySubMeshWidget);

  // --> name
  QLabel* mySubMeshNameLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NAME")), mySubMeshWidget, "mySubMeshNameLab");
  mySubMeshName    = new QLabel(mySubMeshWidget, "mySubMeshName");
  mySubMeshName->setMinimumWidth(100);
  QFrame* line2 = new QFrame(mySubMeshWidget);
  line2->setFrameStyle(QFrame::HLine | QFrame::Sunken);

  // --> nodes
  QLabel* mySubMeshNbNodesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NODES")), mySubMeshWidget, "mySubMeshNbNodesLab");
  mySubMeshNbNodes    = new QLabel(mySubMeshWidget, "mySubMeshNbNodes");
  mySubMeshNbNodes->setMinimumWidth(100);

  // --> elements
  mySubMeshElementsGroup = new QGroupBox(tr("SMESH_MESHINFO_ELEMENTS"), mySubMeshWidget, "mySubMeshElementsGroup");
  mySubMeshElementsGroup->setColumnLayout(0, Qt::Vertical);
  mySubMeshElementsGroup->layout()->setSpacing(0);  mySubMeshElementsGroup->layout()->setMargin(0);
  QGridLayout* mySubMeshElementsGroupLayout = new QGridLayout(mySubMeshElementsGroup->layout());
  mySubMeshElementsGroupLayout->setAlignment(Qt::AlignTop);
  mySubMeshElementsGroupLayout->setSpacing(6);  mySubMeshElementsGroupLayout->setMargin(11);

  // --> elements --> total
  QLabel* mySubMeshNbElementsLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TOTAL")), mySubMeshElementsGroup, "mySubMeshNbElementsLab");
  mySubMeshNbElementsLab->setFont(fnt);
  mySubMeshNbElements    = new QLabel(mySubMeshElementsGroup, "mySubMeshNbElements");
  mySubMeshNbElements->setMinimumWidth(100);
  mySubMeshNbElements->setFont(fnt);

  // --> elements --> edges
  QLabel* mySubMeshNbEdgesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_EDGES")), mySubMeshElementsGroup, "mySubMeshNbEdgesLab");
  mySubMeshNbEdges    = new QLabel(mySubMeshElementsGroup, "mySubMeshNbEdges");
  mySubMeshNbEdges->setMinimumWidth(100);

  // --> elements --> faces
  QLabel* mySubMeshNbFacesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_FACES")), mySubMeshElementsGroup, "mySubMeshNbFacesLab");
  mySubMeshNbFaces    = new QLabel(mySubMeshElementsGroup, "mySubMeshNbFaces");
  mySubMeshNbFaces->setMinimumWidth(100);

  // --> elements --> volumes
  QLabel* mySubMeshNbVolumesLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_VOLUMES")), mySubMeshElementsGroup, "mySubMeshNbVolumesLab");
  mySubMeshNbVolumes    = new QLabel(mySubMeshElementsGroup, "mySubMeshNbVolumes");
  mySubMeshNbVolumes->setMinimumWidth(100);

  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbElementsLab, 0, 0);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbElements,    0, 1);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbEdgesLab,    1, 0);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbEdges,       1, 1);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbFacesLab,    2, 0);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbFaces,       2, 1);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbVolumesLab,  3, 0);
  mySubMeshElementsGroupLayout->addWidget(mySubMeshNbVolumes,     3, 1);

  aSubMeshLayout->addWidget(mySubMeshNameLab,          0, 0);
  aSubMeshLayout->addWidget(mySubMeshName,             0, 1);
  aSubMeshLayout->addMultiCellWidget(line2,      1, 1, 0, 1);
  aSubMeshLayout->addWidget(mySubMeshNbNodesLab,       2, 0);
  aSubMeshLayout->addWidget(mySubMeshNbNodes,          2, 1);
  aSubMeshLayout->addMultiCellWidget(mySubMeshElementsGroup, 3, 3, 0, 1);
  aSubMeshLayout->addItem(new QSpacerItem(5, 5, QSizePolicy::Minimum, QSizePolicy::Expanding), 4, 0);

  // group
  myGroupWidget = new QWidget(myWGStack);
  QGridLayout* myGroupWidgetLayout = new QGridLayout(myGroupWidget);
  myGroupWidgetLayout->setSpacing(6);  myGroupWidgetLayout->setMargin(0);
  myWGStack->addWidget(myGroupWidget);

  // --> name
  QLabel* myGroupNameLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NAME")), myGroupWidget, "myGroupNameLab");
  myGroupName = new QLabel(myGroupWidget, "myGroupName");
  myGroupName->setMinimumWidth(100);
  QFrame* line3 = new QFrame(myGroupWidget);
  line3->setFrameStyle(QFrame::HLine | QFrame::Sunken);

  // --> type
  QLabel* myGroupTypeLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TYPE")), myGroupWidget, "myGroupTypeLab");
  myGroupType = new QLabel(myGroupWidget, "myGroupType");
  myGroupType->setMinimumWidth(100);

  // --> number of entities
  QLabel* myGroupNbLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_ENTITIES")), myGroupWidget, "myGroupNbLab");
  myGroupNb = new QLabel(myGroupWidget, "myGroupNb");
  myGroupNb->setMinimumWidth(100);

  myGroupWidgetLayout->addWidget(myGroupNameLab,       0, 0);
  myGroupWidgetLayout->addWidget(myGroupName,          0, 1);
  myGroupWidgetLayout->addMultiCellWidget(line3, 1, 1, 0, 1);
  myGroupWidgetLayout->addWidget(myGroupTypeLab,       2, 0);
  myGroupWidgetLayout->addWidget(myGroupType,          2, 1);
  myGroupWidgetLayout->addWidget(myGroupNbLab,         3, 0);
  myGroupWidgetLayout->addWidget(myGroupNb,            3, 1);
  myGroupWidgetLayout->addItem(new QSpacerItem(5, 5, QSizePolicy::Minimum, QSizePolicy::Expanding), 4, 0);

  // buttons
  myButtonsGroup = new QGroupBox(this, "myButtonsGroup");
  myButtonsGroup->setColumnLayout(0, Qt::Vertical);
  myButtonsGroup->layout()->setSpacing(0);  myButtonsGroup->layout()->setMargin(0);
  QHBoxLayout* myButtonsGroupLayout = new QHBoxLayout(myButtonsGroup->layout());
  myButtonsGroupLayout->setAlignment(Qt::AlignTop);
  myButtonsGroupLayout->setSpacing(6); myButtonsGroupLayout->setMargin(11);

  // buttons --> OK button
  myOkBtn = new QPushButton(tr("SMESH_BUT_OK" ), myButtonsGroup, "myOkBtn");
  myOkBtn->setAutoDefault(TRUE); myOkBtn->setDefault(TRUE);
  myButtonsGroupLayout->addStretch();
  myButtonsGroupLayout->addWidget(myOkBtn);
  myButtonsGroupLayout->addStretch();

  aTopLayout->addLayout(aSelectLayout);
  aTopLayout->addWidget(myWGStack);
  aTopLayout->addWidget(myButtonsGroup);

  mySMESHGUI->SetActiveDialogBox(this);

  // connect signals
  connect(myOkBtn,                 SIGNAL(clicked()),                      this, SLOT(close()));
  connect(mySelectBtn,             SIGNAL(clicked()),                      this, SLOT(onStartSelection()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()),        this, SLOT(close()));
  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr,          SIGNAL(currentSelectionChanged()),      this, SLOT(onSelectionChanged()));

  // resize and move dialog, then show
  int x, y;
  mySMESHGUI->DefineDlgPosition(this, x, y);
  this->move(x, y);
  this->show();

  // init dialog with current selection
  onSelectionChanged();
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
	  myWGStack->raiseWidget(myMeshWidget);
	  setCaption(tr("SMESH_MESHINFO_TITLE") + " [" + tr("SMESH_OBJECT_MESH") + "]");
	  myMeshName->setText(aSO->GetName().c_str());
	  myMeshNbNodes->setNum((int)aMesh->NbNodes());
	  myMeshNbEdges->setNum((int)aMesh->NbEdges());
	  myMeshNbFaces->setNum((int)aMesh->NbFaces());
	  myMeshNbTriangles->setNum((int)aMesh->NbTriangles());
	  myMeshNbQuadrangles->setNum((int)aMesh->NbQuadrangles());
	  myMeshNbPolygones->setNum( (int)aMesh->NbPolygons() );
	  myMeshNbVolumes->setNum((int)aMesh->NbVolumes());
	  myMeshNbTetra->setNum((int)aMesh->NbTetras());
	  myMeshNbHexa->setNum((int)aMesh->NbHexas());
	  myMeshNbPrism->setNum((int)aMesh->NbPrisms());
	  myMeshNbPyra->setNum((int)aMesh->NbPyramids());
	  myMeshNbPolyhedrones->setNum( (int)aMesh->NbPolyhedrons() );
	  return;
	}
	SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow(anObject);
	if (!aSubMesh->_is_nil()) {
	  myWGStack->raiseWidget(mySubMeshWidget);
	  setCaption(tr("SMESH_MESHINFO_TITLE") + " [" + tr("SMESH_SUBMESH") + "]");
	  mySubMeshName->setText(aSO->GetName().c_str());
	  mySubMeshNbNodes->setNum((int)aSubMesh->GetNumberOfNodes(true));
	  mySubMeshNbElements->setNum((int)aSubMesh->GetNumberOfElements());
	  mySubMeshNbEdges->setNum((int)(aSubMesh->GetElementsByType(SMESH::EDGE)->length()));
	  mySubMeshNbFaces->setNum((int)(aSubMesh->GetElementsByType(SMESH::FACE)->length()));
	  mySubMeshNbVolumes->setNum((int)(aSubMesh->GetElementsByType(SMESH::VOLUME)->length()));
	  return;
	}
	SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(anObject);
	if (!aGroup->_is_nil()) {
	  myWGStack->raiseWidget(myGroupWidget);
	  setCaption(tr("SMESH_MESHINFO_TITLE") + " [" + tr("SMESH_GROUP") + "]");
	  myGroupName->setText(aSO->GetName().c_str());
	  int aType = aGroup->GetType();
	  QString strType;
	  switch (aType) {
	  case SMESH::NODE:
	    strType = "SMESH_MESHINFO_NODES"; break;
	  case SMESH::EDGE:
	    strType = "SMESH_MESHINFO_EDGES"; break;
	  case SMESH::FACE:
	    strType = "SMESH_MESHINFO_FACES"; break;
	  case SMESH::VOLUME:
	    strType = "SMESH_MESHINFO_VOLUMES"; break;
	  default:
	    strType = "SMESH_MESHINFO_ALL_TYPES"; break;
	  }

	  myGroupType->setText(tr(strType));
	  myGroupNb->setNum((int)aGroup->Size());
	  return;
	}
      }
    }
  }
  myWGStack->raiseWidget(0);
  setCaption(tr("SMESH_MESHINFO_TITLE"));
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
void SMESHGUI_MeshInfosDlg::closeEvent (QCloseEvent* e)
{
  mySMESHGUI->ResetState();
  QDialog::closeEvent(e);
}

//=================================================================================
// function : windowActivationChange()
// purpose  : called when window is activated/deactivated
//=================================================================================
void SMESHGUI_MeshInfosDlg::windowActivationChange (bool oldActive)
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
