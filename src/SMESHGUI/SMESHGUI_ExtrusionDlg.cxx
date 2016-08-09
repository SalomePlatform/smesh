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
// File   : SMESHGUI_ExtrusionDlg.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_ExtrusionDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_MeshEditPreview.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include <GEOMBase.h>
#include <SMDS_Mesh.hxx>
#include <SMESH_Actor.h>
#include <SMESH_LogicalFilter.hxx>
#include <SMESH_TypeFilter.hxx>

// SALOME GUI includes
#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SalomeApp_IntSpinBox.h>

// OCCT includes
#include <BRep_Tool.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TopoDS_Vertex.hxx>
#include <gp_XYZ.hxx>

// Qt includes
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QToolButton>
#include <QVBoxLayout>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)
#include <SMESH_NumberFilter.hxx>

#define SPACING 6
#define MARGIN  11

namespace
{
  const char* getLabelText( int typeIndex, bool objSelection )
  {
    const char* typeLbl[3] = { "SMESH_ID_NODES", "SMESH_ID_EDGES", "SMESH_ID_FACES" };
    const char* obj = "SMESH_OBJECTS";
    return objSelection ? obj : typeLbl[ typeIndex ];
  }
}

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================

SMESHGUI_3TypesSelector::SMESHGUI_3TypesSelector( QWidget * parent ):
  QWidget( parent )
{
  SMESHGUI*  gui = SMESHGUI::GetSMESHGUI();
  mySelectionMgr = SMESH::GetSelectionMgr( gui );
  mySelector     = SMESH::GetViewWindow( gui )->GetSelector();
  myFilterDlg    = 0;
  myIdValidator  = new SMESHGUI_IdValidator(this);

  QPixmap image( SMESH::GetResourceMgr( gui )->loadPixmap("SMESH", tr("ICON_SELECT")));

  mySelectBtnGrp = new QButtonGroup( this );
  mySelectBtnGrp->setExclusive( true );

  QVBoxLayout* mainLayout = new QVBoxLayout( this );
  mainLayout->setSpacing( SPACING );
  mainLayout->setMargin( 0 );

  const char* groupLbl[3] = { "SMESH_NODES", "SMESH_EDGES", "SMESH_FACES" };

  for ( int i = 0; i < 3; ++i )
  {
    myGroups[i] = new QGroupBox( tr( groupLbl[i] ), this );
    mainLayout->addWidget( myGroups[i] );
    QGridLayout* layout = new QGridLayout( myGroups[i] );
    layout->setSpacing( SPACING );
    layout->setMargin( MARGIN );

    QPushButton* selBtn = new QPushButton( myGroups[i] );
    selBtn->setIcon( image );
    selBtn->setCheckable( true );
    mySelectBtnGrp->addButton( selBtn, i );
    myLabel    [i] = new QLabel( myGroups[i] );
    myLineEdit [i] = new QLineEdit( myGroups[i] );
    myMeshChk  [i] = new QCheckBox( tr("SMESH_SELECT_WHOLE_MESH"), myGroups[i] );
    myFilterBtn[i] = new QPushButton( tr( "SMESH_BUT_FILTER" ), myGroups[i] );

    myLineEdit[i]->setMaxLength(-1);
    myLabel   [i]->setText( tr( getLabelText( i, true )));

    layout->addWidget(myLabel    [i], 0, 0);
    layout->addWidget(selBtn,         0, 1);
    layout->addWidget(myLineEdit [i], 0, 2, 1, 2);
    layout->addWidget(myFilterBtn[i], 0, 4);
    layout->addWidget(myMeshChk  [i], 1, 0, 1, 5);
    layout->setColumnStretch( 2, 10 );

    connect( myMeshChk  [i], SIGNAL(toggled(bool)),               SLOT(onSelectMesh(bool)));
    connect( myFilterBtn[i], SIGNAL(clicked()),                   SLOT(setFilters()));
    connect( myLineEdit [i], SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
    myIDSource[i] = new SMESH::ListOfIDSources;
  }
  connect( mySelectBtnGrp, SIGNAL(buttonClicked (int)),           SLOT(onSelectType(int)));
  connect(mySelectionMgr, SIGNAL( currentSelectionChanged()),     SLOT(selectionIntoArgument()));

  // Costruction of the logical filter for the elements: mesh/sub-mesh/group
  QList<SUIT_SelectionFilter*> aListOfFilters;
  aListOfFilters.append(new SMESH_TypeFilter (SMESH::MESH));
  aListOfFilters.append(new SMESH_TypeFilter (SMESH::SUBMESH_VERTEX));
  aListOfFilters.append(new SMESH_TypeFilter (SMESH::GROUP_NODE));
  myFilter[0] = 
    new SMESH_LogicalFilter (aListOfFilters, SMESH_LogicalFilter::LO_OR, /*takeOwnership=*/true);
  aListOfFilters.append(0);
  aListOfFilters[0] = new SMESH_TypeFilter (SMESH::MESH);
  aListOfFilters[1] = new SMESH_TypeFilter (SMESH::SUBMESH_EDGE);
  aListOfFilters[2] = new SMESH_TypeFilter (SMESH::GROUP_EDGE);
  aListOfFilters[3] = new SMESH_TypeFilter (SMESH::IDSOURCE_EDGE); // for sub-mesh on group of EDGEs
  myFilter[1] = 
    new SMESH_LogicalFilter (aListOfFilters, SMESH_LogicalFilter::LO_OR, /*takeOwnership=*/true);
  aListOfFilters[0] = new SMESH_TypeFilter (SMESH::MESH);
  aListOfFilters[1] = new SMESH_TypeFilter (SMESH::SUBMESH_FACE);
  aListOfFilters[2] = new SMESH_TypeFilter (SMESH::GROUP_FACE);
  aListOfFilters[3] = new SMESH_TypeFilter (SMESH::IDSOURCE_FACE); // for sub-mesh on group of FACEs
  myFilter[2] = 
    new SMESH_LogicalFilter (aListOfFilters, SMESH_LogicalFilter::LO_OR, /*takeOwnership=*/true);

  myBusy = false;

  myMeshChk[0]->setChecked( true );
  myMeshChk[1]->setChecked( true );
  myMeshChk[2]->setChecked( true );
  mySelectBtnGrp->button(0)->click();
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

SMESHGUI_3TypesSelector::~SMESHGUI_3TypesSelector()
{
  myIDSource[0].out();
  myIDSource[1].out();
  myIDSource[2].out();

  delete myFilter[0];
  delete myFilter[1];
  delete myFilter[2];

  if ( myFilterDlg )
  {
    myFilterDlg->setParent( 0 );
    delete myFilterDlg;
    myFilterDlg = 0;
  }
  disconnect(mySelectionMgr, 0, this, 0);
}

//================================================================================
/*!
 * \brief Slot called when selection changes
 */
//================================================================================

void SMESHGUI_3TypesSelector::selectionIntoArgument()
{
  if (myBusy) return;

  // return if dialog box is inactive
  if ( !isEnabled() )
    return;

  // get a current element type
  int iType = mySelectBtnGrp->checkedId();
  if ( iType < 0 || iType > 2 )
    return;

  QString aString = "";
  int nbObjects = 0;

  // clear
  myBusy = true;
  myLineEdit[ iType ]->setText(aString);
  myIDSource[ iType ]->length (nbObjects);
  myBusy = false;
  if ( !myGroups[ iType ]->isEnabled() )
    return;

  SMESH::SetPointRepresentation(false);

  SALOME_ListIO selected;
  mySelectionMgr->selectedObjects( selected );

  if ( myMeshChk[ iType ]->isChecked() ) // objects selection
    myIDSource[ iType ]->length( selected.Extent() ); // reserve
  myIDSource[ iType ]->length(0);

  SALOME_ListIteratorOfListIO It( selected );
  for ( ; It.More(); It.Next() )
  {
    Handle(SALOME_InteractiveObject) IO = It.Value();

    // get selected mesh
    SMESH::SMESH_Mesh_var mesh = SMESH::GetMeshByIO(IO);
    if ( mesh->_is_nil() )
      continue;
    if ( !myMesh->_is_nil() &&
         IsAnythingSelected() &&
         myMesh->GetId() != mesh->GetId() )
      continue; // other mesh
    myMesh  = mesh;
    myIO    = IO;
    myActor = SMESH::FindActorByEntry( IO->getEntry() );

    if ( myMeshChk[ iType ]->isChecked() ) // objects selection
    {
      SMESH::SMESH_IDSource_var idSrc = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO);
      if ( idSrc->_is_nil() )
        continue;
      mesh = SMESH::SMESH_Mesh::_narrow( idSrc );
      if ( !mesh->_is_nil() ) // if a mesh is selected, stop iteration
      {
        nbObjects = 1;
        myIDSource[ iType ]->length( nbObjects );
        myIDSource[ iType ][ 0 ] = idSrc;
        aString = IO->getName();
        break;
      }
      else // several groups can be selected
      {
        myIDSource[ iType ]->length( nbObjects + 1 );
        myIDSource[ iType ][ nbObjects++ ] = idSrc;
        aString += " " + QString( IO->getName() ) + " ";
      }
    }
    else // get indices of selected elements
    {
      TColStd_IndexedMapOfInteger aMapIndex;
      mySelector->GetIndex(IO,aMapIndex);
      int nbElements = aMapIndex.Extent();
      if ( nbElements > 0 )
      {
        SMESH::long_array_var ids = new SMESH::long_array;
        ids->length( nbElements );
        for ( int i = 0; i < nbElements; ++i )
          aString += QString(" %1").arg( ids[ i ] = aMapIndex( i+1 ));
        addTmpIdSource( ids, iType, nbObjects++ );
      }
      break;
    }
  }

  myIDSource[ iType ]->length( nbObjects );

  myBusy = true;
  myLineEdit[ iType ]->setText(aString);
  myBusy = false;

  emit selectionChanged();
}

//================================================================================
/*!
 * \brief Slot called when text changes in myLineEdit
 */
//================================================================================

void SMESHGUI_3TypesSelector::onTextChange( const QString& theNewText )
{
  // return if busy
  if (myBusy) return;

  // get a current element type
  int iType = 0;
  QLineEdit* le = (QLineEdit*) sender();
  for ( ; iType < 3; ++iType )
    if ( myLineEdit[ iType ] == le )
      break;
  if ( iType < 0 || iType > 2 )
    return;
  if ( !myGroups[ iType ]->isEnabled() )
    return;

  myBusy = true;

  // hilight entered elements/nodes

  myIDSource[ iType ]->length( 0 );

  if ( !myMesh->_is_nil() )
  {
    QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);
    if ( aListId.count() > 0 )
    {
      SMDS_Mesh* aMesh = myActor ? myActor->GetObject()->GetMesh() : 0;

      SMESH::ElementType SMESHType = SMESH::ElementType ( iType+1 );
      SMDSAbs_ElementType SMDSType = SMDSAbs_ElementType( iType+1 );
      const bool isNode = ( SMDSType == SMDSAbs_Node );

      SMESH::long_array_var ids = new SMESH::long_array;
      ids->length( aListId.count() );
      TColStd_MapOfInteger newIndices;
      for (int i = 0; i < aListId.count(); i++) {
        int id = aListId[ i ].toInt();
        bool validId = false;
        if ( id > 0 ) {
          if ( aMesh ) {
            const SMDS_MeshElement * e;
            if ( isNode ) e = aMesh->FindNode( id );
            else          e = aMesh->FindElement( id );
            validId = ( e && e->GetType() == SMDSType );
          } else {
            validId = ( myMesh->GetElementType( id, !isNode ) == SMESHType );
          }
        }
        if ( validId && newIndices.Add( id ))
          ids[ newIndices.Extent()-1 ] = id;
      }
      if ( !newIndices.IsEmpty() ) {
        ids->length( newIndices.Extent() );
        addTmpIdSource( ids, iType, 0 );
      }
      mySelector->AddOrRemoveIndex(myIO, newIndices, false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetCurrentVtkView() )
        aViewWindow->highlight( myIO, true, true );
    }
  }

  emit selectionChanged();

  myBusy = false;
}

//================================================================================
/*!
 * \brief Creates from ids and stores a temporary IDSource
 */
//================================================================================

void SMESHGUI_3TypesSelector::addTmpIdSource( SMESH::long_array_var& ids, int iType, int index )
{
  SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
  SMESH::SMESH_IDSource_var idSrc =
    aMeshEditor->MakeIDSource( ids, SMESH::ElementType( iType+1 ));

  if ( (int) myIDSource[ iType ]->length() <= index )
    myIDSource[ iType ]->length( index + 1 );
  myIDSource[ iType ][ index ] = idSrc;

  myTmpIDSourceList.push_back( idSrc );
}

//================================================================================
/*!
 * \brief Slot called when myMeshChk is checked
 */
//================================================================================

void SMESHGUI_3TypesSelector::onSelectMesh( bool on )
{
  QCheckBox* send = (QCheckBox*)sender();
  for ( int iType = 0; iType < 3; ++iType )
    if ( send == myMeshChk[ iType ])
    {
      myLabel[ iType ]->setText( tr( getLabelText( iType, on )));
      myFilterBtn[ iType ]->setEnabled( !on );
      myIDSource [ iType ]->length(0);
      myBusy = true; 
      myLineEdit [ iType ]->setText("");
      myBusy = false; 
      myLineEdit [ iType ]->setReadOnly( on );
      myLineEdit [ iType ]->setValidator( on ? 0 : myIdValidator );
      mySelectBtnGrp->button(iType)->click();
      break;
    }
    else
    {
      
    }
}

//================================================================================
/*!
 * \brief Slot called when a selection button is clicked
 */
//================================================================================

void SMESHGUI_3TypesSelector::onSelectType(int iType)
{
  if ( iType < 0 || iType > 2 )
    return;

  myIDSource[ iType ]->length(0);
  myLineEdit[ iType ]->setText("");

  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearFilters();

  SVTK_ViewWindow* aViewWindow = SMESH::GetCurrentVtkView();
  if ( myMeshChk[ iType ]->isChecked() )
  {
    if ( aViewWindow ) aViewWindow->SetSelectionMode(ActorSelection);
    mySelectionMgr->installFilter( myFilter[ iType ]);
  }
  else if ( aViewWindow )
  {
    switch ( iType+1 ) {
    case SMESH::NODE: aViewWindow->SetSelectionMode(NodeSelection); break;
    case SMESH::EDGE: aViewWindow->SetSelectionMode(EdgeSelection); break;
    case SMESH::FACE: aViewWindow->SetSelectionMode(FaceSelection); break;
    }
  }

  myLineEdit[ iType ]->setFocus();

  connect(mySelectionMgr, SIGNAL( currentSelectionChanged()), SLOT( selectionIntoArgument()));
  selectionIntoArgument();
}

//================================================================================
/*!
 * \brief Slot called when "Set filter" is clicked
 */
//================================================================================

void SMESHGUI_3TypesSelector::setFilters()
{
  if ( myMesh->_is_nil() ) {
    SUIT_MessageBox::critical(this,
                              tr("SMESH_ERROR"),
                              tr("NO_MESH_SELECTED"));
    return;
  }
  if ( !myFilterDlg )
  {
    QList<int> types;
    types.append( SMESH::NODE );
    types.append( SMESH::EDGE );
    types.append( SMESH::FACE );
    myFilterDlg = new SMESHGUI_FilterDlg( SMESHGUI::GetSMESHGUI(), types );
  }

  QPushButton* send = (QPushButton*)sender();
  for ( int iType = 0; iType < 3; ++iType )
    if ( send == myFilterBtn[ iType ])
    {
      mySelectBtnGrp->button(iType)->click();

      myFilterDlg->Init( SMESH::ElementType( iType+1 ) );
      myFilterDlg->SetSelection();
      myFilterDlg->SetMesh( myMesh );
      myFilterDlg->SetSourceWg( myLineEdit[ iType ]);
      myFilterDlg->show();
      break;
    }
}

//================================================================================
/*!
 * \brief Clear selection
 */
//================================================================================

void SMESHGUI_3TypesSelector::Clear()
{
  myBusy = true;
  for ( int iType = 0; iType < 3; ++iType )
  {
    myIDSource[ iType ]->length(0);
    myLineEdit[ iType ]->setText("");
  }
  myBusy = false;
  selectionIntoArgument();
}

//================================================================================
/*!
 * \brief Enable/disable controls of a type
 */
//================================================================================

void SMESHGUI_3TypesSelector::SetEnabled( bool enable, SMESH::ElementType type )
{
  myBusy = true; 
  for ( int iType = 0; iType < 3; ++iType )
    if ( iType+1 == type || type == SMESH::ALL )
    {
      myGroups[ iType ]->setEnabled( enable );
      if ( !enable ) {
        myIDSource[ iType ]->length(0);
        myLineEdit[ iType ]->setText("");
      }
    }
  myBusy = false;
  selectionIntoArgument();
}

//================================================================================
/*!
 * \brief Checks if anything is selected
 */
//================================================================================

bool SMESHGUI_3TypesSelector::IsAnythingSelected( SMESH::ElementType type )
{
  int nbSel = 0;

  for ( int iType = 0; iType < 3; ++iType )
    if ( iType+1 == type || type == SMESH::ALL )
      nbSel += myIDSource[ iType ]->length();

  return nbSel;
}

//================================================================================
/*!
 * \brief Returns selected elements and most complex type of selected elements
 */
//================================================================================

SMESH::ElementType SMESHGUI_3TypesSelector::GetSelected( SMESH::ListOfIDSources & nodes,
                                                         SMESH::ListOfIDSources & edges,
                                                         SMESH::ListOfIDSources & faces )
{
  nodes = myIDSource[0];
  edges = myIDSource[1];
  faces = myIDSource[2];

  if ( myIDSource[2]->length() > 0 ) return SMESH::FACE;
  if ( myIDSource[1]->length() > 0 ) return SMESH::EDGE;
  if ( myIDSource[0]->length() > 0 ) return SMESH::NODE;
  return SMESH::ALL;
}

//=================================================================================
// function : SMESHGUI_ExtrusionDlg()
// purpose  : constructor
//=================================================================================

SMESHGUI_ExtrusionDlg::SMESHGUI_ExtrusionDlg (SMESHGUI* theModule)
  : SMESHGUI_PreviewDlg( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  SUIT_ResourceMgr* mgr = SMESH::GetResourceMgr( mySMESHGUI );
  QPixmap selectImage ( mgr->loadPixmap("SMESH", tr("ICON_SELECT")));
  QPixmap addImage    ( mgr->loadPixmap("SMESH", tr("ICON_APPEND")));
  QPixmap removeImage ( mgr->loadPixmap("SMESH", tr("ICON_REMOVE")));

  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle(tr("EXTRUSION_ALONG_LINE"));
  setSizeGripEnabled(true);

  QVBoxLayout* SMESHGUI_ExtrusionDlgLayout = new QVBoxLayout(this);
  SMESHGUI_ExtrusionDlgLayout->setSpacing(SPACING);
  SMESHGUI_ExtrusionDlgLayout->setMargin(MARGIN);

  /***************************************************************/
  GroupArguments = new QGroupBox(tr("SMESH_EXTRUSION"), this);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments);
  GroupArgumentsLayout->setSpacing(SPACING);
  GroupArgumentsLayout->setMargin(MARGIN);

  // Controls for elements selection
  SelectorWdg = new SMESHGUI_3TypesSelector( GroupArguments );

  ExtrMethod_RBut0 = new QRadioButton(GroupArguments);
  ExtrMethod_RBut0->setText( tr("SMESH_EXTRUSION_TO_DISTANCE") );
  ExtrMethod_RBut1 = new QRadioButton(GroupArguments);
  ExtrMethod_RBut1->setText( tr("SMESH_EXTRUSION_ALONG_VECTOR") );
  ExtrMethod_RBut2 = new QRadioButton(GroupArguments);
  ExtrMethod_RBut2->setText( tr("SMESH_EXTRUSION_BY_NORMAL") );

  //Control for the Distance selection
  TextLabelDistance = new QLabel(tr("SMESH_DISTANCE"), GroupArguments);
  
  TextLabelDx = new QLabel(tr("SMESH_X"), GroupArguments);
  SpinBox_Dx = new SMESHGUI_SpinBox(GroupArguments);
  
  TextLabelDy = new QLabel(tr("SMESH_Y"), GroupArguments);
  SpinBox_Dy = new SMESHGUI_SpinBox(GroupArguments);

  TextLabelDz = new QLabel(tr("SMESH_Z"), GroupArguments);
  SpinBox_Dz = new SMESHGUI_SpinBox(GroupArguments);

  // Controls for vector selection

  TextLabelVector = new QLabel(tr("SMESH_VECTOR"), GroupArguments);

  SelectVectorButton = new QPushButton( GroupArguments );
  SelectVectorButton->setIcon( selectImage );
  SelectVectorButton->setCheckable( true );
  SelectorWdg->GetButtonGroup()->addButton( SelectVectorButton );

  TextLabelVx = new QLabel(tr("SMESH_DX"), GroupArguments);
  SpinBox_Vx = new SMESHGUI_SpinBox(GroupArguments);

  TextLabelVy = new QLabel(tr("SMESH_DY"), GroupArguments);
  SpinBox_Vy = new SMESHGUI_SpinBox(GroupArguments);

  TextLabelVz = new QLabel(tr("SMESH_DZ"), GroupArguments);
  SpinBox_Vz = new SMESHGUI_SpinBox(GroupArguments);

  TextLabelDist = new QLabel(tr("SMESH_DISTANCE"), GroupArguments);
  SpinBox_VDist = new SMESHGUI_SpinBox(GroupArguments);

  // Controls for nb. steps defining
  TextLabelNbSteps = new QLabel(tr("SMESH_NUMBEROFSTEPS"), GroupArguments);
  SpinBox_NbSteps = new SalomeApp_IntSpinBox(GroupArguments);

  // CheckBox for groups generation
  MakeGroupsCheck = new QCheckBox(tr("SMESH_MAKE_GROUPS"), GroupArguments);

  // CheckBox for ByAverageNormal arg of ExtrusionByNormal()
  ByAverageNormalCheck = new QCheckBox(tr("BY_AVERAGE_NORMAL"), GroupArguments);

  // CheckBox for UseInputElemsOnly arg of ExtrusionByNormal()
  UseInputElemsOnlyCheck = new QCheckBox(tr("USE_INPUT_ELEMS_ONLY"), GroupArguments);

  //Preview check box
  myPreviewCheckBox = new QCheckBox(tr("PREVIEW"), GroupArguments);

  // Base point

  BasePointGrp = new QGroupBox(tr("BASE_POINT"), GroupArguments);
  BasePointGrp->setCheckable(true);
  BasePointGrp->setChecked(false);
  QHBoxLayout* BasePointGrpLayout = new QHBoxLayout(BasePointGrp);
  BasePointGrpLayout->setSpacing(SPACING); BasePointGrpLayout->setMargin(MARGIN);

  SelectBasePointButton = new QPushButton(BasePointGrp);
  SelectBasePointButton->setIcon(selectImage);
  SelectBasePointButton->setCheckable(true);
  SelectorWdg->GetButtonGroup()->addButton( SelectBasePointButton );

  QLabel* XLab  = new QLabel(tr("SMESH_X"), BasePointGrp);
  BasePoint_XSpin = new SMESHGUI_SpinBox(BasePointGrp);
  BasePoint_XSpin->SetValue(0.);
  QLabel* YLab  = new QLabel(tr("SMESH_Y"), BasePointGrp);
  BasePoint_YSpin = new SMESHGUI_SpinBox(BasePointGrp);
  BasePoint_YSpin->SetValue(0.);
  QLabel* ZLab  = new QLabel(tr("SMESH_Z"), BasePointGrp);
  BasePoint_ZSpin = new SMESHGUI_SpinBox(BasePointGrp);
  BasePoint_ZSpin->SetValue(0.);

  BasePointGrpLayout->addWidget(SelectBasePointButton);
  BasePointGrpLayout->addWidget(XLab);
  BasePointGrpLayout->addWidget(BasePoint_XSpin, 1);
  BasePointGrpLayout->addWidget(YLab);
  BasePointGrpLayout->addWidget(BasePoint_YSpin, 1);
  BasePointGrpLayout->addWidget(ZLab);
  BasePointGrpLayout->addWidget(BasePoint_ZSpin, 1);

  // Scales

  ScalesGrp = new QGroupBox(tr("SMESH_SCALES"), GroupArguments);
  QGridLayout* ScalesGrpLayout = new QGridLayout( ScalesGrp );
  ScalesGrpLayout->setSpacing(SPACING); ScalesGrpLayout->setMargin(MARGIN);

  ScalesList = new QListWidget( ScalesGrp );
  ScalesList->setSelectionMode(QListWidget::ExtendedSelection);

  AddScaleButton = new QToolButton( ScalesGrp );
  AddScaleButton->setIcon( addImage );

  RemoveScaleButton = new QToolButton( ScalesGrp );
  RemoveScaleButton->setIcon( removeImage );

  ScaleSpin = new SMESHGUI_SpinBox( ScalesGrp );
  ScaleSpin->SetValue(2);

  LinearScalesCheck = new QCheckBox(tr("LINEAR_SCALES"), ScalesGrp );

  ScalesGrpLayout->addWidget(ScalesList,        0, 0, 4, 1);
  ScalesGrpLayout->addWidget(AddScaleButton,    0, 1);
  ScalesGrpLayout->addWidget(RemoveScaleButton, 2, 1);
  ScalesGrpLayout->addWidget(ScaleSpin,         0, 2);
  ScalesGrpLayout->addWidget(LinearScalesCheck, 4, 0);
  ScalesGrpLayout->setRowMinimumHeight(1, 10);
  ScalesGrpLayout->setRowStretch(3, 10);

  // layouting
  GroupArgumentsLayout->addWidget(SelectorWdg,            0, 0, 1, 9);
  GroupArgumentsLayout->addWidget(ExtrMethod_RBut0,       1, 0, 1, 3);
  GroupArgumentsLayout->addWidget(ExtrMethod_RBut1,       1, 3, 1, 3);
  GroupArgumentsLayout->addWidget(ExtrMethod_RBut2,       1, 6, 1, 3);
  GroupArgumentsLayout->addWidget(TextLabelDistance,      2, 0);
  GroupArgumentsLayout->addWidget(TextLabelDx,            2, 2);
  GroupArgumentsLayout->addWidget(SpinBox_Dx,             2, 3);
  GroupArgumentsLayout->addWidget(TextLabelDy,            2, 4);
  GroupArgumentsLayout->addWidget(SpinBox_Dy,             2, 5);
  GroupArgumentsLayout->addWidget(TextLabelDz,            2, 6);
  GroupArgumentsLayout->addWidget(SpinBox_Dz,             2, 7);
  GroupArgumentsLayout->addWidget(TextLabelVector,        3, 0);
  GroupArgumentsLayout->addWidget(SelectVectorButton,     3, 1);
  GroupArgumentsLayout->addWidget(TextLabelVx,            3, 2);
  GroupArgumentsLayout->addWidget(SpinBox_Vx,             3, 3);
  GroupArgumentsLayout->addWidget(TextLabelVy,            3, 4);
  GroupArgumentsLayout->addWidget(SpinBox_Vy,             3, 5);
  GroupArgumentsLayout->addWidget(TextLabelVz,            3, 6);
  GroupArgumentsLayout->addWidget(SpinBox_Vz,             3, 7);
  GroupArgumentsLayout->addWidget(TextLabelDist,          4, 0);
  GroupArgumentsLayout->addWidget(SpinBox_VDist,          4, 3);
  GroupArgumentsLayout->addWidget(TextLabelNbSteps,       5, 0, 1, 3);
  GroupArgumentsLayout->addWidget(SpinBox_NbSteps,        5, 3);
  GroupArgumentsLayout->addWidget(ByAverageNormalCheck,   6, 0, 1, 4);
  GroupArgumentsLayout->addWidget(UseInputElemsOnlyCheck, 6, 4, 1, 4);
  GroupArgumentsLayout->addWidget(BasePointGrp,           7, 0, 1, 9);
  GroupArgumentsLayout->addWidget(ScalesGrp,              8, 0, 1, 9);
  GroupArgumentsLayout->addWidget(myPreviewCheckBox,      9, 0, 1, 8);
  GroupArgumentsLayout->addWidget(MakeGroupsCheck,        10,0, 1, 8);
  GroupArgumentsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 10, 0);

  /***************************************************************/
  GroupButtons = new QGroupBox(this);
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout(GroupButtons);
  GroupButtonsLayout->setSpacing(SPACING);
  GroupButtonsLayout->setMargin(MARGIN);

  buttonOk = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), GroupButtons);
  buttonOk->setAutoDefault(true);
  buttonOk->setDefault(true);
  buttonApply = new QPushButton(tr("SMESH_BUT_APPLY"), GroupButtons);
  buttonApply->setAutoDefault(true);
  buttonCancel = new QPushButton(tr("SMESH_BUT_CLOSE"), GroupButtons);
  buttonCancel->setAutoDefault(true);
  buttonHelp = new QPushButton(tr("SMESH_BUT_HELP"), GroupButtons);
  buttonHelp->setAutoDefault(true);

  GroupButtonsLayout->addWidget(buttonOk);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addWidget(buttonApply);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget(buttonCancel);
  GroupButtonsLayout->addWidget(buttonHelp);

  /***************************************************************/
  SMESHGUI_ExtrusionDlgLayout->addWidget(GroupArguments);
  SMESHGUI_ExtrusionDlgLayout->addWidget(GroupButtons);

  /* Initialisations */
  SpinBox_Vx->RangeStepAndValidator(COORD_MIN, COORD_MAX, 0.1, "length_precision");
  SpinBox_Vy->RangeStepAndValidator(COORD_MIN, COORD_MAX, 0.1, "length_precision");
  SpinBox_Vz->RangeStepAndValidator(COORD_MIN, COORD_MAX, 0.1, "length_precision");

  SpinBox_Dx->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_Dy->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_Dz->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  
  SpinBox_NbSteps->setRange(1, 999999);
  SpinBox_VDist->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");

  BasePoint_XSpin->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  BasePoint_YSpin->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  BasePoint_ZSpin->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  ScaleSpin->RangeStepAndValidator      (COORD_MIN, COORD_MAX, 1.0, "length_precision");

  ExtrMethod_RBut0->setChecked(true);
  UseInputElemsOnlyCheck->setChecked(true);
  MakeGroupsCheck->setChecked(true);

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox(this);

  myHelpFileName = "extrusion_page.html";

  Init();

  /***************************************************************/
  // signals and slots connections
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  connect(ExtrMethod_RBut0, SIGNAL(clicked()), this, SLOT(ClickOnRadio()));
  connect(ExtrMethod_RBut1, SIGNAL(clicked()), this, SLOT(ClickOnRadio()));
  connect(ExtrMethod_RBut2, SIGNAL(clicked()), this, SLOT(ClickOnRadio()));

  // to update state of the Ok & Apply buttons
  connect(SpinBox_Vx, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));
  connect(SpinBox_Vy, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));
  connect(SpinBox_Vz, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));
  connect(SpinBox_Dx, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));
  connect(SpinBox_Dy, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));
  connect(SpinBox_Dz, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));

  connect(AddScaleButton,    SIGNAL(clicked()), this, SLOT(OnScaleAdded()));
  connect(RemoveScaleButton, SIGNAL(clicked()), this, SLOT(OnScaleRemoved()));

  connect(SelectVectorButton,   SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectBasePointButton,SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(BasePointGrp,         SIGNAL(toggled(bool)), this, SLOT(SetEditCurrentArgument()));
  connect(BasePointGrp,         SIGNAL(toggled(bool)), SelectBasePointButton, SLOT(click()));
  connect(mySMESHGUI,           SIGNAL(SignalDeactivateActiveDialog()), SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr,       SIGNAL(currentSelectionChanged()), SLOT(toDisplaySimulation()));
  connect(SelectorWdg,          SIGNAL(selectionChanged()), this, SLOT(toDisplaySimulation()));
  connect(SelectorWdg,          SIGNAL(selectionChanged()), this, SLOT(CheckIsEnable()));
  /* to close dialog if study change */
  connect(mySMESHGUI,           SIGNAL(SignalCloseAllDialogs()),      this, SLOT(reject()));
  connect(mySMESHGUI,           SIGNAL(SignalActivatedViewManager()), this, SLOT(onOpenView()));
  connect(mySMESHGUI,           SIGNAL(SignalCloseView()),            this, SLOT(onCloseView()));

  connect(SpinBox_Dx,      SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Dy,      SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Dz,      SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Vx,      SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Vy,      SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Vz,      SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_VDist,   SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_NbSteps, SIGNAL(valueChanged(int)),    this, SLOT(toDisplaySimulation()));
  connect(ByAverageNormalCheck,   SIGNAL(toggled(bool)), this, SLOT(toDisplaySimulation()));
  connect(UseInputElemsOnlyCheck, SIGNAL(toggled(bool)), this, SLOT(toDisplaySimulation()));
  connect(AddScaleButton,         SIGNAL(clicked()),     this, SLOT(toDisplaySimulation()));
  connect(RemoveScaleButton,      SIGNAL(clicked()),     this, SLOT(toDisplaySimulation()));
  connect(LinearScalesCheck,      SIGNAL(toggled(bool)), this, SLOT(toDisplaySimulation()));
  connect(BasePointGrp,           SIGNAL(toggled(bool)), this, SLOT(toDisplaySimulation()));
  connect(BasePoint_XSpin, SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(BasePoint_YSpin, SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(BasePoint_ZSpin, SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));

  //To Connect preview check box
  connectPreviewControl();

  /***************************************************************/
  
  ClickOnRadio();
}

//=================================================================================
// function : ~SMESHGUI_ExtrusionDlg()
// purpose  : destructor
//=================================================================================
SMESHGUI_ExtrusionDlg::~SMESHGUI_ExtrusionDlg()
{
}

//=================================================================================
// function : Init()
// purpose  : initialization
//=================================================================================
void SMESHGUI_ExtrusionDlg::Init (bool ResetControls)
{
  if (ResetControls)
  {
    SpinBox_NbSteps->setValue(1);
    SpinBox_VDist->setValue(10);
    SpinBox_Dx->SetValue(0);
    SpinBox_Dy->SetValue(0);
    SpinBox_Dz->SetValue(0);
    SpinBox_Vx->SetValue(0);
    SpinBox_Vy->SetValue(0);
    SpinBox_Vz->SetValue(0);

    myPreviewCheckBox->setChecked(false);
    onDisplaySimulation(false);
  }
  SelectorWdg->Clear();
  CheckIsEnable();
}

//=================================================================================
// function : CheckIsEnable()
// purpose  : Check whether the Ok and Apply buttons should be enabled or not
//=================================================================================
void SMESHGUI_ExtrusionDlg::CheckIsEnable()
{  
  bool anIsEnable = SelectorWdg->IsAnythingSelected() && isValuesValid();

  buttonOk->setEnabled(anIsEnable);
  buttonApply->setEnabled(anIsEnable);
}

//=================================================================================
// function : isValuesValid()
// purpose  : Return true in case if values entered into dialog are valid
//=================================================================================
bool SMESHGUI_ExtrusionDlg::isValuesValid()
{
  double aX, aY, aZ, aModule = 0;
  if ( ExtrMethod_RBut0->isChecked() )
  {
    aX = SpinBox_Dx->GetValue();
    aY = SpinBox_Dy->GetValue();
    aZ = SpinBox_Dz->GetValue();
    aModule = sqrt(aX*aX + aY*aY + aZ*aZ);
  }
  else if ( ExtrMethod_RBut1->isChecked() )
  {
    aX = SpinBox_Vx->GetValue();
    aY = SpinBox_Vy->GetValue();
    aZ = SpinBox_Vz->GetValue();
    aModule = sqrt(aX*aX + aY*aY + aZ*aZ);
    double aVDist = (double)SpinBox_VDist->value();
    aModule *= aVDist;
  }
  else if ( ExtrMethod_RBut2->isChecked() )
  {
    aModule = Abs((double)SpinBox_VDist->value());
  }
  
  return aModule > 1.0E-38;
}

//=======================================================================
//function : getScaleParams
//purpose  : return 3 scaling parameters
//=======================================================================

bool SMESHGUI_ExtrusionDlg::getScaleParams( SMESH::double_array*& scales,
                                            SMESH::double_array*& basePoint )
{
  scales = new SMESH::double_array;
  scales->length( myScalesList.count() );
  for ( int i = 0; i < myScalesList.count(); ++i )
    (*scales)[i] = myScalesList[i];

  basePoint = new SMESH::double_array;
  if ( BasePointGrp->isChecked() )
  {
    basePoint->length( 3 );
    (*basePoint)[0] = BasePoint_XSpin->GetValue();
    (*basePoint)[1] = BasePoint_YSpin->GetValue();
    (*basePoint)[2] = BasePoint_ZSpin->GetValue();
  }

  return ( scales->length() > 0 && LinearScalesCheck->isChecked() );
}

//=================================================================================
// function : ClickOnRadio()
// purpose  : Radio button management
//=================================================================================

void SMESHGUI_ExtrusionDlg::ClickOnRadio()
{
  if ( ExtrMethod_RBut0->isChecked() )
  {
    TextLabelDistance->show();
    TextLabelDx->show();
    SpinBox_Dx->show();
    TextLabelDy->show();
    SpinBox_Dy->show();
    TextLabelDz->show();
    SpinBox_Dz->show();

    TextLabelVector->hide();
    TextLabelVx->hide();
    SpinBox_Vx->hide();
    TextLabelVy->hide();
    SpinBox_Vy->hide();
    TextLabelVz->hide();
    SpinBox_Vz->hide();
    TextLabelDist->hide();
    SpinBox_VDist->hide();
    SelectVectorButton->hide();

    ByAverageNormalCheck->hide();
    UseInputElemsOnlyCheck->hide();

    SelectorWdg->SetEnabled( true, SMESH::ALL );
  }
  else if ( ExtrMethod_RBut1->isChecked() )
  {
    TextLabelDistance->hide();
    TextLabelDx->hide();
    SpinBox_Dx->hide();
    TextLabelDy->hide();
    SpinBox_Dy->hide();
    TextLabelDz->hide();
    SpinBox_Dz->hide();

    TextLabelVector->show();
    TextLabelVx->show();
    SpinBox_Vx->show();
    TextLabelVy->show();
    SpinBox_Vy->show();
    TextLabelVz->show();
    SpinBox_Vz->show();
    TextLabelDist->show();
    SpinBox_VDist->show();
    SelectVectorButton->show();

    ByAverageNormalCheck->hide();
    UseInputElemsOnlyCheck->hide();

    SelectorWdg->SetEnabled( true, SMESH::ALL );
  }
  else if ( ExtrMethod_RBut2->isChecked() )
  {
    TextLabelDistance->hide();
    TextLabelDx->hide();
    SpinBox_Dx->hide();
    TextLabelDy->hide();
    SpinBox_Dy->hide();
    TextLabelDz->hide();
    SpinBox_Dz->hide();

    TextLabelVector->hide();
    TextLabelVx->hide();
    SpinBox_Vx->hide();
    TextLabelVy->hide();
    SpinBox_Vy->hide();
    TextLabelVz->hide();
    SpinBox_Vz->hide();

    TextLabelDist->show();
    SpinBox_VDist->show();
    SelectVectorButton->hide();

    ByAverageNormalCheck->show();
    UseInputElemsOnlyCheck->show();

    SelectorWdg->SetEnabled( false, SMESH::NODE );
    SelectorWdg->SetEnabled( false, SMESH::EDGE );
  }

  BasePointGrp->setEnabled( !ExtrMethod_RBut2->isChecked() );
  ScalesGrp   ->setEnabled( !ExtrMethod_RBut2->isChecked() );


  CheckIsEnable();

  onDisplaySimulation(true);
  // AdjustSize
  qApp->processEvents();
  updateGeometry();
  resize( minimumSizeHint() );
}

//=================================================================================
// function : ClickOnApply()
// purpose  : Called when user presses <Apply> button
//=================================================================================

bool SMESHGUI_ExtrusionDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;

  if (!isValid())
    return false;

  if ( SelectorWdg->IsAnythingSelected() )
  {
    SMESH::DirStruct aVector;
    getExtrusionVector(aVector);

    QStringList aParameters;
    if ( ExtrMethod_RBut0->isChecked() )
    {
      aParameters << SpinBox_Dx->text();
      aParameters << SpinBox_Dy->text();
      aParameters << SpinBox_Dz->text();
    }
    else if ( ExtrMethod_RBut1->isChecked() )
    {
      // only 3 coords in a python dump command :(
      // aParameters << SpinBox_Vx->text();
      // aParameters << SpinBox_Vy->text();
      // aParameters << SpinBox_Vz->text();
      // aParameters << SpinBox_VDist->text();
    }
    else if ( ExtrMethod_RBut2->isChecked() )
    {
      aParameters << SpinBox_VDist->text();
    }

    long aNbSteps = (long)SpinBox_NbSteps->value();
    aParameters << SpinBox_NbSteps->text();

    SMESH::double_array_var scales = new SMESH::double_array;
    scales->length( myScalesList.count() );
    for (int i = 0; i < myScalesList.count(); i++)
    {
      scales[i] = myScalesList[i];
      aParameters << ScalesList->item(i)->text();
    }

    bool meshHadNewTypeBefore = true;
    int  maxSelType = 0;
    const bool makeGroups = ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() );

    try
    {
      SUIT_OverrideCursor aWaitCursor;

      SMESH::SMESH_Mesh_var mesh = SelectorWdg->GetMesh();

      SMESH::ListOfIDSources_var nodes = new SMESH::ListOfIDSources();
      SMESH::ListOfIDSources_var edges = new SMESH::ListOfIDSources();
      SMESH::ListOfIDSources_var faces = new SMESH::ListOfIDSources();
      maxSelType = SelectorWdg->GetSelected( nodes, edges, faces );

      // is it necessary to switch on the next Display Mode?
      SMESH::ElementType newType = (SMESH::ElementType)( maxSelType + 1 );
      SMESH::array_of_ElementType_var oldTypes = mesh->GetTypes();
      meshHadNewTypeBefore = false;
      for ( size_t i = 0; i < oldTypes->length() && !meshHadNewTypeBefore; ++i )
        meshHadNewTypeBefore = ( oldTypes[i] >= newType );

      SMESH::SMESH_MeshEditor_var meshEditor = mesh->GetMeshEditor();
      SMESH::ListOfGroups_var groups;

      mesh->SetParameters( aParameters.join(":").toLatin1().constData() );

      if ( ExtrMethod_RBut2->isVisible() &&
           ExtrMethod_RBut2->isChecked() ) // Extrusion by normal
      {
        double stepSize          = (double) SpinBox_VDist->value();
        long   nbSteps           = (long) SpinBox_NbSteps->value();
        bool   useInputElemsOnly = UseInputElemsOnlyCheck->isChecked();
        bool   byAverageNormal   = ByAverageNormalCheck->isChecked();
        int    dim               = (maxSelType == SMESH::FACE) ? 2 : 1;

        groups = meshEditor->ExtrusionByNormal( faces, stepSize, nbSteps, useInputElemsOnly,
                                                byAverageNormal, makeGroups, dim );
      }
      else
      {
        SMESH::double_array_var scales, basePoint;
        bool linVariation = getScaleParams( scales.out(), basePoint.out() );
        groups = meshEditor->ExtrusionSweepObjects( nodes, edges, faces,
                                                    aVector, aNbSteps,
                                                    scales, linVariation, basePoint,
                                                    makeGroups );
      }

    } catch (...) {
    }

    SMESH_Actor* actor = SelectorWdg->GetActor();
    if ( actor && !meshHadNewTypeBefore )
    {
      unsigned int aMode = actor->GetEntityMode();
      switch ( maxSelType ) {
      case SMESH::NODE: // extrude node -> edges
        actor->SetRepresentation(SMESH_Actor::eEdge);
        actor->SetEntityMode( aMode |= SMESH_Actor::eEdges ); break;
      case SMESH::EDGE: // edge -> faces
        actor->SetRepresentation(SMESH_Actor::eSurface);
        actor->SetEntityMode( aMode |= SMESH_Actor::eFaces ); break;
      case SMESH::FACE: // faces -> volumes
        actor->SetRepresentation(SMESH_Actor::eSurface);
        actor->SetEntityMode( aMode |= SMESH_Actor::eVolumes ); break;
      }
    }
    if ( actor )
      SMESH::Update( actor->getIO(), actor->GetVisibility() );
    if ( makeGroups )
      mySMESHGUI->updateObjBrowser(true); // new groups may appear
    Init(false);
    mySelectionMgr->clearSelected();
    SelectorWdg->Clear();

    SMESHGUI::Modified();
  }
  return true;
}

//=================================================================================
// function : ClickOnOk()
// purpose  : Called when user presses <OK> button
//=================================================================================
void SMESHGUI_ExtrusionDlg::ClickOnOk()
{
  if (ClickOnApply())
    reject();
}

//=================================================================================
// function : reject()
// purpose  : Called when dialog box is closed
//=================================================================================
void SMESHGUI_ExtrusionDlg::reject()
{
  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearFilters();
  //mySelectionMgr->clearSelected();
  if (SMESH::GetCurrentVtkView()) {
    SMESH::RemoveFilters(); // PAL6938 -- clean all mesh entity filters
    SMESH::SetPointRepresentation(false);
    SMESH::SetPickable();
  }
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  mySMESHGUI->ResetState();

  QDialog::reject();
}

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::onOpenView()
{
  if ( mySelector ) {
    SMESH::SetPointRepresentation(false);
  }
  else {
    mySelector = SMESH::GetViewWindow( mySMESHGUI )->GetSelector();
    ActivateThisDialog();
  }
}

//=================================================================================
// function : onCloseView()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::onCloseView()
{
  DeactivateActiveDialog();
  mySelector = 0;
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::ClickOnHelp()
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
// function : SelectionIntoArgument()
// purpose  : Called when selection has changed or other case
//=================================================================================
void SMESHGUI_ExtrusionDlg::SelectionIntoArgument()
{
  // return if dialog box is inactive
  if (!GroupButtons->isEnabled())
    return;

  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);
  if ( aList.IsEmpty() || aList.Extent() > 1 )
    return;

  if ( SelectVectorButton->isChecked() )
  {
    Handle(SALOME_InteractiveObject) IO = aList.First();
    TColStd_IndexedMapOfInteger aMapIndex;
    mySelector->GetIndex(IO,aMapIndex);
    if ( aMapIndex.Extent() != 1 )
      return;
    SMESH_Actor* anActor = SMESH::FindActorByEntry( IO->getEntry() );
    SMDS_Mesh*     aMesh = anActor ? anActor->GetObject()->GetMesh() : 0;
    if ( !aMesh )
      return;

    const SMDS_MeshFace* face =
      dynamic_cast<const SMDS_MeshFace*>(aMesh->FindElement(aMapIndex(1)));
    if (!face)
      return;

    gp_XYZ aNormale = SMESH::getNormale(face);
    SpinBox_Vx->SetValue(aNormale.X());
    SpinBox_Vy->SetValue(aNormale.Y());
    SpinBox_Vz->SetValue(aNormale.Z());
  }
  else if ( SelectBasePointButton->isChecked() )
  {
    if (!BasePointGrp->isChecked())
      return;

    // try to get shape from selection
    Handle(SALOME_InteractiveObject) IO = aList.First();

    // check if geom vertex is selected
    GEOM::GEOM_Object_var aGeomObj = SMESH::IObjectToInterface<GEOM::GEOM_Object>(IO);
    TopoDS_Vertex aVertex;
    if (!aGeomObj->_is_nil()) {
      if (aGeomObj->IsShape() && GEOMBase::GetShape(aGeomObj, aVertex) && !aVertex.IsNull()) {
        gp_Pnt aPnt = BRep_Tool::Pnt(aVertex);
        BasePoint_XSpin->SetValue(aPnt.X());
        BasePoint_YSpin->SetValue(aPnt.Y());
        BasePoint_ZSpin->SetValue(aPnt.Z());
      }
    }

    if ( aVertex.IsNull() )
    {
      // check if smesh node is selected
      SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(IO);
      if (aMesh->_is_nil())
        return;

      QString aString;
      int aNbUnits = SMESH::GetNameOfSelectedNodes(mySelector, IO, aString);
      // return if more than one node is selected
      if (aNbUnits != 1)
        return;

      SMESH_Actor* aMeshActor = SMESH::FindActorByObject(aMesh);
      if (!aMeshActor)
        return;

      SMDS_Mesh* mesh = aMeshActor->GetObject()->GetMesh();
      if (!mesh)
        return;

      const SMDS_MeshNode* n = mesh->FindNode(aString.toLong());
      if (!n)
        return;

      BasePoint_XSpin->SetValue(n->X());
      BasePoint_YSpin->SetValue(n->Y());
      BasePoint_ZSpin->SetValue(n->Z());
    }
  }

  onDisplaySimulation(true);
  CheckIsEnable();
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();

  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearSelected();
  mySelectionMgr->clearFilters();

  if (send == SelectVectorButton)
  {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(FaceSelection);
  }
  else if ( send == SelectBasePointButton )
  {
    SMESH::SetPointRepresentation(true);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(NodeSelection);

    SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter(SMESH::IDSOURCE);
    SMESH_NumberFilter* aVertexFilter      = new SMESH_NumberFilter ("GEOM", TopAbs_VERTEX,
                                                                     1, TopAbs_VERTEX);
    QList<SUIT_SelectionFilter*> aListOfFilters;
    aListOfFilters << aMeshOrSubMeshFilter << aVertexFilter;

    mySelectionMgr->installFilter(new SMESH_LogicalFilter
                                  (aListOfFilters, SMESH_LogicalFilter::LO_OR, true));
  }
  
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  SelectionIntoArgument();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  : Deactivates this dialog
//=================================================================================
void SMESHGUI_ExtrusionDlg::DeactivateActiveDialog()
{
  if (GroupButtons->isEnabled())
  {
    GroupArguments->setEnabled(false);
    GroupButtons->setEnabled(false);
    SelectorWdg->setEnabled(false);
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  : Activates this dialog
//=================================================================================
void SMESHGUI_ExtrusionDlg::ActivateThisDialog()
{
  // Emit a signal to deactivate the active dialog
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupArguments->setEnabled(true);
  GroupButtons->setEnabled(true);
  SelectorWdg->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox(this);
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::enterEvent (QEvent*)
{
  if ( !GroupButtons->isEnabled() ) {
    SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    if ( aViewWindow && !mySelector) {
      mySelector = aViewWindow->GetSelector();
    }
    ActivateThisDialog();
  }
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}

//=================================================================================
// function : isValid
// purpose  :
//=================================================================================
bool SMESHGUI_ExtrusionDlg::isValid()
{
  QString msg;
  bool ok = true;
  if ( ExtrMethod_RBut0->isChecked() ) {
    ok = SpinBox_Dx->isValid( msg, true ) && ok;
    ok = SpinBox_Dy->isValid( msg, true ) && ok;
    ok = SpinBox_Dz->isValid( msg, true ) && ok;
  } else if ( ExtrMethod_RBut1->isChecked() ) {
    ok = SpinBox_Vx->isValid( msg, true ) && ok;
    ok = SpinBox_Vy->isValid( msg, true ) && ok;
    ok = SpinBox_Vz->isValid( msg, true ) && ok;
    ok = SpinBox_VDist->isValid( msg, true ) && ok;
  }
  ok = SpinBox_NbSteps->isValid( msg, true ) && ok;

  if ( BasePointGrp->isChecked()) {
    ok = BasePoint_XSpin->isValid( msg, true ) && ok;
    ok = BasePoint_YSpin->isValid( msg, true ) && ok;
    ok = BasePoint_ZSpin->isValid( msg, true ) && ok;
  }

  if( !ok ) {
    QString str( tr( "SMESH_INCORRECT_INPUT" ) );
    if ( !msg.isEmpty() )
      str += "\n" + msg;
    SUIT_MessageBox::critical( this, tr( "SMESH_ERROR" ), str );
    return false;
  }
  return true;
}

//=================================================================================
// function : onDisplaySimulation
// purpose  : Show/Hide preview
//=================================================================================
void SMESHGUI_ExtrusionDlg::onDisplaySimulation( bool toDisplayPreview )
{
  if (myPreviewCheckBox->isChecked() && toDisplayPreview) {
    if ( SelectorWdg->IsAnythingSelected() && isValid() && isValuesValid())
    {
      //Get input vector
      SMESH::DirStruct aVector;
      getExtrusionVector(aVector);

      //Get Number of the steps
      long aNbSteps = (long)SpinBox_NbSteps->value();
      try
      {
        SUIT_OverrideCursor aWaitCursor;

        SMESH::SMESH_Mesh_var             mesh = SelectorWdg->GetMesh();
        SMESH::SMESH_MeshEditor_var meshEditor = mesh->GetMeshEditPreviewer();
        SMESH::ListOfGroups_var         groups;

        SMESH::ListOfIDSources_var nodes = new SMESH::ListOfIDSources();
        SMESH::ListOfIDSources_var edges = new SMESH::ListOfIDSources();
        SMESH::ListOfIDSources_var faces = new SMESH::ListOfIDSources();
        const int  maxSelType = SelectorWdg->GetSelected( nodes, edges, faces );
        const bool makeGroups = false;

        if ( ExtrMethod_RBut2->isVisible() &&
             ExtrMethod_RBut2->isChecked() ) // Extrusion by normal
        {
          double stepSize          = (double) SpinBox_VDist->value();
          long   nbSteps           = (long) SpinBox_NbSteps->value();
          bool   useInputElemsOnly = UseInputElemsOnlyCheck->isChecked();
          bool   byAverageNormal   = ByAverageNormalCheck->isChecked();
          int    dim               = (maxSelType == SMESH::FACE) ? 2 : 1;

          groups = meshEditor->ExtrusionByNormal( faces, stepSize, nbSteps, useInputElemsOnly,
                                                  byAverageNormal, makeGroups, dim );
        }
        else
        {
          SMESH::double_array_var scales, basePoint;
          bool linVariation = getScaleParams( scales.out(), basePoint.out() );
          groups = meshEditor->ExtrusionSweepObjects( nodes, edges, faces,
                                                      aVector, aNbSteps,
                                                      scales, linVariation, basePoint,
                                                      makeGroups );
        }
        SMESH::MeshPreviewStruct_var aMeshPreviewStruct = meshEditor->GetPreviewData();
        mySimulation->SetData(aMeshPreviewStruct._retn());

      } catch (...) {
        hidePreview();
      }
    } else {
      hidePreview();
    }
  } else {
    hidePreview();
  }
}

//=================================================================================
// function : getExtrusionVector()
// purpose  : get direction of the extrusion
//=================================================================================
void SMESHGUI_ExtrusionDlg::getExtrusionVector(SMESH::DirStruct& aVector)
{
  if ( ExtrMethod_RBut0->isChecked() )
  {
    aVector.PS.x = SpinBox_Dx->GetValue();
    aVector.PS.y = SpinBox_Dy->GetValue();
    aVector.PS.z = SpinBox_Dz->GetValue();
  }
  else if ( ExtrMethod_RBut1->isChecked() )
  {
    gp_XYZ aNormale(SpinBox_Vx->GetValue(),
                    SpinBox_Vy->GetValue(),
                    SpinBox_Vz->GetValue());
    aNormale /= aNormale.Modulus();
    double aVDist = (double)SpinBox_VDist->value();

    aVector.PS.x = aNormale.X()*aVDist;
    aVector.PS.y = aNormale.Y()*aVDist;
    aVector.PS.z = aNormale.Z()*aVDist;
  }
}

//=======================================================================
// function : OnScaleAdded()
// purpose  : Called when user adds Scale to the list
//=======================================================================
void SMESHGUI_ExtrusionDlg::OnScaleAdded()
{
  QString msg;
  if( !ScaleSpin->isValid( msg, true ) ) {
    QString str( tr( "SMESH_INCORRECT_INPUT" ) );
    if ( !msg.isEmpty() )
      str += "\n" + msg;
    SUIT_MessageBox::critical( this, tr( "SMESH_ERROR" ), str );
    return;
  }
  ScalesList->addItem(ScaleSpin->text());
  myScalesList.append(ScaleSpin->GetValue());
}

//=======================================================================
// function : OnScaleRemoved()
// purpose  : Called when user removes Scale(s) from the list
//=======================================================================
void SMESHGUI_ExtrusionDlg::OnScaleRemoved()
{
  QList<QListWidgetItem*> aList = ScalesList->selectedItems();
  QListWidgetItem* anItem;
  int row = 0;
  foreach(anItem, aList) {
    row = ScalesList->row(anItem);
    myScalesList.removeAt(row);
    delete anItem;
  }
  ScalesList->setCurrentRow( row, QItemSelectionModel::Select );
}
