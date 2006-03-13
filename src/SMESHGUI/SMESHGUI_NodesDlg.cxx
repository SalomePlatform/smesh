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
//  File   : SMESHGUI_NodesDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_NodesDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"

#include "SMESH_Actor.h"
#include "SMESH_ActorUtils.h"
#include "SMESH_ObjectDef.h"

#include "SMDS_Mesh.hxx"
#include "SMDS_MeshNode.hxx"

#include "SUIT_Session.h"
#include "SUIT_OverrideCursor.h"
#include "SUIT_ViewWindow.h"
#include "SUIT_ViewManager.h"
#include "SUIT_MessageBox.h"
#include "SUIT_Desktop.h"

#include "SalomeApp_Study.h"
#include "LightApp_SelectionMgr.h"

#include "SVTK_Selector.h"
#include "SVTK_ViewWindow.h"

#include "SALOME_Actor.h"
#include "SALOME_ListIO.hxx"

#include "utilities.h"

// VTK Includes
#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkActorCollection.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>

// QT Includes
#include <qbuttongroup.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qvalidator.h>
#include <qevent.h>

using namespace std;


namespace SMESH {

  void AddNode (SMESH::SMESH_Mesh_ptr theMesh, float x, float y, float z)
  {
    SUIT_OverrideCursor wc;
    try {
      _PTR(SObject) aSobj = SMESH::FindSObject(theMesh);
      SMESH::SMESH_MeshEditor_var aMeshEditor = theMesh->GetMeshEditor();
      aMeshEditor->AddNode(x,y,z);
      _PTR(Study) aStudy = GetActiveStudyDocument();
      CORBA::Long anId = aStudy->StudyId();
      if (TVisualObjPtr aVisualObj = SMESH::GetVisualObj(anId, aSobj->GetID().c_str())) {
	aVisualObj->Update(true);
      }
    } catch (SALOME::SALOME_Exception& exc) {
      INFOS("Follow exception was cought:\n\t" << exc.details.text);
    } catch (const std::exception& exc) {
      INFOS("Follow exception was cought:\n\t" << exc.what());
    } catch (...) {
      INFOS("Unknown exception was cought !!!");
    }
  }

  class TNodeSimulation {
    SVTK_ViewWindow* myViewWindow;

    SALOME_Actor *myPreviewActor;
    vtkDataSetMapper* myMapper;
    vtkPoints* myPoints;

  public:
    TNodeSimulation(SVTK_ViewWindow* theViewWindow):
      myViewWindow(theViewWindow)
    {
      vtkUnstructuredGrid* aGrid = vtkUnstructuredGrid::New();

      // Create points
      myPoints = vtkPoints::New();
      myPoints->SetNumberOfPoints(1);
      myPoints->SetPoint(0,0.0,0.0,0.0);

      // Create cells
      vtkIdList *anIdList = vtkIdList::New();
      anIdList->SetNumberOfIds(1);

      vtkCellArray *aCells = vtkCellArray::New();
      aCells->Allocate(2, 0);

      vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
      aCellTypesArray->SetNumberOfComponents(1);
      aCellTypesArray->Allocate(1);

      anIdList->SetId(0, 0);
      aCells->InsertNextCell(anIdList);
      aCellTypesArray->InsertNextValue(VTK_VERTEX);

      vtkIntArray* aCellLocationsArray = vtkIntArray::New();
      aCellLocationsArray->SetNumberOfComponents(1);
      aCellLocationsArray->SetNumberOfTuples(1);

      aCells->InitTraversal();
      vtkIdType npts;
      aCellLocationsArray->SetValue(0, aCells->GetTraversalLocation(npts));

      aGrid->SetCells(aCellTypesArray, aCellLocationsArray, aCells);

      aGrid->SetPoints(myPoints);
      aGrid->SetCells(aCellTypesArray, aCellLocationsArray,aCells);
      aCellLocationsArray->Delete();
      aCellTypesArray->Delete();
      aCells->Delete();
      anIdList->Delete();

      // Create and display actor
      myMapper = vtkDataSetMapper::New();
      myMapper->SetInput(aGrid);
      aGrid->Delete();

      myPreviewActor = SALOME_Actor::New();
      myPreviewActor->SetInfinitive(true);
      myPreviewActor->VisibilityOff();
      myPreviewActor->PickableOff();
      myPreviewActor->SetMapper(myMapper);

      vtkProperty* aProp = vtkProperty::New();
      aProp->SetRepresentationToPoints();

      float anRGB[3];
      GetColor( "SMESH", "node_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 255, 0 ) );
      aProp->SetColor( anRGB[0], anRGB[1], anRGB[2] );

      float aPointSize = GetFloat( "SMESH:node_size", 3 );
      aProp->SetPointSize( aPointSize );

      myPreviewActor->SetProperty( aProp );
      aProp->Delete();

      myViewWindow->AddActor(myPreviewActor);
    }

    void SetPosition (float x, float y, float z)
    {
      myPoints->SetPoint(0,x,y,z);
      myPoints->Modified();
      SetVisibility(true);
    }

    void SetVisibility (bool theVisibility)
    {
      myPreviewActor->SetVisibility(theVisibility);
      RepaintCurrentView();
    }

    ~TNodeSimulation()
    {
      myViewWindow->RemoveActor(myPreviewActor);
      myPreviewActor->Delete();

      myMapper->RemoveAllInputs();
      myMapper->Delete();

      myPoints->Delete();
    }
  };
}

//=================================================================================
// class    : SMESHGUI_NodesDlg()
// purpose  :
//=================================================================================
SMESHGUI_NodesDlg::SMESHGUI_NodesDlg (SMESHGUI* theModule,
				      const char* name,
				      bool modal,
				      WFlags fl): 
  QDialog(SMESH::GetDesktop(theModule), 
	  name, 
	  modal, 
	  WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | Qt::WDestructiveClose),
  mySelector(SMESH::GetViewWindow(theModule)->GetSelector()),
  mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
  mySMESHGUI(theModule)
{
  mySimulation = new SMESH::TNodeSimulation(SMESH::GetViewWindow( mySMESHGUI ));

  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_NODE")));
  if (!name)
    setName("SMESHGUI_NodesDlg");
  resize(303, 185);
  setCaption(tr("MESH_NODE_TITLE"));
  setSizeGripEnabled(TRUE);
  SMESHGUI_NodesDlgLayout = new QGridLayout(this);
  SMESHGUI_NodesDlgLayout->setSpacing(6);
  SMESHGUI_NodesDlgLayout->setMargin(11);

  /***************************************************************/
  GroupButtons = new QGroupBox(this, "GroupButtons");
  GroupButtons->setGeometry(QRect(10, 10, 281, 48));
  GroupButtons->setTitle(tr("" ));
  GroupButtons->setColumnLayout(0, Qt::Vertical);
  GroupButtons->layout()->setSpacing(0);
  GroupButtons->layout()->setMargin(0);
  GroupButtonsLayout = new QGridLayout(GroupButtons->layout());
  GroupButtonsLayout->setAlignment(Qt::AlignTop);
  GroupButtonsLayout->setSpacing(6);
  GroupButtonsLayout->setMargin(11);
  buttonCancel = new QPushButton(GroupButtons, "buttonCancel");
  buttonCancel->setText(tr("SMESH_BUT_CLOSE" ));
  buttonCancel->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonCancel, 0, 3);
  buttonApply = new QPushButton(GroupButtons, "buttonApply");
  buttonApply->setText(tr("SMESH_BUT_APPLY" ));
  buttonApply->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonApply, 0, 1);
  QSpacerItem* spacer_9 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupButtonsLayout->addItem(spacer_9, 0, 2);
  buttonOk = new QPushButton(GroupButtons, "buttonOk");
  buttonOk->setText(tr("SMESH_BUT_OK" ));
  buttonOk->setAutoDefault(TRUE);
  buttonOk->setDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonOk, 0, 0);
  SMESHGUI_NodesDlgLayout->addWidget(GroupButtons, 2, 0);

  /***************************************************************/
  GroupConstructors = new QButtonGroup(this, "GroupConstructors");
  GroupConstructors->setTitle(tr("MESH_NODE" ));
  GroupConstructors->setExclusive(TRUE);
  GroupConstructors->setColumnLayout(0, Qt::Vertical);
  GroupConstructors->layout()->setSpacing(0);
  GroupConstructors->layout()->setMargin(0);
  GroupConstructorsLayout = new QGridLayout(GroupConstructors->layout());
  GroupConstructorsLayout->setAlignment(Qt::AlignTop);
  GroupConstructorsLayout->setSpacing(6);
  GroupConstructorsLayout->setMargin(11);
  Constructor1 = new QRadioButton(GroupConstructors, "Constructor1");
  Constructor1->setText(tr("" ));
  Constructor1->setPixmap(image0);
  Constructor1->setChecked(TRUE);
  GroupConstructorsLayout->addWidget(Constructor1, 0, 0);
  QSpacerItem* spacer_2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupConstructorsLayout->addItem(spacer_2, 0, 1);
  SMESHGUI_NodesDlgLayout->addWidget(GroupConstructors, 0, 0);

  /***************************************************************/
  GroupCoordinates = new QGroupBox(this, "GroupCoordinates");
  GroupCoordinates->setTitle(tr("SMESH_COORDINATES" ));
  GroupCoordinates->setColumnLayout(0, Qt::Vertical);
  GroupCoordinates->layout()->setSpacing(0);
  GroupCoordinates->layout()->setMargin(0);
  GroupCoordinatesLayout = new QGridLayout(GroupCoordinates->layout());
  GroupCoordinatesLayout->setAlignment(Qt::AlignTop);
  GroupCoordinatesLayout->setSpacing(6);
  GroupCoordinatesLayout->setMargin(11);
  TextLabel_X = new QLabel(GroupCoordinates, "TextLabel_X");
  TextLabel_X->setText(tr("SMESH_X" ));
  GroupCoordinatesLayout->addWidget(TextLabel_X, 0, 0);
  TextLabel_Y = new QLabel(GroupCoordinates, "TextLabel_Y");
  TextLabel_Y->setText(tr("SMESH_Y" ));
  GroupCoordinatesLayout->addWidget(TextLabel_Y, 0, 2);

  TextLabel_Z = new QLabel(GroupCoordinates, "TextLabel_Z");
  TextLabel_Z->setText(tr("SMESH_Z" ));
  GroupCoordinatesLayout->addWidget(TextLabel_Z, 0, 4);

  SpinBox_X = new SMESHGUI_SpinBox(GroupCoordinates, "SpinBox_X");
  GroupCoordinatesLayout->addWidget(SpinBox_X, 0, 1);

  SpinBox_Y = new SMESHGUI_SpinBox(GroupCoordinates, "SpinBox_Y");
  GroupCoordinatesLayout->addWidget(SpinBox_Y, 0, 3);

  SpinBox_Z = new SMESHGUI_SpinBox(GroupCoordinates, "SpinBox_Z");
  GroupCoordinatesLayout->addWidget(SpinBox_Z, 0, 5);

  SMESHGUI_NodesDlgLayout->addWidget(GroupCoordinates, 1, 0);

  /* Initialisation and display */
  Init();
}

//=======================================================================
// function : ~SMESHGUI_NodesDlg()
// purpose  : Destructor
//=======================================================================
SMESHGUI_NodesDlg::~SMESHGUI_NodesDlg()
{
  delete mySimulation;
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::Init ()
{
  /* Get setting of step value from file configuration */
  double step;
  // QString St = SUIT_CONFIG->getSetting("xxxxxxxxxxxxx");  TODO
  // step = St.toDouble();                                    TODO
  step = 25.0;

  /* min, max, step and decimals for spin boxes */
  SpinBox_X->RangeStepAndValidator(-999.999, +999.999, step, 3);
  SpinBox_Y->RangeStepAndValidator(-999.999, +999.999, step, 3);
  SpinBox_Z->RangeStepAndValidator(-999.999, +999.999, step, 3);
  SpinBox_X->SetValue(0.0);
  SpinBox_Y->SetValue(0.0);
  SpinBox_Z->SetValue(0.0);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  /* signals and slots connections */
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply, SIGNAL(clicked()), this, SLOT(ClickOnApply()));

  connect(SpinBox_X, SIGNAL (valueChanged(double)), SLOT(ValueChangedInSpinBox(double)));
  connect(SpinBox_Y, SIGNAL (valueChanged(double)), SLOT(ValueChangedInSpinBox(double)));
  connect(SpinBox_Z, SIGNAL (valueChanged(double)), SLOT(ValueChangedInSpinBox(double)));

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(SelectionIntoArgument()));
  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), SLOT(DeactivateActiveDialog()));
  /* to close dialog if study frame change */
  connect(mySMESHGUI, SIGNAL (SignalStudyFrameChanged()), SLOT(ClickOnCancel()));

  this->show();

  // set selection mode
  SMESH::SetPointRepresentation(true);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(NodeSelection);

  SelectionIntoArgument();
}

//=================================================================================
// function : ValueChangedInSpinBox()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::ValueChangedInSpinBox (double newValue)
{
  if (!myMesh->_is_nil()) {
    double vx = SpinBox_X->GetValue();
    double vy = SpinBox_Y->GetValue();
    double vz = SpinBox_Z->GetValue();

    mySimulation->SetPosition(vx,vy,vz);
  }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::ClickOnOk()
{
  if (ClickOnApply())
    ClickOnCancel();
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool SMESHGUI_NodesDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;

  if (myMesh->_is_nil()) {
    SUIT_MessageBox::warn1(SMESHGUI::desktop(), tr("SMESH_WRN_WARNING"),
                           tr("MESH_IS_NOT_SELECTED"), tr("SMESH_BUT_OK"));
    return false;
  }

  /* Recup args and call method */
  double x = SpinBox_X->GetValue();
  double y = SpinBox_Y->GetValue();
  double z = SpinBox_Z->GetValue();
  mySimulation->SetVisibility(false);
  SMESH::AddNode(myMesh,x,y,z);
  SMESH::SetPointRepresentation(true);

  // select myMesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);
  if (aList.Extent() != 1) {
    if (SVTK_ViewWindow* aViewWindow = SMESH::GetCurrentVtkView()) {
      vtkActorCollection *aCollection = aViewWindow->getRenderer()->GetActors();
      aCollection->InitTraversal();
      while (vtkActor *anAct = aCollection->GetNextActor()) {
        if (SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)) {
          if (anActor->hasIO()) {
            if (SMESH_MeshObj *aMeshObj = dynamic_cast<SMESH_MeshObj*>(anActor->GetObject().get())) {
              if (myMesh->_is_equivalent(aMeshObj->GetMeshServer())) {
                aList.Clear();
                aList.Append(anActor->getIO());
                mySelectionMgr->setSelectedObjects(aList, false);
                break;
              }
            }
          }
        }
      }
    }
  }
  return true;
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::ClickOnCancel()
{
  disconnect(mySelectionMgr, 0, this, 0);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);

  mySimulation->SetVisibility(false);
  SMESH::SetPointRepresentation(false);
  mySMESHGUI->ResetState();

  reject();
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_NodesDlg::SelectionIntoArgument()
{
  if (!GroupConstructors->isEnabled())
    return;

  mySimulation->SetVisibility(false);
  SMESH::SetPointRepresentation(true);

  const SALOME_ListIO& aList = mySelector->StoredIObjects();
  if (aList.Extent() == 1) {
    Handle(SALOME_InteractiveObject) anIO = aList.First();
    if (anIO->hasEntry()) {
      myMesh = SMESH::GetMeshByIO(anIO);
      if (myMesh->_is_nil()) return;
      QString aText;
      if (SMESH::GetNameOfSelectedNodes(mySelector,anIO,aText) == 1) {
	if (SMESH_Actor* anActor = SMESH::FindActorByObject(myMesh.in())) {
	  if (SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh()) {
	    if (const SMDS_MeshNode* aNode = aMesh->FindNode(aText.toInt())) {
	      SpinBox_X->SetValue(aNode->X());
	      SpinBox_Y->SetValue(aNode->Y());
	      SpinBox_Z->SetValue(aNode->Z());
            }
	  }
	}
      }
      mySimulation->SetPosition(SpinBox_X->GetValue(),
				SpinBox_Y->GetValue(),
				SpinBox_Z->GetValue());
    }
  }
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::closeEvent (QCloseEvent*)
{
  this->ClickOnCancel(); /* same than click on cancel button */
}

//=================================================================================
// function : hideEvent()
// purpose  : caused by ESC key
//=================================================================================
void SMESHGUI_NodesDlg::hideEvent (QHideEvent*)
{
  if (!isMinimized())
    ClickOnCancel();
}

//=================================================================================
// function : enterEvent()
// purpose  : to reactivate this dialog box when mouse enter onto the window
//=================================================================================
void SMESHGUI_NodesDlg::enterEvent(QEvent*)
{
  if (!GroupConstructors->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  : public slot to deactivate if active
//=================================================================================
void SMESHGUI_NodesDlg::DeactivateActiveDialog()
{
  if (GroupConstructors->isEnabled()) {
    GroupConstructors->setEnabled(false);
    GroupCoordinates->setEnabled(false);
    GroupButtons->setEnabled(false);
    mySimulation->SetVisibility(false);
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::ActivateThisDialog()
{
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupConstructors->setEnabled(true);
  GroupCoordinates->setEnabled(true);
  GroupButtons->setEnabled(true);

  SMESH::SetPointRepresentation(true);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(NodeSelection);

  SelectionIntoArgument();
}
