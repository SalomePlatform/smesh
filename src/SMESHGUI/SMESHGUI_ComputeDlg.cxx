// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File   : SMESHGUI_ComputeDlg.cxx
// Author : Edward AGAPOV, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_ComputeDlg.h"

#include "SMDS_Mesh.hxx"
#include "SMDS_SetIterator.hxx"
#include "SMESHGUI.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_HypothesesUtils.h"
#include "SMESHGUI_MeshEditPreview.h"
#include "SMESHGUI_MeshInfosBox.h"
#include "SMESHGUI_MeshOrderDlg.h"
#include "SMESHGUI_MeshOrderOp.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESH_Actor.h"
#include "SMESH_ActorUtils.h"

// SALOME GEOM includes
#include <GEOMBase.h>
#include <GEOM_Actor.h>
#include <GEOM_wrap.hxx>

// SALOME GUI includes
#include <LightApp_SelectionMgr.h>
#include <LightApp_UpdateFlags.h>
#include <QtxComboBox.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SalomeApp_Application.h>

// SALOME KERNEL includes
#include <SALOMEDS_SObject.hxx>
#include <SALOMEDSClient_SObject.hxx>
#include <SALOMEDS_wrap.hxx>
#include "utilities.h"

#include CORBA_SERVER_HEADER(SMESH_Group)

// OCCT includes
#include <BRepBndLib.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <Poly_Triangulation.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>

#include <Standard_ErrorHandler.hxx>

// Qt includes
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QTimerEvent>
#include <QProgressBar>

// VTK includes
#include <vtkProperty.h>

// STL includes
#include <vector>
#include <set>

#ifndef WIN32
#include <sys/sysinfo.h>
#endif

#define SPACING 6
#define MARGIN  11

#define COLONIZE(str)   (QString(str).contains(":") > 0 ? QString(str) : QString(str) + " :" )
#define __SHAPE_RGB__ 250, 0, 250

enum TCol {
  COL_ALGO = 0, COL_SHAPE, COL_ERROR, COL_SHAPEID, COL_PUBLISHED, COL_BAD_MESH, NB_COLUMNS
};

//using namespace SMESH;

namespace SMESH
{
  //=============================================================================
  /*!
   * \brief Allocate some memory at construction and release it at destruction.
   * Is used to be able to continue working after mesh generation or visualization
   * break due to lack of memory
   */
  //=============================================================================

  struct MemoryReserve
  {
    char* myBuf;
    MemoryReserve(): myBuf( new char[1024*1024*1] ){} // 1M
    void release() { delete [] myBuf; myBuf = 0; }
    ~MemoryReserve() { release(); }
  };

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
      myProperty->SetColor( __SHAPE_RGB__ );
      myProperty->SetAmbientColor( __SHAPE_RGB__ );
      myProperty->SetDiffuseColor( __SHAPE_RGB__ );
      //myProperty->SetSpecularColor( __SHAPE_RGB__ );
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
      std::string mainEntry;
      if ( !aMainShape->_is_nil() )
        mainEntry = aMainShape->GetStudyEntry();
      return ( myMainEntry == mainEntry &&
               myBuiltSubs.find( subShapeID ) != myBuiltSubs.end() );
    }
    // -----------------------------------------------------------------------
    void Show( int subShapeID, GEOM::GEOM_Object_var aMainShape, bool only = false)
    {
      SVTK_ViewWindow* aViewWindow  = SMESH::GetViewWindow( SMESHGUI::GetSMESHGUI() );
      SUIT_ResourceMgr* resMgr = SMESH::GetResourceMgr( SMESHGUI::GetSMESHGUI() );
      std::string mainEntry;
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
          if ( GEOM_Actor* anActor = getActor( exp.Current() ) ) {
            int UNbIsos = resMgr->integerValue( "Geometry", "iso_number_u", 1);
            int VNbIsos = resMgr->integerValue( "Geometry", "iso_number_v", 1);
            int aNbIsos[2] = { UNbIsos ? UNbIsos : 1, VNbIsos ? VNbIsos : 1 };
            anActor->SetNbIsos( aNbIsos );
            myShownActors.push_back( anActor );
          }
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
    std::string                myMainEntry;
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
      if ( index < 0 || index >= (int) myActors.size() )
        return 0;
      GEOM_Actor* & actor = myActors[ index ];
      if ( !actor ) {
        actor = GEOM_Actor::New();
        if ( actor ) {
          actor->SetShape(shape,0,0);
          // actor->SetProperty(myProperty);
          // actor->SetShadingProperty(myProperty);
          // actor->SetWireframeProperty(myProperty);
          // actor->SetPreviewProperty(myProperty);
          actor->PickableOff();
          //
          actor->SetWidth( myProperty->GetLineWidth() );
          actor->SetIsosWidth( myProperty->GetLineWidth() );
          actor->SetIsosColor( __SHAPE_RGB__ );
          actor->SetColor( __SHAPE_RGB__ );
          // if ( shape.ShapeType() == TopAbs_EDGE )
          //   actor->SubShapeOn();
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
        for ( ; ex.More(); ex.Next()) {
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
      double deflection = Max( aXmax-aXmin, Max ( aYmax-aYmin, aZmax-aZmin)) * 0.01 *4;
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
      CASE2TEXT( COMPERR_OK               );
      CASE2TEXT( COMPERR_BAD_INPUT_MESH   );
      CASE2TEXT( COMPERR_STD_EXCEPTION    );
      CASE2TEXT( COMPERR_OCC_EXCEPTION    );
    case SMESH::COMPERR_SLM_EXCEPTION: break; // avoid double "Salome exception"
      CASE2TEXT( COMPERR_EXCEPTION        );
      CASE2TEXT( COMPERR_MEMORY_PB        );
      CASE2TEXT( COMPERR_BAD_SHAPE        );
      CASE2TEXT( COMPERR_CANCELED         );
      CASE2TEXT( COMPERR_NO_MESH_ON_SHAPE );
      CASE2TEXT( COMPERR_BAD_PARMETERS    );
    case SMESH::COMPERR_ALGO_FAILED:
      if ( strlen(comment) == 0 )
        text = QObject::tr("COMPERR_ALGO_FAILED");
      break;
    case SMESH::COMPERR_WARNING:
      text = QObject::tr( (comment && strlen(comment)) ? "COMPERR_WARNING" : "COMPERR_UNKNOWN");
      break;
    default:
      text = QString("#%1").arg( -errCode );
    }
    if ( text.length() > 0 ) text += ". ";
    return text + comment;
  }
  // -----------------------------------------------------------------------
  /*!
   * \brief Return SO of a sub-shape
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
          {
            GEOM::GEOM_Object_var mainGO = geom->GetMainShape();
            if ( aMainShape->IsSame( mainGO ))
              subSO = it->Value();
          }
        }
      }
    }
    return subSO;
  }
  // -----------------------------------------------------------------------
  /*!
   * \brief Return sub-shape by ID. WARNING: UnRegister() must be called on a result
   */
  GEOM::GEOM_Object_ptr getSubShape( int subShapeID, GEOM::GEOM_Object_var aMainShape)
  {
    GEOM::GEOM_Object_var aSubShape;
    if ( subShapeID == 1 ) {
      aSubShape = aMainShape;
      aSubShape->Register();
    }
    else if ( _PTR(SObject) so = getSubShapeSO( subShapeID, aMainShape )) {
      aSubShape = SMESH::SObjectToInterface<GEOM::GEOM_Object>( so );
      aSubShape->Register();
    }
    else {
      aSubShape = SMESH::GetSubShape( aMainShape, subShapeID );
      // future call of UnRegister() will delete a servant of this new object
    }
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
   * \brief Return text describing a sub-shape
   */
  QString shapeText(int subShapeID, GEOM::GEOM_Object_var aMainShape )
  {
    QString text;
    if ( _PTR(SObject) aSO = getSubShapeSO( subShapeID, aMainShape )) {
      text  = aSO->GetName().c_str();
      text += QString(" (%1)").arg( aSO->GetID().c_str() );
    }
    else {
      text = QString("#%1").arg( subShapeID );
      GEOM::GEOM_Object_wrap shape = getSubShape( subShapeID, aMainShape );
      QString typeName = shapeTypeName( shape );
      if ( typeName.length() )
        text += QString(" (%1)").arg(typeName);
    }
    return text;
  }
  // -----------------------------------------------------------------------
  /*!
   * \brief Return a list of selected rows
   */
  int getSelectedRows(QTableWidget* table, QList<int>& rows)
  {
    rows.clear();
    QList<QTableWidgetSelectionRange> selRanges = table->selectedRanges();
    QTableWidgetSelectionRange range;
    foreach( range, selRanges )
    {
      for ( int row = range.topRow(); row <= range.bottomRow(); ++row )
        if ( !rows.count( row ))
             rows.append( row );
    }
    if ( rows.isEmpty() && table->currentRow() > -1 )
      if ( !rows.count( table->currentRow() ))
        rows.append( table->currentRow() );

    return rows.count();
  }

} // namespace SMESH


// =========================================================================================
/*!
 * \brief Dialog to compute a mesh and show computation errors
 */
//=======================================================================

SMESHGUI_ComputeDlg::SMESHGUI_ComputeDlg( QWidget* parent, bool ForEval )
 : SMESHGUI_Dialog( parent, false, true, Close | Help )
{
  QVBoxLayout* aDlgLay = new QVBoxLayout (mainFrame());
  aDlgLay->setMargin( 0 );
  aDlgLay->setSpacing( SPACING );

  QFrame* aMainFrame = createMainFrame(mainFrame(),ForEval);

  aDlgLay->addWidget(aMainFrame);

  aDlgLay->setStretchFactor(aMainFrame, 1);
}

// =========================================================================================
/*!
 * \brief Destructor
 */
//=======================================================================

SMESHGUI_ComputeDlg::~SMESHGUI_ComputeDlg()
{
}

//=======================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's fields
//=======================================================================

QFrame* SMESHGUI_ComputeDlg::createMainFrame (QWidget* theParent, bool ForEval)
{
  QFrame* aFrame = new QFrame(theParent);

  SUIT_ResourceMgr* rm = resourceMgr();
  QPixmap iconCompute (rm->loadPixmap("SMESH", tr("ICON_COMPUTE")));

  // constructor

  QGroupBox* aPixGrp;
  if(ForEval) {
    aPixGrp = new QGroupBox(tr("EVAL_DLG"), aFrame);
  }
  else {
    aPixGrp = new QGroupBox(tr("CONSTRUCTOR"), aFrame);
  }
  QButtonGroup* aBtnGrp = new QButtonGroup(this);
  QHBoxLayout* aPixGrpLayout = new QHBoxLayout(aPixGrp);
  aPixGrpLayout->setMargin(MARGIN); aPixGrpLayout->setSpacing(SPACING);

  QRadioButton* aRBut = new QRadioButton(aPixGrp);
  aRBut->setIcon(iconCompute);
  aRBut->setChecked(true);
  aPixGrpLayout->addWidget(aRBut);
  aBtnGrp->addButton(aRBut, 0);

  // Mesh name

  QGroupBox* nameBox = new QGroupBox(tr("SMESH_MESHINFO_NAME"), aFrame );
  QHBoxLayout* nameBoxLayout = new QHBoxLayout(nameBox);
  nameBoxLayout->setMargin(MARGIN); nameBoxLayout->setSpacing(SPACING);
  myMeshName = new QLabel(nameBox);
  nameBoxLayout->addWidget(myMeshName);

  // Mesh Info

  myBriefInfo = new SMESHGUI_MeshInfosBox(false, aFrame);
  myFullInfo  = new SMESHGUI_MeshInfosBox(true,  aFrame);

  // Computation errors

  myCompErrorGroup = new QGroupBox(tr("ERRORS"), aFrame);
  myWarningLabel = new QLabel(QString("<b>%1</b>").arg(tr("COMPUTE_WARNING")), myCompErrorGroup);
  myTable        = new QTableWidget( 1, NB_COLUMNS, myCompErrorGroup);
  myShowBtn      = new QPushButton(tr("SHOW_SHAPE"), myCompErrorGroup);
  myPublishBtn   = new QPushButton(tr("PUBLISH_SHAPE"), myCompErrorGroup);
  myBadMeshBtn   = new QPushButton(tr("SHOW_BAD_MESH"), myCompErrorGroup);
  myBadMeshToGroupBtn = new QPushButton(tr("GROUP_OF_BAD_MESH"), myCompErrorGroup);

  //myTable->setReadOnly( true ); // VSR: check
  myTable->setEditTriggers( QAbstractItemView::NoEditTriggers );
  myTable->hideColumn( COL_PUBLISHED );
  myTable->hideColumn( COL_SHAPEID );
  myTable->hideColumn( COL_BAD_MESH );
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  myTable->horizontalHeader()->setResizeMode( COL_ERROR, QHeaderView::Interactive );
#else
  myTable->horizontalHeader()->setSectionResizeMode( COL_ERROR, QHeaderView::Interactive );
#endif
  myTable->setWordWrap( true );
  myTable->horizontalHeader()->setStretchLastSection( true );
  myTable->setMinimumWidth( 500 );

  QStringList headers;
  headers << tr( "COL_ALGO_HEADER" );
  headers << tr( "COL_SHAPE_HEADER" );
  headers << tr( "COL_ERROR_HEADER" );
  headers << tr( "COL_SHAPEID_HEADER" );
  headers << tr( "COL_PUBLISHED_HEADER" );

  myTable->setHorizontalHeaderLabels( headers );

  // layouting
  QGridLayout* grpLayout = new QGridLayout(myCompErrorGroup);
  grpLayout->setSpacing(SPACING);
  grpLayout->setMargin(MARGIN);
  grpLayout->addWidget( myWarningLabel,      0, 0, 1, 4 );
  grpLayout->addWidget( myTable,             1, 0, 1, 4 );
  grpLayout->addWidget( myShowBtn,           2, 0 );
  grpLayout->addWidget( myPublishBtn,        2, 1 );
  grpLayout->addWidget( myBadMeshBtn,        2, 2 );
  grpLayout->addWidget( myBadMeshToGroupBtn, 2, 3 );
  grpLayout->setColumnStretch( 3, 1 );

  // Hypothesis definition errors

  myHypErrorGroup = new QGroupBox(tr("SMESH_WRN_MISSING_PARAMETERS"), aFrame);
  QHBoxLayout* myHypErrorGroupLayout = new QHBoxLayout(myHypErrorGroup);
  myHypErrorGroupLayout->setMargin(MARGIN);
  myHypErrorGroupLayout->setSpacing(SPACING);
  myHypErrorLabel = new QLabel(myHypErrorGroup);
  myHypErrorGroupLayout->addWidget(myHypErrorLabel);

  // Memory Lack Label

  myMemoryLackGroup = new QGroupBox(tr("ERRORS"), aFrame);
  QVBoxLayout* myMemoryLackGroupLayout = new QVBoxLayout(myMemoryLackGroup);
  myMemoryLackGroupLayout->setMargin(MARGIN);
  myMemoryLackGroupLayout->setSpacing(SPACING);
  QLabel* memLackLabel = new QLabel(tr("MEMORY_LACK"), myMemoryLackGroup);
  QFont bold = memLackLabel->font(); bold.setBold(true);
  memLackLabel->setFont( bold );
  memLackLabel->setMinimumWidth(300);
  myMemoryLackGroupLayout->addWidget(memLackLabel);

  // add all widgets to aFrame
  QVBoxLayout* aLay = new QVBoxLayout(aFrame);
  aLay->setMargin( MARGIN );
  aLay->setSpacing( SPACING );
  aLay->addWidget( aPixGrp );
  aLay->addWidget( nameBox );
  aLay->addWidget( myBriefInfo );
  aLay->addWidget( myFullInfo );
  aLay->addWidget( myHypErrorGroup );
  aLay->addWidget( myCompErrorGroup );
  aLay->addWidget( myMemoryLackGroup );
  aLay->setStretchFactor( myCompErrorGroup, 1 );

  ((QPushButton*) button( OK ))->setDefault( true );

  return aFrame;
}

//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_BaseComputeOp::SMESHGUI_BaseComputeOp()
  : SMESHGUI_Operation(), myCompDlg( 0 )
{
  myTShapeDisplayer = new SMESH::TShapeDisplayer();
  myBadMeshDisplayer = 0;

  //myHelpFileName = "/files/about_meshes.htm"; // V3
  myHelpFileName = "about_meshes_page.html"; // V4
}

SMESH::SMESH_Mesh_ptr SMESHGUI_BaseComputeOp::getMesh()
{
  LightApp_SelectionMgr* Sel = selectionMgr();
  SALOME_ListIO selected; Sel->selectedObjects( selected );
  Handle(SALOME_InteractiveObject) anIO = selected.First();
  SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(anIO);
  return myMesh->_is_nil() ? aMesh._retn() : SMESH::SMESH_Mesh::_duplicate( myMesh );
}

//================================================================================
/*!
 * \brief Start operation
 * \purpose Init dialog fields, connect signals and slots, show dialog
 */
//================================================================================

void SMESHGUI_BaseComputeOp::startOperation()
{
  // create compute dialog if not created before
  computeDlg();

  myMesh      = SMESH::SMESH_Mesh::_nil();
  myMainShape = GEOM::GEOM_Object::_nil();
  myCurShape  = GEOM::GEOM_Object::_nil();

  // check selection
  LightApp_SelectionMgr *Sel = selectionMgr();
  SALOME_ListIO selected; Sel->selectedObjects( selected );

  int nbSel = selected.Extent();
  if (nbSel != 1) {
    SUIT_MessageBox::warning(desktop(),
                             tr("SMESH_WRN_WARNING"),
                             tr("SMESH_WRN_NO_AVAILABLE_DATA"));
    onCancel();
    return;
  }

  myIObject = selected.First();
  CORBA::Object_var anObj = SMESH::IObjectToObject( myIObject );

  myMesh = SMESH::SMESH_Mesh::_narrow(anObj);
  if ( myMesh->_is_nil() )
  {
    SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow(anObj);
    if ( !aSubMesh->_is_nil() )
    {
      myMesh      = aSubMesh->GetFather();
      myCurShape  = aSubMesh->GetSubShape();
    }
  }
  else
  {
    myCurShape = myMesh->GetShapeToMesh();
  }

  if (myMesh->_is_nil()) {
    SUIT_MessageBox::warning(desktop(),
                             tr("SMESH_WRN_WARNING"),
                             tr("SMESH_WRN_NO_AVAILABLE_DATA"));
    onCancel();
    return;
  }

  myMainShape = myMesh->GetShapeToMesh();

  SMESHGUI_Operation::startOperation();
}

//================================================================================
//================================================================================

SMESHGUI_ComputeDlg_QThread::SMESHGUI_ComputeDlg_QThread(SMESH::SMESH_Gen_var gen,
                                                         SMESH::SMESH_Mesh_var mesh,
                                                         GEOM::GEOM_Object_var mainShape)
{
  myResult = false;
  myGen = gen;
  myMesh = mesh;
  myMainShape = mainShape;
}

void SMESHGUI_ComputeDlg_QThread::run()
{
  myResult = myGen->Compute(myMesh, myMainShape);
}

bool SMESHGUI_ComputeDlg_QThread::result()
{
  return myResult;
}

void SMESHGUI_ComputeDlg_QThread::cancel()
{
  myGen->CancelCompute(myMesh, myMainShape);
}

//================================================================================
//================================================================================

SMESHGUI_ComputeDlg_QThreadQDialog::
SMESHGUI_ComputeDlg_QThreadQDialog(QWidget             * parent,
                                   SMESH::SMESH_Gen_var  gen,
                                   SMESH::SMESH_Mesh_var mesh,
                                   GEOM::GEOM_Object_var mainShape)
  : QDialog(parent,
            Qt::WindowSystemMenuHint |
            Qt::WindowCloseButtonHint |
            Qt::Dialog |
            Qt::WindowMaximizeButtonHint),
    qthread(gen, mesh, mainShape)
{
  // --
  setWindowTitle(tr("TITLE"));
  setMinimumWidth( 200 );

  cancelButton = new QPushButton(tr("CANCEL"));
  cancelButton->setDefault(true);
  cancelButton->setCheckable(true);

  QLabel * nbNodesName = new QLabel(tr("SMESH_MESHINFO_NODES"), this );
  QLabel * nbElemsName = new QLabel(tr("SMESH_MESHINFO_ELEMENTS"), this );
  nbNodesLabel = new QLabel("0", this );
  nbElemsLabel = new QLabel("0", this );
#ifndef WIN32
  QLabel * freeRAMName = new QLabel(tr("SMESH_FREERAM"), this );
  freeRAMLabel = new QLabel("", this );
#endif
  progressBar  = new QProgressBar(this);
  progressBar->setMinimum( 0 );
  progressBar->setMaximum( 1000 );

  QGridLayout* layout = new QGridLayout(this);
  layout->setMargin( MARGIN );
  layout->setSpacing( SPACING );
  int row = 0;
  layout->addWidget(nbNodesName,  row,   0);
  layout->addWidget(nbNodesLabel, row++, 1);
  layout->addWidget(nbElemsName,  row,   0);
  layout->addWidget(nbElemsLabel, row++, 1);
#ifndef WIN32
  layout->addWidget(freeRAMName,  row,   0);
  layout->addWidget(freeRAMLabel, row++, 1);
#endif
  layout->addWidget(progressBar,  row++, 0, 1, 2);
  layout->addWidget(cancelButton, row++, 0, 1, 2);
  adjustSize();
  update();

  connect(cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));
  // --
  startTimer(300); // millisecs
  qthread.start();
}

bool SMESHGUI_ComputeDlg_QThreadQDialog::result()
{
  return qthread.result();
}

void SMESHGUI_ComputeDlg_QThreadQDialog::onCancel()
{
  qthread.cancel();
  cancelButton->setText( tr("CANCELING"));
  cancelButton->setEnabled(false);
}

void SMESHGUI_ComputeDlg_QThreadQDialog::timerEvent(QTimerEvent *event)
{
  if ( !cancelButton->isChecked() ) // not yet cancelled
    progressBar->setValue( progressBar->maximum() * qthread.getMesh()->GetComputeProgress() );

  if(qthread.isFinished())
  {
    close();
  }
  else
  {
    nbNodesLabel->setText( QString("%1").arg( qthread.getMesh()->NbNodes() ));
    nbElemsLabel->setText( QString("%1").arg( qthread.getMesh()->NbElements() ));
#ifndef WIN32
    struct sysinfo si;
    const int err = sysinfo( &si );
    if ( err )
      freeRAMLabel->setText("");
    else
      freeRAMLabel->setText( tr("SMESH_GIGABYTE").arg
                             ( si.freeram * si.mem_unit /1024./1024./1024., 0, 'f', 2 ));
#endif
  }
  event->accept();
}

void SMESHGUI_ComputeDlg_QThreadQDialog::closeEvent(QCloseEvent *event)
{
  if(qthread.isRunning())
  {
      event->ignore();
      return;
    }
  event->accept();
}

//================================================================================
/*!
 * \brief computeMesh()
*/
//================================================================================

void SMESHGUI_BaseComputeOp::computeMesh()
{
  // COMPUTE MESH

  SMESH::MemoryReserve aMemoryReserve;

  SMESH::compute_error_array_var aCompErrors;
  QString                        aHypErrors;

  bool computeFailed = true, memoryLack = false;

  _PTR(SObject) aMeshSObj = SMESH::FindSObject(myMesh);
  if ( !aMeshSObj ) // IPAL 21340
    return;
  bool hasShape = myMesh->HasShapeToMesh();
  bool shapeOK = myMainShape->_is_nil() ? !hasShape : hasShape;
  if ( shapeOK )
  {
    myCompDlg->myMeshName->setText( aMeshSObj->GetName().c_str() );
    SMESH::SMESH_Gen_var gen = getSMESHGUI()->GetSMESHGen();
    SMESH::algo_error_array_var errors = gen->GetAlgoState(myMesh,myMainShape);
    if ( errors->length() > 0 ) {
      aHypErrors = SMESH::GetMessageOnAlgoStateErrors( errors.in() );
    }
    if ( myMesh->HasModificationsToDiscard() && // issue 0020693
         SUIT_MessageBox::question( desktop(), tr( "SMESH_WARNING" ),
                                    tr( "FULL_RECOMPUTE_QUESTION" ),
                                    tr( "SMESH_BUT_YES" ), tr( "SMESH_BUT_NO" ), 1, 0 ) == 0 )
      myMesh->Clear();
    SUIT_OverrideCursor aWaitCursor;
    try {
      OCC_CATCH_SIGNALS;
      SMESHGUI_ComputeDlg_QThreadQDialog qthreaddialog(desktop(), gen, myMesh, myCurShape);
      qthreaddialog.exec();
      computeFailed = !qthreaddialog.result();
    }
    catch(const SALOME::SALOME_Exception & S_ex) {
      memoryLack = true;
    }
    try {
      OCC_CATCH_SIGNALS;
      aCompErrors = gen->GetComputeErrors( myMesh, myMainShape );
      // check if there are memory problems
      for ( CORBA::ULong i = 0; (i < aCompErrors->length()) && !memoryLack; ++i )
        memoryLack = ( aCompErrors[ i ].code == SMESH::COMPERR_MEMORY_PB );
    }
    catch(const SALOME::SALOME_Exception & S_ex) {
      memoryLack = true;
    }

    if ( !memoryLack && !SMDS_Mesh::CheckMemory(true) ) { // has memory to show dialog boxes?
      memoryLack = true;
    }

    // NPAL16631: if ( !memoryLack )
    {
      _PTR(SObject) sobj = SMESH::GetActiveStudyDocument()->FindObjectID(myIObject->getEntry());
      SMESH::ModifiedMesh( sobj,
                           !computeFailed && aHypErrors.isEmpty(),
                           myMesh->NbNodes() == 0);
      update( UF_ObjBrowser | UF_Model );

      // SHOW MESH
      // NPAL16631: if ( getSMESHGUI()->automaticUpdate() )
      SUIT_ResourceMgr* resMgr = SMESH::GetResourceMgr( SMESHGUI::GetSMESHGUI() );
      bool limitExceeded;
      long limitSize = resMgr->integerValue( "SMESH", "update_limit", 500000 );
      int entities = SMESH_Actor::eAllEntity;
      int hidden = 0;
      long nbElements = 0;
      if ( !memoryLack )
      {
        // List of objects that will be updated automatically
        typedef QList< QPair< SMESH::SMESH_IDSource_var, _PTR(SObject) > > TListOf_IDSrc_SObj;
        TListOf_IDSrc_SObj aListToUpdate;
        SMESH::SMESH_IDSource_var aMeshObj =
          SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( aMeshSObj );
        // put Mesh into list
        aListToUpdate.append( TListOf_IDSrc_SObj::value_type( aMeshObj, aMeshSObj ));
        SMESH::submesh_array_var aSubMeshes = myMesh->GetSubMeshes();
        // put SubMeshes into list
        for ( CORBA::ULong i = 0; i < aSubMeshes->length(); i++ )
        {
          SMESH::SMESH_subMesh_var sm = aSubMeshes[i];
          if ( CORBA::is_nil( sm ) ) continue;
          _PTR(SObject) smSObj = SMESH::ObjectToSObject( sm );
          if ( !smSObj ) continue;
          SMESH::SMESH_IDSource_var aSubMeshObj =
            SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( smSObj );
          SMESH_Actor *anActor = SMESH::FindActorByObject( aSubMeshObj );
          if ( anActor && anActor->GetVisibility() )
            aListToUpdate.append( TListOf_IDSrc_SObj::value_type( aSubMeshObj, smSObj ));
        }
        // put Groups into list
        SMESH::ListOfGroups_var aGroups = myMesh->GetGroups();
        for ( size_t i = 0; i < aGroups->length(); ++i )
        {
          SMESH::SMESH_GroupBase_var aGrp = aGroups[i];
          if ( CORBA::is_nil( aGrp ) ) continue;
          SMESH::SMESH_Group_var  aStdGroup  = SMESH::SMESH_Group::_narrow( aGrp );
          if ( !aStdGroup->_is_nil() ) continue; // don't update standalone groups
          _PTR(SObject) aGroupSO = SMESH::FindSObject( aGrp );
          if ( !aGroupSO ) continue;
          SMESH::SMESH_IDSource_var aGroupObj =
            SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( aGroupSO );
          SMESH_Actor *anActor = SMESH::FindActorByObject( aGroupObj );
          if ( anActor && anActor->GetVisibility() )
            aListToUpdate.append( TListOf_IDSrc_SObj::value_type( aGroupObj, aGroupSO ));
        }

        // update mesh, sub-mesh and groups, if it's possible
        TListOf_IDSrc_SObj::iterator anIter;
        for ( anIter = aListToUpdate.begin(); anIter != aListToUpdate.end(); anIter++ )
        {
          SMESH::SMESH_Mesh_var aMesh =
            SMESH::SMESH_Mesh::_narrow( SMESH::SObjectToObject( (*anIter).second ));

          if ( getSMESHGUI()->automaticUpdate( (*anIter).first, &entities, &limitExceeded,
                                               &hidden, &nbElements ) )
          {
            try {
              OCC_CATCH_SIGNALS;
              bool toDisplay = false;
              if ( !aMesh->_is_nil() ) // display only a mesh
              {
                toDisplay = true;
                SMESH_Actor *anActor = SMESH::FindActorByObject( aMesh );
                if ( !anActor ) anActor = SMESH::CreateActor( (*anIter).second->GetStudy(),
                                                              (*anIter).second->GetID().c_str(),
                                                              /*clearLog =*/ true );
                if ( anActor ) // actor is not created for an empty mesh
                {
                  anActor->SetEntityMode( entities );
                  SMESH::DisplayActor( SMESH::GetActiveWindow(), anActor );
                }
              }
              Handle(SALOME_InteractiveObject) anIO = new SALOME_InteractiveObject
                ( (*anIter).second->GetID().c_str(), "SMESH", (*anIter).second->GetName().c_str() );
              SMESH::Update(anIO, toDisplay);

              if ( limitExceeded && !aMesh->_is_nil() )
              {
                QStringList hiddenMsg;
                if ( hidden & SMESH_Actor::e0DElements ) hiddenMsg << tr( "SMESH_ELEMS0D" );
                if ( hidden & SMESH_Actor::eEdges )      hiddenMsg << tr( "SMESH_EDGES" );
                if ( hidden & SMESH_Actor::eFaces )      hiddenMsg << tr( "SMESH_FACES" );
                if ( hidden & SMESH_Actor::eVolumes )    hiddenMsg << tr( "SMESH_VOLUMES" );
                if ( hidden & SMESH_Actor::eBallElem )   hiddenMsg << tr( "SMESH_BALLS" );
                SUIT_MessageBox::warning( desktop(),
                                          tr( "SMESH_WRN_WARNING" ),
                                          tr( "SMESH_WRN_SIZE_INC_LIMIT_EXCEEDED" ).
                                          arg( nbElements ).
                                          arg( limitSize ).
                                          arg( hiddenMsg.join(", ")));
              }
            }
            catch (...) {
#ifdef _DEBUG_
              MESSAGE ( "Exception thrown during mesh visualization" );
#endif
              if ( SMDS_Mesh::CheckMemory(true) ) { // has memory to show warning?
                SMESH::OnVisuException();
              }
              else {
                memoryLack = true;
              }
            }
          }
          else if ( limitExceeded && !aMesh->_is_nil() )
          {
            SUIT_MessageBox::warning( desktop(),
                                      tr( "SMESH_WRN_WARNING" ),
                                      tr( "SMESH_WRN_SIZE_LIMIT_EXCEEDED" ).
                                      arg( nbElements ).arg( limitSize ) );
          }
        }
      }
      if ( LightApp_SelectionMgr *Sel = selectionMgr() )
      {
        SALOME_ListIO selected;
        selected.Append( myIObject );
        Sel->setSelectedObjects( selected );
      }
    }
  }

  if ( memoryLack )
    aMemoryReserve.release();

  myCompDlg->setWindowTitle
    ( tr( computeFailed ? "SMESH_WRN_COMPUTE_FAILED" : "SMESH_COMPUTE_SUCCEED" ));

  // SHOW ERRORS

  bool noCompError = ( !aCompErrors.operator->() || aCompErrors->length() == 0 );
  bool noHypoError = ( aHypErrors.isEmpty() );

  SUIT_ResourceMgr* resMgr = SMESH::GetResourceMgr( SMESHGUI::GetSMESHGUI() );
  int aNotifyMode = resMgr->integerValue( "SMESH", "show_result_notification" );

  bool isShowResultDlg = true;
  switch( aNotifyMode ) {
  case 0: // show the mesh computation result dialog NEVER
    isShowResultDlg = false;
    commit();
    break;
  case 1: // show the mesh computation result dialog if there are some errors
    if ( memoryLack || !noCompError || !noHypoError )
      isShowResultDlg = true;
    else
    {
      isShowResultDlg = false;
      commit();
    }
    break;
  default: // show the result dialog after each mesh computation
    isShowResultDlg = true;
  }

  // SHOW RESULTS
  if ( isShowResultDlg )
    showComputeResult( memoryLack, noCompError,aCompErrors, noHypoError, aHypErrors );
}

void SMESHGUI_BaseComputeOp::showComputeResult( const bool theMemoryLack,
                                                const bool theNoCompError,
                                                SMESH::compute_error_array_var& theCompErrors,
                                                const bool theNoHypoError,
                                                const QString& theHypErrors )
{
  bool hasShape = myMesh->HasShapeToMesh();
  SMESHGUI_ComputeDlg* aCompDlg = computeDlg();
  aCompDlg->myMemoryLackGroup->hide();

  if ( theMemoryLack )
  {
    aCompDlg->myMemoryLackGroup->show();
    aCompDlg->myFullInfo->hide();
    aCompDlg->myBriefInfo->hide();
    aCompDlg->myHypErrorGroup->hide();
    aCompDlg->myCompErrorGroup->hide();
  }
  else if ( theNoCompError && theNoHypoError )
  {
    SMESH::long_array_var aRes = myMesh->GetMeshInfo();
    aCompDlg->myFullInfo->SetMeshInfo( aRes );
    aCompDlg->myFullInfo->show();
    aCompDlg->myBriefInfo->hide();
    aCompDlg->myHypErrorGroup->hide();
    aCompDlg->myCompErrorGroup->hide();
  }
  else
  {
    bool onlyWarnings = !theNoCompError; // == valid mesh computed but there are errors reported
    for ( CORBA::ULong i = 0; i < theCompErrors->length() && onlyWarnings; ++i )
      onlyWarnings = ( theCompErrors[ i ].code == SMESH::COMPERR_WARNING ||
                       theCompErrors[ i ].code == SMESH::COMPERR_NO_MESH_ON_SHAPE );

    // full or brief mesh info
    SMESH::long_array_var aRes = myMesh->GetMeshInfo();
    if ( onlyWarnings ) {
      aCompDlg->myFullInfo->SetMeshInfo( aRes );
      aCompDlg->myFullInfo->show();
      aCompDlg->myBriefInfo->hide();
    } else {
      aCompDlg->myBriefInfo->SetMeshInfo( aRes );
      aCompDlg->myBriefInfo->show();
      aCompDlg->myFullInfo->hide();
    }

    // pbs of hypo dfinitions
    if ( theNoHypoError ) {
      aCompDlg->myHypErrorGroup->hide();
    } else {
      aCompDlg->myHypErrorGroup->show();
      aCompDlg->myHypErrorLabel->setText( theHypErrors );
    }

    // table of errors
    if ( theNoCompError )
    {
      aCompDlg->myCompErrorGroup->hide();
    }
    else
    {
      aCompDlg->myCompErrorGroup->show();

      if ( onlyWarnings )
        aCompDlg->myWarningLabel->show();
      else
        aCompDlg->myWarningLabel->hide();

      if ( !hasShape ) {
        aCompDlg->myPublishBtn->hide();
        aCompDlg->myShowBtn->hide();
      }
      else {
        aCompDlg->myPublishBtn->show();
        aCompDlg->myShowBtn->show();
      }

      // fill table of errors
      QTableWidget* tbl = aCompDlg->myTable;
      tbl->setRowCount( theCompErrors->length() );
      if ( !hasShape ) tbl->hideColumn( COL_SHAPE );
      else             tbl->showColumn( COL_SHAPE );
      tbl->setColumnWidth( COL_ERROR, 200 );

      bool hasBadMesh = false;
      for ( int row = 0; row < (int) theCompErrors->length(); ++row )
      {
        SMESH::ComputeError & err = theCompErrors[ row ];

        QString text = err.algoName.in();
        if ( !tbl->item( row, COL_ALGO ) ) tbl->setItem( row, COL_ALGO, new QTableWidgetItem( text ) );
        else tbl->item( row, COL_ALGO )->setText( text );

        text = SMESH::errorText( err.code, err.comment.in() );
        if ( !tbl->item( row, COL_ERROR ) ) tbl->setItem( row, COL_ERROR, new QTableWidgetItem( text ) );
        else tbl->item( row, COL_ERROR )->setText( text );

        text = QString("%1").arg( err.subShapeID );
        if ( !tbl->item( row, COL_SHAPEID ) ) tbl->setItem( row, COL_SHAPEID, new QTableWidgetItem( text ) );
        else tbl->item( row, COL_SHAPEID )->setText( text );

        text = hasShape ? SMESH::shapeText( err.subShapeID, myMainShape ) : QString("");
        if ( !tbl->item( row, COL_SHAPE ) ) tbl->setItem( row, COL_SHAPE, new QTableWidgetItem( text ) );
        else tbl->item( row, COL_SHAPE )->setText( text );

        text = ( !hasShape || SMESH::getSubShapeSO( err.subShapeID, myMainShape )) ? "PUBLISHED" : "";
        if ( !tbl->item( row, COL_PUBLISHED ) ) tbl->setItem( row, COL_PUBLISHED, new QTableWidgetItem( text ) );
        else tbl->item( row, COL_PUBLISHED )->setText( text ); // if text=="", "PUBLISH" button enabled

        text = err.hasBadMesh ? "hasBadMesh" : "";
        if ( !tbl->item( row, COL_BAD_MESH ) ) tbl->setItem( row, COL_BAD_MESH, new QTableWidgetItem( text ) );
        else tbl->item( row, COL_BAD_MESH )->setText( text );
        if ( err.hasBadMesh ) hasBadMesh = true;

        //tbl->item( row, COL_ERROR )->setWordWrap( true ); // VSR: TODO ???
        tbl->resizeRowToContents( row );
      }
      tbl->resizeColumnToContents( COL_ALGO );
      tbl->resizeColumnToContents( COL_SHAPE );
      tbl->setWordWrap( true );

      if ( hasBadMesh ) {
        aCompDlg->myBadMeshBtn->show();
        aCompDlg->myBadMeshToGroupBtn->show();
      }
      else {
        aCompDlg->myBadMeshBtn->hide();
        aCompDlg->myBadMeshToGroupBtn->hide();
      }
      tbl->setCurrentCell(0,0);
      currentCellChanged(); // to update buttons
    }
  }
  // show dialog and wait, becase Compute can be invoked from Preview operation
  //aCompDlg->exec(); // this way it becomes modal - impossible to rotate model in the Viewer
  aCompDlg->show();
}

//================================================================================
/*!
 * \brief Stops operation
 */
//================================================================================

void SMESHGUI_BaseComputeOp::stopOperation()
{
  SMESHGUI_Operation::stopOperation();
  if ( myTShapeDisplayer )
    myTShapeDisplayer->SetVisibility( false );
  if ( myBadMeshDisplayer ) {
    myBadMeshDisplayer->SetVisibility( false );
    // delete it in order not to have problems at its destruction when the viewer
    // where it worked is dead due to e.g. study closing
    delete myBadMeshDisplayer;
    myBadMeshDisplayer = 0;
  }
  myIObject.Nullify();
}

//================================================================================
/*!
 * \brief publish selected sub-shape
 */
//================================================================================

void SMESHGUI_BaseComputeOp::onPublishShape()
{
  GEOM::GEOM_Gen_var      geomGen = SMESH::GetGEOMGen();
  SALOMEDS::Study_var       study = SMESHGUI::GetSMESHGen()->GetCurrentStudy();
  GEOM::GEOM_Object_var meshShape = myMesh->GetShapeToMesh();

  QStringList entryList;
  QList<int> rows;
  SMESH::getSelectedRows( table(), rows );
  int row;
  foreach ( row, rows )
  {
    int curSub = table()->item(row, COL_SHAPEID)->text().toInt();
    GEOM::GEOM_Object_wrap shape = SMESH::getSubShape( curSub, myMainShape );
    if ( !shape->_is_nil() && ! SMESH::getSubShapeSO( curSub, myMainShape ))
    {
      if ( !SMESH::getSubShapeSO( 1, myMainShape )) // the main shape not published
      {
        QString name = GEOMBase::GetDefaultName( SMESH::shapeTypeName( myMainShape, "MAIN_SHAPE" ));
        SALOMEDS::SObject_wrap so = geomGen->AddInStudy( study, myMainShape,
                                                         name.toLatin1().data(),
                                                         GEOM::GEOM_Object::_nil());
        // look for myMainShape in the table
        for ( int r = 0, nr = table()->rowCount(); r < nr; ++r ) {
          if ( table()->item( r, COL_SHAPEID )->text() == "1" ) {
            if ( so->_is_nil() ) {
              CORBA::String_var name  = so->GetName();
              CORBA::String_var entry = so->GetID();
              QString       shapeText = QString("%1 (%2)").arg( name.in() ).arg( entry.in() );
              table()->item( r, COL_SHAPE     )->setText( shapeText );
              table()->item( r, COL_PUBLISHED )->setText( entry.in() );
            }
            break;
          }
        }
        if ( curSub == 1 ) continue;
      }
      QString name = GEOMBase::GetDefaultName( SMESH::shapeTypeName( shape, "ERROR_SHAPE" ));
      SALOMEDS::SObject_wrap so = geomGen->AddInStudy( study, shape,
                                                       name.toLatin1().data(), myMainShape);
      if ( !so->_is_nil() ) {
        CORBA::String_var name  = so->GetName();
        CORBA::String_var entry = so->GetID();
        QString       shapeText = QString("%1 (%2)").arg( name.in() ).arg( entry.in() );
        table()->item( row, COL_SHAPE     )->setText( shapeText );
        table()->item( row, COL_PUBLISHED )->setText( entry.in() );
        entryList.push_back( entry.in() );
      }
    }
  }
  getSMESHGUI()->getApp()->updateObjectBrowser();
  getSMESHGUI()->getApp()->browseObjects( entryList, /*isApplyAndClose=*/true );

  currentCellChanged(); // to update buttons
}

//================================================================================
/*!
 * \brief show mesh elements preventing computation of a submesh of current row
 */
//================================================================================

void SMESHGUI_BaseComputeOp::onShowBadMesh()
{
  myTShapeDisplayer->SetVisibility( false );
  QList<int> rows;
  if ( SMESH::getSelectedRows( table(), rows ) == 1 ) {
    bool hasBadMesh = ( !table()->item(rows.front(), COL_BAD_MESH)->text().isEmpty() );
    if ( hasBadMesh ) {
      int curSub = table()->item(rows.front(), COL_SHAPEID)->text().toInt();
      SMESHGUI* gui = getSMESHGUI();
      SMESH::SMESH_Gen_var gen = gui->GetSMESHGen();
      SVTK_ViewWindow*    view = SMESH::GetViewWindow( gui );
      if ( myBadMeshDisplayer ) delete myBadMeshDisplayer;
      myBadMeshDisplayer = new SMESHGUI_MeshEditPreview( view );
      SMESH::MeshPreviewStruct_var aMeshData = gen->GetBadInputElements(myMesh,curSub);
      double aPointSize = SMESH::GetFloat("SMESH:node_size",3);
      double aLineWidth = SMESH::GetFloat("SMESH:element_width",1);
      vtkProperty* prop = vtkProperty::New();
      prop->SetLineWidth( aLineWidth * 3 );
      prop->SetPointSize( aPointSize * 3 );
      prop->SetColor( 250, 0, 250 );
      myBadMeshDisplayer->GetActor()->SetProperty( prop );
      myBadMeshDisplayer->SetData( aMeshData._retn() );
      prop->Delete();
    }
  }
}

//================================================================================
/*!
 * \brief create groups of bad mesh elements preventing computation of a submesh of current row
 */
//================================================================================

void SMESHGUI_BaseComputeOp::onGroupOfBadMesh()
{
  QList<int> rows;
  SMESH::getSelectedRows( table(), rows );
  int row;
  foreach ( row, rows )
  {
    bool hasBadMesh = ( !table()->item(row, COL_BAD_MESH)->text().isEmpty() );
    if ( hasBadMesh ) {
      int     curSub = table()->item(rows.front(), COL_SHAPEID)->text().toInt();
      QString grName = table()->item(rows.front(), COL_SHAPE)->text();
      if ( grName.isEmpty() ) grName = "bad mesh";
      else                    grName = "bad mesh of " + grName;
      SMESH::SMESH_Gen_var gen = getSMESHGUI()->GetSMESHGen();
      SMESH::ListOfGroups_var groups
        ( gen->MakeGroupsOfBadInputElements(myMesh,curSub,grName.toLatin1().data()) );
      update( UF_ObjBrowser | UF_Model );
      if( LightApp_Application* anApp = dynamic_cast<LightApp_Application*>( application() ))
      {
        QStringList anEntryList;
        for ( size_t i = 0; i < groups->length(); ++i )
          if ( _PTR(SObject) so = SMESH::FindSObject( groups[i] ))
            anEntryList.append( so->GetID().c_str() );

        if ( !anEntryList.isEmpty())
          anApp->browseObjects( anEntryList, true, false );
      }
    }
  }
}

//================================================================================
/*!
 * \brief SLOT called when a selected cell in table() changed
 */
//================================================================================

void SMESHGUI_BaseComputeOp::currentCellChanged()
{
  myTShapeDisplayer->SetVisibility( false );
  if ( myBadMeshDisplayer )
    myBadMeshDisplayer->SetVisibility( false );

  bool publishEnable = 0, showEnable = 0, showOnly = 1, hasBadMesh = 0;
  QList<int> rows;
  int nbSelected = SMESH::getSelectedRows( table(), rows );
  int row;
  foreach ( row, rows )
  {
    bool hasData     = ( !table()->item( row, COL_SHAPE )->text().isEmpty() );
    bool isPublished = ( !table()->item( row, COL_PUBLISHED )->text().isEmpty() );
    if ( hasData && !isPublished )
      publishEnable = true;

    int curSub = table()->item( row, COL_SHAPEID )->text().toInt();
    bool prsReady = myTShapeDisplayer->HasReadyActorsFor( curSub, myMainShape );
    if ( prsReady ) {
      myTShapeDisplayer->Show( curSub, myMainShape, showOnly );
      showOnly = false;
    }
    else {
      showEnable = true;
    }

    if ( !table()->item(row, COL_BAD_MESH)->text().isEmpty() )
      hasBadMesh = true;
  }
  myCompDlg->myPublishBtn->setEnabled( publishEnable );
  myCompDlg->myShowBtn   ->setEnabled( showEnable );
  myCompDlg->myBadMeshBtn->setEnabled( hasBadMesh && ( nbSelected == 1 ));
  myCompDlg->myBadMeshToGroupBtn->setEnabled( hasBadMesh && ( nbSelected == 1 ));
}

//================================================================================
/*!
 * \brief update preview
 */
//================================================================================

void SMESHGUI_BaseComputeOp::onPreviewShape()
{
  if ( myTShapeDisplayer )
  {
    SUIT_OverrideCursor aWaitCursor;
    QList<int> rows;
    SMESH::getSelectedRows( table(), rows );

    bool showOnly = true;
    int row;
    foreach ( row, rows )
    {
      int curSub = table()->item( row, COL_SHAPEID )->text().toInt();
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

SMESHGUI_BaseComputeOp::~SMESHGUI_BaseComputeOp()
{
  delete myCompDlg;
  myCompDlg = 0;
  delete myTShapeDisplayer;
  if ( myBadMeshDisplayer )
    delete myBadMeshDisplayer;
}

//================================================================================
/*!
 * \brief Gets dialog of compute operation
 * \retval SMESHGUI_ComputeDlg* - pointer to dialog of this operation
 */
//================================================================================

SMESHGUI_ComputeDlg* SMESHGUI_BaseComputeOp::computeDlg() const
{
  if ( !myCompDlg )
  {
    SMESHGUI_BaseComputeOp* me = (SMESHGUI_BaseComputeOp*)this;
    me->myCompDlg = new SMESHGUI_ComputeDlg( desktop(), false );
    // connect signals and slots
    connect(myCompDlg->myShowBtn,           SIGNAL (clicked()), SLOT(onPreviewShape()));
    connect(myCompDlg->myPublishBtn,        SIGNAL (clicked()), SLOT(onPublishShape()));
    connect(myCompDlg->myBadMeshBtn,        SIGNAL (clicked()), SLOT(onShowBadMesh()));
    connect(myCompDlg->myBadMeshToGroupBtn, SIGNAL (clicked()), SLOT(onGroupOfBadMesh()));

    QTableWidget* aTable = me->table();
    connect(aTable, SIGNAL(itemSelectionChanged()), SLOT(currentCellChanged()));
    connect(aTable, SIGNAL(currentCellChanged(int,int,int,int)), SLOT(currentCellChanged()));
  }
  return myCompDlg;
}

//================================================================================
/*!
 * \brief returns from compute mesh result dialog
 */
//================================================================================

bool SMESHGUI_BaseComputeOp::onApply()
{
  return true;
}

//================================================================================
/*!
 * \brief Return a table
 */
//================================================================================

QTableWidget* SMESHGUI_BaseComputeOp::table()
{
  return myCompDlg->myTable;
}


//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_ComputeOp::SMESHGUI_ComputeOp()
 : SMESHGUI_BaseComputeOp()
{
  myHelpFileName = "constructing_meshes_page.html#compute_anchor";
}


//================================================================================
/*!
 * \brief Desctructor
*/
//================================================================================

SMESHGUI_ComputeOp::~SMESHGUI_ComputeOp()
{
}

//================================================================================
/*!
 * \brief perform it's intention action: compute mesh
 */
//================================================================================

void SMESHGUI_ComputeOp::startOperation()
{
  SMESHGUI_BaseComputeOp::startOperation();
  if (myMesh->_is_nil())
    return;
  computeMesh();
}

//================================================================================
/*!
 * \brief check the same operations on the same mesh
 */
//================================================================================

bool SMESHGUI_BaseComputeOp::isValid(  SUIT_Operation* theOp  ) const
{
  SMESHGUI_BaseComputeOp* baseOp = dynamic_cast<SMESHGUI_BaseComputeOp*>( theOp );
  bool ret = true;
  if ( !myMesh->_is_nil() && baseOp ) {
    SMESH::SMESH_Mesh_var aMesh = baseOp->getMesh();
    if ( !aMesh->_is_nil() && aMesh->GetId() == myMesh->GetId() ) ret = false;
  }
  return ret;
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
 * \retval LightApp_Dialog* - pointer to dialog of this operation
 */
//================================================================================

LightApp_Dialog* SMESHGUI_ComputeOp::dlg() const
{
  return computeDlg();
}

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================

SMESHGUI_PrecomputeOp::SMESHGUI_PrecomputeOp()
  : SMESHGUI_BaseComputeOp(),
    myActiveDlg( 0 ),
    myDlg( 0 ),
    myPreviewDisplayer( 0 ),
    myOrderMgr( 0 )
{
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

SMESHGUI_PrecomputeOp::~SMESHGUI_PrecomputeOp()
{
  delete myDlg;
  myDlg = 0;
  delete myOrderMgr;
  myOrderMgr = 0;
  myActiveDlg = 0;
  if ( myPreviewDisplayer )
    delete myPreviewDisplayer;
  myPreviewDisplayer = 0;
}

//================================================================================
/*!
 * \brief Gets current dialog of this operation
 * \retval LightApp_Dialog* - pointer to dialog of this operation
 */
//================================================================================

LightApp_Dialog* SMESHGUI_PrecomputeOp::dlg() const
{
  return myActiveDlg;
}

//================================================================================
/*!
 * \brief perform it's intention action: prepare data
 */
//================================================================================

void SMESHGUI_PrecomputeOp::startOperation()
{
  myHelpFileName = "constructing_meshes_page.html#preview_anchor"; // other anchor onCompute()

  if ( !myDlg )
  {
    myDlg = new SMESHGUI_PrecomputeDlg( desktop() );
    
    // connect signals
    connect( myDlg, SIGNAL( preview() ), this, SLOT( onPreview() ) );
    connect( myDlg, SIGNAL( dlgOk() ), this, SLOT( onCompute() ) );
    connect( myDlg, SIGNAL( dlgApply() ), this, SLOT( onCompute() ) );
  }
  myActiveDlg = myDlg;

  // connect signal to compute dialog. which will be shown after Compute mesh operation
  SMESHGUI_ComputeDlg* cmpDlg = computeDlg();
  if ( cmpDlg )
  {
    // disconnect signals
    disconnect( cmpDlg, SIGNAL( dlgOk() ), this, SLOT( onOk() ) );
    disconnect( cmpDlg, SIGNAL( dlgApply() ), this, SLOT( onApply() ) );
    disconnect( cmpDlg, SIGNAL( dlgCancel() ), this, SLOT( onCancel() ) );
    disconnect( cmpDlg, SIGNAL( dlgClose() ), this, SLOT( onCancel() ) );
    disconnect( cmpDlg, SIGNAL( dlgHelp() ), this, SLOT( onHelp() ) );

    // connect signals
    if( cmpDlg->testButtonFlags( QtxDialog::OK ) )
      connect( cmpDlg, SIGNAL( dlgOk() ), this, SLOT( onOk() ) );
    if( cmpDlg->testButtonFlags( QtxDialog::Apply ) )
      connect( cmpDlg, SIGNAL( dlgApply() ), this, SLOT( onApply() ) );
    if( cmpDlg->testButtonFlags( QtxDialog::Help ) )
      connect( cmpDlg, SIGNAL( dlgHelp() ), this, SLOT( onHelp() ) );
    if( cmpDlg->testButtonFlags( QtxDialog::Cancel ) )
      connect( cmpDlg, SIGNAL( dlgCancel() ), this, SLOT( onCancel() ) );
    if( cmpDlg->testButtonFlags( QtxDialog::Close ) )
      connect( cmpDlg, SIGNAL( dlgClose() ), this, SLOT( onCancel() ) );
  }

  SMESHGUI_BaseComputeOp::startOperation();
  if (myMesh->_is_nil())
    return;

  if (myDlg->getPreviewMode() == -1)
  {
    // nothing to preview
    SUIT_MessageBox::warning(desktop(),
                             tr("SMESH_WRN_WARNING"),
                             tr("SMESH_WRN_NOTHING_PREVIEW"));
    onCancel();
    return;
  }

  // disconnect slot from preview dialog to have Apply from results of compute operation only 
  disconnect( myDlg, SIGNAL( dlgOk() ), this, SLOT( onOk() ) );
  disconnect( myDlg, SIGNAL( dlgApply() ), this, SLOT( onApply() ) );

  myDlg->show();
}

//================================================================================
/*!
 * \brief Stops operation
 */
//================================================================================

void SMESHGUI_PrecomputeOp::stopOperation()
{
  if ( myPreviewDisplayer )
  {
    myPreviewDisplayer->SetVisibility( false );
    delete myPreviewDisplayer;
    myPreviewDisplayer = 0;
  }
  myMapShapeId.clear();
  SMESHGUI_BaseComputeOp::stopOperation();
}

//================================================================================
/*!
 * \brief reinitialize dialog after operaiton become active again
 */
//================================================================================

void SMESHGUI_PrecomputeOp::resumeOperation()
{
  if ( myActiveDlg == myDlg )
    initDialog();
  SMESHGUI_BaseComputeOp::resumeOperation();
}

//================================================================================
/*!
 * \brief perform it's intention action: reinitialise dialog
 */
//================================================================================

void SMESHGUI_PrecomputeOp::initDialog()
{
  QList<int> modes;

  QMap<int, int> modeMap;
  _PTR(SObject)  pMesh = studyDS()->FindObjectID( myIObject->getEntry() );
  getAssignedAlgos( pMesh, modeMap );
  if ( modeMap.contains( SMESH::DIM_3D ) )
  {
    if ( modeMap.contains( SMESH::DIM_2D ) )
      modes.append( SMESH::DIM_2D );
    if ( modeMap.contains( SMESH::DIM_1D ) )
      modes.append( SMESH::DIM_1D );
  }
  else if ( modeMap.contains( SMESH::DIM_2D ) )
  {
    if ( modeMap.contains( SMESH::DIM_1D ) )
      modes.append( SMESH::DIM_1D );
  }

  myOrderMgr = new SMESHGUI_MeshOrderMgr( myDlg->getMeshOrderBox() );
  myOrderMgr->SetMesh( myMesh );
  bool isOrder = myOrderMgr->GetMeshOrder(myPrevOrder);
  myDlg->getMeshOrderBox()->setVisible(isOrder);
  if ( !isOrder ) {
    delete myOrderMgr;
    myOrderMgr = 0;
  }

  myDlg->setPreviewModes( modes );
}

//================================================================================
/*!
 * \brief detect asigned mesh algorithms
 */
//================================================================================

void SMESHGUI_PrecomputeOp::getAssignedAlgos(_PTR(SObject)  theMesh,
                                             QMap<int,int>& theModeMap)
{
  if ( !theMesh ) return;

  _PTR(SObject)          aHypFolder;
  _PTR(GenericAttribute) anAttr;
  int aPart = SMESH::Tag_RefOnAppliedAlgorithms;
  if ( theMesh->FindSubObject( aPart, aHypFolder ))
  {
    _PTR(ChildIterator) anIter =
      SMESH::GetActiveStudyDocument()->NewChildIterator( aHypFolder );
    for ( ; anIter->More(); anIter->Next() )
    {
      _PTR(SObject) anObj = anIter->Value();
      _PTR(SObject) aRefObj;
      if ( anObj->ReferencedObject( aRefObj ) )
        anObj = aRefObj;
      else
        continue;

      if ( anObj->FindAttribute( anAttr, "AttributeName" ) )
      {
        CORBA::Object_var aVar = _CAST(SObject,anObj)->GetObject();
        if ( CORBA::is_nil( aVar ) )
          continue;

        SMESH::SMESH_Algo_var algo;
        for( int dim = SMESH::DIM_1D; dim <= SMESH::DIM_3D; dim++ )
        {
          switch(dim) {
          case SMESH::DIM_1D: algo = SMESH::SMESH_1D_Algo::_narrow( aVar ); break;
          case SMESH::DIM_2D: algo = SMESH::SMESH_2D_Algo::_narrow( aVar ); break;
          case SMESH::DIM_3D: algo = SMESH::SMESH_3D_Algo::_narrow( aVar ); break;
          default: break;
          }
          if ( !algo->_is_nil() )
          {
            theModeMap[ dim ] = 0;
            if ( theModeMap.size() == 3 )
              return;
            break;
          }
        }
      }
    }
  }

  // check sub-meshes
  for ( aPart = SMESH::Tag_SubMeshOnEdge; aPart <= SMESH::Tag_LastSubMesh; ++aPart )
  {
    if ( !theMesh->FindSubObject( aPart, aHypFolder ))
      continue;

    _PTR(ChildIterator) anIter =
      SMESH::GetActiveStudyDocument()->NewChildIterator( aHypFolder );
    for ( anIter->InitEx(true); anIter->More(); anIter->Next() )
    {
      _PTR(SObject) anObj = anIter->Value();
      _PTR(SObject) aRefObj;
      if ( anObj->ReferencedObject( aRefObj ) )
        anObj = aRefObj;
      else
        continue;

      if ( anObj->FindAttribute( anAttr, "AttributeName" ))
      {
        CORBA::Object_var aVar = _CAST(SObject,anObj)->GetObject();
        if ( CORBA::is_nil( aVar ) )
          continue;

        SMESH::SMESH_Algo_var algo;
        for( int dim = SMESH::DIM_1D; dim <= SMESH::DIM_3D; dim++ )
        {
          switch(dim) {
          case SMESH::DIM_1D: algo = SMESH::SMESH_1D_Algo::_narrow( aVar ); break;
          case SMESH::DIM_2D: algo = SMESH::SMESH_2D_Algo::_narrow( aVar ); break;
          case SMESH::DIM_3D: algo = SMESH::SMESH_3D_Algo::_narrow( aVar ); break;
          default: break;
          }
          if ( !algo->_is_nil() )
          {
            theModeMap[ dim ] = 0;
            if ( theModeMap.size() == 3 )
              return;
            break;
          }
        }
      }
    }
  }
}

//================================================================================
/*!
 * \brief perform it's intention action: compute mesh
 */
//================================================================================

void SMESHGUI_PrecomputeOp::onCompute()
{
  myDlg->hide();
  if (myOrderMgr && myOrderMgr->IsOrderChanged())
    myOrderMgr->SetMeshOrder();
  myMapShapeId.clear();
  myActiveDlg = computeDlg();
  myHelpFileName = "constructing_meshes_page.html#compute_anchor";
  computeMesh();
}

//================================================================================
/*!
 * \brief perform it's intention action: compute mesh
 */
//================================================================================

void SMESHGUI_PrecomputeOp::onCancel()
{
  QObject* curDlg = sender();
  if ( curDlg == computeDlg() && myActiveDlg == myDlg )
  {
    // return from error messages
    myDlg->show();
    return;
  }

  bool isRestoreOrder = false;
  if ( myActiveDlg == myDlg  && !myMesh->_is_nil() && myMapShapeId.count() )
  {
    // ask to remove already computed mesh elements
    if ( SUIT_MessageBox::question( desktop(), tr( "SMESH_WARNING" ),
                                    tr( "CLEAR_SUBMESH_QUESTION" ),
                                    tr( "SMESH_BUT_DELETE" ), tr( "SMESH_BUT_NO" ), 0, 1 ) == 0 )
    {
      // remove all submeshes for collected shapes
      QMap<int,int>::const_iterator it = myMapShapeId.constBegin();
      for ( ; it != myMapShapeId.constEnd(); ++it )
        myMesh->ClearSubMesh( it.key() );
      isRestoreOrder = true;
    }
  }

  // return previous mesh order
  if (myOrderMgr && myOrderMgr->IsOrderChanged()) {
    if (!isRestoreOrder)
      isRestoreOrder = 
        (SUIT_MessageBox::question( desktop(), tr( "SMESH_WARNING" ),
                                    tr( "SMESH_REJECT_MESH_ORDER" ),
                                    tr( "SMESH_BUT_YES" ), tr( "SMESH_BUT_NO" ), 0, 1 ) == 0);
    if (isRestoreOrder)
      myOrderMgr->SetMeshOrder(myPrevOrder);
  }

  delete myOrderMgr;
  myOrderMgr = 0;

  myMapShapeId.clear();
  SMESHGUI_BaseComputeOp::onCancel();
}

//================================================================================
/*!
 * \brief perform it's intention action: preview mesh
 */
//================================================================================

void SMESHGUI_PrecomputeOp::onPreview()
{
  if ( !myDlg || myMesh->_is_nil() || myMainShape->_is_nil() )
    return;

  _PTR(SObject) aMeshSObj = SMESH::FindSObject(myMesh);
  if ( !aMeshSObj )
    return;

  // set modified submesh priority if any
  if (myOrderMgr && myOrderMgr->IsOrderChanged())
    myOrderMgr->SetMeshOrder();

  // Compute preview of mesh,
  // i.e. compute mesh till indicated dimension
  int dim = myDlg->getPreviewMode();

  SMESH::MemoryReserve aMemoryReserve;

  SMESH::compute_error_array_var aCompErrors;
  QString                        aHypErrors;

  bool computeFailed = true, memoryLack = false;

  SMESHGUI_ComputeDlg* aCompDlg = computeDlg();
  aCompDlg->myMeshName->setText( aMeshSObj->GetName().c_str() );

  SMESHGUI* gui = getSMESHGUI();
  SMESH::SMESH_Gen_var gen = gui->GetSMESHGen();
  SMESH::algo_error_array_var errors = gen->GetAlgoState(myMesh,myMainShape);
  if ( errors->length() > 0 ) {
    aHypErrors = SMESH::GetMessageOnAlgoStateErrors( errors.in() );
  }

  SUIT_OverrideCursor aWaitCursor;

  SVTK_ViewWindow*    view = SMESH::GetViewWindow( gui );
  if ( myPreviewDisplayer ) delete myPreviewDisplayer;
  myPreviewDisplayer = new SMESHGUI_MeshEditPreview( view );
  
  SMESH::long_array_var aShapesId = new SMESH::long_array();
  try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
    OCC_CATCH_SIGNALS;
#endif
      
    SMESH::MeshPreviewStruct_var previewData =
      gen->Precompute(myMesh, myMainShape, (SMESH::Dimension)dim, aShapesId);

    SMESH::MeshPreviewStruct* previewRes = previewData._retn();
    if ( previewRes && previewRes->nodesXYZ.length() > 0 )
    {
      computeFailed = false;
      myPreviewDisplayer->SetData( previewRes );
      // append shape indeces with computed mesh entities
      for ( int i = 0, n = aShapesId->length(); i < n; i++ )
        myMapShapeId[ aShapesId[ i ] ] = 0;
    }
    else
      myPreviewDisplayer->SetVisibility(false);
  }
  catch(const SALOME::SALOME_Exception & S_ex){
    memoryLack = true;
    myPreviewDisplayer->SetVisibility(false);
  }

  try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
    OCC_CATCH_SIGNALS;
#endif
    aCompErrors = gen->GetComputeErrors( myMesh, myMainShape );
    // check if there are memory problems
    for ( CORBA::ULong i = 0; (i < aCompErrors->length()) && !memoryLack; ++i )
      memoryLack = ( aCompErrors[ i ].code == SMESH::COMPERR_MEMORY_PB );
  }
  catch(const SALOME::SALOME_Exception & S_ex){
    memoryLack = true;
  }

  if ( memoryLack )
    aMemoryReserve.release();

  bool noCompError = ( !aCompErrors.operator->() || aCompErrors->length() == 0 );
  bool noHypoError = ( aHypErrors.isEmpty() );

  SUIT_ResourceMgr* resMgr = SMESH::GetResourceMgr( gui );
  int aNotifyMode = resMgr->integerValue( "SMESH", "show_result_notification" );

  bool isShowError = true;
  switch( aNotifyMode ) {
  case 0: // show the mesh computation result dialog NEVER
    isShowError = false;
    break;
  case 1: // show the mesh computation result dialog if there are some errors
  default: // show the result dialog after each mesh computation
    if ( !computeFailed && !memoryLack && noCompError && noHypoError )
      isShowError = false;
    break;
  }

  aWaitCursor.suspend();
  // SHOW ERRORS
  if ( isShowError )
  {
    myDlg->hide();
    aCompDlg->setWindowTitle
      ( tr( computeFailed ? "SMESH_WRN_COMPUTE_FAILED" : "SMESH_COMPUTE_SUCCEED" ));
    showComputeResult( memoryLack, noCompError, aCompErrors, noHypoError, aHypErrors );
  }
}


//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_PrecomputeDlg::SMESHGUI_PrecomputeDlg( QWidget* parent )
 : SMESHGUI_Dialog( parent, false, false, OK | Cancel | Help ),
   myOrderBox(0)
{
  setWindowTitle( tr( "CAPTION" ) );

  setButtonText( OK, tr( "COMPUTE" ) );
  QFrame* main = mainFrame();
  main->setMinimumWidth( 300 );

  QVBoxLayout* layout = new QVBoxLayout( main );

  myOrderBox = new SMESHGUI_MeshOrderBox( main );
  layout->addWidget(myOrderBox);

  QFrame* frame = new QFrame( main );
  layout->setMargin(0); layout->setSpacing(0);
  layout->addWidget( frame );

  QHBoxLayout* frameLay = new QHBoxLayout( frame );
  frameLay->setMargin(0); frameLay->setSpacing(SPACING);
  
  myPreviewMode = new QtxComboBox( frame );
  frameLay->addWidget( myPreviewMode );

  myPreviewBtn = new QPushButton( tr( "PREVIEW" ), frame );
  frameLay->addWidget( myPreviewBtn );

  connect( myPreviewBtn, SIGNAL( clicked( bool ) ), this, SIGNAL( preview() ) );
}

//================================================================================
/*!
 * \brief Destructor
*/
//================================================================================

SMESHGUI_PrecomputeDlg::~SMESHGUI_PrecomputeDlg()
{
}

//================================================================================
/*!
 * \brief Sets available preview modes
*/
//================================================================================

void SMESHGUI_PrecomputeDlg::setPreviewModes( const QList<int>& theModes )
{
  myPreviewMode->clear();
  QList<int>::const_iterator it = theModes.constBegin();
  for ( int i = 0; it != theModes.constEnd(); ++it, i++ )
  {
    QString mode = QString( "PREVIEW_%1" ).arg( *it );
    myPreviewMode->addItem( tr( mode.toLatin1().data() ) );
    myPreviewMode->setId( i, *it );
  }
  myPreviewBtn->setEnabled( !theModes.isEmpty() );
}

//================================================================================
/*!
 * \brief Returns current preview mesh mode
*/
//================================================================================

int SMESHGUI_PrecomputeDlg::getPreviewMode() const
{
  return myPreviewMode->currentId().toInt();
}

//================================================================================
/*!
 * \brief Returns current preview mesh mode
*/
//================================================================================

SMESHGUI_MeshOrderBox* SMESHGUI_PrecomputeDlg::getMeshOrderBox() const
{
  return myOrderBox;
}


//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_EvaluateOp::SMESHGUI_EvaluateOp()
 : SMESHGUI_BaseComputeOp()
{
  myHelpFileName = "constructing_meshes_page.html#evaluate_anchor";
}


//================================================================================
/*!
 * \brief Desctructor
*/
//================================================================================

SMESHGUI_EvaluateOp::~SMESHGUI_EvaluateOp()
{
}

//================================================================================
/*!
 * \brief perform it's intention action: compute mesh
 */
//================================================================================

void SMESHGUI_EvaluateOp::startOperation()
{
  SMESHGUI_BaseComputeOp::evaluateDlg();
  SMESHGUI_BaseComputeOp::startOperation();
  if (myMesh->_is_nil())
    return;
  evaluateMesh();
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
 * \retval LightApp_Dialog* - pointer to dialog of this operation
 */
//================================================================================

LightApp_Dialog* SMESHGUI_EvaluateOp::dlg() const
{
  return evaluateDlg();
}

//================================================================================
/*!
 * \brief evaluateMesh()
*/
//================================================================================

void SMESHGUI_BaseComputeOp::evaluateMesh()
{
  // EVALUATE MESH

  SMESH::MemoryReserve aMemoryReserve;

  SMESH::compute_error_array_var aCompErrors;
  QString                        aHypErrors;

  bool evaluateFailed = true, memoryLack = false;
  SMESH::long_array_var aRes;

  _PTR(SObject) aMeshSObj = SMESH::FindSObject(myMesh);
  if ( !aMeshSObj ) //  IPAL21340
    return;

  bool hasShape = myMesh->HasShapeToMesh();
  bool shapeOK = myMainShape->_is_nil() ? !hasShape : hasShape;
  if ( shapeOK )
  {
    myCompDlg->myMeshName->setText( aMeshSObj->GetName().c_str() );
    SMESH::SMESH_Gen_var gen = getSMESHGUI()->GetSMESHGen();
    SMESH::algo_error_array_var errors = gen->GetAlgoState(myMesh,myMainShape);
    if ( errors->length() > 0 ) {
      aHypErrors = SMESH::GetMessageOnAlgoStateErrors( errors.in() );
    }
    SUIT_OverrideCursor aWaitCursor;
    try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
      OCC_CATCH_SIGNALS;
#endif
      aRes = gen->Evaluate(myMesh, myMainShape);
    }
    catch(const SALOME::SALOME_Exception & S_ex){
      memoryLack = true;
    }

    try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
      OCC_CATCH_SIGNALS;
#endif
      aCompErrors = gen->GetComputeErrors( myMesh, myMainShape );
    }
    catch(const SALOME::SALOME_Exception & S_ex){
      memoryLack = true;
    }
  }

  if ( memoryLack )
    aMemoryReserve.release();

  evaluateFailed =  ( aCompErrors->length() > 0 );
  myCompDlg->setWindowTitle
    ( tr( evaluateFailed ? "SMESH_WRN_EVALUATE_FAILED" : "SMESH_EVALUATE_SUCCEED" ));

  // SHOW ERRORS

  bool noCompError = ( !aCompErrors.operator->() || aCompErrors->length() == 0 );
  bool noHypoError = ( aHypErrors.isEmpty() );

  //SUIT_ResourceMgr* resMgr = SMESH::GetResourceMgr( SMESHGUI::GetSMESHGUI() );
  //int aNotifyMode = resMgr->integerValue( "SMESH", "show_result_notification" );

  bool isShowResultDlg = true;
  //if( noHypoError )
  //switch( aNotifyMode ) {
  //case 0: // show the mesh computation result dialog NEVER
  //isShowResultDlg = false;
  //commit();
  //break;
  //case 1: // show the mesh computation result dialog if there are some errors
  //if ( memoryLack || !noHypoError )
  //  isShowResultDlg = true;
  //else
  //{
  //  isShowResultDlg = false;
  //  commit();
  //}
  //break;
  //default: // show the result dialog after each mesh computation
  //isShowResultDlg = true;
  //}

  // SHOW RESULTS
  if ( isShowResultDlg )
    showEvaluateResult( aRes, memoryLack, noCompError, aCompErrors,
                        noHypoError, aHypErrors);
}


void SMESHGUI_BaseComputeOp::showEvaluateResult(const SMESH::long_array& theRes,
                                                const bool theMemoryLack,
                                                const bool theNoCompError,
                                                SMESH::compute_error_array_var& theCompErrors,
                                                const bool theNoHypoError,
                                                const QString& theHypErrors)
{
  bool hasShape = myMesh->HasShapeToMesh();
  SMESHGUI_ComputeDlg* aCompDlg = evaluateDlg();
  aCompDlg->myMemoryLackGroup->hide();

  if ( theMemoryLack )
  {
    aCompDlg->myMemoryLackGroup->show();
    aCompDlg->myFullInfo->hide();
    aCompDlg->myBriefInfo->hide();
    aCompDlg->myHypErrorGroup->hide();
    aCompDlg->myCompErrorGroup->hide();
  }
  else if ( theNoCompError && theNoHypoError )
  {
    aCompDlg->myFullInfo->SetMeshInfo( theRes );
    aCompDlg->myFullInfo->show();
    aCompDlg->myBriefInfo->hide();
    aCompDlg->myHypErrorGroup->hide();
    aCompDlg->myCompErrorGroup->hide();
  }
  else
  {
    QTableWidget* tbl = aCompDlg->myTable;
    aCompDlg->myBriefInfo->SetMeshInfo( theRes );
    aCompDlg->myBriefInfo->show();
    aCompDlg->myFullInfo->hide();

    if ( theNoHypoError ) {
      aCompDlg->myHypErrorGroup->hide();
    }
    else {
      aCompDlg->myHypErrorGroup->show();
      aCompDlg->myHypErrorLabel->setText( theHypErrors );
    }

    if ( theNoCompError ) {
      aCompDlg->myCompErrorGroup->hide();
    }
    else {
      aCompDlg->myCompErrorGroup->show();

      aCompDlg->myPublishBtn->hide();
      aCompDlg->myShowBtn->hide();

      // fill table of errors
      tbl->setRowCount( theCompErrors->length() );
      if ( !hasShape ) tbl->hideColumn( COL_SHAPE );
      else             tbl->showColumn( COL_SHAPE );
      tbl->setColumnWidth( COL_ERROR, 200 );

      bool hasBadMesh = false;
      for ( int row = 0; row < (int) theCompErrors->length(); ++row )
      {
        SMESH::ComputeError & err = theCompErrors[ row ];

        QString text = err.algoName.in();
        if ( !tbl->item( row, COL_ALGO ) ) tbl->setItem( row, COL_ALGO, new QTableWidgetItem( text ) );
        else tbl->item( row, COL_ALGO )->setText( text );

        text = SMESH::errorText( err.code, err.comment.in() );
        if ( !tbl->item( row, COL_ERROR ) ) tbl->setItem( row, COL_ERROR, new QTableWidgetItem( text ) );
        else tbl->item( row, COL_ERROR )->setText( text );

        text = QString("%1").arg( err.subShapeID );
        if ( !tbl->item( row, COL_SHAPEID ) ) tbl->setItem( row, COL_SHAPEID, new QTableWidgetItem( text ) );
        else tbl->item( row, COL_SHAPEID )->setText( text );

        text = hasShape ? SMESH::shapeText( err.subShapeID, myMainShape ) : QString("");
        if ( !tbl->item( row, COL_SHAPE ) ) tbl->setItem( row, COL_SHAPE, new QTableWidgetItem( text ) );
        else tbl->item( row, COL_SHAPE )->setText( text );

        text = ( !hasShape || SMESH::getSubShapeSO( err.subShapeID, myMainShape )) ? "PUBLISHED" : "";
        if ( !tbl->item( row, COL_PUBLISHED ) ) tbl->setItem( row, COL_PUBLISHED, new QTableWidgetItem( text ) );
        else tbl->item( row, COL_PUBLISHED )->setText( text ); // if text=="", "PUBLISH" button enabled

        text = err.hasBadMesh ? "hasBadMesh" : "";
        if ( !tbl->item( row, COL_BAD_MESH ) ) tbl->setItem( row, COL_BAD_MESH, new QTableWidgetItem( text ) );
        else tbl->item( row, COL_BAD_MESH )->setText( text );
        if ( err.hasBadMesh ) hasBadMesh = true;

        //tbl->item( row, COL_ERROR )->setWordWrap( true ); // VSR: TODO ???
        tbl->resizeRowToContents( row );
      }
      tbl->resizeColumnToContents( COL_ALGO );
      tbl->resizeColumnToContents( COL_SHAPE );
      tbl->setWordWrap( true );

      if ( hasBadMesh )
      {
        aCompDlg->myBadMeshBtn->show();
        aCompDlg->myBadMeshToGroupBtn->show();
      }
      else
      {
        aCompDlg->myBadMeshBtn->hide();
        aCompDlg->myBadMeshToGroupBtn->hide();
      }
      tbl->setCurrentCell(0,0);
      currentCellChanged(); // to update buttons
    }
  }
  // show dialog and wait, becase Compute can be invoked from Preview operation
  //aCompDlg->exec(); // this way it becomes modal - impossible to rotate model in the Viewer
  aCompDlg->show();
}


//================================================================================
/*!
 * \brief Gets dialog of evaluate operation
 * \retval SMESHGUI_ComputeDlg* - pointer to dialog of this operation
 */
//================================================================================

SMESHGUI_ComputeDlg* SMESHGUI_BaseComputeOp::evaluateDlg() const
{
  if ( !myCompDlg )
  {
    SMESHGUI_BaseComputeOp* me = (SMESHGUI_BaseComputeOp*)this;
    me->myCompDlg = new SMESHGUI_ComputeDlg( desktop(), true );
    // connect signals and slots
    connect(myCompDlg->myShowBtn,    SIGNAL (clicked()), SLOT(onPreviewShape()));
    connect(myCompDlg->myPublishBtn, SIGNAL (clicked()), SLOT(onPublishShape()));
    connect(myCompDlg->myBadMeshBtn, SIGNAL (clicked()), SLOT(onShowBadMesh()));
    QTableWidget* aTable = me->table();
    connect(aTable, SIGNAL(itemSelectionChanged()), SLOT(currentCellChanged()));
    connect(aTable, SIGNAL(currentCellChanged(int,int,int,int)), SLOT(currentCellChanged()));
  }
  return myCompDlg;
}

