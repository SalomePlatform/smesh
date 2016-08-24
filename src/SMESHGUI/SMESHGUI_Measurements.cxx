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
//  File   : SMESHGUI_Measurements.cxx
//  Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

#include "SMESHGUI_Measurements.h"

#include "SMESH_Actor.h"
#include "SMESHGUI.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include <SMESH_TypeFilter.hxx>
#include <SMESH_LogicalFilter.hxx>

#include <LightApp_SelectionMgr.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SVTK_ViewWindow.h>
#include <SALOME_ListIO.hxx>

#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QTabWidget>
#include <QVBoxLayout>

#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkDataSetMapper.h>
#include <VTKViewer_CellLocationsArray.h>
#include <vtkProperty.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)
#include CORBA_SERVER_HEADER(SMESH_Measurements)

const int SPACING = 6;            // layout spacing
const int MARGIN  = 9;            // layout margin
const int MAX_NB_FOR_EDITOR = 40; // max nb of items in the ID list editor field

// Uncomment as soon as elements are supported by Min Distance operation
//#define MINDIST_ENABLE_ELEMENT

// Uncomment as soon as objects are supported by Min Distance operation
//#define MINDIST_ENABLE_OBJECT

/*!
  \class SMESHGUI_MinDistance
  \brief Minimum distance measurement widget.
  
  Widget to calculate minimum distance between two objects.
*/

/*!
  \brief Constructor.
  \param parent parent widget
*/
SMESHGUI_MinDistance::SMESHGUI_MinDistance( QWidget* parent )
: QWidget( parent ), myCurrentTgt( FirstTgt ), myFirstActor( 0 ), mySecondActor( 0 ), myPreview( 0 )
{
  QGroupBox*    aFirstTgtGrp = new QGroupBox( tr( "FIRST_TARGET" ), this );
  QRadioButton* aFNode       = new QRadioButton( tr( "NODE" ),    aFirstTgtGrp );
  QRadioButton* aFElem       = new QRadioButton( tr( "ELEMENT" ), aFirstTgtGrp );
  QRadioButton* aFObject     = new QRadioButton( tr( "OBJECT" ),  aFirstTgtGrp );
  myFirstTgt                 = new QLineEdit( aFirstTgtGrp );

  QGridLayout* fl = new QGridLayout( aFirstTgtGrp );
  fl->setMargin( MARGIN );
  fl->setSpacing( SPACING );
  fl->addWidget( aFNode,     0, 0 );
  fl->addWidget( aFElem,     0, 1 );
  fl->addWidget( aFObject,   0, 2 );
  fl->addWidget( myFirstTgt, 1, 0, 1, 3 );

  myFirst = new QButtonGroup( this );
  myFirst->addButton( aFNode,   NodeTgt );
  myFirst->addButton( aFElem,   ElementTgt );
  myFirst->addButton( aFObject, ObjectTgt );

  QGroupBox*    aSecondTgtGrp = new QGroupBox( tr( "SECOND_TARGET" ), this );
  QRadioButton* aSOrigin      = new QRadioButton( tr( "ORIGIN" ),  aSecondTgtGrp );
  QRadioButton* aSNode        = new QRadioButton( tr( "NODE" ),    aSecondTgtGrp );
  QRadioButton* aSElem        = new QRadioButton( tr( "ELEMENT" ), aSecondTgtGrp );
  QRadioButton* aSObject      = new QRadioButton( tr( "OBJECT" ),  aSecondTgtGrp );
  mySecondTgt                 = new QLineEdit( aSecondTgtGrp );

  QGridLayout* sl = new QGridLayout( aSecondTgtGrp );
  sl->setMargin( MARGIN );
  sl->setSpacing( SPACING );
  sl->addWidget( aSOrigin,   0, 0 );
  sl->addWidget( aSNode,     0, 1 );
  sl->addWidget( aSElem,     0, 2 );
  sl->addWidget( aSObject,   0, 3 );
  sl->addWidget( mySecondTgt, 1, 0, 1, 4 );

  mySecond = new QButtonGroup( this );
  mySecond->addButton( aSOrigin, OriginTgt );
  mySecond->addButton( aSNode,   NodeTgt );
  mySecond->addButton( aSElem,   ElementTgt );
  mySecond->addButton( aSObject, ObjectTgt );

  QPushButton* aCompute = new QPushButton( tr( "COMPUTE" ), this );
  
  QGroupBox* aResults = new QGroupBox( tr( "RESULT" ), this );
  QLabel* aDxLab   = new QLabel( "dX", aResults );
  myDX             = new QLineEdit( aResults );
  QLabel* aDyLab   = new QLabel( "dY", aResults );
  myDY             = new QLineEdit( aResults );
  QLabel* aDzLab   = new QLabel( "dZ", aResults );
  myDZ             = new QLineEdit( aResults );
  QLabel* aDistLab = new QLabel( tr( "DISTANCE" ), aResults );
  myDistance       = new QLineEdit( aResults );

  QGridLayout* rl = new QGridLayout( aResults );
  rl->setMargin( MARGIN );
  rl->setSpacing( SPACING );
  rl->addWidget( aDxLab,     0, 0 );
  rl->addWidget( myDX,       0, 1 );
  rl->addWidget( aDyLab,     1, 0 );
  rl->addWidget( myDY,       1, 1 );
  rl->addWidget( aDzLab,     2, 0 );
  rl->addWidget( myDZ,       2, 1 );
  rl->addWidget( aDistLab,   0, 2 );
  rl->addWidget( myDistance, 0, 3 );

  QGridLayout* l = new QGridLayout( this );
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );

  l->addWidget( aFirstTgtGrp,  0, 0, 1, 2 );
  l->addWidget( aSecondTgtGrp, 1, 0, 1, 2 );
  l->addWidget( aCompute,      2, 0 );
  l->addWidget( aResults,      3, 0, 1, 2 );
  l->setColumnStretch( 1, 5 );
  l->setRowStretch( 4, 5 );

  aFNode->setChecked( true );
  aSOrigin->setChecked( true );
#ifndef MINDIST_ENABLE_ELEMENT
  aFElem->setEnabled( false );   // NOT AVAILABLE YET
  aSElem->setEnabled( false );   // NOT AVAILABLE YET
#endif
#ifndef MINDIST_ENABLE_OBJECT
  aFObject->setEnabled( false ); // NOT AVAILABLE YET
  aSObject->setEnabled( false ); // NOT AVAILABLE YET
#endif
  myDX->setReadOnly( true );
  myDY->setReadOnly( true );
  myDZ->setReadOnly( true );
  myDistance->setReadOnly( true );

  myValidator = new SMESHGUI_IdValidator( this, 1 );

  myFirstTgt->installEventFilter( this );
  mySecondTgt->installEventFilter( this );

  connect( myFirst,     SIGNAL( buttonClicked( int ) ),  this, SLOT( firstChanged() ) );
  connect( mySecond,    SIGNAL( buttonClicked( int ) ),  this, SLOT( secondChanged() ) );
  connect( aCompute,    SIGNAL( clicked() ),             this, SLOT( compute() ) );
  connect( myFirstTgt,  SIGNAL( textEdited( QString ) ), this, SLOT( firstEdited() ) );
  connect( mySecondTgt, SIGNAL( textEdited( QString ) ), this, SLOT( secondEdited() ) );

  QList<SUIT_SelectionFilter*> filters;
  filters.append( new SMESH_TypeFilter( SMESH::MESHorSUBMESH ) );
  filters.append( new SMESH_TypeFilter( SMESH::GROUP ) );
  myFilter = new SMESH_LogicalFilter( filters, SMESH_LogicalFilter::LO_OR );

  mySecondTgt->setEnabled( mySecond->checkedId() != OriginTgt );
  clear();

  //setTarget( FirstTgt );
  selectionChanged();
}

/*!
  \brief Destructor
*/
SMESHGUI_MinDistance::~SMESHGUI_MinDistance()
{
  erasePreview();
  if ( myPreview )
    myPreview->Delete();
}

/*!
  \brief Event filter
  \param o object
  \param o event
  \return \c true if event is filtered or \c false otherwise
*/
bool SMESHGUI_MinDistance::eventFilter( QObject* o, QEvent* e )
{
  if ( e->type() == QEvent::FocusIn ) {
    if ( o == myFirstTgt )
      setTarget( FirstTgt );
    else if ( o == mySecondTgt )
      setTarget( SecondTgt );
  }
  return QWidget::eventFilter( o, e );
}

/*!
  \brief Setup selection mode depending on the current widget state
*/
void SMESHGUI_MinDistance::updateSelection()
{
  LightApp_SelectionMgr* selMgr = SMESHGUI::selectionMgr();

  disconnect( selMgr, 0, this, 0 );
  selMgr->clearFilters();
  
  bool nodeMode = ( myCurrentTgt == FirstTgt  && myFirst->checkedId() == NodeTgt ) ||
                  ( myCurrentTgt == SecondTgt && mySecond->checkedId() == NodeTgt );
  bool elemMode = ( myCurrentTgt == FirstTgt  && myFirst->checkedId() == ElementTgt ) ||
                  ( myCurrentTgt == SecondTgt && mySecond->checkedId() == ElementTgt );
  bool objMode  = ( myCurrentTgt == FirstTgt  && myFirst->checkedId() == ObjectTgt ) ||
                  ( myCurrentTgt == SecondTgt && mySecond->checkedId() == ObjectTgt ) ||
                  ( myCurrentTgt == NoTgt );

  if ( nodeMode ) {
    SMESH::SetPointRepresentation( true );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
      aViewWindow->SetSelectionMode( NodeSelection );
  }
  else if ( elemMode ) {
    SMESH::SetPointRepresentation( false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
      aViewWindow->SetSelectionMode( CellSelection );
  }
  else if ( objMode ) {
    SMESH::SetPointRepresentation( false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
      aViewWindow->SetSelectionMode( ActorSelection );
    selMgr->installFilter( myFilter );
  }

  connect( selMgr, SIGNAL( currentSelectionChanged() ), this, SLOT( selectionChanged() ) );

  if ( myCurrentTgt == FirstTgt )
    firstEdited();
  else if ( myCurrentTgt == SecondTgt )
    secondEdited();

  //selectionChanged();
}

/*!
  \brief Deactivate widget
*/
void SMESHGUI_MinDistance::deactivate()
{
  disconnect( SMESHGUI::selectionMgr(), 0, this, 0 );
}

/*!
  \brief Set current target for selection
  \param target new target ID
*/
void SMESHGUI_MinDistance::setTarget( int target )
{
  if ( myCurrentTgt != target ) {
    myCurrentTgt = target;
    updateSelection();
  }
}

/*!
  \brief Erase preview actor
*/
void SMESHGUI_MinDistance::erasePreview()
{
  SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow();
  if ( aViewWindow && myPreview ) {
    aViewWindow->RemoveActor( myPreview );
    aViewWindow->Repaint();
  }
}

/*!
  \brief Display preview actor
*/
void SMESHGUI_MinDistance::displayPreview()
{
  SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow();
  if ( aViewWindow && myPreview ) {
    aViewWindow->AddActor( myPreview );
    aViewWindow->Repaint();
  }
}

/*!
  \brief Create preview actor
  \param x1 X coordinate of first point
  \param y1 X coordinate of first point
  \param z1 Y coordinate of first point
  \param x2 Y coordinate of second point
  \param y2 Z coordinate of second point
  \param z2 Z coordinate of second point
*/
void SMESHGUI_MinDistance::createPreview( double x1, double y1, double z1, double x2, double y2, double z2 )
{
  if ( myPreview )
    myPreview->Delete();

  vtkUnstructuredGrid* aGrid = vtkUnstructuredGrid::New();
  // create points
  vtkPoints* aPoints = vtkPoints::New();
  aPoints->SetNumberOfPoints( 2 );
  aPoints->SetPoint( 0, x1, y1, z1 );
  aPoints->SetPoint( 1, x2, y2, z2 );
  aGrid->SetPoints( aPoints );
  aPoints->Delete();
  // create cells
  vtkIdList* anIdList = vtkIdList::New();
  anIdList->SetNumberOfIds( 2 );
  vtkCellArray* aCells = vtkCellArray::New();
  aCells->Allocate( 2, 0);
  vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
  aCellTypesArray->SetNumberOfComponents( 1 );
  aCellTypesArray->Allocate( 1 );
  anIdList->SetId( 0, 0 ); anIdList->SetId( 1, 1 );
  aCells->InsertNextCell( anIdList );
  aCellTypesArray->InsertNextValue( VTK_LINE );
  anIdList->Delete();
  VTKViewer_CellLocationsArray* aCellLocationsArray = VTKViewer_CellLocationsArray::New();
  aCellLocationsArray->SetNumberOfComponents( 1 );
  aCellLocationsArray->SetNumberOfTuples( 1 );
  aCells->InitTraversal();
  for( vtkIdType idType = 0, *pts, npts; aCells->GetNextCell( npts, pts ); idType++ )
    aCellLocationsArray->SetValue( idType, aCells->GetTraversalLocation( npts ) );
  aGrid->SetCells( aCellTypesArray, aCellLocationsArray, aCells );
  aCellLocationsArray->Delete();
  aCellTypesArray->Delete();
  aCells->Delete();
  // create actor
  vtkDataSetMapper* aMapper = vtkDataSetMapper::New();
  aMapper->SetInputData( aGrid );
  aGrid->Delete();
  myPreview = SALOME_Actor::New();
  myPreview->PickableOff();
  myPreview->SetMapper( aMapper );
  myPreview->SetResolveCoincidentTopology(true);
  aMapper->Delete();
  vtkProperty* aProp = vtkProperty::New();
  aProp->SetRepresentationToWireframe();
  aProp->SetColor( 250, 0, 250 );
  aProp->SetPointSize( 5 );
  aProp->SetLineWidth( 3 );
  myPreview->SetProperty( aProp );
  aProp->Delete();
}

/*!
  \brief Called when selection is changed
*/
void SMESHGUI_MinDistance::selectionChanged()
{
  SUIT_OverrideCursor wc;

  SALOME_ListIO selected;
  SMESHGUI::selectionMgr()->selectedObjects( selected );

  if ( selected.Extent() == 1 ) {
    Handle(SALOME_InteractiveObject) IO = selected.First();
    SMESH::SMESH_IDSource_var obj = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( IO );
    if ( !CORBA::is_nil( obj ) ) {
      if ( myCurrentTgt == FirstTgt ) {
        myFirstSrc = obj;
        myFirstActor = SMESH::FindActorByEntry( IO->getEntry() );
        if ( myFirst->checkedId() == ObjectTgt ) {
          QString aName;
          SMESH::GetNameOfSelectedIObjects( SMESHGUI::selectionMgr(), aName );
          myFirstTgt->setText( aName );
        }
        else {
          SVTK_Selector* selector = SMESH::GetViewWindow()->GetSelector();
          QString ID;
          int nb = 0;
          if ( myFirstActor && selector ) {
            nb = myFirst->checkedId() == NodeTgt ? 
              SMESH::GetNameOfSelectedElements( selector, IO, ID ) :
              SMESH::GetNameOfSelectedNodes( selector, IO, ID );
          }
          if ( nb == 1 )
            myFirstTgt->setText( ID.trimmed() );
          else
            myFirstTgt->clear();
        }
      }
      else if ( myCurrentTgt == SecondTgt ) {
        mySecondSrc = obj;
        mySecondActor = SMESH::FindActorByEntry( IO->getEntry() );
        if ( mySecond->checkedId() == ObjectTgt ) {
          QString aName;
          SMESH::GetNameOfSelectedIObjects( SMESHGUI::selectionMgr(), aName );
          mySecondTgt->setText( aName );
        }
        else {
          SVTK_Selector* selector = SMESH::GetViewWindow()->GetSelector();
          QString ID;
          int nb = 0;
          if ( mySecondActor && selector ) {
            nb = mySecond->checkedId() == NodeTgt ? 
              SMESH::GetNameOfSelectedElements( selector, IO, ID ) :
              SMESH::GetNameOfSelectedNodes( selector, IO, ID );
          }
          if ( nb == 1 )
            mySecondTgt->setText( ID.trimmed() );
          else
            mySecondTgt->clear();
        }
      }
    }
  }
  clear();
}

/*!
  \brief Called when first target mode is changed by the user
*/
void SMESHGUI_MinDistance::firstChanged()
{
  myFirstSrc = SMESH::SMESH_IDSource::_nil();
  myFirstTgt->clear();
  myFirstTgt->setReadOnly( myFirst->checkedId() == ObjectTgt );
  myFirstTgt->setValidator( myFirst->checkedId() == ObjectTgt ? 0 : myValidator );
  setTarget( FirstTgt );
  updateSelection();
  clear();
}

/*!
  \brief Called when second target mode is changed by the user
*/
void SMESHGUI_MinDistance::secondChanged()
{
  mySecondSrc = SMESH::SMESH_IDSource::_nil();
  mySecondTgt->setEnabled( mySecond->checkedId() != OriginTgt );
  mySecondTgt->setReadOnly( mySecond->checkedId() == ObjectTgt );
  mySecondTgt->setValidator( mySecond->checkedId() == ObjectTgt ? 0 : myValidator );
  mySecondTgt->clear();
  setTarget( mySecond->checkedId() != OriginTgt ? SecondTgt : NoTgt );
  updateSelection();
  clear();
}

/*!
  \brief Called when first target is edited by the user
*/
void SMESHGUI_MinDistance::firstEdited()
{
  setTarget( FirstTgt );
  if ( sender() == myFirstTgt )
    clear();
  SVTK_Selector* selector = SMESH::GetViewWindow()->GetSelector();
  if ( myFirstActor && selector ) {
    Handle(SALOME_InteractiveObject) IO = myFirstActor->getIO();
    if ( myFirst->checkedId() == NodeTgt || myFirst->checkedId() == ElementTgt ) {
      TColStd_MapOfInteger ID;
      ID.Add( myFirstTgt->text().toLong() );
      selector->AddOrRemoveIndex( IO, ID, false );
    }
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
      aViewWindow->highlight( IO, true, true );
  }
}

/*!
  \brief Called when second target is edited by the user
*/
void SMESHGUI_MinDistance::secondEdited()
{
  setTarget( SecondTgt );
  if ( sender() == mySecondTgt )
    clear();
  QString text = mySecondTgt->text();
  if ( !mySecondActor )
  {
    selectionChanged();
    mySecondTgt->setText( text );
  }
  SVTK_Selector* selector = SMESH::GetViewWindow()->GetSelector();
  if ( mySecondActor && selector ) {
    Handle(SALOME_InteractiveObject) IO = mySecondActor->getIO();
    if ( mySecond->checkedId() == NodeTgt || mySecond->checkedId() == ElementTgt ) {
      if ( !text.isEmpty() ) {
        TColStd_MapOfInteger ID;
        ID.Add( text.toLong() );
        selector->AddOrRemoveIndex( IO, ID, false );
      }
    }
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
      aViewWindow->highlight( IO, true, true );
  }
}

/*!
  \brief Compute the minimum distance between targets
*/
void SMESHGUI_MinDistance::compute()
{
  SUIT_OverrideCursor wc;
  SMESH::IDSource_wrap s1;
  SMESH::IDSource_wrap s2;
  bool isOrigin = mySecond->checkedId() == OriginTgt;

  // process first target
  if ( !CORBA::is_nil( myFirstSrc ) ) {
    if ( myFirst->checkedId() == NodeTgt || myFirst->checkedId() == ElementTgt ) {
      SMESH::SMESH_Mesh_var m = myFirstSrc->GetMesh();
      long id = myFirstTgt->text().toLong();
      if ( !CORBA::is_nil( m ) && id ) {
        SMESH::long_array_var ids = new SMESH::long_array();
        ids->length( 1 );
        ids[0] = id;
        SMESH::SMESH_MeshEditor_var me = m->GetMeshEditor();
        s1 = me->MakeIDSource( ids.in(), myFirst->checkedId() == NodeTgt ? SMESH::NODE : SMESH::FACE );
      }
    }
    else {
      s1 = myFirstSrc;
      s1->Register();
    }
  }

  // process second target
  if ( !CORBA::is_nil( mySecondSrc ) ) {
    if ( mySecond->checkedId() == NodeTgt || mySecond->checkedId() == ElementTgt ) {
      SMESH::SMESH_Mesh_var m = mySecondSrc->GetMesh();
      long id = mySecondTgt->text().toLong();
      if ( !CORBA::is_nil( m ) && id ) {
        SMESH::long_array_var ids = new SMESH::long_array();
        ids->length( 1 );
        ids[0] = id;
        SMESH::SMESH_MeshEditor_var me = m->GetMeshEditor();
        s2 = me->MakeIDSource( ids.in(), mySecond->checkedId() == NodeTgt ? SMESH::NODE : SMESH::FACE );
      }
    }
    else {
      s2 = mySecondSrc;
      s2->Register();
    }
  }

  if ( !CORBA::is_nil( s1 ) && ( !CORBA::is_nil( s2 ) || isOrigin ) ) {
    // compute min distance
    int precision = SMESHGUI::resourceMgr()->integerValue( "SMESH", "length_precision", 6 );
    SMESH::Measurements_var measure = SMESHGUI::GetSMESHGen()->CreateMeasurements();
    SMESH::Measure result = measure->MinDistance( s1.in(), s2.in() );
    measure->UnRegister();
    myDX->setText( QString::number( result.minX, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
    myDY->setText( QString::number( result.minY, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
    myDZ->setText( QString::number( result.minZ, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
    myDistance->setText( QString::number( result.value, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
    // update preview actor
    erasePreview();
    double x1, y1, z1, x2, y2, z2;
    SMESH::double_array_var coord = s1->GetMesh()->GetNodeXYZ( result.node1 );
    x1 = coord[0]; y1 = coord[1]; z1 = coord[2];
    if ( isOrigin ) {
      x2 = y2 = z2 = 0.;
    }
    else {
      coord = s2->GetMesh()->GetNodeXYZ( result.node2 );
      x2 = coord[0]; y2 = coord[1]; z2 = coord[2];
    }
    createPreview( x1, y1, z1, x2, y2, z2 );
    displayPreview();
  }
  else {
    clear();
  }
}

/*!
  \brief Reset the widget to the initial state (nullify result fields)
*/
void SMESHGUI_MinDistance::clear()
{
  myDX->clear();
  myDY->clear();
  myDZ->clear();
  myDistance->clear();
  erasePreview();
}

/*!
  \class SMESHGUI_BoundingBox
  \brief Bounding box measurement widget.
  
  Widget to calculate bounding box of the selected object(s).
*/

/*!
  \brief Constructor.
  \param parent parent widget
*/
SMESHGUI_BoundingBox::SMESHGUI_BoundingBox( QWidget* parent )
: QWidget( parent ), myActor( 0 ), myPreview( 0 )
{
  QGroupBox* aSourceGrp = new QGroupBox( tr( "SOURCE" ), this );
  QRadioButton* aObjects  = new QRadioButton( tr( "OBJECTS" ),  aSourceGrp );
  QRadioButton* aNodes    = new QRadioButton( tr( "NODES" ),    aSourceGrp );
  QRadioButton* aElements = new QRadioButton( tr( "ELEMENTS" ), aSourceGrp );
  mySource = new QLineEdit( aSourceGrp );
  
  QGridLayout* fl = new QGridLayout( aSourceGrp );
  fl->setMargin( MARGIN );
  fl->setSpacing( SPACING );
  fl->addWidget( aObjects,   0, 0 );
  fl->addWidget( aNodes,     0, 1 );
  fl->addWidget( aElements,  0, 2 );
  fl->addWidget( mySource,   1, 0, 1, 3 );
  
  mySourceMode = new QButtonGroup( this );
  mySourceMode->addButton( aObjects,  ObjectsSrc );
  mySourceMode->addButton( aNodes,    NodesSrc );
  mySourceMode->addButton( aElements, ElementsSrc );

  QPushButton* aCompute = new QPushButton( tr( "COMPUTE" ), this );

  QGroupBox* aResults = new QGroupBox( tr( "RESULT" ), this );
  QLabel* aXminLab = new QLabel( "Xmin", aResults );
  myXmin           = new QLineEdit( aResults );
  QLabel* aXmaxLab = new QLabel( "Xmax", aResults );
  myXmax           = new QLineEdit( aResults );
  QLabel* aDxLab   = new QLabel( "dX", aResults );
  myDX             = new QLineEdit( aResults );
  QLabel* aYminLab = new QLabel( "Ymin", aResults );
  myYmin           = new QLineEdit( aResults );
  QLabel* aYmaxLab = new QLabel( "Ymax", aResults );
  myYmax           = new QLineEdit( aResults );
  QLabel* aDyLab   = new QLabel( "dY", aResults );
  myDY             = new QLineEdit( aResults );
  QLabel* aZminLab = new QLabel( "Zmin", aResults );
  myZmin           = new QLineEdit( aResults );
  QLabel* aZmaxLab = new QLabel( "Zmax", aResults );
  myZmax           = new QLineEdit( aResults );
  QLabel* aDzLab   = new QLabel( "dZ", aResults );
  myDZ             = new QLineEdit( aResults );

  QGridLayout* rl  = new QGridLayout( aResults );
  rl->setMargin( MARGIN );
  rl->setSpacing( SPACING );
  rl->addWidget( aXminLab,   0, 0 );
  rl->addWidget( myXmin,     0, 1 );
  rl->addWidget( aXmaxLab,   0, 2 );
  rl->addWidget( myXmax,     0, 3 );
  rl->addWidget( aDxLab,     0, 4 );
  rl->addWidget( myDX,       0, 5 );
  rl->addWidget( aYminLab,   1, 0 );
  rl->addWidget( myYmin,     1, 1 );
  rl->addWidget( aYmaxLab,   1, 2 );
  rl->addWidget( myYmax,     1, 3 );
  rl->addWidget( aDyLab,     1, 4 );
  rl->addWidget( myDY,       1, 5 );
  rl->addWidget( aZminLab,   2, 0 );
  rl->addWidget( myZmin,     2, 1 );
  rl->addWidget( aZmaxLab,   2, 2 );
  rl->addWidget( myZmax,     2, 3 );
  rl->addWidget( aDzLab,     2, 4 );
  rl->addWidget( myDZ,       2, 5 );

  QGridLayout* l = new QGridLayout( this );
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );

  l->addWidget( aSourceGrp, 0, 0, 1, 2 );
  l->addWidget( aCompute,   1, 0 );
  l->addWidget( aResults,   2, 0, 1, 2 );
  l->setColumnStretch( 1, 5 );
  l->setRowStretch( 3, 5 );

  aObjects->setChecked( true );
  myXmin->setReadOnly( true );
  myXmax->setReadOnly( true );
  myDX->setReadOnly( true );
  myYmin->setReadOnly( true );
  myYmax->setReadOnly( true );
  myDY->setReadOnly( true );
  myZmin->setReadOnly( true );
  myZmax->setReadOnly( true );
  myDZ->setReadOnly( true );

  myValidator = new SMESHGUI_IdValidator( this );

  connect( mySourceMode, SIGNAL( buttonClicked( int ) ),  this, SLOT( sourceChanged() ) );
  connect( aCompute,     SIGNAL( clicked() ),             this, SLOT( compute() ) );
  connect( mySource,     SIGNAL( textEdited( QString ) ), this, SLOT( sourceEdited() ) );

  QList<SUIT_SelectionFilter*> filters;
  filters.append( new SMESH_TypeFilter( SMESH::MESHorSUBMESH ) );
  filters.append( new SMESH_TypeFilter( SMESH::GROUP ) );
  myFilter = new SMESH_LogicalFilter( filters, SMESH_LogicalFilter::LO_OR );

  clear();
}

/*!
  \brief Destructor
*/
SMESHGUI_BoundingBox::~SMESHGUI_BoundingBox()
{
  erasePreview();
  if ( myPreview )
    myPreview->Delete();
}

/*!
  \brief Setup selection mode depending on the current widget state
*/
void SMESHGUI_BoundingBox::updateSelection()
{
  LightApp_SelectionMgr* selMgr = SMESHGUI::selectionMgr();

  disconnect( selMgr, 0, this, 0 );
  selMgr->clearFilters();
  
  bool nodeMode = mySourceMode->checkedId() == NodesSrc;
  bool elemMode = mySourceMode->checkedId() == ElementsSrc;
  bool objMode  = mySourceMode->checkedId() == ObjectsSrc;

  if ( nodeMode ) {
    SMESH::SetPointRepresentation( true );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
      aViewWindow->SetSelectionMode( NodeSelection );
  }
  else if ( elemMode ) {
    SMESH::SetPointRepresentation( false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
      aViewWindow->SetSelectionMode( CellSelection );
  }
  else if ( objMode ) {
    SMESH::SetPointRepresentation( false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
      aViewWindow->SetSelectionMode( ActorSelection );
    selMgr->installFilter( myFilter );
  }

  connect( selMgr, SIGNAL( currentSelectionChanged() ), this, SLOT( selectionChanged() ) );

  sourceEdited();

  if ( mySource->text().isEmpty() )
    selectionChanged();
}

/*!
  \brief Deactivate widget
*/
void SMESHGUI_BoundingBox::deactivate()
{
  disconnect( SMESHGUI::selectionMgr(), 0, this, 0 );
}

/*!
  \brief Erase preview actor
*/
void SMESHGUI_BoundingBox::erasePreview()
{
  SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow();
  if ( aViewWindow && myPreview ) {
    aViewWindow->RemoveActor( myPreview );
    aViewWindow->Repaint();
  }
}

/*!
  \brief Display preview actor
*/
void SMESHGUI_BoundingBox::displayPreview()
{
  SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow();
  if ( aViewWindow && myPreview ) {
    aViewWindow->AddActor( myPreview );
    aViewWindow->Repaint();
  }
}

/*!
  \brief Create preview actor
  \param minX min X coordinate of bounding box
  \param maxX max X coordinate of bounding box
  \param minY min Y coordinate of bounding box
  \param maxY max Y coordinate of bounding box
  \param minZ min Z coordinate of bounding box
  \param maxZ max Z coordinate of bounding box
*/
void SMESHGUI_BoundingBox::createPreview( double minX, double maxX, double minY, double maxY, double minZ, double maxZ )
{
  if ( myPreview )
    myPreview->Delete();

  vtkUnstructuredGrid* aGrid = vtkUnstructuredGrid::New();
  // create points
  vtkPoints* aPoints = vtkPoints::New();
  aPoints->SetNumberOfPoints( 8 );
  aPoints->SetPoint( 0, minX, minY, minZ );
  aPoints->SetPoint( 1, maxX, minY, minZ );
  aPoints->SetPoint( 2, minX, maxY, minZ );
  aPoints->SetPoint( 3, maxX, maxY, minZ );
  aPoints->SetPoint( 4, minX, minY, maxZ );
  aPoints->SetPoint( 5, maxX, minY, maxZ );
  aPoints->SetPoint( 6, minX, maxY, maxZ );
  aPoints->SetPoint( 7, maxX, maxY, maxZ );
  aGrid->SetPoints( aPoints );
  aPoints->Delete();
  // create cells
  // connectivity: 0-1 0-4 0-2 1-5 1-3 2-6 2-3 3-7 4-6 4-5 5-7 6-7
  vtkIdList* anIdList = vtkIdList::New();
  anIdList->SetNumberOfIds( 2 );
  vtkCellArray* aCells = vtkCellArray::New();
  aCells->Allocate( 2*12, 0);
  vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
  aCellTypesArray->SetNumberOfComponents( 1 );
  aCellTypesArray->Allocate( 12 );
  anIdList->SetId( 0, 0 ); anIdList->SetId( 1, 1 );
  aCells->InsertNextCell( anIdList );
  aCellTypesArray->InsertNextValue( VTK_LINE );
  anIdList->SetId( 0, 0 ); anIdList->SetId( 1, 4 );
  aCells->InsertNextCell( anIdList );
  aCellTypesArray->InsertNextValue( VTK_LINE );
  anIdList->SetId( 0, 0 ); anIdList->SetId( 1, 2 );
  aCells->InsertNextCell( anIdList );
  aCellTypesArray->InsertNextValue( VTK_LINE );
  anIdList->SetId( 0, 1 ); anIdList->SetId( 1, 5 );
  aCells->InsertNextCell( anIdList );
  aCellTypesArray->InsertNextValue( VTK_LINE );
  anIdList->SetId( 0, 1 ); anIdList->SetId( 1, 3 );
  aCells->InsertNextCell( anIdList );
  aCellTypesArray->InsertNextValue( VTK_LINE );
  anIdList->SetId( 0, 2 ); anIdList->SetId( 1, 6 );
  aCells->InsertNextCell( anIdList );
  aCellTypesArray->InsertNextValue( VTK_LINE );
  anIdList->SetId( 0, 2 ); anIdList->SetId( 1, 3 );
  aCells->InsertNextCell( anIdList );
  aCellTypesArray->InsertNextValue( VTK_LINE );
  anIdList->SetId( 0, 3 ); anIdList->SetId( 1, 7 );
  aCells->InsertNextCell( anIdList );
  aCellTypesArray->InsertNextValue( VTK_LINE );
  anIdList->SetId( 0, 4 ); anIdList->SetId( 1, 6 );
  aCells->InsertNextCell( anIdList );
  aCellTypesArray->InsertNextValue( VTK_LINE );
  anIdList->SetId( 0, 4 ); anIdList->SetId( 1, 5 );
  aCells->InsertNextCell( anIdList );
  aCellTypesArray->InsertNextValue( VTK_LINE );
  anIdList->SetId( 0, 5 ); anIdList->SetId( 1, 7 );
  aCells->InsertNextCell( anIdList );
  aCellTypesArray->InsertNextValue( VTK_LINE );
  anIdList->SetId( 0, 6 ); anIdList->SetId( 1, 7 );
  aCells->InsertNextCell( anIdList );
  aCellTypesArray->InsertNextValue( VTK_LINE );
  anIdList->Delete();
  VTKViewer_CellLocationsArray* aCellLocationsArray = VTKViewer_CellLocationsArray::New();
  aCellLocationsArray->SetNumberOfComponents( 1 );
  aCellLocationsArray->SetNumberOfTuples( 12 );
  aCells->InitTraversal();
  for( vtkIdType idType = 0, *pts, npts; aCells->GetNextCell( npts, pts ); idType++ )
    aCellLocationsArray->SetValue( idType, aCells->GetTraversalLocation( npts ) );
  aGrid->SetCells( aCellTypesArray, aCellLocationsArray, aCells );
  aCellLocationsArray->Delete();
  aCellTypesArray->Delete();
  aCells->Delete();
  // create actor
  vtkDataSetMapper* aMapper = vtkDataSetMapper::New();
  aMapper->SetInputData( aGrid );
  aGrid->Delete();
  myPreview = SALOME_Actor::New();
  myPreview->PickableOff();
  myPreview->SetMapper( aMapper );
  aMapper->Delete();
  vtkProperty* aProp = vtkProperty::New();
  aProp->SetRepresentationToWireframe();
  aProp->SetColor( 250, 0, 250 );
  aProp->SetPointSize( 5 );
  aProp->SetLineWidth( 3 );
  myPreview->SetProperty( aProp );
  aProp->Delete();
}

/*!
  \brief Called when selection is changed
*/
void SMESHGUI_BoundingBox::selectionChanged()
{
  SUIT_OverrideCursor wc;

  SALOME_ListIO selected;
  SMESHGUI::selectionMgr()->selectedObjects( selected );

  if ( selected.Extent() == 1 ) {
    Handle(SALOME_InteractiveObject) IO = selected.First();
    SMESH::SMESH_IDSource_var obj = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( IO );
    if ( !CORBA::is_nil( obj ) ) {
      mySrc.clear();
      mySrc.append( obj );
      myActor = SMESH::FindActorByEntry( IO->getEntry() );
      if ( mySourceMode->checkedId() == ObjectsSrc ) {
        QString aName;
        SMESH::GetNameOfSelectedIObjects( SMESHGUI::selectionMgr(), aName );
        mySource->setText( aName );
      }
      else {
        SVTK_Selector* selector = SMESH::GetViewWindow()->GetSelector();
        QString ID;
        int nb = 0;
        if ( myActor && selector ) {
          nb = mySourceMode->checkedId() == NodesSrc ? 
            SMESH::GetNameOfSelectedElements( selector, IO, ID ) :
            SMESH::GetNameOfSelectedNodes( selector, IO, ID );
        }
        if ( nb > 0 ) {
          myIDs = ID.trimmed();
          if ( nb < MAX_NB_FOR_EDITOR ) {
            mySource->setReadOnly( false );
            if ( mySource->validator() != myValidator )
              mySource->setValidator( myValidator );
            mySource->setText( ID.trimmed() );
          }
          else {
            mySource->setReadOnly( true );
            mySource->setValidator( 0 );
            mySource->setText( tr( "SELECTED_NB_OBJ" ).arg( nb )
                               .arg( mySourceMode->checkedId() == NodesSrc ? tr( "NB_NODES" ) : tr( "NB_ELEMENTS") ) );
          }
        }
        else {
          myIDs = "";
          mySource->clear();
          mySource->setReadOnly( false );
          mySource->setValidator( myValidator );
        }
      }
    }
  }
  else if ( selected.Extent() > 1 ) {
    myIDs = "";
    SALOME_ListIteratorOfListIO It( selected );
    mySrc.clear();
    myActor = 0;
    if ( mySourceMode->checkedId() == ObjectsSrc ) {
      for( ; It.More(); It.Next()){
        Handle(SALOME_InteractiveObject) IO = It.Value();
        SMESH::SMESH_IDSource_var obj = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( IO );
        if ( !CORBA::is_nil( obj ) ) {
          mySrc.append( obj );
        }
      }
      QString aName;
      SMESH::GetNameOfSelectedIObjects( SMESHGUI::selectionMgr(), aName );
      mySource->setText( aName );
    }
    else {
      mySource->clear();
    }
  }
  clear();
}

/*!
  \brief Called when source mode is changed by the user
*/
void SMESHGUI_BoundingBox::sourceChanged()
{
  myIDs = "";
  mySource->clear();
  mySource->setReadOnly( mySourceMode->checkedId() == ObjectsSrc );
  mySource->setValidator( mySourceMode->checkedId() == ObjectsSrc ? 0 : myValidator );
  updateSelection();
  clear();
}

/*!
  \brief Called when source mode is edited by the user
*/
void SMESHGUI_BoundingBox::sourceEdited()
{
  if ( sender() == mySource )
    clear();
  SVTK_Selector* selector = SMESH::GetViewWindow()->GetSelector();
  if ( myActor && selector ) {
    Handle(SALOME_InteractiveObject) IO = myActor->getIO();
    if ( mySourceMode->checkedId() == NodesSrc || mySourceMode->checkedId() == ElementsSrc ) {
      TColStd_MapOfInteger ID;
      if ( !mySource->isReadOnly() )
        myIDs = mySource->text();
      QStringList ids = myIDs.split( " ", QString::SkipEmptyParts );
      foreach ( QString id, ids )
        ID.Add( id.trimmed().toLong() );
      selector->AddOrRemoveIndex( IO, ID, false );
    }
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
      aViewWindow->highlight( IO, true, true );
  }
}

/*!
  \brief Calculate bounding box of the selected object(s)
*/
void SMESHGUI_BoundingBox::compute()
{
  SUIT_OverrideCursor wc;
  SMESH::ListOfIDSources_var srcList = new SMESH::ListOfIDSources();
  if ( mySourceMode->checkedId() == NodesSrc || mySourceMode->checkedId() == ElementsSrc ) {
    if ( mySrc.count() > 0 && !CORBA::is_nil( mySrc[0] ) ) {
      SMESH::SMESH_Mesh_var m = mySrc[0]->GetMesh();
      QStringList ids = myIDs.split( " ", QString::SkipEmptyParts );
      if ( !CORBA::is_nil( m ) && ids.count() > 0 ) {
        SMESH::long_array_var ids_in = new SMESH::long_array();
        ids_in->length( ids.count() );
        for( int i = 0; i < ids.count(); i++ )
          ids_in[i] = ids[i].trimmed().toLong();
        SMESH::SMESH_MeshEditor_var me = m->GetMeshEditor();
        SMESH::SMESH_IDSource_var s = me->MakeIDSource( ids_in.in(), mySourceMode->checkedId() == NodesSrc ? SMESH::NODE : SMESH::FACE ); 
        srcList->length( 1 );
        srcList[0] = s;
      }
    }
  }
  else {
    srcList->length( mySrc.count() );
    for( int i = 0; i < mySrc.count(); i++ ) {
      srcList[i] = mySrc[i];
      mySrc[i]->Register();
    }
  }
  if ( srcList->length() > 0 ) {
    // compute bounding box
    int precision = SMESHGUI::resourceMgr()->integerValue( "SMESH", "length_precision", 6 );
    SMESH::Measurements_var measure = SMESHGUI::GetSMESHGen()->CreateMeasurements();
    SMESH::Measure result = measure->BoundingBox( srcList.in() );
    SALOME::UnRegister( srcList );
    measure->UnRegister();
    myXmin->setText( QString::number( result.minX, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
    myXmax->setText( QString::number( result.maxX, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
    myDX->setText( QString::number( result.maxX-result.minX, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
    myYmin->setText( QString::number( result.minY, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
    myYmax->setText( QString::number( result.maxY, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
    myDY->setText( QString::number( result.maxY-result.minY, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
    myZmin->setText( QString::number( result.minZ, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
    myZmax->setText( QString::number( result.maxZ, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
    myDZ->setText( QString::number( result.maxZ-result.minZ, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
    // update preview actor
    erasePreview();
    createPreview( result.minX, result.maxX, result.minY, result.maxY, result.minZ, result.maxZ );
    displayPreview();
  }
  else {
    clear();
  }
}

/*!
  \brief Reset the widget to the initial state (nullify result fields)
*/
void SMESHGUI_BoundingBox::clear()
{
  myXmin->clear();
  myXmax->clear();
  myDX->clear();
  myYmin->clear();
  myYmax->clear();
  myDY->clear();
  myZmin->clear();
  myZmax->clear();
  myDZ->clear();
  erasePreview();
}

/*!
  \class SMESHGUI_BasicProperties
  \brief basic properties measurement widget.
  
  Widget to calculate length, area or volume for the selected object(s).
*/

/*!
  \brief Constructor.
  \param parent parent widget
*/
SMESHGUI_BasicProperties::SMESHGUI_BasicProperties( QWidget* parent )
: QWidget( parent )
{
  // Property (length, area or volume)
  QGroupBox* aPropertyGrp = new QGroupBox( tr( "PROPERTY" ), this );

  QRadioButton* aLength = new QRadioButton( tr( "LENGTH" ), aPropertyGrp );
  QRadioButton* anArea = new QRadioButton( tr( "AREA" ), aPropertyGrp );
  QRadioButton* aVolume = new QRadioButton( tr( "VOLUME" ), aPropertyGrp );

  myMode = new QButtonGroup( this );
  myMode->addButton( aLength, Length );
  myMode->addButton( anArea, Area );
  myMode->addButton( aVolume, Volume );

  QHBoxLayout* aPropertyLayout = new QHBoxLayout;
  aPropertyLayout->addWidget( aLength );
  aPropertyLayout->addWidget( anArea );
  aPropertyLayout->addWidget( aVolume );

  aPropertyGrp->setLayout( aPropertyLayout );

  // Source object
  QGroupBox* aSourceGrp = new QGroupBox( tr( "SOURCE_MESH_SUBMESH_GROUP" ), this );

  mySource = new QLineEdit( aSourceGrp );
  mySource->setReadOnly( true );
    
  QHBoxLayout* aSourceLayout = new QHBoxLayout;
  aSourceLayout->addWidget( mySource );
  
  aSourceGrp->setLayout( aSourceLayout );

  // Compute button
  QPushButton* aCompute = new QPushButton( tr( "COMPUTE" ), this );

  // Result of computation (length, area or volume)
  myResultGrp = new QGroupBox( this );

  myResult = new QLineEdit;
  myResult->setReadOnly( true );

  QHBoxLayout* aResultLayout = new QHBoxLayout;
  aResultLayout->addWidget( myResult );
  
  myResultGrp->setLayout( aResultLayout );

  // Layout
  QGridLayout* aMainLayout = new QGridLayout( this );
  aMainLayout->setMargin( MARGIN );
  aMainLayout->setSpacing( SPACING );

  aMainLayout->addWidget( aPropertyGrp, 0, 0, 1, 2 );
  aMainLayout->addWidget( aSourceGrp, 1, 0, 1, 2 );
  aMainLayout->addWidget( aCompute,   2, 0 );
  aMainLayout->addWidget( myResultGrp, 3, 0, 1, 2 );
  aMainLayout->setColumnStretch( 1, 5 );
  aMainLayout->setRowStretch( 4, 5 );

  // Initial state
  setMode( Length );
  
  // Connections
  connect( myMode, SIGNAL( buttonClicked( int ) ),  this, SLOT( modeChanged( int ) ) );
  connect( aCompute, SIGNAL( clicked() ), this, SLOT( compute() ) );
  
  // Selection filter
  QList<SUIT_SelectionFilter*> filters;
  filters.append( new SMESH_TypeFilter( SMESH::MESHorSUBMESH ) );
  filters.append( new SMESH_TypeFilter( SMESH::GROUP ) );
  myFilter = new SMESH_LogicalFilter( filters, SMESH_LogicalFilter::LO_OR );
}

/*!
  \brief Destructor
*/
SMESHGUI_BasicProperties::~SMESHGUI_BasicProperties()
{
}

/*!
  \brief Sets the measurement mode.
  \param theMode the mode to set (length, area or volume meausurement)
*/
void SMESHGUI_BasicProperties::setMode( const Mode theMode )
{
  QRadioButton* aButton = qobject_cast<QRadioButton*>( myMode->button( theMode ) );
  if ( aButton ) {
    aButton->setChecked( true );
    modeChanged( theMode );
  }
}

/*!
  \brief Setup the selection mode.
*/
void SMESHGUI_BasicProperties::updateSelection()
{
  LightApp_SelectionMgr* selMgr = SMESHGUI::selectionMgr();

  disconnect( selMgr, 0, this, 0 );
  selMgr->clearFilters();
  
  SMESH::SetPointRepresentation( false );
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() ) {
    aViewWindow->SetSelectionMode( ActorSelection );
  }
  selMgr->installFilter( myFilter );
  
  connect( selMgr, SIGNAL( currentSelectionChanged() ), this, SLOT( selectionChanged() ) );

  if ( mySource->text().isEmpty() )
    selectionChanged();
}

/*!
  \brief Deactivate widget
*/
void SMESHGUI_BasicProperties::deactivate()
{
  disconnect( SMESHGUI::selectionMgr(), 0, this, 0 );
}

/*!
  \brief Called when selection is changed
*/
void SMESHGUI_BasicProperties::selectionChanged()
{
  SUIT_OverrideCursor wc;

  SALOME_ListIO selected;
  SMESHGUI::selectionMgr()->selectedObjects( selected );

  if ( selected.Extent() == 1 ) {
    Handle(SALOME_InteractiveObject) IO = selected.First();
    SMESH::SMESH_IDSource_var obj = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( IO );
    if ( !CORBA::is_nil( obj ) ) {
      mySrc = obj;

      QString aName;
      SMESH::GetNameOfSelectedIObjects( SMESHGUI::selectionMgr(), aName );
      mySource->setText( aName );
    }
  }

  clear();
}

/*!
  \brief Called when the measurement mode selection is changed.
  \param theMode the selected mode
*/
void SMESHGUI_BasicProperties::modeChanged( int theMode )
{
  clear();

  if ( theMode == Length ) {
    myResultGrp->setTitle( tr("LENGTH") );
  } else if ( theMode == Area ) {
    myResultGrp->setTitle( tr("AREA") );
  } else if ( theMode == Volume ) {
    myResultGrp->setTitle( tr("VOLUME") );
  }
}

/*!
  \brief Calculate length, area or volume for the selected object(s)
*/
void SMESHGUI_BasicProperties::compute()
{
  SUIT_OverrideCursor wc;

  SMESH::SMESH_IDSource_var source;

  if ( !CORBA::is_nil( mySrc ) ) {
    // compute
    int precision = SMESHGUI::resourceMgr()->integerValue( "SMESH", "length_precision", 6 );
    SMESH::Measurements_var measure = SMESHGUI::GetSMESHGen()->CreateMeasurements();

    double result = 0;

    if ( myMode->checkedId() == Length ) {
      result = measure->Length( mySrc.in() );
    } else if ( myMode->checkedId() == Area ) {
      result = measure->Area( mySrc.in() );
    } else if ( myMode->checkedId() == Volume ) {
      result = measure->Volume( mySrc.in() );
    }
    
    measure->UnRegister();

    myResult->setText( QString::number( result, precision > 0 ? 'f' : 'g', qAbs( precision ) ) );
  } else {
    clear();
  }
}

/*!
  \brief Reset the widget to the initial state (nullify the result field)
*/
void SMESHGUI_BasicProperties::clear()
{
  myResult->clear();
}

/*!
  \class SMESHGUI_MeshInfoDlg
  \brief Centralized dialog box for the measurements
*/

/*!
  \brief Constructor
  \param parent parent widget
  \param page specifies the dialog page to be shown at the start-up
*/
SMESHGUI_MeasureDlg::SMESHGUI_MeasureDlg( QWidget* parent, int page )
: QDialog( parent )
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle( tr( "MEASUREMENTS" ) );
  setSizeGripEnabled( true );

  SUIT_ResourceMgr* resMgr = SMESHGUI::resourceMgr();

  myTabWidget = new QTabWidget( this );

  // min distance

  myMinDist = new SMESHGUI_MinDistance( myTabWidget );
  int aMinDistInd = myTabWidget->addTab( myMinDist, resMgr->loadPixmap( "SMESH", tr( "ICON_MEASURE_MIN_DIST" ) ), tr( "MIN_DIST" ) );

  // bounding box
  
  myBndBox = new SMESHGUI_BoundingBox( myTabWidget );
  int aBndBoxInd = myTabWidget->addTab( myBndBox, resMgr->loadPixmap( "SMESH", tr( "ICON_MEASURE_BND_BOX" ) ), tr( "BND_BOX" ) );

  // basic properties
  
  myBasicProps = new SMESHGUI_BasicProperties( myTabWidget );
  int aBasicPropInd = myTabWidget->addTab( myBasicProps, resMgr->loadPixmap( "SMESH", tr( "ICON_MEASURE_BASIC_PROPS" ) ), tr( "BASIC_PROPERTIES" ) );

  // buttons
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

  int anInd = -1;
  if ( page == MinDistance ) {
    anInd = aMinDistInd;
  } else if ( page == BoundingBox ) {
    anInd = aBndBoxInd;
  } else if ( page == Length || page == Area || page == Volume ) {
    myBasicProps->setMode( (SMESHGUI_BasicProperties::Mode)(page - Length) );
    anInd = aBasicPropInd;
  }

  if ( anInd >= 0 ) {
    myTabWidget->setCurrentIndex( anInd );
  }

  connect( okBtn,       SIGNAL( clicked() ),              this, SLOT( reject() ) );
  connect( helpBtn,     SIGNAL( clicked() ),              this, SLOT( help() ) );
  connect( myTabWidget, SIGNAL( currentChanged( int  ) ), this, SLOT( updateSelection() ) );
  connect( SMESHGUI::GetSMESHGUI(),  SIGNAL( SignalDeactivateActiveDialog() ), this, SLOT( deactivate() ) );
  connect( SMESHGUI::GetSMESHGUI(),  SIGNAL( SignalCloseAllDialogs() ),        this, SLOT( reject() ) );

  updateSelection();
}

/*!
  \brief Destructor
*/
SMESHGUI_MeasureDlg::~SMESHGUI_MeasureDlg()
{
}

/*!
  \brief Perform clean-up actions on the dialog box closing.
*/
void SMESHGUI_MeasureDlg::reject()
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
void SMESHGUI_MeasureDlg::keyPressEvent( QKeyEvent* e )
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
void SMESHGUI_MeasureDlg::enterEvent( QEvent* )
{
  activate();
}

/*!
  \brief Setup selection mode depending on the current dialog box state.
*/
void SMESHGUI_MeasureDlg::updateSelection()
{
  if ( myTabWidget->currentIndex() == MinDistance )
    myMinDist->updateSelection();
  else if ( myTabWidget->currentIndex() == BoundingBox )
    myBndBox->updateSelection();
  else {
    myBndBox->erasePreview();
    myBasicProps->updateSelection();
  }
}

/*!
  \brief Show help page
*/
void SMESHGUI_MeasureDlg::help()
{
  QString aHelpFile;
  if ( myTabWidget->currentIndex() == MinDistance ) {
    aHelpFile = "measurements_page.html#min_distance_anchor";
  } else if ( myTabWidget->currentIndex() == BoundingBox ) {
    aHelpFile = "measurements_page.html#bounding_box_anchor";
  } else {
    aHelpFile = "measurements_page.html#basic_properties_anchor";
  }

  SMESH::ShowHelpFile( aHelpFile );
}

/*!
  \brief Activate dialog box
*/
void SMESHGUI_MeasureDlg::activate()
{
  SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();
  SMESHGUI::GetSMESHGUI()->SetActiveDialogBox( this );
  myTabWidget->setEnabled( true );
  updateSelection();
}

/*!
  \brief Deactivate dialog box
*/
void SMESHGUI_MeasureDlg::deactivate()
{
  myBasicProps->deactivate();
  myMinDist->deactivate();
  myBndBox->deactivate();
  myTabWidget->setEnabled( false );
  disconnect( SMESHGUI::selectionMgr(), SIGNAL( currentSelectionChanged() ), this, SLOT( updateInfo() ) );
}
