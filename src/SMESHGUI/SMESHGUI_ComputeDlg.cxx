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
// File   : SMESHGUI_ComputeDlg.cxx
// Author : Edward AGAPOV, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_ComputeDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_HypothesesUtils.h"
#include "SMESHGUI_MeshEditPreview.h"
#include "SMESH_ActorUtils.h"

#include <SMDS_SetIterator.hxx>
#include <SMDS_Mesh.hxx>

// SALOME GEOM includes
#include <GEOMBase.h>
#include <GEOM_Actor.h>

// SALOME GUI includes
#include <LightApp_SelectionMgr.h>
#include <LightApp_UpdateFlags.h>
#include <SALOME_ListIO.hxx>
#include <SVTK_ViewWindow.h>
#include <SVTK_ViewModel.h>
#include <SalomeApp_Application.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>
#include <QtxComboBox.h>

// SALOME KERNEL includes
#include <SALOMEDS_SObject.hxx>
#include <SALOMEDSClient_SObject.hxx>

// OCCT includes
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

// VTK includes
#include <vtkProperty.h>

// STL includes
#include <vector>
#include <set>

#define SPACING 6
#define MARGIN  11

#define COLONIZE(str)   (QString(str).contains(":") > 0 ? QString(str) : QString(str) + " :" )

static void addSeparator( QWidget* parent )
{
  QGridLayout* l = qobject_cast<QGridLayout*>( parent->layout() );
  int row  = l->rowCount();
  int cols = l->columnCount();
  for ( int i = 0; i < cols; i++ ) {
    QFrame* hline = new QFrame( parent );
    hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    l->addWidget( hline, row, i );
  }
}

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
      if ( index < 0 || index >= myActors.size() )
        return 0;
      GEOM_Actor* & actor = myActors[ index ];
      if ( !actor ) {
        actor = GEOM_Actor::New();
        if ( actor ) {
          actor->SetShape(shape,0,0);
          actor->SetProperty(myProperty);
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
      CASE2TEXT( COMPERR_OK            );
      CASE2TEXT( COMPERR_BAD_INPUT_MESH);
      CASE2TEXT( COMPERR_STD_EXCEPTION );
      CASE2TEXT( COMPERR_OCC_EXCEPTION );
    case SMESH::COMPERR_SLM_EXCEPTION: break; // avoid double "Salome exception"
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
      text = aSO->GetName().c_str();
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
        rows.append( row );
    }
    if ( rows.isEmpty() && table->currentRow() > -1 )
      rows.append( table->currentRow() );

    return rows.count();
  }

} // namespace SMESH


// =========================================================================================
/*!
 * \brief Box showing mesh info
 */
// =========================================================================================

SMESHGUI_MeshInfosBox::SMESHGUI_MeshInfosBox(const bool full, QWidget* theParent)
  : QGroupBox( tr("SMESH_MESHINFO_TITLE"), theParent ), myFull( full )
{
  QGridLayout* l = new QGridLayout(this);
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );

  QFont italic = font(); italic.setItalic(true);
  QFont bold   = font(); bold.setBold(true);

  QLabel* lab;
  int row = 0;

  // title
  lab = new QLabel( this );
  lab->setMinimumWidth(100); lab->setFont( italic );
  l->addWidget( lab, row, 0 );
  // --
  lab = new QLabel(tr("SMESH_MESHINFO_ORDER0"), this );
  lab->setMinimumWidth(100); lab->setFont( italic );
  l->addWidget( lab, row, 1 );
  // --
  lab = new QLabel(tr("SMESH_MESHINFO_ORDER1"), this );
  lab->setMinimumWidth(100); lab->setFont( italic );
  l->addWidget( lab, row, 2 );
  // --
  lab = new QLabel(tr("SMESH_MESHINFO_ORDER2"), this );
  lab->setMinimumWidth(100); lab->setFont( italic );
  l->addWidget( lab, row, 3 );

  if ( myFull )
  {
    // nodes
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NODES")), this );
    lab->setFont( bold );
    l->addWidget( lab,           row, 0 );
    // --
    myNbNode = new QLabel( this );
    l->addWidget( myNbNode,      row, 1 );

    addSeparator(this);          // add separator

    // edges
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_EDGES")), this );
    lab->setFont( bold );
    l->addWidget( lab,           row, 0 );
    // --
    myNbEdge = new QLabel( this );
    l->addWidget( myNbEdge,      row, 1 );
    // --
    myNbLinEdge = new QLabel( this );
    l->addWidget( myNbLinEdge,   row, 2 );
    // --
    myNbQuadEdge = new QLabel( this );
    l->addWidget( myNbQuadEdge,  row, 3 );

    addSeparator(this);          // add separator

    // faces
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_FACES")), this);
    lab->setFont( bold );
    l->addWidget( lab,           row, 0 );
    // --
    myNbFace     = new QLabel( this );
    l->addWidget( myNbFace,      row, 1 );
    // --
    myNbLinFace  = new QLabel( this );
    l->addWidget( myNbLinFace,   row, 2 );
    // --
    myNbQuadFace = new QLabel( this );
    l->addWidget( myNbQuadFace,  row, 3 );
    // --
    row++;                       // increment row count
    // ... triangles
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TRIANGLES")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbTrai     = new QLabel( this );
    l->addWidget( myNbTrai,      row, 1 );
    // --
    myNbLinTrai  = new QLabel( this );
    l->addWidget( myNbLinTrai,   row, 2 );
    // --
    myNbQuadTrai = new QLabel( this );
    l->addWidget( myNbQuadTrai,  row, 3 );
    // --
    row++;                       // increment row count
    // ... quadrangles
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_QUADRANGLES")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbQuad     = new QLabel( this );
    l->addWidget( myNbQuad,      row, 1 );
    // --
    myNbLinQuad  = new QLabel( this );
    l->addWidget( myNbLinQuad,   row, 2 );
    // --
    myNbQuadQuad = new QLabel( this );
    l->addWidget( myNbQuadQuad,  row, 3 );
    // --
    row++;                       // increment row count
    // ... poligones
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_POLYGONES")), this );
    l->addWidget( lab,           row, 0 );
    myNbPolyg    = new QLabel( this );
    l->addWidget( myNbPolyg,     row, 1 );

    addSeparator(this);          // add separator

    // volumes
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_VOLUMES")), this);
    lab->setFont( bold );
    l->addWidget( lab,           row, 0 );
    // --
    myNbVolum     = new QLabel( this );
    l->addWidget( myNbVolum,     row, 1 );
    // --
    myNbLinVolum  = new QLabel( this );
    l->addWidget( myNbLinVolum,  row, 2 );
    // --
    myNbQuadVolum = new QLabel( this );
    l->addWidget( myNbQuadVolum, row, 3 );
    // --
    row++;                       // increment row count
    // ... tetras
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_TETRAS")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbTetra     = new QLabel( this );
    l->addWidget( myNbTetra,     row, 1 );
    // --
    myNbLinTetra  = new QLabel( this );
    l->addWidget( myNbLinTetra,  row, 2 );
    // --
    myNbQuadTetra = new QLabel( this );
    l->addWidget( myNbQuadTetra, row, 3 );
    // --
    row++;                       // increment row count
    // ... hexas
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_HEXAS")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbHexa      = new QLabel( this );
    l->addWidget( myNbHexa,      row, 1 );
    // --
    myNbLinHexa   = new QLabel( this );
    l->addWidget( myNbLinHexa,   row, 2 );
    // --
    myNbQuadHexa  = new QLabel( this );
    l->addWidget( myNbQuadHexa,  row, 3 );
    // --
    row++;                       // increment row count
    // ... pyras
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_PYRAS")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbPyra      = new QLabel( this );
    l->addWidget( myNbPyra,      row, 1 );
    // --
    myNbLinPyra   = new QLabel( this );
    l->addWidget( myNbLinPyra,   row, 2 );
    // --
    myNbQuadPyra  = new QLabel( this );
    l->addWidget( myNbQuadPyra,  row, 3 );
    // --
    row++;                       // increment row count
    // ... prisms
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_PRISMS")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbPrism     = new QLabel( this );
    l->addWidget( myNbPrism,     row, 1 );
    // --
    myNbLinPrism  = new QLabel( this );
    l->addWidget( myNbLinPrism,  row, 2 );
    // --
    myNbQuadPrism = new QLabel( this );
    l->addWidget( myNbQuadPrism, row, 3 );
    // --
    row++;                       // increment row count
    // ... polyedres
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_POLYEDRES")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbPolyh     = new QLabel( this );
    l->addWidget( myNbPolyh,     row, 1 );
  }
  else
  {
    // nodes
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NODES")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbNode      = new QLabel( this );
    l->addWidget( myNbNode,      row, 1 );

    // edges
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_EDGES")), this );
    l->addWidget( lab,           row, 0 );
    // --
    myNbEdge      = new QLabel( this );
    l->addWidget( myNbEdge,      row, 1 );
    // --
    myNbLinEdge   = new QLabel( this );
    l->addWidget( myNbLinEdge,   row, 2 );
    // --
    myNbQuadEdge  = new QLabel( this );
    l->addWidget( myNbQuadEdge,  row, 3 );

    // faces
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_FACES")), this);
    l->addWidget( lab,           row, 0 );
    // --
    myNbFace      = new QLabel( this );
    l->addWidget( myNbFace,      row, 1 );
    // --
    myNbLinFace   = new QLabel( this );
    l->addWidget( myNbLinFace,   row, 2 );
    // --
    myNbQuadFace  = new QLabel( this );
    l->addWidget( myNbQuadFace,  row, 3 );

    // volumes
    row = l->rowCount();         // retrieve current row count
    // --
    lab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_VOLUMES")), this);
    l->addWidget( lab,           row, 0 );
    // --
    myNbVolum     = new QLabel( this );
    l->addWidget( myNbVolum,     row, 1 );
    // --
    myNbLinVolum  = new QLabel( this );
    l->addWidget( myNbLinVolum,  row, 2 );
    // --
    myNbQuadVolum = new QLabel( this );
    l->addWidget( myNbQuadVolum, row, 3 );
  }
}

// =========================================================================================
/*!
 * \brief Set mesh info
 */
// =========================================================================================

void SMESHGUI_MeshInfosBox::SetInfoByMesh(SMESH::SMESH_Mesh_var mesh)
{
  const SMESH::ElementOrder lin = SMESH::ORDER_LINEAR;
  int nbTot, nbLin;

  // nodes
  myNbNode     ->setText( QString("%1").arg( mesh->NbNodes() ));

  // edges
  nbTot = mesh->NbEdges(), nbLin = mesh->NbEdgesOfOrder(lin);
  myNbEdge     ->setText( QString("%1").arg( nbTot ));
  myNbLinEdge  ->setText( QString("%1").arg( nbLin ));
  myNbQuadEdge ->setText( QString("%1").arg( nbTot - nbLin ));

  // faces
  nbTot = mesh->NbFaces(), nbLin = mesh->NbFacesOfOrder(lin);
  myNbFace     ->setText( QString("%1").arg( nbTot ));
  myNbLinFace  ->setText( QString("%1").arg( nbLin ));
  myNbQuadFace ->setText( QString("%1").arg( nbTot - nbLin ));

  // volumes
  nbTot = mesh->NbVolumes(), nbLin = mesh->NbVolumesOfOrder(lin);
  myNbVolum    ->setText( QString("%1").arg( nbTot ));
  myNbLinVolum ->setText( QString("%1").arg( nbLin ));
  myNbQuadVolum->setText( QString("%1").arg( nbTot - nbLin ));

  if ( myFull )
  {
    // triangles
    nbTot = mesh->NbTriangles(), nbLin = mesh->NbTrianglesOfOrder(lin);
    myNbTrai     ->setText( QString("%1").arg( nbTot ));
    myNbLinTrai  ->setText( QString("%1").arg( nbLin ));
    myNbQuadTrai ->setText( QString("%1").arg( nbTot - nbLin ));
    // quadrangles
    nbTot = mesh->NbQuadrangles(), nbLin = mesh->NbQuadranglesOfOrder(lin);
    myNbQuad     ->setText( QString("%1").arg( nbTot ));
    myNbLinQuad  ->setText( QString("%1").arg( nbLin ));
    myNbQuadQuad ->setText( QString("%1").arg( nbTot - nbLin ));
    // poligones
    myNbPolyg    ->setText( QString("%1").arg( mesh->NbPolygons() ));

    // tetras
    nbTot = mesh->NbTetras(), nbLin = mesh->NbTetrasOfOrder(lin);
    myNbTetra    ->setText( QString("%1").arg( nbTot ));
    myNbLinTetra ->setText( QString("%1").arg( nbLin ));
    myNbQuadTetra->setText( QString("%1").arg( nbTot - nbLin ));
    // hexas
    nbTot = mesh->NbHexas(), nbLin = mesh->NbHexasOfOrder(lin);
    myNbHexa     ->setText( QString("%1").arg( nbTot ));
    myNbLinHexa  ->setText( QString("%1").arg( nbLin ));
    myNbQuadHexa ->setText( QString("%1").arg( nbTot - nbLin ));
    // pyras
    nbTot = mesh->NbPyramids(), nbLin = mesh->NbPyramidsOfOrder(lin);
    myNbPyra     ->setText( QString("%1").arg( nbTot ));
    myNbLinPyra  ->setText( QString("%1").arg( nbLin ));
    myNbQuadPyra ->setText( QString("%1").arg( nbTot - nbLin ));
    // prisms
    nbTot = mesh->NbPrisms(), nbLin = mesh->NbPrismsOfOrder(lin);
    myNbPrism    ->setText( QString("%1").arg( nbTot ));
    myNbLinPrism ->setText( QString("%1").arg( nbLin ));
    myNbQuadPrism->setText( QString("%1").arg( nbTot - nbLin ));
    // polyedres
    myNbPolyh    ->setText( QString("%1").arg( mesh->NbPolyhedrons() ));
  }
}

// =========================================================================================
/*!
 * \brief Dialog to compute a mesh and show computation errors
 */
//=======================================================================

SMESHGUI_ComputeDlg::SMESHGUI_ComputeDlg( QWidget* parent )
 : SMESHGUI_Dialog( parent, false, true, Close/* | Help*/ )
{
  QVBoxLayout* aDlgLay = new QVBoxLayout (mainFrame());
  aDlgLay->setMargin( 0 );
  aDlgLay->setSpacing( SPACING );

  QFrame* aMainFrame = createMainFrame  (mainFrame());

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

QFrame* SMESHGUI_ComputeDlg::createMainFrame (QWidget* theParent)
{
  QFrame* aFrame = new QFrame(theParent);

  SUIT_ResourceMgr* rm = resourceMgr();
  QPixmap iconCompute (rm->loadPixmap("SMESH", tr("ICON_COMPUTE")));

  // constructor

  QGroupBox* aPixGrp = new QGroupBox(tr("CONSTRUCTOR"), aFrame);
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
  myTable      = new QTableWidget( 1, NB_COLUMNS, myCompErrorGroup);
  myShowBtn    = new QPushButton(tr("SHOW_SHAPE"), myCompErrorGroup);
  myPublishBtn = new QPushButton(tr("PUBLISH_SHAPE"), myCompErrorGroup);
  myBadMeshBtn = new QPushButton(tr("SHOW_BAD_MESH"), myCompErrorGroup);

  //myTable->setReadOnly( true ); // VSR: check
  myTable->setEditTriggers( QAbstractItemView::NoEditTriggers );
  myTable->hideColumn( COL_PUBLISHED );
  myTable->hideColumn( COL_SHAPEID );
  myTable->hideColumn( COL_BAD_MESH );
  myTable->horizontalHeader()->setResizeMode( COL_ERROR, QHeaderView::Interactive );

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
  grpLayout->addWidget( myTable,      0, 0, 4, 1 );
  grpLayout->addWidget( myShowBtn,    0, 1 );
  grpLayout->addWidget( myPublishBtn, 1, 1 );
  grpLayout->addWidget( myBadMeshBtn, 2, 1 );
  grpLayout->setRowStretch( 3, 1 );

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
  aLay->setMargin( 0 );
  aLay->setSpacing( 0 );
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
  : SMESHGUI_Operation(),
    myCompDlg( 0 )
{
  myTShapeDisplayer = new SMESH::TShapeDisplayer();
  myBadMeshDisplayer = 0;

  //myHelpFileName = "/files/about_meshes.htm"; // V3
  myHelpFileName = "about_meshes_page.html"; // V4
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
  myMesh = SMESH::GetMeshByIO(myIObject);
  if (myMesh->_is_nil()) {
    SUIT_MessageBox::warning(desktop(),
			     tr("SMESH_WRN_WARNING"),
			     tr("SMESH_WRN_NO_AVAILABLE_DATA"));
    onCancel();

  }
  myMainShape = myMesh->GetShapeToMesh();

  SMESHGUI_Operation::startOperation();
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
  bool hasShape = myMesh->HasShapeToMesh();
  bool shapeOK = myMainShape->_is_nil() ? !hasShape : hasShape;
  if ( shapeOK && aMeshSObj )
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
      if (gen->Compute(myMesh, myMainShape))
        computeFailed = false;
    }
    catch(const SALOME::SALOME_Exception & S_ex){
      memoryLack = true;
    }
    try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
      OCC_CATCH_SIGNALS;
#endif
      aCompErrors = gen->GetComputeErrors( myMesh, myMainShape );
      // check if there are memory problems
      for ( int i = 0; (i < aCompErrors->length()) && !memoryLack; ++i )
        memoryLack = ( aCompErrors[ i ].code == SMESH::COMPERR_MEMORY_PB );
    }
    catch(const SALOME::SALOME_Exception & S_ex){
      memoryLack = true;
    }

    // NPAL16631: if ( !memoryLack )
    {
      SMESH::ModifiedMesh(aMeshSObj, !computeFailed, myMesh->NbNodes() == 0);
      update( UF_ObjBrowser | UF_Model );

      // SHOW MESH
      // NPAL16631: if ( getSMESHGUI()->automaticUpdate() )
      if ( !memoryLack && getSMESHGUI()->automaticUpdate() )
      {
        try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
          OCC_CATCH_SIGNALS;
#endif
          SMESH::Update(myIObject, true);
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
      LightApp_SelectionMgr *Sel = selectionMgr();
      if ( Sel )
      {
	SALOME_ListIO selected;
	selected.Append( myIObject );
	Sel->setSelectedObjects( selected );
      }
    }
  }

  if ( memoryLack )
    aMemoryReserve.release();

  myCompDlg->setWindowTitle(tr( computeFailed ? "SMESH_WRN_COMPUTE_FAILED" : "SMESH_COMPUTE_SUCCEED"));

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
						const bool     theNoHypoError,
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
    aCompDlg->myFullInfo->SetInfoByMesh( myMesh );
    aCompDlg->myFullInfo->show();
    aCompDlg->myBriefInfo->hide();
    aCompDlg->myHypErrorGroup->hide();
    aCompDlg->myCompErrorGroup->hide();
  }
  else
  {
    QTableWidget* tbl = aCompDlg->myTable;
    aCompDlg->myBriefInfo->SetInfoByMesh( myMesh );
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

      if ( !hasShape ) {
        aCompDlg->myPublishBtn->hide();
        aCompDlg->myShowBtn->hide();
      }
      else {
        aCompDlg->myPublishBtn->show();
        aCompDlg->myShowBtn->show();
      }

      // fill table of errors
      tbl->setRowCount( theCompErrors->length() );
      if ( !hasShape ) tbl->hideColumn( COL_SHAPE );
      else             tbl->showColumn( COL_SHAPE );
      tbl->setColumnWidth( COL_ERROR, 200 );

      bool hasBadMesh = false;
      for ( int row = 0; row < theCompErrors->length(); ++row )
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

      if ( hasBadMesh )
        aCompDlg->myBadMeshBtn->show();
      else
        aCompDlg->myBadMeshBtn->hide();

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
 * \brief publish selected subshape
 */
//================================================================================

void SMESHGUI_BaseComputeOp::onPublishShape()
{
  GEOM::GEOM_Gen_var geomGen = SMESH::GetGEOMGen();
  SALOMEDS::Study_var study = SMESHGUI::GetSMESHGen()->GetCurrentStudy();

  QList<int> rows;
  SMESH::getSelectedRows( table(), rows );
  int row;
  foreach ( row, rows )
  {
    int curSub = table()->item(row, COL_SHAPEID)->text().toInt();
    GEOM::GEOM_Object_var shape = SMESH::getSubShape( curSub, myMainShape );
    if ( !shape->_is_nil() && ! SMESH::getSubShapeSO( curSub, myMainShape ))
    {
      if ( !SMESH::getSubShapeSO( 1, myMainShape )) // the main shape not published
      {
        QString name = GEOMBase::GetDefaultName( SMESH::shapeTypeName( myMainShape, "MAIN_SHAPE" ));
        SALOMEDS::SObject_var so =
          geomGen->AddInStudy( study, myMainShape, name.toLatin1().data(), GEOM::GEOM_Object::_nil());
        // look for myMainShape in the table
        for ( int r = 0, nr = table()->rowCount(); r < nr; ++r ) {
          if ( table()->item( r, COL_SHAPEID )->text() == "1" ) {
            if ( so->_is_nil() ) {
              table()->item( r, COL_SHAPE )->setText( so->GetName() );
              table()->item( r, COL_PUBLISHED )->setText( so->GetID() );
            }
            break;
          }
        }
        if ( curSub == 1 ) continue;
      }
      QString name = GEOMBase::GetDefaultName( SMESH::shapeTypeName( shape, "ERROR_SHAPE" ));
      SALOMEDS::SObject_var so = geomGen->AddInStudy( study, shape, name.toLatin1().data(), myMainShape);
      if ( !so->_is_nil() ) {
        table()->item( row, COL_SHAPE )->setText( so->GetName() );
        table()->item( row, COL_PUBLISHED )->setText( so->GetID() );
      }
    }
  }
  getSMESHGUI()->getApp()->updateObjectBrowser();
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
      vtkFloatingPointType aPointSize = SMESH::GetFloat("SMESH:node_size",3);
      vtkFloatingPointType aLineWidth = SMESH::GetFloat("SMESH:element_width",1);
      // delete property !!!!!!!!!!
      vtkProperty* prop = vtkProperty::New();
      prop->SetLineWidth( aLineWidth * 3 );
      prop->SetPointSize( aPointSize * 3 );
      prop->SetColor( 250, 0, 250 );
      myBadMeshDisplayer->GetActor()->SetProperty( prop );
      myBadMeshDisplayer->SetData( aMeshData._retn() );
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
    me->myCompDlg = new SMESHGUI_ComputeDlg( desktop() );
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
  computeMesh();
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
 myDlg( 0 ),
 myActiveDlg( 0 ),
 myPreviewDisplayer( 0 )
{
  myHelpFileName = "preview_meshes_page.html"; // V4
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
  if ( !myDlg )
  {
    myDlg = new SMESHGUI_PrecomputeDlg( desktop() );
    
    // connect signals
    connect( myDlg, SIGNAL( preview() ), this, SLOT( onPreview() ) );
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
 * \brief perform it's intention action: reinitialise dialog
 */
//================================================================================

void SMESHGUI_PrecomputeOp::resumeOperation()
{
  if ( myActiveDlg == myDlg )
    initDialog();
  SMESHGUI_BaseComputeOp::resumeOperation();
}

void SMESHGUI_PrecomputeOp::initDialog()
{
  QList<int> modes;
  QMap<int, int> modeMap;
  _PTR(SObject)          aHypRoot;
  _PTR(GenericAttribute) anAttr;
  int aPart = SMESH::Tag_RefOnAppliedAlgorithms;

  _PTR(SObject) pMesh = studyDS()->FindObjectID( myIObject->getEntry() );
  if ( pMesh && pMesh->FindSubObject( aPart, aHypRoot ) )
  {
    _PTR(ChildIterator) anIter =
      SMESH::GetActiveStudyDocument()->NewChildIterator( aHypRoot );
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

        SMESH::SMESH_Algo_var algo = SMESH::SMESH_3D_Algo::_narrow( aVar );
        if ( !algo->_is_nil() )
        {
	  modeMap[ SMESH::DIM_1D ] = 0;
	  modeMap[ SMESH::DIM_2D ] = 0;
        }
        else
        {
          algo = SMESH::SMESH_2D_Algo::_narrow( aVar );
          if ( !algo->_is_nil() )
            modeMap[ SMESH::DIM_2D ] = 0;
        }
      }
    }
  }
  if ( modeMap.contains( SMESH::DIM_1D ) )
    modes.append( SMESH::DIM_1D );
  if ( modeMap.contains( SMESH::DIM_2D ) )
    modes.append( SMESH::DIM_2D );

  myDlg->setPreviewModes( modes );
}

//================================================================================
/*!
 * \brief perform it's intention action: 
 */
//================================================================================

bool SMESHGUI_PrecomputeOp::onApply()
{
  QObject* obj = sender();
  if ( obj != myDlg && myActiveDlg == myDlg )
    return true; // just return from error messages
  if ( myActiveDlg == myDlg )
  {
    myDlg->hide();
    myMapShapeId.clear();
    myActiveDlg = computeDlg();
    computeMesh();
  }

  return true;
}

//================================================================================
/*!
 * \brief perform it's intention action: compute mesh
 */
//================================================================================

void SMESHGUI_PrecomputeOp::onCancel()
{
  QObject* curDlg = sender();
  if ( curDlg == computeDlg() )
  {
    if ( myActiveDlg == myDlg ) // return from error messages
      myDlg->show();

    return;
  }

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
        myMesh->ClearSubMesh( *it );
    }
  }
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
    for ( int i = 0; (i < aCompErrors->length()) && !memoryLack; ++i )
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
    aCompDlg->setWindowTitle(tr( computeFailed ? "SMESH_WRN_COMPUTE_FAILED" : "SMESH_COMPUTE_SUCCEED"));
    showComputeResult( memoryLack, noCompError, aCompErrors, noHypoError, aHypErrors );
  }
}


//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_PrecomputeDlg::SMESHGUI_PrecomputeDlg( QWidget* parent )
 : SMESHGUI_Dialog( parent, false, false, OK | Cancel | Help )
{
  setWindowTitle( tr( "CAPTION" ) );

  setButtonText( OK, tr( "COMPUTE" ) );
  QFrame* main = mainFrame();

  QVBoxLayout* layout = new QVBoxLayout( main );

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
  return myPreviewMode->currentId();
}
