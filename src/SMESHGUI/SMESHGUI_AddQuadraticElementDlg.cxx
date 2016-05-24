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
// File   : SMESHGUI_AddMeshElementDlg.cxx
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_AddQuadraticElementDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_GroupUtils.h"
#include "SMESHGUI_IdValidator.h"

#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>
#include <SMESH_FaceOrientationFilter.h>
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

#include <Qtx.h>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// VTK includes
#include <vtkIdList.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkCellType.h>

// Qt includes
#include <QComboBox>
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

namespace
{

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

  static std::vector<int> FirstPolygonIds;
  static std::vector<int> LastPolygonIds;

  void ReverseConnectivity( std::vector<vtkIdType> & ids, SMDSAbs_EntityType type,
                            bool toReverse, // inverse element
                            bool toVtkOrder ) // smds connectivity to vtk one
  {
    if ( toReverse ) // first reverse smds order
    {
      const std::vector<int>& index = SMDS_MeshCell::reverseSmdsOrder(type, ids.size());
      SMDS_MeshCell::applyInterlace( index, ids );
    }
    if ( toVtkOrder ) // from smds to vtk connectivity
    {
      const std::vector<int>& index = SMDS_MeshCell::toVtkOrder(type);
      SMDS_MeshCell::applyInterlace( index, ids );
    }
  }
}
namespace SMESH
{
  class TElementSimulationQuad
  {
    SalomeApp_Application* myApplication;
    SUIT_ViewWindow* myViewWindow;
    SVTK_ViewWindow* myVTKViewWindow;

    SALOME_Actor* myPreviewActor;
    vtkDataSetMapper* myMapper;
    vtkUnstructuredGrid* myGrid;
    //vtkProperty* myBackProp, *myProp;

    //double myRGB[3], myBackRGB[3];

    SALOME_Actor* myFaceOrientation;
    vtkPolyDataMapper* myFaceOrientationDataMapper;
    SMESH_FaceOrientationFilter* myFaceOrientationFilter;

  public:
    TElementSimulationQuad (SalomeApp_Application* theApplication)
    {
      myApplication = theApplication;
      SUIT_ViewManager* mgr = theApplication->activeViewManager();
      if (!mgr) return;
      myViewWindow = mgr->getActiveView();
      myVTKViewWindow = GetVtkViewWindow(myViewWindow);

      myGrid = vtkUnstructuredGrid::New();

      // Create and display actor
      myMapper = vtkDataSetMapper::New();
      myMapper->SetInputData(myGrid);

      myPreviewActor = SALOME_Actor::New();
      myPreviewActor->PickableOff();
      myPreviewActor->VisibilityOff();
      myPreviewActor->SetMapper(myMapper);

      QColor ffc, bfc;
      int delta;
      vtkProperty* myProp = vtkProperty::New();
      SMESH::GetColor( "SMESH", "preview_color", ffc, delta, "0, 255, 0|-100" ) ;
   
      myProp->SetColor( ffc.red() / 255. , ffc.green() / 255. , ffc.blue() / 255. );
      myPreviewActor->SetProperty( myProp );
      myProp->Delete();

      vtkProperty* myBackProp = vtkProperty::New();
      bfc = Qtx::mainColorToSecondary(ffc, delta);
      myBackProp->SetColor( bfc.red() / 255. , bfc.green() / 255. , bfc.blue() / 255. );
      myPreviewActor->SetBackfaceProperty( myBackProp );
      myBackProp->Delete();

      myVTKViewWindow->AddActor(myPreviewActor);

      // Orientation of faces
      myFaceOrientationFilter = SMESH_FaceOrientationFilter::New();
      myFaceOrientationFilter->SetInputData(myGrid);

      myFaceOrientationDataMapper = vtkPolyDataMapper::New();
      myFaceOrientationDataMapper->SetInputConnection(myFaceOrientationFilter->GetOutputPort());

      myFaceOrientation = SALOME_Actor::New();
      myFaceOrientation->PickableOff();
      myFaceOrientation->VisibilityOff();
      myFaceOrientation->SetMapper(myFaceOrientationDataMapper);

      vtkProperty* anOrientationProp = vtkProperty::New();
      double aRGB[3];
      GetColor( "SMESH", "orientation_color", aRGB[0], aRGB[1], aRGB[2], QColor( 255, 255, 255 ) );
      anOrientationProp->SetColor( aRGB[0], aRGB[1], aRGB[2] );
      myFaceOrientation->SetProperty( anOrientationProp );
      anOrientationProp->Delete();

      myVTKViewWindow->AddActor(myFaceOrientation);
    }

    typedef std::vector<vtkIdType> TVTKIds;
    void SetPosition (SMESH_Actor*       theActor,
                      SMDSAbs_EntityType theType,
                      TVTKIds&           theIds,
                      const int          theMode,
                      const bool         theReverse)
    {
      vtkUnstructuredGrid *aGrid = theActor->GetUnstructuredGrid();
      myGrid->SetPoints(aGrid->GetPoints());

      //add points

      ReverseConnectivity( theIds, theType, theReverse, /*toVtkOrder=*/true);

      myGrid->Reset();
      vtkIdList *anIds = vtkIdList::New();

      for (int i = 0, iEnd = theIds.size(); i < iEnd; i++) {
        anIds->InsertId(i,theIds[i]);
        //std::cout << i<< ": " << theIds[i] << std::endl;
      }

      vtkIdType aType = SMDS_MeshCell::toVtkType(theType);
      myGrid->InsertNextCell(aType,anIds);
      anIds->Delete();

      myGrid->Modified();

      myPreviewActor->GetMapper()->Update();
      myPreviewActor->SetRepresentation( theMode );
      SetVisibility(true, theActor->GetFacesOriented());
    }


    void SetVisibility (bool theVisibility, bool theShowOrientation = false)
    {
      myPreviewActor->SetVisibility(theVisibility);
      myFaceOrientation->SetVisibility(theShowOrientation);
      RepaintCurrentView();
    }


    ~TElementSimulationQuad()
    {
      if (FindVtkViewWindow(myApplication->activeViewManager(), myViewWindow)) {
        myVTKViewWindow->RemoveActor(myPreviewActor);
        myVTKViewWindow->RemoveActor(myFaceOrientation);
      }
      myPreviewActor->Delete();
      myFaceOrientation->Delete();

      myMapper->RemoveAllInputs();
      myMapper->Delete();

      myFaceOrientationFilter->Delete();

      myFaceOrientationDataMapper->RemoveAllInputs();
      myFaceOrientationDataMapper->Delete();

      myGrid->Delete();

      //       myProp->Delete();
      //       myBackProp->Delete();
    }
  };
}

/*!
  \class BusyLocker
  \brief Simple 'busy state' flag locker.
  \internal
*/
class BusyLocker
{
public:
  //! Constructor. Sets passed boolean flag to \c true.
  BusyLocker( bool& busy ) : myBusy( busy ) { myBusy = true; }
  //! Destructor. Clear external boolean flag passed as parameter to the constructor to \c false.
  ~BusyLocker() { myBusy = false; }
private:
  bool& myBusy; //! External 'busy state' boolean flag
};

/*!
  \class IdEditItem
  \brief Simple editable table item.
  \internal
*/
class IdEditItem: public QTableWidgetItem
{
public:
  IdEditItem(const QString& text );
  ~IdEditItem();

  QWidget* createEditor() const;
};

IdEditItem::IdEditItem(const QString& text )
  : QTableWidgetItem(text, QTableWidgetItem::UserType+100)
{
}

IdEditItem::~IdEditItem()
{
}

QWidget* IdEditItem::createEditor() const
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
                                                                  const SMDSAbs_EntityType theType )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
    myBusy( false ),
    myGeomType( theType )
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );

  SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>
    (SUIT_Session::session()->activeApplication());

  mySimulation = new SMESH::TElementSimulationQuad (anApp);
  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  QString anElementName;

  switch ( myGeomType ) {
  case SMDSEntity_Quad_Edge:
    anElementName = QString("QUADRATIC_EDGE");
    break;
  case SMDSEntity_Quad_Triangle:
    anElementName = QString("QUADRATIC_TRIANGLE");
    break;
  case SMDSEntity_Quad_Quadrangle:
    anElementName = QString("QUADRATIC_QUADRANGLE");
    break;
  case SMDSEntity_Quad_Polygon:
    anElementName = QString("QUADRATIC_POLYGON");
    break;
  case SMDSEntity_BiQuad_Quadrangle:
    anElementName = QString("BIQUADRATIC_QUADRANGLE");
    break;
  case SMDSEntity_BiQuad_Triangle:
    anElementName = QString("BIQUADRATIC_TRIANGLE");
    break;
  case SMDSEntity_Quad_Tetra:
    anElementName = QString("QUADRATIC_TETRAHEDRON");
    break;
  case SMDSEntity_Quad_Pyramid:
    anElementName = QString("QUADRATIC_PYRAMID");
    break;
  case SMDSEntity_Quad_Penta:
    anElementName = QString("QUADRATIC_PENTAHEDRON");
    break;
  case SMDSEntity_Quad_Hexa:
    anElementName = QString("QUADRATIC_HEXAHEDRON");
    break;
  case SMDSEntity_TriQuad_Hexa:
    anElementName = QString("TRIQUADRATIC_HEXAHEDRON");
    break;
  default:
    myGeomType = SMDSEntity_Quad_Edge;
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

  // Corner nodes
  QLabel* aCornerNodesLabel = new QLabel(tr("SMESH_CORNER_NODES"), GroupArguments);
  myCornerSelectButton = new QPushButton(GroupArguments);
  myCornerSelectButton->setIcon(image1);
  myCornerNodes = new QLineEdit(GroupArguments);

  // Mid-edge nodes
  myTable = new QTableWidget(GroupArguments);

  // Mid-face nodes
  myMidFaceLabel = new QLabel(tr("SMESH_MIDFACE_NODES"), GroupArguments);
  myMidFaceSelectButton = new QPushButton(GroupArguments);
  myMidFaceSelectButton->setIcon(image1);
  myMidFaceNodes = new QLineEdit(GroupArguments);
  myMidFaceNodes->setValidator(new SMESHGUI_IdValidator(this, 6));

  // Central node
  myCenterLabel = new QLabel(tr("SMESH_CENTER_NODE"), GroupArguments);
  myCenterSelectButton = new QPushButton(GroupArguments);
  myCenterSelectButton->setIcon(image1);
  myCenterNode = new QLineEdit(GroupArguments);
  myCenterNode->setValidator(new SMESHGUI_IdValidator(this, 1));

  myReverseCB = new QCheckBox(tr("SMESH_REVERSE"), GroupArguments);

  aGroupArgumentsLayout->addWidget(aCornerNodesLabel,     0, 0);
  aGroupArgumentsLayout->addWidget(myCornerSelectButton,  0, 1);
  aGroupArgumentsLayout->addWidget(myCornerNodes,         0, 2);
  aGroupArgumentsLayout->addWidget(myTable,               1, 0, 1, 3);
  aGroupArgumentsLayout->addWidget(myMidFaceLabel,        2, 0);
  aGroupArgumentsLayout->addWidget(myMidFaceSelectButton, 2, 1);
  aGroupArgumentsLayout->addWidget(myMidFaceNodes,        2, 2);
  aGroupArgumentsLayout->addWidget(myCenterLabel,         3, 0);
  aGroupArgumentsLayout->addWidget(myCenterSelectButton,  3, 1);
  aGroupArgumentsLayout->addWidget(myCenterNode,          3, 2);
  aGroupArgumentsLayout->addWidget(myReverseCB,           4, 0, 1, 3);

    /***************************************************************/
  GroupGroups = new QGroupBox( tr( "SMESH_ADD_TO_GROUP" ), this );
  GroupGroups->setCheckable( true );
  QHBoxLayout* GroupGroupsLayout = new QHBoxLayout(GroupGroups);
  GroupGroupsLayout->setSpacing(SPACING);
  GroupGroupsLayout->setMargin(MARGIN);

  TextLabel_GroupName = new QLabel( tr( "SMESH_GROUP" ), GroupGroups );
  ComboBox_GroupName = new QComboBox( GroupGroups );
  ComboBox_GroupName->setEditable( true );
  ComboBox_GroupName->setInsertPolicy( QComboBox::NoInsert );

  GroupGroupsLayout->addWidget( TextLabel_GroupName );
  GroupGroupsLayout->addWidget( ComboBox_GroupName, 1 );

  /***************************************************************/
  GroupButtons = new QGroupBox(this);
  QHBoxLayout* aGroupButtonsLayout = new QHBoxLayout(GroupButtons);
  aGroupButtonsLayout->setSpacing(SPACING);
  aGroupButtonsLayout->setMargin(MARGIN);

  buttonOk = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), GroupButtons);
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
  aDialogLayout->addWidget(GroupGroups);
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
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  /* reset "Add to group" control */
  GroupGroups->setChecked( false );

  myActor = 0;
  myNbMidFaceNodes = 0;
  myNbCenterNodes = 0;

  int aNumRows = 0;

  switch (myGeomType) {
  case SMDSEntity_Quad_Edge:
    aNumRows = 1;
    myNbCorners = 2;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_edges
    break;
  case SMDSEntity_Quad_Triangle:
    aNumRows = 3;
    myNbCorners = 3;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_triangles
    break;
  case SMDSEntity_BiQuad_Triangle:
    aNumRows = 3;
    myNbCorners = 3;
    myNbCenterNodes = 1;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_triangles
    break;
  case SMDSEntity_Quad_Quadrangle:
    aNumRows = 4;
    myNbCorners = 4;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_quadrangles
    break;
  case SMDSEntity_BiQuad_Quadrangle:
    aNumRows = 4;
    myNbCorners = 4;
    myNbCenterNodes = 1;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_quadrangles
    break;
  case SMDSEntity_Quad_Polygon:
    aNumRows = 5;
    myNbCorners = 0; // no limit
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_polygons
    break;
  case SMDSEntity_Quad_Tetra:
    aNumRows = 6;
    myNbCorners = 4;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_tetrahedrons
    break;
  case SMDSEntity_Quad_Pyramid:
    aNumRows = 8;
    myNbCorners = 5;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_pyramids
    break;
  case SMDSEntity_Quad_Penta:
    aNumRows = 9;
    myNbCorners = 6;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_pentahedrons
    break;
  case SMDSEntity_Quad_Hexa:
    aNumRows = 12;
    myNbCorners = 8;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_hexahedrons
    break;
  case SMDSEntity_TriQuad_Hexa:
    aNumRows = 12;
    myNbCorners = 8;
    myNbMidFaceNodes = 6;
    myNbCenterNodes = 1;
    myHelpFileName = "adding_quadratic_elements_page.html#?"; //Adding_hexahedrons
    break;
  default:;
  }

  myMidFaceLabel       ->setVisible( myNbMidFaceNodes );
  myMidFaceSelectButton->setVisible( myNbMidFaceNodes );
  myMidFaceNodes       ->setVisible( myNbMidFaceNodes );
  myCenterLabel        ->setVisible( myNbCenterNodes );
  myCenterSelectButton ->setVisible( myNbCenterNodes );
  myCenterNode         ->setVisible( myNbCenterNodes );

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
    myTable->item( row, 0 )->setFlags(0);

    IdEditItem* anEditItem = new IdEditItem( "" );
    anEditItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
    myTable->setItem(row, 1, anEditItem);

    myTable->setItem( row, 2, new QTableWidgetItem( "" ) );
    myTable->item( row, 2 )->setFlags(0);
  }

  /* signals and slots connections */
  connect(myCornerSelectButton, SIGNAL(clicked()), SLOT(SetCurrentSelection()));
  connect(myMidFaceSelectButton, SIGNAL(clicked()), SLOT(SetCurrentSelection()));
  connect(myCenterSelectButton, SIGNAL(clicked()), SLOT(SetCurrentSelection()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(SelectionIntoArgument()));
  connect(myTable,        SIGNAL(cellDoubleClicked(int, int)), SLOT(onCellDoubleClicked(int, int)));
  connect(myTable,        SIGNAL(cellChanged (int, int)), SLOT(onCellTextChange(int, int)));
  connect(myCornerNodes,  SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(myMidFaceNodes, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(myCenterNode,  SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(myReverseCB,    SIGNAL(stateChanged(int)), SLOT(onReverse(int)));

  connect(buttonOk, SIGNAL(clicked()),     SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), SLOT(reject()));
  connect(buttonApply, SIGNAL(clicked()),  SLOT(ClickOnApply()));
  connect(buttonHelp, SIGNAL(clicked()),   SLOT(ClickOnHelp()));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), SLOT(DeactivateActiveDialog()));
  connect(mySMESHGUI, SIGNAL (SignalStudyFrameChanged()), SLOT(reject()));
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), SLOT(reject()));
  connect(mySMESHGUI, SIGNAL (SignalActivatedViewManager()), SLOT(onOpenView()));
  connect(mySMESHGUI, SIGNAL (SignalCloseView()), SLOT(onCloseView()));

  myCurrentLineEdit = myCornerNodes;

  // set selection mode
  SMESH::SetPointRepresentation(true);

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( NodeSelection );

  SelectionIntoArgument();
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================

bool SMESHGUI_AddQuadraticElementDlg::ClickOnApply()
{
  if( !isValid() )
    return false;

  if ( mySMESHGUI->isActiveStudyLocked() || myBusy || !IsValid() )
    return false;

  BusyLocker lock( myBusy );

  std::vector<vtkIdType> anIds;

  switch (myGeomType) {
  case SMDSEntity_Quad_Edge:
    anIds.push_back(myTable->item(0, 0)->text().toInt());
    anIds.push_back(myTable->item(0, 2)->text().toInt());
    anIds.push_back(myTable->item(0, 1)->text().toInt());
    break;
  case SMDSEntity_Quad_Triangle:
  case SMDSEntity_Quad_Quadrangle:
  case SMDSEntity_Quad_Polygon:
  case SMDSEntity_BiQuad_Triangle:
  case SMDSEntity_BiQuad_Quadrangle:
  case SMDSEntity_Quad_Tetra:
  case SMDSEntity_Quad_Pyramid:
  case SMDSEntity_Quad_Penta:
  case SMDSEntity_Quad_Hexa:
  case SMDSEntity_TriQuad_Hexa:
    for ( int row = 0; row < myNbCorners; row++ )
      anIds.push_back(myTable->item(row, 0)->text().toInt());
    for ( int row = 0; row < myTable->rowCount(); row++ )
      anIds.push_back(myTable->item(row, 1)->text().toInt());
    if ( myNbMidFaceNodes )
    {
      QStringList aListId = myMidFaceNodes->text().split(" ", QString::SkipEmptyParts);
      for (int i = 0; i < aListId.count(); i++)
        anIds.push_back( aListId[ i ].toInt() );
    }
    if ( myNbCenterNodes )
    {
      QStringList aListId = myCenterNode->text().split(" ", QString::SkipEmptyParts);
      anIds.push_back( aListId[ 0 ].toInt() );
    }
    break;
  default:;
  }
  if ( myReverseCB->isChecked())
    ReverseConnectivity( anIds, myGeomType, /*toReverse=*/true, /*toVtkOrder=*/false );

  int aNumberOfIds = anIds.size();
  SMESH::long_array_var anArrayOfIdeces = new SMESH::long_array;
  anArrayOfIdeces->length( aNumberOfIds );

  for (int i = 0; i < aNumberOfIds; i++)
    anArrayOfIdeces[i] = anIds[ i ];

  bool addToGroup = GroupGroups->isChecked();
  QString aGroupName;

  SMESH::SMESH_GroupBase_var aGroup;
  int idx = 0;
  if( addToGroup ) {
    aGroupName = ComboBox_GroupName->currentText();
    for ( int i = 1; i <= ComboBox_GroupName->count(); i++ ) {
      QString aName = ComboBox_GroupName->itemText( i );
      if ( aGroupName == aName && ( i == ComboBox_GroupName->currentIndex() || idx == 0 ) )
        idx = i;
    }
    if ( idx > 0 && idx <= myGroups.count() ) {
      SMESH::SMESH_GroupOnGeom_var aGeomGroup = SMESH::SMESH_GroupOnGeom::_narrow( myGroups[idx-1] );
      if ( !aGeomGroup->_is_nil() ) {
        int res = SUIT_MessageBox::question( this, tr( "SMESH_WRN_WARNING" ),
                                             tr( "MESH_GEOM_GRP_CHOSEN" ).arg( aGroupName ),
                                             tr( "SMESH_BUT_YES" ), tr( "SMESH_BUT_NO" ), 0, 1 );
        if ( res == 1 ) return false;
      }
      SMESH::SMESH_GroupOnFilter_var aFilterGroup = SMESH::SMESH_GroupOnFilter::_narrow( myGroups[idx-1] );
      if ( !aFilterGroup->_is_nil() ) {
        int res = SUIT_MessageBox::question( this, tr( "SMESH_WRN_WARNING" ),
                                             tr( "MESH_FILTER_GRP_CHOSEN" ).arg( aGroupName ),
                                             tr( "SMESH_BUT_YES" ), tr( "SMESH_BUT_NO" ), 0, 1 );
        if ( res == 1 ) return false;
      }
      aGroup = myGroups[idx-1];
    }
  }

  SMESH::ElementType anElementType = SMESH::ALL;
  long anElemId = -1, nbElemsBefore = 0;
  SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
  switch (myGeomType) {
  case SMDSEntity_Quad_Edge:
    anElementType = SMESH::EDGE;
    nbElemsBefore = myMesh->NbEdges();
    anElemId = aMeshEditor->AddEdge(anArrayOfIdeces.inout()); break;
  case SMDSEntity_Quad_Triangle:
  case SMDSEntity_Quad_Quadrangle:
  case SMDSEntity_BiQuad_Triangle:
  case SMDSEntity_BiQuad_Quadrangle:
    anElementType = SMESH::FACE;
    nbElemsBefore = myMesh->NbFaces();
    anElemId = aMeshEditor->AddFace(anArrayOfIdeces.inout()); break;
  case SMDSEntity_Quad_Polygon:
    anElementType = SMESH::FACE;
    nbElemsBefore = myMesh->NbFaces();
    anElemId = aMeshEditor->AddQuadPolygonalFace(anArrayOfIdeces.inout()); break;
  case SMDSEntity_Quad_Tetra:
  case SMDSEntity_Quad_Pyramid:
  case SMDSEntity_Quad_Penta:
  case SMDSEntity_Quad_Hexa:
  case SMDSEntity_TriQuad_Hexa:
    anElementType = SMESH::VOLUME;
    nbElemsBefore = myMesh->NbVolumes();
    anElemId = aMeshEditor->AddVolume(anArrayOfIdeces.inout()); break;
  default: break;
  }

  if ( anElemId != -1 && addToGroup && !aGroupName.isEmpty() ) {
    SMESH::SMESH_Group_var aGroupUsed;
    if ( aGroup->_is_nil() ) {
      // create new group
      aGroupUsed = SMESH::AddGroup( myMesh, anElementType, aGroupName );
      if ( !aGroupUsed->_is_nil() ) {
        myGroups.append(SMESH::SMESH_GroupBase::_duplicate(aGroupUsed));
        ComboBox_GroupName->addItem( aGroupName );
      }
    }
    else {
      SMESH::SMESH_GroupOnGeom_var     aGeomGroup = SMESH::SMESH_GroupOnGeom::_narrow( aGroup );
      SMESH::SMESH_GroupOnFilter_var aFilterGroup = SMESH::SMESH_GroupOnFilter::_narrow( aGroup );
      if ( !aGeomGroup->_is_nil() ) {
        aGroupUsed = myMesh->ConvertToStandalone( aGeomGroup );
        if ( !aGroupUsed->_is_nil() && idx > 0 ) {
          myGroups[idx-1] = SMESH::SMESH_GroupBase::_duplicate(aGroupUsed);
          SMESHGUI::GetSMESHGUI()->getApp()->updateObjectBrowser();
        }
      }
      else if ( !aFilterGroup->_is_nil() ) {
        aGroupUsed = myMesh->ConvertToStandalone( aFilterGroup );
        if ( !aGroupUsed->_is_nil() && idx > 0 ) {
          myGroups[idx-1] = SMESH::SMESH_GroupBase::_duplicate(aGroupUsed);
          SMESHGUI::GetSMESHGUI()->getApp()->updateObjectBrowser();
        }
      }
      else
        aGroupUsed = SMESH::SMESH_Group::_narrow( aGroup );
    }

    if ( !aGroupUsed->_is_nil() ) {
      SMESH::long_array_var anIdList = new SMESH::long_array;
      anIdList->length( 1 );
      anIdList[0] = anElemId;
      aGroupUsed->Add( anIdList.inout() );
    }
  }

  if ( nbElemsBefore == 0  )
  {
    // 1st element of the type has been added, update actor to show this entity
    unsigned int aMode = myActor->GetEntityMode();
    switch ( anElementType ) {
    case SMESH::EDGE:
      myActor->SetRepresentation(SMESH_Actor::eEdge);
      myActor->SetEntityMode( aMode |= SMESH_Actor::eEdges ); break;
    case SMESH::FACE:
      myActor->SetRepresentation(SMESH_Actor::eSurface);
      myActor->SetEntityMode( aMode |= SMESH_Actor::eFaces ); break;
    case SMESH::VOLUME:
      myActor->SetRepresentation(SMESH_Actor::eSurface);
      myActor->SetEntityMode( aMode |= SMESH_Actor::eVolumes ); break;
    default:;
    }
  }

  SALOME_ListIO aList; aList.Append( myActor->getIO() );
  mySelector->ClearIndex();
  mySelectionMgr->setSelectedObjects( aList, false );

  mySimulation->SetVisibility(false);
  SMESH::UpdateView();

  UpdateTable();
  SetCurrentSelection();

  updateButtons();

  SMESHGUI::Modified();

  return true;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================

void SMESHGUI_AddQuadraticElementDlg::ClickOnOk()
{
  if ( ClickOnApply() )
    reject();
}

//=================================================================================
// function : reject()
// purpose  :
//=================================================================================

void SMESHGUI_AddQuadraticElementDlg::reject()
{
  mySelectionMgr->clearSelected();
  mySimulation->SetVisibility(false);
  SMESH::SetPointRepresentation(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( ActorSelection );
  disconnect(mySelectionMgr, 0, this, 0);
  mySMESHGUI->ResetState();
  QDialog::reject();
}

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::onOpenView()
{
  if ( mySelector && mySimulation ) {
    mySimulation->SetVisibility(false);
    SMESH::SetPointRepresentation(false);
  }
  else {
    mySelector = SMESH::GetViewWindow( mySMESHGUI )->GetSelector();
    mySimulation = new SMESH::TElementSimulationQuad(
      dynamic_cast<SalomeApp_Application*>( mySMESHGUI->application() ) );
    ActivateThisDialog();
  }
}

//=================================================================================
// function : onCloseView()
// purpose  :
//=================================================================================
void SMESHGUI_AddQuadraticElementDlg::onCloseView()
{
  DeactivateActiveDialog();
  mySelector = 0;
  delete mySimulation;
  mySimulation = 0;
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
  BusyLocker lock( myBusy );

  mySimulation->SetVisibility(false);

  // hilight entered nodes
  SMDS_Mesh* aMesh = 0;
  if (myActor)
    aMesh = myActor->GetObject()->GetMesh();

  QLineEdit* send = (QLineEdit*)sender();
  if (send == myCornerNodes ||
      send == myMidFaceNodes ||
      send == myCenterNode)
    myCurrentLineEdit = send;

  if (aMesh) {
    TColStd_MapOfInteger newIndices;

    QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);
    bool allOk = true;
    for (int i = 0; i < aListId.count(); i++) {
      if ( const SMDS_MeshNode * n = aMesh->FindNode( aListId[ i ].toInt() ) )
      {
        newIndices.Add( n->GetID() );
      }
      else
      {
        allOk = false;
        break;
      }
    }

    mySelector->AddOrRemoveIndex( myActor->getIO(), newIndices, false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->highlight( myActor->getIO(), true, true );

    if ( myCurrentLineEdit == myCornerNodes )
      UpdateTable( allOk );
  }

  updateButtons();
  displaySimulation();
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection has changed
//=================================================================================

void SMESHGUI_AddQuadraticElementDlg::SelectionIntoArgument()
{
  if (myBusy) return;
  BusyLocker lock( myBusy );

  QString aCurrentEntry = myEntry;

  if ( myCurrentLineEdit )
  {
    // clear
    myActor = 0;

    myCurrentLineEdit->setText("");

    if (!GroupButtons->isEnabled()) // inactive
      return;

    mySimulation->SetVisibility(false);

    // get selected mesh
    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects(aList);

    if (aList.Extent() != 1)
    {
      UpdateTable();
      updateButtons();
      return;
    }

    Handle(SALOME_InteractiveObject) anIO = aList.First();
    myEntry = anIO->getEntry();
    myMesh = SMESH::GetMeshByIO(anIO);
    if (myMesh->_is_nil()) {
      updateButtons();
      return;
    }

    myActor = SMESH::FindActorByEntry(anIO->getEntry());

  }

  // process groups
  if ( !myMesh->_is_nil() && myEntry != aCurrentEntry ) {
    SMESH::ElementType anElementType = SMESH::ALL;
    switch ( myGeomType ) {
    case SMDSEntity_Quad_Edge:
      anElementType = SMESH::EDGE; break;
    case SMDSEntity_Quad_Triangle:
    case SMDSEntity_Quad_Quadrangle:
    case SMDSEntity_Quad_Polygon:
    case SMDSEntity_BiQuad_Triangle:
    case SMDSEntity_BiQuad_Quadrangle:
      anElementType = SMESH::FACE; break;
    case SMDSEntity_Quad_Tetra:
    case SMDSEntity_Quad_Pyramid:
    case SMDSEntity_Quad_Penta:
    case SMDSEntity_Quad_Hexa:
    case SMDSEntity_TriQuad_Hexa:
      anElementType = SMESH::VOLUME; break;
    default:;
    }
    myGroups.clear();
    ComboBox_GroupName->clear();
    ComboBox_GroupName->addItem( QString() );
    SMESH::ListOfGroups aListOfGroups = *myMesh->GetGroups();
    for ( int i = 0, n = aListOfGroups.length(); i < n; i++ ) {
      SMESH::SMESH_GroupBase_var aGroup = aListOfGroups[i];
      if ( !aGroup->_is_nil() && aGroup->GetType() == anElementType ) {
        QString aGroupName( aGroup->GetName() );
        if ( !aGroupName.isEmpty() ) {
          myGroups.append(SMESH::SMESH_GroupBase::_duplicate(aGroup));
          ComboBox_GroupName->addItem( aGroupName );
        }
      }
    }
  }

  if (!myActor) {
    updateButtons();
    return;
  }

  // get selected nodes
  QString aString = "";
  int nbNodes = SMESH::GetNameOfSelectedNodes(mySelector,myActor->getIO(),aString);

  if ( myCurrentLineEdit )
  {
    if ( myCurrentLineEdit != myCenterNode || nbNodes == 1 )
      myCurrentLineEdit->setText(aString);

    if ( myCurrentLineEdit == myCornerNodes )
      UpdateTable();
  }
  else if ( myTable->isEnabled() && nbNodes == 1 )
  {
    int theRow = myTable->currentRow(), theCol = myTable->currentColumn();
    if ( theCol == 1 )
      myTable->item(theRow, 1)->setText(aString);
  }

  updateButtons();
  displaySimulation();
}

//=================================================================================
// function : displaySimulation()
// purpose  :
//=================================================================================

void SMESHGUI_AddQuadraticElementDlg::displaySimulation()
{
  if ( IsValid() )
  {
    SMESH::TElementSimulationQuad::TVTKIds anIds;

    // Collect ids from the dialog
    int anID;
    bool ok;
    int aDisplayMode = VTK_SURFACE;

    if ( myGeomType == SMDSEntity_Quad_Edge )
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
      if ( myNbMidFaceNodes )
      {
        QStringList aListId = myMidFaceNodes->text().split(" ", QString::SkipEmptyParts);
        for (int i = 0; i < aListId.count(); i++)
          anIds.push_back( myActor->GetObject()->GetNodeVTKId( aListId[ i ].toInt() ));
      }
      if ( myNbCenterNodes )
      {
        QStringList aListId = myCenterNode->text().split(" ", QString::SkipEmptyParts);
        anIds.push_back( myActor->GetObject()->GetNodeVTKId( aListId[ 0 ].toInt() ));
      }
    }

    mySimulation->SetPosition(myActor,myGeomType,anIds,aDisplayMode,myReverseCB->isChecked());
  }
  else
  {
    mySimulation->SetVisibility(false);
  }
  SMESH::UpdateView();
}

//=================================================================================
// function : SetCurrentSelection()
// purpose  :
//=================================================================================

void SMESHGUI_AddQuadraticElementDlg::SetCurrentSelection()
{
  QPushButton* send = (QPushButton*)sender();
  myCurrentLineEdit = 0;

  if (send == myCornerSelectButton)
    myCurrentLineEdit = myCornerNodes;
  else if ( send == myMidFaceSelectButton )
    myCurrentLineEdit = myMidFaceNodes;
  else if ( send == myCenterSelectButton )
    myCurrentLineEdit = myCenterNode;

  if ( myCurrentLineEdit )
  {
    myCurrentLineEdit->setFocus();
    SelectionIntoArgument();
  }
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
  if ( !GroupConstructors->isEnabled() ) {
    SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    if ( aViewWindow && !mySelector && !mySimulation) {
      mySelector = aViewWindow->GetSelector();
      mySimulation = new SMESH::TElementSimulationQuad(
        dynamic_cast<SalomeApp_Application*>( mySMESHGUI->application() ) );
    }
    ActivateThisDialog();
  }
}

//=================================================================================
// function : onReverse()
// purpose  :
//=================================================================================

void SMESHGUI_AddQuadraticElementDlg::onReverse (int state)
{
  mySimulation->SetVisibility(false);
  displaySimulation();
  updateButtons();
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
  std::set< int > okIDs;
  for ( int row = 0; row < myTable->rowCount(); row++ )
  {
    int anID =  myTable->item(row, 1)->text().toInt(&ok);
    if ( !ok )
      return false;

    const SMDS_MeshNode * aNode = aMesh->FindNode(anID);
    if ( !aNode )
      return false;
    okIDs.insert( anID );
  }

  QStringList aListId;
  if ( myNbMidFaceNodes )
    aListId += myMidFaceNodes->text().split(" ", QString::SkipEmptyParts);
  if ( myNbCenterNodes )
    aListId += myCenterNode->text().split(" ", QString::SkipEmptyParts);

  for (int i = 0; i < aListId.count(); i++)
  {
    int anID = aListId[ i ].toInt(&ok);
    if ( !ok )
      return false;

    if ( !aMesh->FindNode(anID) )
      return false;
    okIDs.insert( anID );
  }

  return (int) okIDs.size() == myTable->rowCount() + myNbMidFaceNodes + myNbCenterNodes;
}

//=================================================================================
// function : UpdateTable()
// purpose  :
//=================================================================================

void SMESHGUI_AddQuadraticElementDlg::UpdateTable( bool theConersValidity )
{
  QStringList aListCorners = myCornerNodes->text().split(" ", QString::SkipEmptyParts);

  if ( myGeomType == SMDSEntity_Quad_Polygon )        // POLYGON
  {
    if ( aListCorners.count() < 3 )
      theConersValidity = false;

    if ( aListCorners.count() != myTable->rowCount() && theConersValidity )
    {
      // adjust nb of rows for the polygon
      int oldNbRows = myTable->rowCount();
      myTable->setRowCount( aListCorners.count() );
      for ( int row = oldNbRows; row < myTable->rowCount(); row++ )
      {
        myTable->setItem( row, 0, new QTableWidgetItem( "" ) );
        myTable->item( row, 0 )->setFlags(0);

        IdEditItem* anEditItem = new IdEditItem( "" );
        anEditItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
        myTable->setItem(row, 1, anEditItem);

        myTable->setItem( row, 2, new QTableWidgetItem( "" ) );
        myTable->item( row, 2 )->setFlags(0);
      }
      myNbCorners = aListCorners.count();

      // fill FirstPolygonIds and LastPolygonIds
      FirstPolygonIds.resize( aListCorners.count() );
      LastPolygonIds .resize( aListCorners.count() );
      for ( int i = 0; i < aListCorners.count(); ++i )
      {
        FirstPolygonIds[i] = i;
        LastPolygonIds [i] = i+1;
      }
      LastPolygonIds.back() = 0;

      myNbCorners = aListCorners.count();
    }
  }

  if ( aListCorners.count() == myNbCorners && theConersValidity )
  {
    myTable->setEnabled( true );

    // clear the Middle column
    for ( int row = 0; row < myTable->rowCount(); row++ )
      myTable->item( row, 1 )->setText("");

    int* aFirstColIds = 0;
    int* aLastColIds  = 0;

    switch (myGeomType) {
    case SMDSEntity_Quad_Edge:
      aFirstColIds = FirstEdgeIds;
      aLastColIds  = LastEdgeIds;
      break;
    case SMDSEntity_Quad_Triangle:
    case SMDSEntity_BiQuad_Triangle:
      aFirstColIds = FirstTriangleIds;
      aLastColIds  = LastTriangleIds;
      break;
    case SMDSEntity_Quad_Quadrangle:
    case SMDSEntity_BiQuad_Quadrangle:
      aFirstColIds = FirstQuadrangleIds;
      aLastColIds  = LastQuadrangleIds;
      break;
    case SMDSEntity_Quad_Polygon:
      aFirstColIds = & FirstPolygonIds[0];
      aLastColIds  = & LastPolygonIds[0];
      break;
    case SMDSEntity_Quad_Tetra:
      aFirstColIds = FirstTetrahedronIds;
      aLastColIds  = LastTetrahedronIds;
      break;
    case SMDSEntity_Quad_Pyramid:
      aFirstColIds = FirstPyramidIds;
      aLastColIds  = LastPyramidIds;
      break;
    case SMDSEntity_Quad_Penta:
      aFirstColIds = FirstPentahedronIds;
      aLastColIds  = LastPentahedronIds;
      break;
    case SMDSEntity_Quad_Hexa:
    case SMDSEntity_TriQuad_Hexa:
      aFirstColIds = FirstHexahedronIds;
      aLastColIds  = LastHexahedronIds;
      break;
    default:;
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
  myCurrentLineEdit = 0;
  displaySimulation();
  updateButtons();
}

//=================================================================================
// function : onCellTextChange()
// purpose  :
//=================================================================================

void SMESHGUI_AddQuadraticElementDlg::onCellTextChange(int theRow, int theCol)
{
  myCurrentLineEdit = 0;
  displaySimulation();
  updateButtons();
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

//=======================================================================
//function : updateButtons
//purpose  : 
//=======================================================================

void SMESHGUI_AddQuadraticElementDlg::updateButtons()
{
  bool valid = IsValid();
  buttonOk->setEnabled( valid );
  buttonApply->setEnabled( valid );
}

//=================================================================================
// function : isValid
// purpose  :
//=================================================================================

bool SMESHGUI_AddQuadraticElementDlg::isValid()
{
  if( GroupGroups->isChecked() && ComboBox_GroupName->currentText().isEmpty() ) {
    SUIT_MessageBox::warning( this, tr( "SMESH_WRN_WARNING" ), tr( "GROUP_NAME_IS_EMPTY" ) );
    return false;
  }
  return true;
}
