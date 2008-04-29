// SMESH SMESHGUI : GUI for SMESH component
//
// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
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
// File   : SMESHGUI_AddMeshElementDlg.cxx
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
//

// SMESH includes
#include "SMESHGUI_AddQuadraticElementDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"

#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>
#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_ViewManager.h>

#include <LightApp_SelectionMgr.h>

#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>

#include <SALOME_ListIO.hxx>

#include <SalomeApp_Application.h>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// VTK includes
#include <vtkIdList.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>
#include <vtkCellType.h>

// Qt includes
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QTableWidget>
#include <QKeyEvent>
#include <QButtonGroup>

// STL includes
#include <vector>

#define SPACING 6
#define MARGIN  11

namespace SMESH
{
  void ReverseConnectivity( std::vector<int> & ids, int type )
  {
    // for reverse connectivity of other types keeping the first id, see
    // void SMESH_VisualObjDef::buildElemPrs() in SMESH_Object.cxx:900
    const int* conn = 0;
   
    switch ( type ) {
    case QUAD_TETRAHEDRON: {
      static int aConn[] = {0,2,1,3,6,5,4,7,9,8};
      conn = aConn;
      break;
    }
    case QUAD_PYRAMID: {
      static int aConn[] = {0,3,2,1,4,8,7,6,5,9,12,11,10};
      conn = aConn;
      break;
    }
    case QUAD_PENTAHEDRON: {
      static int aConn[] = {0,2,1,3,5,4,8,7,6,11,10,9,12,14,13};
      conn = aConn;
      break;
    }
    case QUAD_HEXAHEDRON: {
      static int aConn[] = {0,3,2,1,4,7,6,5,11,10,9,8,15,14,13,12,16,19,18,17};
      conn = aConn;
      break;
    }
    case QUAD_EDGE: {
      static int aConn[] = {1,0,2};
      conn = aConn;
      break;
    }
    case QUAD_TRIANGLE: {
      static int aConn[] = {0,2,1,5,4,3};
      conn = aConn;
      break;
    }
    case QUAD_QUADRANGLE: {
      static int aConn[] = {0,3,2,1,7,6,5,4};
      conn = aConn;
      break;
    }
    default:;
    }
    if ( !conn ) {
      reverse( ids.begin(), ids.end() );
    }
    else {
      std::vector<int> aRevIds( ids.size() );
      for ( int i = 0; i < ids.size(); i++)
        aRevIds[ i ] = ids[ conn[ i ]];
      ids = aRevIds;
    }
  }

  class TElementSimulation {
    SalomeApp_Application* myApplication;
    SUIT_ViewWindow* myViewWindow;
    SVTK_ViewWindow* myVTKViewWindow;

    SALOME_Actor* myPreviewActor;
    vtkDataSetMapper* myMapper;
    vtkUnstructuredGrid* myGrid;
    //vtkProperty* myBackProp, *myProp;

    vtkFloatingPointType myRGB[3], myBackRGB[3];

  public:
    TElementSimulation (SalomeApp_Application* theApplication)
    {
      myApplication = theApplication;
      SUIT_ViewManager* mgr = theApplication->activeViewManager();
      if (!mgr) return;
      myViewWindow = mgr->getActiveView();
      myVTKViewWindow = GetVtkViewWindow(myViewWindow);

      myGrid = vtkUnstructuredGrid::New();

      // Create and display actor
      myMapper = vtkDataSetMapper::New();
      myMapper->SetInput(myGrid);

      myPreviewActor = SALOME_Actor::New();
      myPreviewActor->PickableOff();
      myPreviewActor->VisibilityOff();
      myPreviewActor->SetMapper(myMapper);

      vtkProperty* myProp = vtkProperty::New();
      GetColor( "SMESH", "fill_color", myRGB[0], myRGB[1], myRGB[2], QColor( 0, 170, 255 ) );
      myProp->SetColor( myRGB[0], myRGB[1], myRGB[2] );
      myPreviewActor->SetProperty( myProp );
      myProp->Delete();

      vtkProperty* myBackProp = vtkProperty::New();
      GetColor( "SMESH", "backface_color", myBackRGB[0], myBackRGB[1], myBackRGB[2], QColor( 0, 0, 255 ) );
      myBackProp->SetColor( myBackRGB[0], myBackRGB[1], myBackRGB[2] );
      myPreviewActor->SetBackfaceProperty( myBackProp );
      myBackProp->Delete();

      myVTKViewWindow->AddActor(myPreviewActor);
    }

    typedef std::vector<vtkIdType> TVTKIds;
    void SetPosition (SMESH_Actor* theActor,
                      const int    theType,
                      TVTKIds&     theIds,
		      const int    theMode,
                      const bool   theReverse)
    {
      vtkUnstructuredGrid *aGrid = theActor->GetUnstructuredGrid();
      myGrid->SetPoints(aGrid->GetPoints());

      //add points

      vtkIdType aType = 0;

      switch (theType) {
      case QUAD_EDGE:
        aType = VTK_QUADRATIC_EDGE;
        break;
      case QUAD_TRIANGLE:
        aType = VTK_QUADRATIC_TRIANGLE; 
        break;
      case QUAD_QUADRANGLE:
        aType = VTK_QUADRATIC_QUAD; 
        break;
      case QUAD_TETRAHEDRON:
        aType = VTK_QUADRATIC_TETRA; 
        break;
      case QUAD_PYRAMID:
        //aType = VTK_QUADRATIC_PYRAMID; // NOT SUPPORTED IN VTK4.2
        aType = VTK_CONVEX_POINT_SET;
        break;
      case QUAD_PENTAHEDRON:
        //aType = VTK_QUADRATIC_WEDGE; // NOT SUPPORTED IN VTK4.2
        aType = VTK_CONVEX_POINT_SET;
        break; 
      case QUAD_HEXAHEDRON:
        aType = VTK_QUADRATIC_HEXAHEDRON;
        break;
      }

      // take care of orientation
      if ( aType == VTK_CONVEX_POINT_SET ) {
        if ( theReverse && theMode == VTK_SURFACE ) {
          //myPreviewActor->GetProperty()->SetColor( myBackRGB[0], myBackRGB[1], myBackRGB[2] );
        }
      }
      else {
        // VTK cell connectivity opposites the MED one for volumic elements
        if ( theIds.size() > 8 ? !theReverse : theReverse ) {
          ReverseConnectivity( theIds, theType );
        }
      }
            
      myGrid->Reset();
      vtkIdList *anIds = vtkIdList::New();
      
      for (int i = 0, iEnd = theIds.size(); i < iEnd; i++) {
        anIds->InsertId(i,theIds[i]);
        //std::cout << i<< ": " << theIds[i] << std::endl;
      }
      
      myGrid->InsertNextCell(aType,anIds);
      anIds->Delete();
      
      myGrid->Modified();

      myPreviewActor->GetMapper()->Update();
      myPreviewActor->SetRepresentation( theMode );
      SetVisibility(true);

      // restore normal orientation
      if ( aType == VTK_CONVEX_POINT_SET ) {
        if ( theReverse  && theMode == VTK_SURFACE ) {
          //myPreviewActor->GetProperty()->SetColor( myRGB[0], myRGB[1], myRGB[2] );
        }
      }
    }


    void SetVisibility (bool theVisibility)
    {
      myPreviewActor->SetVisibility(theVisibility);
      RepaintCurrentView();
    }


    ~TElementSimulation()
    {
      if (FindVtkViewWindow(myApplication->activeViewManager(), myViewWindow)) {
	myVTKViewWindow->RemoveActor(myPreviewActor);
      }
      myPreviewActor->Delete();

      myMapper->RemoveAllInputs();
      myMapper->Delete();

      myGrid->Delete();

//       myProp->Delete();
//       myBackProp->Delete();
    }
  };
}


// Define the sequences of ids
static int FirstEdgeIds[] = {0};
static int LastEdgeIds[] =  {1};

static int FirstTriangleIds[] = {0,1,2};
static int LastTriangleIds[] =  {1,2,0};

static int FirstQuadrangleIds[] = {0,1,2,3};
static int LastQuadrangleIds[] =  {1,2,3,0};

static int FirstTetrahedronIds[] = {0,1,2,3,3,3};
static int LastTetrahedronIds[] =  {1,2,0,0,1,2};

static int FirstPyramidIds[] = {0,1,2,3,4,4,4,4};
static int LastPyramidIds[] =  {1,2,3,0,0,1,2,3};

static int FirstPentahedronIds[] = {0,1,2,3,4,5,0,1,2};
static int LastPentahedronIds[] =  {1,2,0,4,5,3,3,4,5};

static int FirstHexahedronIds[] = {0,1,2,3,4,5,6,7,0,1,2,3};
static int LastHexahedronIds[] =  {1,2,3,0,5,6,7,4,4,5,6,7};



class SMESHGUI_IdEditItem: public QTableWidgetItem
{
public:
    SMESHGUI_IdEditItem(const QString& text ):
      QTableWidgetItem(text, QTableWidgetItem::UserType+100) {};
  ~SMESHGUI_IdEditItem() {};

  QWidget* createEditor() const;
};

QWidget* SMESHGUI_IdEditItem::createEditor() const
{
  QLineEdit *aLineEdit = new QLineEdit(text(), tableWidget());
  aLineEdit->setValidator( new SMESHGUI_IdValidator(tableWidget(), 1) );
  return aLineEdit;
}

//=================================================================================
// function : SMESHGUI_AddQuadraticElementDlg()
// purpose  : constructor
//=================================================================================
SMESHGUI_AddQuadraticElementDlg::SMESHGUI_AddQuadraticElementDlg( SMESHGUI* theModule,
								  const int theType )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
    myType( theType )
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );

  SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>
    (SUIT_Session::session()->activeApplication());
  
  mySimulation = new SMESH::TElementSimulation (anApp);
  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  QString anElementName;

  switch ( myType ) {
  case QUAD_EDGE:
    anElementName = QString("QUADRATIC_EDGE");
    break;
  case QUAD_TRIANGLE:
    anElementName = QString("QUADRATIC_TRIANGLE");
    break; 
  case QUAD_QUADRANGLE:
    anElementName = QString("QUADRATIC_QUADRANGLE");
    break;
  case QUAD_TETRAHEDRON:
    anElementName = QString("QUADRATIC_TETRAHEDRON");
    break;
  case QUAD_PYRAMID:
    anElementName = QString("QUADRATIC_PYRAMID");
    break;
  case QUAD_PENTAHEDRON:
    anElementName = QString("QUADRATIC_PENTAHEDRON");
    break;
  case QUAD_HEXAHEDRON:
    anElementName = QString("QUADRATIC_HEXAHEDRON");
    break;
  default:
    myType = QUAD_EDGE;
    anElementName = QString("QUADRATIC_EDGE");
  }

  QString iconName           = tr(QString("ICON_DLG_%1").arg(anElementName).toLatin1().data());
  QString caption            = tr(QString("SMESH_ADD_%1_TITLE").arg(anElementName).toLatin1().data());
  QString argumentsGrTitle   = tr(QString("SMESH_ADD_%1").arg(anElementName).toLatin1().data());
  QString constructorGrTitle = tr(QString("SMESH_%1").arg(anElementName).toLatin1().data());
  
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", iconName));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  setWindowTitle(caption);
  
  setSizeGripEnabled(true);

  QVBoxLayout* aDialogLayout = new QVBoxLayout(this);
  aDialogLayout->setSpacing(SPACING);
  aDialogLayout->setMargin(MARGIN);

  /***************************************************************/
  GroupConstructors = new QGroupBox(constructorGrTitle, this);
  QButtonGroup* ButtonGroup = new QButtonGroup(this);
  QHBoxLayout* aGroupConstructorsLayout = new QHBoxLayout(GroupConstructors);
  aGroupConstructorsLayout->setSpacing(SPACING);
  aGroupConstructorsLayout->setMargin(MARGIN);

  myRadioButton1 = new QRadioButton(GroupConstructors);
  myRadioButton1->setIcon(image0);
  aGroupConstructorsLayout->addWidget(myRadioButton1);
  ButtonGroup->addButton(myRadioButton1, 0);

  /***************************************************************/
  GroupArguments = new QGroupBox(argumentsGrTitle, this);
  QGridLayout* aGroupArgumentsLayout = new QGridLayout(GroupArguments);
  aGroupArgumentsLayout->setSpacing(SPACING);
  aGroupArgumentsLayout->setMargin(MARGIN);

  QLabel* aCornerNodesLabel = new QLabel(tr("SMESH_CORNER_NODES"), GroupArguments);
  mySelectButton = new QPushButton(GroupArguments);
  mySelectButton->setIcon(image1);
  myCornerNodes = new QLineEdit(GroupArguments);

  myTable = new QTableWidget(GroupArguments);

  myReverseCB = new QCheckBox(tr("SMESH_REVERSE"), GroupArguments);

  aGroupArgumentsLayout->addWidget(aCornerNodesLabel, 0, 0);
  aGroupArgumentsLayout->addWidget(mySelectButton,    0, 1);
  aGroupArgumentsLayout->addWidget(myCornerNodes,     0, 2);
  aGroupArgumentsLayout->addWidget(myTable,           1, 0, 1, 3); 
  aGroupArgumentsLayout->addWidget(myReverseCB,       2, 0, 1, 3);
  
  /***************************************************************/
  GroupButtons = new QGroupBox(this);
  QHBoxLayout* aGroupButtonsLayout = new QHBoxLayout(GroupButtons);
  aGroupButtonsLayout->setSpacing(SPACING);
  aGroupButtonsLayout->setMargin(MARGIN);

  buttonOk = new QPushButton(tr("SMESH_BUT_OK"), GroupButtons);
  buttonOk->setAutoDefault(true);
  buttonOk->setDefault(true);
  buttonApply = new QPushButton(tr("SMESH_BUT_APPLY"), GroupButtons);
  buttonApply->setAutoDefault(true);
  buttonCancel = new QPushButton(tr("SMESH_BUT_CLOSE"), GroupButtons);
  buttonCancel->setAutoDefault(true);
  buttonHelp = new QPushButton(tr("SMESH_BUT_HELP"), GroupButtons);
  buttonHelp->setAutoDefault(true);

  aGroupButtonsLayout->addWidget(buttonOk);
  aGroupButtonsLayout->addSpacing(10);
  aGroupButtonsLayout->addWidget(buttonApply);
  aGroupButtonsLayout->addSpacing(10);
  aGroupButtonsLayout->addStretch();
  aGroupButtonsLayout->addWidget(buttonCancel);
  aGroupButtonsLayout->addWidget(buttonHelp);

  /***************************************************************/
  aDialogLayout->addWidget(GroupConstructors);
  aDialogLayout->addWidget(GroupArguments);
  aDialogLayout->addWidget(GroupButtons);

  Init(); /* Initialisations */
}

//=================================================================================
// function : ~SMESHGUI_AddQuadraticElementDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_AddQuadraticElementDlg::~SMESHGUI_AddQuadraticElementDlg()
{
  delete mySimulation;
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::Init()
{
  myRadioButton1->setChecked(true);
  myIsEditCorners = true;
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  
  myActor = 0;

  int aNumRows;

  switch (myType) {
  case QUAD_EDGE:
    aNumRows = 1;
    myNbCorners = 2;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_edges
    break;
  case QUAD_TRIANGLE:
    aNumRows = 3;
    myNbCorners = 3;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_triangles
    break;
  case QUAD_QUADRANGLE:
    aNumRows = 4;
    myNbCorners = 4;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_quadrangles
    break;
  case QUAD_TETRAHEDRON:
    aNumRows = 6;
    myNbCorners = 4;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_tetrahedrons
    break;
  case QUAD_PYRAMID:
    aNumRows = 8;
    myNbCorners = 5;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_pyramids
    break;
  case QUAD_PENTAHEDRON:
    aNumRows = 9;
    myNbCorners = 6;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_pentahedrons
    break; 
  case QUAD_HEXAHEDRON:
    aNumRows = 12;
    myNbCorners = 8;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_hexahedrons
    break;
  }
    
  myCornerNodes->setValidator(new SMESHGUI_IdValidator(this, myNbCorners));

  /* initialize table */
  myTable->setColumnCount(3);
  myTable->setRowCount(aNumRows);

  QStringList aColLabels;
  aColLabels.append(tr("SMESH_FIRST"));
  aColLabels.append(tr("SMESH_MIDDLE"));
  aColLabels.append(tr("SMESH_LAST"));
  myTable->setHorizontalHeaderLabels(aColLabels);
  
  for ( int col = 0; col < myTable->columnCount(); col++ )
    myTable->setColumnWidth(col, 80);

  //myTable->setColumnReadOnly(0, true); // VSR: TODO
  //myTable->setColumnReadOnly(2, true); // VSR: TODO

  myTable->setEnabled( false );
  
  for ( int row = 0; row < myTable->rowCount(); row++ )
  {
    myTable->setItem( row, 0, new QTableWidgetItem( "" ) );

    SMESHGUI_IdEditItem* anEditItem = new SMESHGUI_IdEditItem( "" );
    anEditItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
    myTable->setItem(row, 1, anEditItem);

    myTable->setItem( row, 2, new QTableWidgetItem( "" ) );
  }
  
  /* signals and slots connections */
  connect(mySelectButton, SIGNAL(clicked()), SLOT(SetEditCorners()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(SelectionIntoArgument()));
  connect(myTable,        SIGNAL(cellDoubleClicked(int, int)), SLOT(onCellDoubleClicked(int, int)));
  connect(myTable,        SIGNAL(cellChanged (int, int)), SLOT(onCellTextChange(int, int)));
  connect(myCornerNodes,  SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(myReverseCB,    SIGNAL(stateChanged(int)), SLOT(onReverse(int)));

  connect(buttonOk, SIGNAL(clicked()),     SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), SLOT(ClickOnCancel()));
  connect(buttonApply, SIGNAL(clicked()),  SLOT(ClickOnApply()));
  connect(buttonHelp, SIGNAL(clicked()),   SLOT(ClickOnHelp()));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), SLOT(DeactivateActiveDialog()));
  connect(mySMESHGUI, SIGNAL (SignalStudyFrameChanged()), SLOT(ClickOnCancel()));

  // set selection mode
  SMESH::SetPointRepresentation(true);

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( NodeSelection );

  myBusy = false;

  SetEditCorners();
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::ClickOnApply()
{
  if (IsValid() && !mySMESHGUI->isActiveStudyLocked()) {
    myBusy = true;
    
    std::vector<int> anIds;

    switch (myType) {
    case QUAD_EDGE:
      anIds.push_back(myTable->item(0, 0)->text().toInt());
      anIds.push_back(myTable->item(0, 2)->text().toInt());
      anIds.push_back(myTable->item(0, 1)->text().toInt());
      break;
    case QUAD_TRIANGLE:
    case QUAD_QUADRANGLE:
    case QUAD_TETRAHEDRON:
    case QUAD_PYRAMID:
    case QUAD_PENTAHEDRON:
    case QUAD_HEXAHEDRON:
      for ( int row = 0; row < myNbCorners; row++ )
	anIds.push_back(myTable->item(row, 0)->text().toInt());
      for ( int row = 0; row < myTable->rowCount(); row++ )
	anIds.push_back(myTable->item(row, 1)->text().toInt());
      break;
    }
    if ( myReverseCB->isChecked())
      SMESH::ReverseConnectivity( anIds, myType );
    
    int aNumberOfIds =  anIds.size();
    SMESH::long_array_var anArrayOfIdeces = new SMESH::long_array;
    anArrayOfIdeces->length( aNumberOfIds );
    
    for (int i = 0; i < aNumberOfIds; i++)
      anArrayOfIdeces[i] = anIds[ i ];

    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
    switch (myType) {
    case QUAD_EDGE:
      aMeshEditor->AddEdge(anArrayOfIdeces.inout()); break;
    case QUAD_TRIANGLE:
    case QUAD_QUADRANGLE:
      aMeshEditor->AddFace(anArrayOfIdeces.inout()); break;
    case QUAD_TETRAHEDRON:
    case QUAD_PYRAMID:
    case QUAD_PENTAHEDRON: 
    case QUAD_HEXAHEDRON:
      aMeshEditor->AddVolume(anArrayOfIdeces.inout()); break;
    }
    
    SALOME_ListIO aList; aList.Append( myActor->getIO() );
    mySelector->ClearIndex();
    mySelectionMgr->setSelectedObjects( aList, false );

    SMESH::UpdateView();
    mySimulation->SetVisibility(false);
    
    buttonOk->setEnabled(false);
    buttonApply->setEnabled(false);

    UpdateTable();
    SetEditCorners();

    myBusy = false;
  }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::ClickOnOk()
{
  ClickOnApply();
  ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::ClickOnCancel()
{
  mySelectionMgr->clearSelected();
  mySimulation->SetVisibility(false);
  SMESH::SetPointRepresentation(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( ActorSelection );
  disconnect(mySelectionMgr, 0, this, 0);
  mySMESHGUI->ResetState();
  reject();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::ClickOnHelp()
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
// function : onTextChange()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::onTextChange (const QString& theNewText)
{
  if (myBusy) return;
  myBusy = true;
  
  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  mySimulation->SetVisibility(false);

  // hilight entered nodes
  SMDS_Mesh* aMesh = 0;
  if (myActor)
    aMesh = myActor->GetObject()->GetMesh();

  if (aMesh) {
    TColStd_MapOfInteger newIndices;
    
    QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);
    bool allOk = true;
    for (int i = 0; i < aListId.count(); i++) {
      if( const SMDS_MeshNode * n = aMesh->FindNode( aListId[ i ].toInt() ) )
	newIndices.Add( n->GetID() );
      else
	{
	  allOk = false;
	  break;
	}
    }
    
    mySelector->AddOrRemoveIndex( myActor->getIO(), newIndices, false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->highlight( myActor->getIO(), true, true );
    
    if ( sender() == myCornerNodes )
      UpdateTable( allOk );
  }
  
  if( IsValid() ) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }

  if ( sender() == myTable )
    displaySimulation();
  
  myBusy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection has changed
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::SelectionIntoArgument()
{
  if (myBusy) return;
  
  if ( myIsEditCorners )
    {
      // clear
      myActor = 0;
      
      myBusy = true;
      myCornerNodes->setText("");
      myBusy = false;
      
      if (!GroupButtons->isEnabled()) // inactive
	return;
      
      buttonOk->setEnabled(false);
      buttonApply->setEnabled(false);
      
      mySimulation->SetVisibility(false);
      
      // get selected mesh
      SALOME_ListIO aList;
      mySelectionMgr->selectedObjects(aList,SVTK_Viewer::Type());
      
      if (aList.Extent() != 1)
	{
	  UpdateTable();
	  return;
	}
      
      Handle(SALOME_InteractiveObject) anIO = aList.First();
      myMesh = SMESH::GetMeshByIO(anIO);
      if (myMesh->_is_nil())
	return;
      
      myActor = SMESH::FindActorByEntry(anIO->getEntry());
  
    }
  
  if (!myActor)
    return;
  
  // get selected nodes
  QString aString = "";
  int nbNodes = SMESH::GetNameOfSelectedNodes(mySelector,myActor->getIO(),aString);
  
  if ( myIsEditCorners )
    {
      myBusy = true;
      myCornerNodes->setText(aString);
      myBusy = false;
      
      UpdateTable();
    }
  else if ( myTable->isEnabled() && nbNodes == 1 )
    {
      myBusy = true;
      int theRow = myTable->currentRow(), theCol = myTable->currentColumn();
      if ( theCol == 1 )
	myTable->item(theRow, 1)->setText(aString);
      myBusy = false;
    }
  
  if ( IsValid() )
    {
      buttonOk->setEnabled( true );
      buttonApply->setEnabled( true );
    }

  displaySimulation();
}

//=================================================================================
// function : displaySimulation()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::displaySimulation()
{
  if (!myIsEditCorners) {
    SMESH::TElementSimulation::TVTKIds anIds;
    
    // Collect ids from the dialog
    int anID;
    bool ok;
    int aDisplayMode = VTK_SURFACE;

    if ( myType == QUAD_EDGE )
      {
	anIds.push_back( myActor->GetObject()->GetNodeVTKId( myTable->item(0, 0)->text().toInt() ) );
	anIds.push_back( myActor->GetObject()->GetNodeVTKId( myTable->item(0, 2)->text().toInt() ) );
	anID = myTable->item(0, 1)->text().toInt(&ok);
	if (!ok) anID = myTable->item(0, 0)->text().toInt();
	anIds.push_back( myActor->GetObject()->GetNodeVTKId(anID) );
	aDisplayMode = VTK_WIREFRAME;
      }
    else
      {
	for ( int row = 0; row < myNbCorners; row++ )
	  anIds.push_back( myActor->GetObject()->GetNodeVTKId( myTable->item(row, 0)->text().toInt() ) );
	
	for ( int row = 0; row < myTable->rowCount(); row++ )
	  {
	    anID = myTable->item(row, 1)->text().toInt(&ok);
	    if (!ok) {
	      anID = myTable->item(row, 0)->text().toInt();
	      aDisplayMode = VTK_WIREFRAME;
	    }
	    anIds.push_back( myActor->GetObject()->GetNodeVTKId(anID) );
	  }
      }
    
    mySimulation->SetPosition(myActor,myType,anIds,aDisplayMode,myReverseCB->isChecked());
    SMESH::UpdateView();
  }
}

//=================================================================================
// function : SetEditCorners()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::SetEditCorners()
{
  myCornerNodes->setFocus();
  myIsEditCorners = true;
  
  SelectionIntoArgument();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::DeactivateActiveDialog()
{
  if (GroupConstructors->isEnabled()) {
    GroupConstructors->setEnabled(false);
    GroupArguments->setEnabled(false);
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
void SMESHGUI_AddQuadraticElementDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();

  GroupConstructors->setEnabled(true);
  GroupArguments->setEnabled(true);
  GroupButtons->setEnabled(true);

  SMESH::SetPointRepresentation(true);

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( NodeSelection );
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::enterEvent (QEvent*)
{
  if (GroupConstructors->isEnabled())
    return;
  ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::closeEvent (QCloseEvent*)
{
  /* same than click on cancel button */
  ClickOnCancel();
}

//=================================================================================
// function : hideEvent()
// purpose  : caused by ESC key
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::hideEvent (QHideEvent*)
{
  if (!isMinimized())
    ClickOnCancel();
}

//=================================================================================
// function : onReverse()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::onReverse (int state)
{
  if (!IsValid())
    return;

  if (state >= 0) {
    mySimulation->SetVisibility(false);
    displaySimulation();
  }
}


//=================================================================================
// function : IsValid()
// purpose  :
//=================================================================================
bool SMESHGUI_AddQuadraticElementDlg::IsValid()
{
  SMDS_Mesh* aMesh = 0;
  if (myActor)
    aMesh = myActor->GetObject()->GetMesh();
  if (!aMesh)
    return false;

  bool ok;
  
  for ( int row = 0; row < myTable->rowCount(); row++ )
    {
      int anID =  myTable->item(row, 1)->text().toInt(&ok);
      if ( !ok )
	return false;
      
      const SMDS_MeshNode * aNode = aMesh->FindNode(anID);
      if ( !aNode )
	return false;
    }
  
  return true;
}

//=================================================================================
// function : UpdateTable()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::UpdateTable( bool theConersValidity )
{
  QStringList aListCorners = myCornerNodes->text().split(" ", QString::SkipEmptyParts);
  
  if ( aListCorners.count() == myNbCorners && theConersValidity )
    {
      myTable->setEnabled( true );
      
      // clear the Middle column 
      for ( int row = 0; row < myTable->rowCount(); row++ )
	myTable->item( row, 1 )->setText("");
      
      int* aFirstColIds;
      int* aLastColIds;

      switch (myType) {
      case QUAD_EDGE:
	aFirstColIds = FirstEdgeIds;
	aLastColIds  = LastEdgeIds;
	break;
      case QUAD_TRIANGLE:
	aFirstColIds = FirstTriangleIds;
	aLastColIds  = LastTriangleIds;
	break;
      case QUAD_QUADRANGLE:
	aFirstColIds = FirstQuadrangleIds;
	aLastColIds  = LastQuadrangleIds;
	break;
      case QUAD_TETRAHEDRON:
	aFirstColIds = FirstTetrahedronIds;
	aLastColIds  = LastTetrahedronIds;
	break;
      case QUAD_PYRAMID:
	aFirstColIds = FirstPyramidIds;
	aLastColIds  = LastPyramidIds;
	break;
      case QUAD_PENTAHEDRON:
	aFirstColIds = FirstPentahedronIds;
	aLastColIds  = LastPentahedronIds;
	break; 
      case QUAD_HEXAHEDRON:
	aFirstColIds = FirstHexahedronIds;
	aLastColIds  = LastHexahedronIds;
	break;
      }
      
      // fill the First and the Last columns
      for (int i = 0, iEnd = myTable->rowCount(); i < iEnd; i++)
	myTable->item( i, 0 )->setText( aListCorners[ aFirstColIds[i] ] );
      
      for (int i = 0, iEnd = myTable->rowCount(); i < iEnd; i++)
	myTable->item( i, 2 )->setText( aListCorners[ aLastColIds[i] ] );
    }
  else
    {
      // clear table
      for ( int row = 0; row < myTable->rowCount(); row++ )
        for ( int col = 0; col < myTable->columnCount(); col++ )
	  if ( QTableWidgetItem* aTWI = myTable->item(row, col) ) aTWI->setText("");
      
      myTable->setEnabled( false );
    }
}


//=================================================================================
// function : onTableActivate()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::onCellDoubleClicked( int theRow, int theCol )
{
  if ( theCol == 1 )
    myIsEditCorners = false;
  
  displaySimulation();
}


//=================================================================================
// function : onCellTextChange()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::onCellTextChange(int theRow, int theCol)
{
  onTextChange( myTable->item(theRow, theCol)->text() );
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}
