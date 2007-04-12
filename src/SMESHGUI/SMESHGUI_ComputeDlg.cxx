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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHGUI_ComputeDlg.cxx
//  Author : Edward AGAPOV
//  Module : SMESH

#include "SMESHGUI_ComputeDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_HypothesesUtils.h"

#include "SMDS_SetIterator.hxx"

#include "GEOMBase.h"
#include "GEOM_Actor.h"

#include "LightApp_SelectionMgr.h"
#include "LightApp_UpdateFlags.h"
#include "SALOMEDSClient_SObject.hxx"
#include "SALOME_ListIO.hxx"
#include "SVTK_ViewWindow.h"
#include "SVTK_ViewModel.h"
#include "SalomeApp_Tools.h"
#include "SalomeApp_Application.h"
#include "SUIT_ResourceMgr.h"
#include "SUIT_OverrideCursor.h"
#include "SUIT_MessageBox.h"
#include "SUIT_Desktop.h"
#include "SUIT_Study.h"
#include "OB_Browser.h"

// OCCT Includes
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>

#include <TopLoc_Location.hxx>
#include <Poly_Triangulation.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

// QT Includes
#include <qframe.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qtable.h>

#include <vtkProperty.h>

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#include <vector>
#include <set>


#define SPACING 5
#define MARGIN  10

enum TCol { COL_ALGO = 0, COL_SHAPE, COL_ERROR, COL_SHAPEID, COL_PUBLISHED, NB_COLUMNS };

using namespace SMESH;

namespace SMESH {
  
  // =========================================================================================
  /*!
   * \brief Class showing shapes without publishing
   */
  // =========================================================================================

  class TShapeDisplayer
  {
  public:
    // -----------------------------------------------------------------------
    TShapeDisplayer(): myViewWindow(0)
    {
      myProperty = vtkProperty::New();
      myProperty->SetRepresentationToWireframe();
      myProperty->SetColor( 250, 0, 250 );
      myProperty->SetAmbientColor( 250, 0, 250 );
      myProperty->SetDiffuseColor( 250, 0, 250 );
      //myProperty->SetSpecularColor( 250, 0, 250 );
      myProperty->SetLineWidth( 5 );
    }
    // -----------------------------------------------------------------------
    ~TShapeDisplayer()
    {
      DeleteActors();
      myProperty->Delete();
    }
    // -----------------------------------------------------------------------
    void DeleteActors()
    {
      if ( hasViewWindow() ) {
        TActorIterator actorIt = actorIterator();
        while ( actorIt.more() )
          if (VTKViewer_Actor* anActor = actorIt.next()) {
            myViewWindow->RemoveActor( anActor );
            //anActor->Delete();
          }
      }
      myIndexToShape.Clear();
      myActors.clear();
      myShownActors.clear();
      myBuiltSubs.clear();
    }
    // -----------------------------------------------------------------------
    void SetVisibility (bool theVisibility)
    {
      TActorIterator actorIt = shownIterator();
      while ( actorIt.more() )
        if (VTKViewer_Actor* anActor = actorIt.next())
          anActor->SetVisibility(theVisibility);
      SMESH::RepaintCurrentView();
    }
    // -----------------------------------------------------------------------
    bool HasReadyActorsFor (int subShapeID, GEOM::GEOM_Object_var aMainShape )
    {
      string mainEntry;
      if ( !aMainShape->_is_nil() )
        mainEntry = aMainShape->GetStudyEntry();
      return ( myMainEntry == mainEntry &&
               myBuiltSubs.find( subShapeID ) != myBuiltSubs.end() );
    }
    // -----------------------------------------------------------------------
    void Show( int subShapeID, GEOM::GEOM_Object_var aMainShape, bool only = false)
    {
      SVTK_ViewWindow* aViewWindow  = SMESH::GetViewWindow( SMESHGUI::GetSMESHGUI() );
      string mainEntry;
      if ( !aMainShape->_is_nil() )
        mainEntry = aMainShape->GetStudyEntry();
      if ( myMainEntry != mainEntry || aViewWindow != myViewWindow ) { // remove actors
        DeleteActors();
        TopoDS_Shape aShape;
        if ( !aMainShape->_is_nil() && GEOMBase::GetShape(aMainShape, aShape)) {
          checkTriangulation( aShape );
          TopExp::MapShapes(aShape, myIndexToShape);
          myActors.resize( myIndexToShape.Extent(), 0 );
          myShownActors.reserve( myIndexToShape.Extent() );
        }
        myMainEntry  = mainEntry;
        myViewWindow = aViewWindow;
      }
      if ( only ) { // hide shown actors
        TActorIterator actorIt = shownIterator();
        while ( actorIt.more() )
          if (VTKViewer_Actor* anActor = actorIt.next())
            anActor->SetVisibility(false);
        myShownActors.clear();
      }
      // find actors to show
      TopoDS_Shape aShape = myIndexToShape( subShapeID );
      if ( !aShape.IsNull() ) {
        TopAbs_ShapeEnum type( aShape.ShapeType() >= TopAbs_WIRE ? TopAbs_EDGE : TopAbs_FACE );
        for ( TopExp_Explorer exp( aShape, type ); exp.More(); exp.Next() ) {
          //checkTriangulation( exp.Current() );
          if ( GEOM_Actor* anActor = getActor( exp.Current() ))
            myShownActors.push_back( anActor );
        }
        if ( type == TopAbs_FACE ) {
          for ( TopExp_Explorer exp( aShape, TopAbs_EDGE ); exp.More(); exp.Next() ) {
            const TopoDS_Edge & edge = TopoDS::Edge( exp.Current() );
            if ( !BRep_Tool::Degenerated( edge ))
              if ( GEOM_Actor* anActor = getActor( exp.Current() ))
                myShownActors.push_back( anActor );
          }
        }
      }
      myBuiltSubs.insert( subShapeID );
      SetVisibility(true);
    }
    // -----------------------------------------------------------------------

  private:

    typedef std::vector<GEOM_Actor*> TActorVec;
    TActorVec                  myActors;
    TActorVec                  myShownActors;
    TopTools_IndexedMapOfShape myIndexToShape;
    string                     myMainEntry;
    SVTK_ViewWindow*           myViewWindow;
    vtkProperty*               myProperty;
    std::set<int>              myBuiltSubs;

    // -----------------------------------------------------------------------
    typedef SMDS_SetIterator< GEOM_Actor*, TActorVec::const_iterator> TActorIterator;
    TActorIterator actorIterator() {
      return TActorIterator( myActors.begin(), myActors.end() );
    }
    TActorIterator shownIterator() {
      return TActorIterator( myShownActors.begin(), myShownActors.end() );
    }
    // -----------------------------------------------------------------------
    GEOM_Actor* getActor(const TopoDS_Shape& shape)
    {
      int index = myIndexToShape.FindIndex( shape ) - 1;
      if ( index < 0 || index >= myActors.size() )
        return 0;
      GEOM_Actor* & actor = myActors[ index ];
      if ( !actor ) {
        actor = GEOM_Actor::New();
        if ( actor ) {
          actor->setInputShape(shape,0,0);
          //actor->SetProperty(myProperty);
          actor->SetShadingProperty(myProperty);
          actor->SetWireframeProperty(myProperty);
          actor->SetPreviewProperty(myProperty);
          actor->PickableOff();
          //         if ( shape.ShapeType() == TopAbs_EDGE )
          //           actor->SubShapeOn();
          myViewWindow->AddActor( actor );
        }
      }
      return actor;
    }
    // -----------------------------------------------------------------------
    void checkTriangulation(const TopoDS_Shape& shape)
    {
      TopLoc_Location aLoc;
      Standard_Boolean alreadymesh = Standard_True;
      TopExp_Explorer ex(shape, TopAbs_FACE);
      if ( ex.More() )
        for (; ex.More(); ex.Next()) {
          const TopoDS_Face& aFace = TopoDS::Face(ex.Current());
          Handle(Poly_Triangulation) aPoly = BRep_Tool::Triangulation(aFace,aLoc);
          if(aPoly.IsNull()) { alreadymesh = Standard_False; break; }
        }
      else
        for (ex.Init(shape, TopAbs_EDGE); ex.More(); ex.Next()) {
          const TopoDS_Edge& edge = TopoDS::Edge(ex.Current());
          Handle(Poly_Polygon3D) aPoly = BRep_Tool::Polygon3D(edge, aLoc);
          if(aPoly.IsNull()) { alreadymesh = Standard_False; break; }
        }
      if (alreadymesh) return;
      // Compute default deflection
      Bnd_Box B;
      BRepBndLib::Add(shape, B);
      Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
      B.Get(aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);
      double deflection = Max( aXmax-aXmin , Max ( aYmax-aYmin , aZmax-aZmin)) * 0.01 *4;
      BRepMesh_IncrementalMesh MESH(shape,deflection);
    }
    // -----------------------------------------------------------------------
    bool hasViewWindow() const
    {
      if ( !myViewWindow ) return false;

      if ( SalomeApp_Application* anApp = SMESHGUI::GetSMESHGUI()->getApp() )
        return FindVtkViewWindow( anApp->getViewManager(SVTK_Viewer::Type(), false ),
                                  myViewWindow );
      return false;
    }
  };

  // =========================================================================================
  /*!
   * \brief Return text describing an error
   */
#define CASE2TEXT(enum) case SMESH::enum: text = QObject::tr( #enum ); break;
  QString errorText(int errCode, const char* comment)
  {
    QString text;
    switch ( errCode ) {
      CASE2TEXT( COMPERR_OK            );
      CASE2TEXT( COMPERR_BAD_INPUT_MESH);
      CASE2TEXT( COMPERR_STD_EXCEPTION );
      CASE2TEXT( COMPERR_OCC_EXCEPTION );
      CASE2TEXT( COMPERR_SLM_EXCEPTION );
      CASE2TEXT( COMPERR_EXCEPTION     );
      CASE2TEXT( COMPERR_MEMORY_PB     );
      CASE2TEXT( COMPERR_BAD_SHAPE     );
    case SMESH::COMPERR_ALGO_FAILED:
      if ( strlen(comment) == 0 )
        text = QObject::tr("COMPERR_ALGO_FAILED");
      break;
    default:
      text = QString("#%1").arg( -errCode );
    }
    if ( text.length() > 0 ) text += ". ";
    return text + comment;
  }
  // -----------------------------------------------------------------------
  /*!
   * \brief Return SO of a subshape
   */
  _PTR(SObject) getSubShapeSO( int subShapeID, GEOM::GEOM_Object_var aMainShape)
  {
    _PTR(SObject) so = SMESH::FindSObject(aMainShape);
    if ( subShapeID == 1 || !so )
      return so;
    _PTR(ChildIterator) it;
    if (_PTR(Study) study = SMESH::GetActiveStudyDocument())
      it =  study->NewChildIterator(so);
    _PTR(SObject) subSO;
    if ( it ) {
      for ( it->InitEx(true); !subSO && it->More(); it->Next() ) {
        GEOM::GEOM_Object_var geom = SMESH::SObjectToInterface<GEOM::GEOM_Object>( it->Value() );
        if ( !geom->_is_nil() ) {
          GEOM::ListOfLong_var list = geom->GetSubShapeIndices();
          if ( list->length() == 1 && list[0] == subShapeID )
            subSO = it->Value();
        }
      }
    }
    return subSO;
  }
  // -----------------------------------------------------------------------
  /*!
   * \brief Return subshape by ID
   */
  GEOM::GEOM_Object_ptr getSubShape( int subShapeID, GEOM::GEOM_Object_var aMainShape)
  {
    GEOM::GEOM_Object_var aSubShape;
    if ( subShapeID == 1 )
      aSubShape = aMainShape;
    else if ( _PTR(SObject) so = getSubShapeSO( subShapeID, aMainShape ))
      aSubShape = SMESH::SObjectToInterface<GEOM::GEOM_Object>( so );
    else
      aSubShape = SMESH::GetSubShape( aMainShape, subShapeID );
    return aSubShape._retn();
  }
  // -----------------------------------------------------------------------
  /*!
   * \brief Return shape type name
   */
#define CASE2NAME(enum) case GEOM::enum: name = QObject::tr( "GEOM_" #enum ); break;
  QString shapeTypeName(GEOM::GEOM_Object_var aShape, const char* dflt = "" )
  {
    QString name = dflt;
    if ( !aShape->_is_nil() ) {
      switch ( aShape->GetShapeType() ) {
      CASE2NAME( VERTEX    );
      CASE2NAME( EDGE      );
      CASE2NAME( WIRE      );
      CASE2NAME( FACE      );
      CASE2NAME( SHELL     );
      CASE2NAME( SOLID     );
      CASE2NAME( COMPSOLID );
      CASE2NAME( COMPOUND  );
      default:;
      }
    }
    return name;
  }
  // -----------------------------------------------------------------------
  /*!
   * \brief Return text describing a subshape
   */
  QString shapeText(int subShapeID, GEOM::GEOM_Object_var aMainShape )
  {
    QString text;
    if ( _PTR(SObject) aSO = getSubShapeSO( subShapeID, aMainShape ))
      text = aSO->GetName();
    else {
      text = QString("#%1").arg( subShapeID );
      QString typeName = shapeTypeName( getSubShape( subShapeID, aMainShape ));
      if ( typeName.length() )
        text += QString(" (%1)").arg(typeName);
    }
    return text;
  }
  // -----------------------------------------------------------------------
  /*!
   * \brief Return text describing a subshape
   */
  bool getSelectedRows(QTable* table, list< int > & rows)
  {
    rows.clear();
    int nbSel = table->numSelections();
    for ( int i = 0; i < nbSel; ++i )
    {
      QTableSelection selected = table->selection(i);
      if ( !selected.isActive() ) continue;
      for ( int row = selected.topRow(); row <= selected.bottomRow(); ++row )
        rows.push_back( row );
    }
    return !rows.empty();
  }
  
} // namespace SMESH


// =========================================================================================
/*!
 * \brief Dialog to compute a mesh and show computation errors
 */
//=======================================================================

SMESHGUI_ComputeDlg::SMESHGUI_ComputeDlg(): SMESHGUI_Dialog( 0, false, true, OK | Help )
{
  QVBoxLayout* aDlgLay = new QVBoxLayout (mainFrame(), 0, SPACING);

  QFrame* aMainFrame = createMainFrame  (mainFrame());

  aDlgLay->addWidget(aMainFrame);

  aDlgLay->setStretchFactor(aMainFrame, 1);
}

//=======================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's fields
//=======================================================================

#define CASE2HEADER(enum) case enum: header = QObject::tr( #enum "_HEADER" ); break;

QFrame* SMESHGUI_ComputeDlg::createMainFrame (QWidget* theParent)
{
  QFrame* aFrame = new QFrame(theParent);

  SUIT_ResourceMgr* rm = resourceMgr();
  QPixmap iconCompute (rm->loadPixmap("SMESH", tr("ICON_COMPUTE")));

  // constructor

  QButtonGroup* aPixGrp = new QButtonGroup(1, Qt::Vertical, tr("CONSTRUCTOR"), aFrame);
  aPixGrp->setExclusive(TRUE);
  QRadioButton* aRBut = new QRadioButton(aPixGrp);
  aRBut->setPixmap(iconCompute);
  aRBut->setChecked(TRUE);

  // Mesh Info

  QGroupBox* infoGrp = new QGroupBox( 2, Qt::Horizontal,
                                      tr("SMESH_MESHINFO_TITLE"), aFrame, "infoGrp" );
  QLabel* nodeLabel = new QLabel(tr("SMESH_MESHINFO_NODES"), infoGrp );
  myNbNodesLabel    = new QLabel("0", infoGrp );
  QLabel* edgeLabel = new QLabel(tr("SMESH_MESHINFO_EDGES"), infoGrp );
  myNbEdgesLabel    = new QLabel("0", infoGrp );
  QLabel* faceLabel = new QLabel(tr("SMESH_MESHINFO_FACES"), infoGrp);
  myNbFacesLabel    = new QLabel("0", infoGrp );
  QLabel* volumeLbl = new QLabel(tr("SMESH_MESHINFO_VOLUMES"), infoGrp);
  myNbVolumLabel    = new QLabel("0", infoGrp );

  // errors

  QGroupBox* errorGrp = new QGroupBox(tr("ERRORS"), aFrame, "errorGrBox");
  myTable      = new QTable( 1, NB_COLUMNS, errorGrp, "myTable");
  myShowBtn    = new QPushButton(tr("SHOW_SHAPE"), errorGrp, "myShowBtn");
  myPublishBtn = new QPushButton(tr("PUBLISH_SHAPE"), errorGrp, "myPublishBtn");

  myTable->setReadOnly( TRUE );
  myTable->hideColumn( COL_PUBLISHED );
  myTable->hideColumn( COL_SHAPEID );
  myTable->setColumnStretchable( COL_ERROR, 1 );
  for ( int col = 0; col < NB_COLUMNS; ++col ) {
    QString header;
    switch ( col ) {
    CASE2HEADER( COL_ALGO     );
    CASE2HEADER( COL_SHAPE    );
    CASE2HEADER( COL_ERROR    );
    CASE2HEADER( COL_SHAPEID  );
    CASE2HEADER( COL_PUBLISHED);
    }
    myTable->horizontalHeader()->setLabel( col, header );
  }

  errorGrp->setColumnLayout(0, Qt::Vertical);
  errorGrp->layout()->setSpacing(0);
  errorGrp->layout()->setMargin(0);
  QGridLayout* grpLayout = new QGridLayout(errorGrp->layout());
  grpLayout->setAlignment(Qt::AlignTop);
  grpLayout->setSpacing(SPACING);
  grpLayout->setMargin(MARGIN);
  grpLayout->addMultiCellWidget( myTable,   0, 2, 0, 0 );
  grpLayout->addWidget         ( myShowBtn,    0, 1 );
  grpLayout->addWidget         ( myPublishBtn, 1, 1 );
  grpLayout->setRowStretch( 2, 1 );

  QVBoxLayout* aLay = new QVBoxLayout(aFrame);
  aLay->addWidget( aPixGrp );
  aLay->addWidget( infoGrp );
  aLay->addWidget( errorGrp );
  aLay->setStretchFactor( errorGrp, 1 );

  return aFrame;
}
//================================================================================
/*!
 * \brief Show mesh info
 */
//================================================================================

void SMESHGUI_ComputeDlg::SetMeshInfo(int nbNodes, int nbEdges, int nbFaces, int nbVolums)
{
  myNbNodesLabel->setText(QString("%1").arg(nbNodes));
  myNbEdgesLabel->setText(QString("%1").arg(nbEdges));
  myNbFacesLabel->setText(QString("%1").arg(nbFaces));
  myNbVolumLabel->setText(QString("%1").arg(nbVolums));
}

//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_ComputeOp::SMESHGUI_ComputeOp()
{
  myDlg = new SMESHGUI_ComputeDlg;
  myTShapeDisplayer = new TShapeDisplayer();

  // connect signals and slots
  connect(myDlg->myShowBtn,    SIGNAL (clicked()), SLOT(onPreviewShape()));
  connect(myDlg->myPublishBtn, SIGNAL (clicked()), SLOT(onPublishShape()));
  connect(table(),SIGNAL(selectionChanged()), SLOT(currentCellChanged()));
}

//=======================================================================
// function : startOperation()
// purpose  : Init dialog fields, connect signals and slots, show dialog
//=======================================================================

void SMESHGUI_ComputeOp::startOperation()
{
  SMESHGUI_Operation::startOperation();

  SMESH::SMESH_Mesh_var          aMesh;
  SMESH::compute_error_array_var anErrors;

  myMainShape = GEOM::GEOM_Object::_nil();

  // COMPUTE MESH

  bool computeFailed = true;
  int nbNodes = 0, nbEdges = 0, nbFaces = 0, nbVolums = 0;

  LightApp_SelectionMgr *Sel = selectionMgr();
  SALOME_ListIO selected; Sel->selectedObjects( selected );

  int nbSel = selected.Extent();
  if (nbSel != 1) {
    SUIT_MessageBox::warn1(desktop(),
                           tr("SMESH_WRN_WARNING"),
                           tr("SMESH_WRN_NO_AVAILABLE_DATA"),
                           tr("SMESH_BUT_OK"));
    onCancel();
    return;
  }

  Handle(SALOME_InteractiveObject) IObject = selected.First();
  aMesh = SMESH::GetMeshByIO(IObject);
  if (!aMesh->_is_nil()) {
    myMainShape = aMesh->GetShapeToMesh();
    if ( !myMainShape->_is_nil() ) {
      SMESH::SMESH_Gen_var gen = getSMESHGUI()->GetSMESHGen();
      SMESH::algo_error_array_var errors = gen->GetAlgoState(aMesh,myMainShape);
      if ( errors->length() > 0 ) {
        SUIT_MessageBox::warn1(desktop(), tr("SMESH_WRN_WARNING"),
                               SMESH::GetMessageOnAlgoStateErrors( errors.in() ),
                               tr("SMESH_BUT_OK"));
        onCancel();
        return;
      }
      try {
        if (gen->Compute(aMesh, myMainShape)) {
          computeFailed = false;
        }
        else {
          anErrors = gen->GetComputeErrors( aMesh, myMainShape );
//           if ( anErrors->length() == 0 ) {
//             SUIT_MessageBox::warn1(desktop(),
//                                    tr("SMESH_WRN_WARNING"),
//                                    tr("SMESH_WRN_COMPUTE_FAILED"),
//                                    tr("SMESH_BUT_OK"));
//             onCancel();
//             return;
//           }
        }
        nbNodes = aMesh->NbNodes();
        nbEdges = aMesh->NbEdges();
        nbFaces = aMesh->NbFaces();
        nbVolums = aMesh->NbVolumes();
        _PTR(SObject) aMeshSObj = SMESH::FindSObject(aMesh);
        SMESH::ModifiedMesh(aMeshSObj, !computeFailed, nbNodes == 0);
      }
      catch(const SALOME::SALOME_Exception & S_ex){
        SalomeApp_Tools::QtCatchCorbaException(S_ex);
      }
      update( UF_ObjBrowser | UF_Model );

      if ( getSMESHGUI()->automaticUpdate() ) {
        SVTK_ViewWindow* aVTKView = SMESH::GetViewWindow(getSMESHGUI(), true);
        if (aVTKView) {
          int anId = study()->id();
          TVisualObjPtr aVisualObj = SMESH::GetVisualObj(anId, IObject->getEntry());
          if (aVisualObj) {
            aVisualObj->Update();
            SMESH_Actor* anActor = SMESH::FindActorByEntry(IObject->getEntry());
            if (!anActor) {
              anActor = SMESH::CreateActor(studyDS(), IObject->getEntry());
              if (anActor) {
                SMESH::DisplayActor(aVTKView, anActor); //apo
                SMESH::FitAll();
              }
            }
            SMESH::RepaintCurrentView();
            Sel->setSelectedObjects( selected );
          }
        }
      }
    }
  }

  // SHOW Mesh Infos

  myDlg->SetMeshInfo( nbNodes, nbEdges, nbFaces, nbVolums);
  myDlg->setCaption(tr( computeFailed ? "SMESH_WRN_COMPUTE_FAILED" : "SMESH_COMPUTE_SUCCEED"));

  // SHOW ERRORS

  bool noError = ( !anErrors.operator->() || anErrors->length() == 0 );

  QTable* tbl = myDlg->myTable;

  if ( noError )
  {
    tbl->setNumRows(0);
  }
  else
  {
    // fill table of errors
    tbl->setNumRows( anErrors->length() );
    bool hasShape = aMesh->HasShapeToMesh();
    if ( !hasShape ) tbl->hideColumn( COL_SHAPE );
    else             tbl->showColumn( COL_SHAPE );
    tbl->setColumnWidth( COL_ERROR, 200 );

    for ( int row = 0; row < anErrors->length(); ++row )
    {
      SMESH::ComputeError & err = anErrors[ row ];
      tbl->setText( row, COL_ALGO,    err.algoName.in() );
      tbl->setText( row, COL_ERROR,   errorText( err.code, err.comment.in() ));
      tbl->setText( row, COL_SHAPEID, QString("%1").arg( err.subShapeID ));

      QString text = hasShape ? shapeText( err.subShapeID, myMainShape ) : QString("");
      tbl->setText( row, COL_SHAPE,   text );

      text = ( !hasShape || getSubShapeSO( err.subShapeID, myMainShape )) ? "PUBLISHED" : "";
      tbl->setText( row, COL_PUBLISHED, text ); // if text=="", "PUBLISH" button enabled

      tbl->item( row, COL_ERROR )->setWordWrap( TRUE );
      tbl->adjustRow( row );
    }
    tbl->adjustColumn( COL_ALGO );
    tbl->adjustColumn( COL_SHAPE );

    tbl->setCurrentCell(0,0);
    currentCellChanged(); // to update buttons
  }

  myDlg->show();
}

//================================================================================
/*!
 * \brief Stops operation
 */
//================================================================================

void SMESHGUI_ComputeOp::stopOperation()
{
  SMESHGUI_Operation::stopOperation();
  myTShapeDisplayer->SetVisibility( false );
}

//================================================================================
/*!
 * \brief publish selected subshape
 */
//================================================================================

void SMESHGUI_ComputeOp::onPublishShape()
{
  GEOM::GEOM_Gen_var geomGen = SMESH::GetGEOMGen();
  SALOMEDS::Study_var study = SMESHGUI::GetSMESHGen()->GetCurrentStudy();

  list< int > rows;
  list< int >::iterator row;
  getSelectedRows( table(), rows );
  for ( row = rows.begin(); row != rows.end(); ++row )
  {
    int curSub = table()->text(*row, COL_SHAPEID).toInt();
    GEOM::GEOM_Object_var shape = getSubShape( curSub, myMainShape );
    if ( !shape->_is_nil() && ! getSubShapeSO( curSub, myMainShape ))
    {
      if ( !getSubShapeSO( 1, myMainShape )) // the main shape not published
      {
        QString name = GEOMBase::GetDefaultName( shapeTypeName( myMainShape, "MAIN_SHAPE" ));
        SALOMEDS::SObject_var so =
          geomGen->AddInStudy( study, myMainShape, name, GEOM::GEOM_Object::_nil());
        // look for myMainShape in the table
        for ( int r = 0, nr = table()->numRows(); r < nr; ++r ) {
          if ( table()->text(r, COL_SHAPEID) == "1" ) {
            if ( so->_is_nil() ) {
              table()->setText( r, COL_SHAPE, so->GetName() );
              table()->setText( r, COL_PUBLISHED, so->GetID() );
            }
            break;
          }
        }
        if ( curSub == 1 ) continue;
      }
      QString name = GEOMBase::GetDefaultName( shapeTypeName( shape, "ERROR_SHAPE" ));
      SALOMEDS::SObject_var so = geomGen->AddInStudy( study, shape, name, myMainShape);
      if ( !so->_is_nil() ) {
        table()->setText( *row, COL_SHAPE, so->GetName() );
        table()->setText( *row, COL_PUBLISHED, so->GetID() );
      }
    }
  }
  getSMESHGUI()->getApp()->updateObjectBrowser();
  currentCellChanged(); // to update buttons
}

//================================================================================
/*!
 * \brief SLOT called when a selected cell in table() changed
 */
//================================================================================

void SMESHGUI_ComputeOp::currentCellChanged()
{
  myTShapeDisplayer->SetVisibility( false );

  bool publishEnable = 0, showEnable = 0, showOnly = 1;
  list< int > rows;
  list< int >::iterator row;
  getSelectedRows( table(), rows );
  for ( row = rows.begin(); row != rows.end(); ++row )
  {
    bool hasData     = ( !table()->text(*row, COL_SHAPE).isEmpty() );
    bool isPublished = ( !table()->text(*row, COL_PUBLISHED).isEmpty() );
    if ( hasData && !isPublished )
      publishEnable = true;

    int curSub = table()->text(*row, COL_SHAPEID).toInt();
    bool prsReady = myTShapeDisplayer->HasReadyActorsFor( curSub, myMainShape );
    if ( prsReady ) {
      myTShapeDisplayer->Show( curSub, myMainShape, showOnly );
      showOnly = false;
    }
    else {
      showEnable = true;
    }
  }
  myDlg->myPublishBtn->setEnabled( publishEnable );
  myDlg->myShowBtn->setEnabled( showEnable );
}

//================================================================================
/*!
 * \brief update preview
 */
//================================================================================

void SMESHGUI_ComputeOp::onPreviewShape()
{
  if ( myTShapeDisplayer )
  {
    SUIT_OverrideCursor aWaitCursor;
    list< int > rows;
    list< int >::iterator row;
    getSelectedRows( table(), rows );

    bool showOnly = true;
    for ( row = rows.begin(); row != rows.end(); ++row )
    {
      int curSub = table()->text(*row, COL_SHAPEID).toInt();
      if ( curSub > 0 ) {
        myTShapeDisplayer->Show( curSub, myMainShape, showOnly );
        showOnly = false;
      }
    }
    currentCellChanged(); // to update buttons
  }
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

SMESHGUI_ComputeOp::~SMESHGUI_ComputeOp()
{
  if ( myTShapeDisplayer ) delete myTShapeDisplayer;
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
 * \retval LightApp_Dialog* - pointer to dialog of this operation
 */
//================================================================================

LightApp_Dialog* SMESHGUI_ComputeOp::dlg() const
{
  return myDlg;
}

//================================================================================
/*!
 * \brief perform it's intention action: compute mesh
 */
//================================================================================

bool SMESHGUI_ComputeOp::onApply()
{
  return true;
}

//================================================================================
/*!
 * \brief Return a table
 */
//================================================================================

QTable* SMESHGUI_ComputeOp::table()
{
  return myDlg->myTable;
}
