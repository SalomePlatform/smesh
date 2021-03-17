// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

/////////////////////////////////////////////////////////////////
/// \package libSMESH_Swig
/// \brief Python API for %SMESH GUI.
///
/// See SMESH_Swig class for %SMESH GUI Python API.
///
/// \par Note about publishing of mesh objects in the study
///
/// Mesh objects are automatically published in the study. It is
/// not needed to specify a target SALOMEDS object (SObject) or
/// even name (%SMESH engine will will default name if it is not
/// specified).
/// So, normally functions of %SMESH GUI Python API like
///   AddNewMesh
///   AddNewHypothesis
///   AddNewAlgorithms
///   etc.
/// may only be needed to be called in case if mesh objects are
/// created when no active study is set to %SMESH component.
/// In this case, mentioned functions can be used to forcibly
/// publish existing objects in the currently active study.
///
/// Note that if there are no open study, these methods will do
/// nothing.
///
/// \par Note about selection
///
/// In SALOME, selection is automatically synchronlized between
/// all GUI elements (like Object browser, view windows, etc).
/// This means that any changes to selection applied with
/// select() methods, will automatically apply to all view
/// windows, taking into account selection modes switched in
/// each particular view window (e.g. if you select edges, while
/// in some view window Face selection mode is switched on,
/// selection will not be immediately applied to this view
/// window.
/////////////////////////////////////////////////////////////////

#include "libSMESH_Swig.h"

#include <SVTK_Selector.h>

#include <SMESHGUI.h>
#include <SMESHGUI_Utils.h>
#include <SMESHGUI_Displayer.h>
#include <SMESHGUI_VTKUtils.h>
#include <SMESH_Actor.h>

// SALOME KERNEL includes
#include <Utils_ORB_INIT.hxx>
#include <Utils_SINGLETON.hxx>
#include <SALOMEDSClient_ClientFactory.hxx>
#include <SALOMEDS_Study.hxx>

#include <utilities.h>

// SALOME GUI includes
#include <SUIT_Session.h>
#include <SUIT_ViewManager.h>
#include <SALOME_Prs.h>
#include <SUIT_ViewWindow.h>
#include <SVTK_ViewWindow.h>
#include <SVTK_ViewModel.h>
#include <SALOME_Event.h>
#include <SalomeApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SVTK_RenderWindowInteractor.h>
#include <VTKViewer_Algorithm.h>

// OCCT includes
#include <TopAbs.hxx>
#include <TColStd_MapOfInteger.hxx>

// Qt includes
#include <QApplication>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

// VTK includes
#include <vtkActorCollection.h>
#include <vtkRenderer.h>

namespace
{
  ///////////////////////////////////////////////////////////////
  /// \internal
  /// \brief Print deprecation warning to termninal.
  /// \param function Interface function's name.
  /// \param replacement Replacement (new) function's name
  ///        (if there's any).
  ///////////////////////////////////////////////////////////////
  void deprecated(const char* function, const char* replacement = 0)
  {
    if ( replacement )
      printf("libSMESH_Swig: method '%s' is deprecated; use '%s' instead.\n", function, replacement);
    else
      printf("libSMESH_Swig: method '%s' is deprecated.\n", function);
  }

  ///////////////////////////////////////////////////////////////
  /// \internal
  /// \brief Get CORBA object by its IOR.
  /// \param ior Object's IOR.
  /// \return CORBA object (nil object if it isn't found).
  ///////////////////////////////////////////////////////////////
  CORBA::Object_var string2object(const std::string& ior)
  {
    return SalomeApp_Application::orb()->string_to_object(ior.c_str());
  }

  ///////////////////////////////////////////////////////////////
  /// \internal
  /// \brief Get study object by its study UID or IOR.
  /// \param uid Object's study UID or IOR.
  /// \return Pointer to study object (null object if it isn't
  ///         found).
  ///////////////////////////////////////////////////////////////
  _PTR(SObject) uid2object(const std::string& uid)
  {
    _PTR(SObject) sobject;
    {
      _PTR(Study) study = SMESH::getStudy();
      if ( study )
      {
        sobject = study->FindObjectID( uid );
        if ( !sobject )
          sobject = study->FindObjectIOR( uid );
      }
    }
    return sobject;
  }

  ///////////////////////////////////////////////////////////////
  /// \internal
  /// \brief Get view window by its identifier.
  /// \param uid Window's identifier.
  /// \return Pointer to the view window (0 if it isn't found).
  ///////////////////////////////////////////////////////////////
  SALOME_View* uid2wnd(int uid, bool create = false)
  {
    SALOME_View* window = 0;

    SUIT_Session* session = SUIT_Session::session();
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( session->activeApplication() );
    if ( app )
    {
      if ( uid )
      {
        ViewManagerList vms = app->viewManagers();
        for ( int i = 0; i < vms.count() && !window; i++ )
        {
          SUIT_ViewManager* vm = vms[i];
          QVector<SUIT_ViewWindow*> vws = vm->getViews();
          for ( int j = 0; j < vws.count() && !window; j++)
          {
            SUIT_ViewWindow* vw = vws[0];
            if ( uid == vw->getId() )
              window = dynamic_cast<SALOME_View*>( vm->getViewModel() );
          }
        }
      }
      else
      {
        SUIT_ViewManager* vm = app->getViewManager( SVTK_Viewer::Type(), create );
        if ( vm )
        {
          window = dynamic_cast<SALOME_View*>( vm->getViewModel() );
        }
      }
    }
    return window;
  }

  ///////////////////////////////////////////////////////////////
  /// \internal
  /// \brief Get all view windows.
  /// \return List of view windows.
  ///////////////////////////////////////////////////////////////
  QList<SALOME_View*> windows()
  {
    QList<SALOME_View*> views;
    SUIT_Session* session = SUIT_Session::session();
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( session->activeApplication() );
    if ( app )
    {
      ViewManagerList vms = app->viewManagers();
      foreach( SUIT_ViewManager* vm, vms )
      {
        if ( vm && vm->getType() == SVTK_Viewer::Type() )
        {
          SALOME_View* view = dynamic_cast<SALOME_View*>( vm->getViewModel() );
          if ( view )
            views << view;
        }
      }
    }
    return views;
  }

  ///////////////////////////////////////////////////////////////
  /// \internal
  /// \brief Get mesh actor from view.
  /// \param view Pointer to the view window.
  /// \param uid Mesh object's study UID.
  /// \return Mesh actor (0 if it isn't found).
  ///////////////////////////////////////////////////////////////
  SMESH_Actor* actorFromView(SALOME_View* view, const char* uid)
  {
    SMESH_Actor* actor = 0;
    SVTK_Viewer* model = dynamic_cast<SVTK_Viewer*>( view );
    if ( model )
    {
      SUIT_ViewWindow* vw = model->getViewManager()->getActiveView();
      if ( vw )
      {
        actor = SMESH::FindActorByEntry( vw, uid );
      }
    }
    return actor;
  }

  ///////////////////////////////////////////////////////////////
  /// \internal
  /// \brief Get mesh object's visual properties.
  /// \param view Pointer to the view window.
  /// \param uid Mesh object's study UID.
  /// \return Properties data structure.
  ///////////////////////////////////////////////////////////////
  Properties properties(SALOME_View* view, const char* uid)
  {
    Properties props;
    SMESH_Actor* actor = actorFromView( view, uid );
    if ( actor )
    {
      actor->GetNodeColor( props.nodeColor.r,
                           props.nodeColor.g,
                           props.nodeColor.b );
      props.markerType = actor->GetMarkerType();
      props.markerScale = actor->GetMarkerScale();

      actor->GetEdgeColor( props.edgeColor.r,
                           props.edgeColor.g,
                           props.edgeColor.b );
      props.edgeWidth = qMax( (int)actor->GetLineWidth(), 1 );

      actor->GetSufaceColor( props.surfaceColor.r,
                             props.surfaceColor.g,
                             props.surfaceColor.b,
                             props.surfaceColor.delta );

      actor->GetVolumeColor( props.volumeColor.r,
                             props.volumeColor.g,
                             props.volumeColor.b,
                             props.volumeColor.delta );

      actor->Get0DColor( props.elem0dColor.r,
                         props.elem0dColor.g,
                         props.elem0dColor.b );
      props.elem0dSize = qMax( (int)actor->Get0DSize(), 1 );

      actor->GetBallColor( props.ballColor.r,
                           props.ballColor.g,
                           props.ballColor.b );
      props.ballScale = qMax( actor->GetBallScale(), 1e-2 );

      actor->GetOutlineColor( props.outlineColor.r,
                              props.outlineColor.g,
                              props.outlineColor.b );
      props.outlineWidth = qMax( (int)actor->GetOutlineWidth(), 1 );

      actor->GetFacesOrientationColor( props.orientationColor.r,
                                       props.orientationColor.g,
                                       props.orientationColor.b );
      props.orientationScale = actor->GetFacesOrientationScale();
      props.orientation3d = actor->GetFacesOrientation3DVectors();

      props.shrinkFactor = actor->GetShrinkFactor();

      props.opacity = actor->GetOpacity();
    }
    return props;
  }

  ///////////////////////////////////////////////////////////////
  /// \internal
  /// \brief Set mesh object's visual properties.
  /// \param view Pointer to the view window.
  /// \param uid Mesh object's study UID.
  /// \param props Properties data structure.
  ///////////////////////////////////////////////////////////////
  void setProperties(SALOME_View* view, const char* uid, const Properties& props)
  {
    SMESH_Actor* actor = actorFromView( view, uid );
    if ( actor )
    {
      actor->SetNodeColor( props.nodeColor.r,
                           props.nodeColor.g,
                           props.nodeColor.b );
      if ( props.markerType != VTK::MT_USER )
        actor->SetMarkerStd( props.markerType, props.markerScale );

      actor->SetEdgeColor( props.edgeColor.r,
                           props.edgeColor.g,
                           props.edgeColor.b );
      actor->SetLineWidth( qMax( (double)props.edgeWidth, 1. ) );

      actor->SetSufaceColor( props.surfaceColor.r,
                             props.surfaceColor.g,
                             props.surfaceColor.b,
                             props.surfaceColor.delta );

      actor->SetVolumeColor( props.volumeColor.r,
                             props.volumeColor.g,
                             props.volumeColor.b,
                             props.volumeColor.delta );

      actor->Set0DColor( props.elem0dColor.r,
                         props.elem0dColor.g,
                         props.elem0dColor.b );
      actor->Set0DSize( qMax( (double)props.elem0dSize, 1. ) );

      actor->SetBallColor( props.ballColor.r,
                           props.ballColor.g,
                           props.ballColor.b );
      actor->SetBallScale( qMax( props.ballScale, 1e-2 ) );

      actor->SetOutlineColor( props.outlineColor.r,
                              props.outlineColor.g,
                              props.outlineColor.b );
      actor->SetOutlineWidth( qMax( (double)props.outlineWidth, 1. ) );

      actor->SetFacesOrientationColor( props.orientationColor.r,
                                       props.orientationColor.g,
                                       props.orientationColor.b );
      actor->SetFacesOrientationScale( props.orientationScale );
      actor->SetFacesOrientation3DVectors( props.orientation3d );

      actor->SetShrinkFactor( props.shrinkFactor );

      actor->SetOpacity( props.opacity );

      view->Repaint();
    }
  }
} // end of anonymous namespace

/////////////////////////////////////////////////////////////////
/// \enum EntityMode
/// \brief Enumeration for mesh entities.
/// \var EntityMode Entity0d
/// \brief 0D elements.
/// \var EntityMode EntityEdges
/// \brief Edges.
/// \var EntityMode EntityFaces
/// \brief Faces.
/// \var EntityMode EntityVolumes
/// \brief Volumes.
/// \var EntityMode EntityBalls
/// \brief Ball elements.
/// \var EntityMode EntityAll
/// \brief All elements.
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/// \enum SelectionMode
/// \brief Selection mode.
/// \var SelectionMode Undefined
/// \brief Undefined selection mode.
/// \var SelectionMode Node
/// \brief Selection of mesh nodes.
/// \var SelectionMode Cell
/// \brief Selection of any mesh cells.
/// \var SelectionMode EdgeOfCell
/// \brief Selection of pseudo-edges specified by couple of nodes.
/// \var SelectionMode Edge
/// \brief Selection of edges.
/// \var SelectionMode Face
/// \brief Selection of faces.
/// \var SelectionMode Volume
/// \brief Selection of volumes
/// \var SelectionMode Actor
/// \brief Selection of whole actors (meshes, sub-meshes, groups).
/// \var SelectionMode Elem0D
/// \brief Selection of 0D elements.
/// \var SelectionMode Ball
/// \brief Selection of ball ellements.
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/// \enum DisplayMode
/// \brief Display mode.
/// \var DisplayMode UndefinedMode
/// \brief Undefined display mode.
/// \var DisplayMode PointMode
/// \brief Point representation.
/// \var DisplayMode EdgeMode
/// \brief Wireframe representation.
/// \var DisplayMode SurfaceMode
/// \brief Surface representation.
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/// \struct ColorData
/// \brief Color data, in RGBf format.
/// \var ColorData::r
/// \brief Red color's component (0.0:1.0).
/// \var ColorData::g
/// \brief Green color's component (0.0:1.0).
/// \var ColorData::b
/// \brief Blue color's component (0.0:1.0).
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/// \brief Constructor.
/////////////////////////////////////////////////////////////////
ColorData::ColorData()
  : r( 0 ), g( 0 ), b( 0 )
{}

/////////////////////////////////////////////////////////////////
/// \struct BicolorData
/// \brief Bi-color data, in RGBf format.
/// \var BicolorData::r
/// \brief Red color's component (0.0:1.0).
/// \var BicolorData::g
/// \brief Green color's component (0.0:1.0).
/// \var BicolorData::b
/// \brief Blue color's component (0.0:1.0).
/// \var BicolorData::delta
/// \brief Shift for backface color (-100:100).
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/// \brief Constructor.
/////////////////////////////////////////////////////////////////
BicolorData::BicolorData()
  : r( 0 ), g( 0 ), b( 0 ), delta( 0 )
{}

/////////////////////////////////////////////////////////////////
/// \struct Properties
/// \brief Mesh object presentation's properties.
/// \var Properties::nodeColor
/// \brief Node color.
/// \var Properties::markerType
/// \brief Node standard marker type.
/// \var Properties::markerScale
/// \brief Node scale factor.
/// \var Properties::edgeColor
/// \brief Edges color.
/// \var Properties::edgeWidth
/// \brief Edges width.
/// \var Properties::surfaceColor
/// \brief Faces color.
/// \var Properties::volumeColor
/// \brief Volumes color.
/// \var Properties::elem0dColor
/// \brief 0D elements color.
/// \var Properties::elem0dSize
/// \brief 0D elements size.
/// \var Properties::ballColor
/// \brief Ball elements color.
/// \var Properties::ballScale
/// \brief Ball elements scale factor.
/// \var Properties::outlineColor
/// \brief Outlines color.
/// \var Properties::outlineWidth
/// \brief Outlines width.
/// \var Properties::orientationColor
/// \brief Face orientation vectors color.
/// \var Properties::orientationScale
/// \brief Face orientation vectors scale factor.
/// \var Properties::orientation3d
/// \brief Face orientation vectors 3d flag.
/// \var Properties::shrinkFactor
/// \brief Shrink coefficient.
/// \var Properties::opacity
/// \brief Opacity.
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/// \brief Constructor.
/////////////////////////////////////////////////////////////////
Properties::Properties()
  : markerType( VTK::MT_NONE ), markerScale( VTK::MS_NONE ),
    edgeWidth( 1 ), elem0dSize( 1 ), ballScale( 1 ), outlineWidth( 1 ),
    orientationScale( 0 ), orientation3d( false ), shrinkFactor( 0 ),
    opacity( 1 )
{}

/////////////////////////////////////////////////////////////////
/// \typedef nodeColorStruct
/// \deprecated Use ColorData instead.
/// \typedef edgeColorStruct
/// \deprecated Use ColorData instead.
/// \typedef surfaceColorStruct
/// \deprecated Use BicolorData instead.
/// \typedef volumeColorStruct
/// \deprecated Use BicolorData instead.
/// \typedef actorAspect
/// \deprecated Use Properties instead.
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/// \class SMESH_Swig
/// \brief %SMESH GUI Python interface.
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/// \brief Constructor.
/////////////////////////////////////////////////////////////////
SMESH_Swig::SMESH_Swig()
{
  init();
}

/////////////////////////////////////////////////////////////////
/// \brief Destructor.
/////////////////////////////////////////////////////////////////
SMESH_Swig::~SMESH_Swig()
{
}

/////////////////////////////////////////////////////////////////
/// \internal
/// \brief Initialize interface.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::init()
{
  class TInitEvent: public SALOME_Event
  {
  public:
    TInitEvent() {}
    virtual void Execute()
    {
      SUIT_Session* session = SUIT_Session::session();
      SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( session->activeApplication() );

      if ( !SMESHGUI::GetSMESHGUI() )
        app->loadModule( "Mesh" );
    }
  };

  // load SMESH GUI if it's not yet loaded
  if ( SUIT_Session::session() )
  {
    ProcessVoidEvent( new TInitEvent() );
    SMESHGUI::GetSMESHGen()->UpdateStudy();
  }
}

/////////////////////////////////////////////////////////////////
/// \brief Publish object.
/// \param ior IOR of the mesh object to publish.
/// \param name Study name of the object; if not given,
///             name is assigned automatically.
/// \return UID of the data object.
/////////////////////////////////////////////////////////////////
const char* SMESH_Swig::publish(const char* ior, const char* name)
{
  init();

  std::string uid;
  CORBA::Object_var object = string2object( ior );
  if ( !CORBA::is_nil( object ) )
    {
      SALOMEDS::SObject_var sobject =
        SMESHGUI::GetSMESHGen()->PublishInStudy( SALOMEDS::SObject::_nil(),
                                                 object.in(),
                                                 name );
      if ( !CORBA::is_nil( sobject ) )
        {
          uid = sobject->GetID();
        }
      sobject->UnRegister();
    }

  return strdup( uid.c_str() );
}

/////////////////////////////////////////////////////////////////
/// \brief Set new study name of given object.
/// \param uid Object's study UID or IOR.
/// \param name New name of the object.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::rename(const char* uid, const char* name)
{
  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    sobject->SetAttrString( "AttributeName", name );
}

/////////////////////////////////////////////////////////////////
/// \brief Display mesh object.
/// \param uid Object's study UID or IOR.
/// \param viewUid View window UID (0 means currently active view
///                window; if there's no view, it is created).
///                Default: 0.
/// \param updateViewer "Update view" flag. Default: \c true.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::display(const char* uid, int viewUid, bool updateViewer)
{
  class TDisplayEvent: public SALOME_Event
  {
  private:
    const char* myUid;
    int myViewUid;
    bool myIsUpdate;
  public:
    TDisplayEvent(const char* uid, int viewUid, bool updateViewer)
      : myUid( uid ), myViewUid( viewUid ), myIsUpdate( updateViewer ) {}
    virtual void Execute()
    {
      SALOME_View* view = uid2wnd( myViewUid, true ); // create view if it's not present
      if ( view )
        LightApp_Displayer::FindDisplayer( "Mesh", true )->Display( myUid, myIsUpdate, view );
    }
  };

  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ProcessVoidEvent( new TDisplayEvent( sobject->GetID().c_str(), viewUid, updateViewer ) );
}

/////////////////////////////////////////////////////////////////
/// \brief Erase mesh object.
/// \param uid Object's study UID or IOR.
/// \param viewUid View window UID (0 means currently active view
///                window); -1 means "all view windows".
///                Default: 0.
/// \param updateViewer "Update view" flag. Default: \c true.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::erase(const char* uid, int viewUid, bool updateViewer)
{
  class TEraseEvent: public SALOME_Event
  {
  private:
    const char* myUid;
    int myViewUid;
    bool myIsUpdate;
  public:
    TEraseEvent(const char* uid, int viewUid, bool updateViewer)
      : myUid( uid ), myViewUid( viewUid ), myIsUpdate( updateViewer ) {}
    virtual void Execute()
    {
      if ( myViewUid == -1 )
      {
        QList<SALOME_View*> views = windows();
        foreach( SALOME_View* view, views )
          LightApp_Displayer::FindDisplayer( "Mesh", true )->Erase( myUid, true, myIsUpdate, view );
      }
      else
      {
        SALOME_View* view = uid2wnd( myViewUid );
        if ( view )
          LightApp_Displayer::FindDisplayer( "Mesh", true )->Erase( myUid, true, myIsUpdate, view );
      }
    }
  };

  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ProcessVoidEvent( new TEraseEvent( sobject->GetID().c_str(), viewUid, updateViewer ) );
}

/////////////////////////////////////////////////////////////////
/// \brief Update mesh object.
/// \param uid Object's study UID or IOR.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::update(const char* uid)
{
  class TUpdateEvent: public SALOME_Event
  {
  private:
    const char* myUid;
  public:
    TUpdateEvent( const char* uid ) : myUid( uid ) {}
    virtual void Execute()
    {
      Handle(SALOME_InteractiveObject) io =
        new SALOME_InteractiveObject( myUid, "SMESH", "" );
      SMESH::Update( io, true );
    }
  };

  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ProcessVoidEvent( new TUpdateEvent( uid ) );
}

/////////////////////////////////////////////////////////////////
/// \internal
class TGetPropsEvent: public SALOME_Event
{
public:
  typedef Properties TResult;
  TResult myResult;
  const char* myUid;
  int myViewUid;

  TGetPropsEvent( const char* uid, int viewUid )
    : myUid( uid ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    myResult = properties( view, myUid );
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Get mesh object's visual properties.
/// \param uid Mesh object's study UID or IOR.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/// \return Properties data structure.
/////////////////////////////////////////////////////////////////
Properties SMESH_Swig::properties(const char* uid, int viewUid)
{
  Properties props;

  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    props = ProcessEvent( new TGetPropsEvent( sobject->GetID().c_str(), viewUid ) );

  return props;
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TSetPropsEvent: public SALOME_Event
{
public:
  const char* myUid;
  Properties myProps;
  int myViewUid;

  TSetPropsEvent( const char* uid, const Properties& props, int viewUid )
    : myUid( uid ), myProps( props), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    if ( myViewUid == -1 )
    {
      QList<SALOME_View*> views = windows();
      foreach( SALOME_View* view, views )
      {
        setProperties( view, myUid, myProps );
      }
    }
    else
    {
      SALOME_View* view = uid2wnd( myViewUid );
      setProperties( view, myUid, myProps );
    }
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Set mesh object's visual properties.
/// \param uid Mesh object's study UID or IOR.
/// \param props Properties data structure.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::setProperties(const char* uid, const Properties& props, int viewUid)
{
  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ProcessVoidEvent( new TSetPropsEvent( sobject->GetID().c_str(), props, viewUid ) );
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TGetNodeNumberingEvent: public SALOME_Event
{
public:
  typedef bool TResult;
  TResult myResult;
  const char* myUid;
  int myViewUid;

  TGetNodeNumberingEvent( const char* uid, int viewUid )
    : myResult( false ), myUid( uid ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    SMESH_Actor* actor = actorFromView( view, myUid );
    if ( actor )
      myResult = actor->GetPointsLabeled();
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Check if nodes numbering is switched on.
/// \param uid Mesh object's study UID or IOR.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/// \return \c true if nodes numbering is switched on;
///         \c false otherwise.
/////////////////////////////////////////////////////////////////
bool SMESH_Swig::nodesNumbering(const char* uid, int viewUid)
{
  bool numbering = false;

  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    numbering = ProcessEvent( new TGetNodeNumberingEvent( sobject->GetID().c_str(), viewUid ) );

  return numbering;
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TSetNodeNumberingEvent: public SALOME_Event
{
public:
  const char* myUid;
  bool myNumbering;
  int myViewUid;

  TSetNodeNumberingEvent( const char* uid, bool numbering, int viewUid )
    : myUid( uid ), myNumbering( numbering ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    SMESH_Actor* actor = actorFromView( view, myUid );
    if ( actor )
    {
      actor->SetPointsLabeled( myNumbering );
      if ( view )
        view->Repaint();
    }
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Switch nodes numbering on/off.
/// \param uid Mesh object's study UID or IOR.
/// \param numbering \c true to switch nodes numbering on;
///                  \c false otherwise.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::setNodesNumbering(const char* uid, bool numbering, int viewUid)
{
  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ProcessVoidEvent( new TSetNodeNumberingEvent( sobject->GetID().c_str(), numbering, viewUid ) );
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TGetElementNumberingEvent: public SALOME_Event
{
public:
  typedef bool TResult;
  TResult myResult;
  const char* myUid;
  int myViewUid;

  TGetElementNumberingEvent( const char* uid, int viewUid )
    : myResult( false ), myUid( uid ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    SMESH_Actor* actor = actorFromView( view, myUid );
    if ( actor )
      myResult = actor->GetCellsLabeled();
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Check if elements numbering is switched on.
/// \param uid Mesh object's study UID or IOR.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/// \return \c true if elements numbering is switched on;
///         \c false otherwise.
/////////////////////////////////////////////////////////////////
bool SMESH_Swig::elementsNumbering(const char* uid, int viewUid)
{
  bool numbering = false;

  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    numbering = ProcessEvent( new TGetElementNumberingEvent( sobject->GetID().c_str(), viewUid ) );

  return numbering;
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TSetElementNumberingEvent: public SALOME_Event
{
public:
  const char* myUid;
  bool myNumbering;
  int myViewUid;

  TSetElementNumberingEvent( const char* uid, bool numbering, int viewUid )
    : myUid( uid ), myNumbering( numbering ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    SMESH_Actor* actor = actorFromView( view, myUid );
    if ( actor )
    {
      actor->SetCellsLabeled( myNumbering );
      if ( view )
        view->Repaint();
    }
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Switch elements numbering on/off.
/// \param uid Mesh object's study UID or IOR.
/// \param numbering \c true to switch elements numbering on;
///                  \c false otherwise.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::setElementsNumbering(const char* uid, bool numbering, int viewUid)
{
  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ProcessVoidEvent( new TSetElementNumberingEvent( sobject->GetID().c_str(), numbering, viewUid ) );
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TGetDisplayModeEvent: public SALOME_Event
{
public:
  typedef DisplayMode TResult;
  TResult myResult;
  const char* myUid;
  int myViewUid;

  TGetDisplayModeEvent( const char* uid, int viewUid )
    : myResult( UndefinedMode ), myUid( uid ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    SMESH_Actor* actor = actorFromView( view, myUid );
    if ( actor )
      myResult = (DisplayMode)actor->GetRepresentation();
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Get mesh object's display mode.
/// \param uid Mesh object's study UID or IOR.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/// \return Display mode (UndefinedMode if actor isn't found).
/////////////////////////////////////////////////////////////////
DisplayMode SMESH_Swig::displayMode(const char* uid, int viewUid)
{
  DisplayMode mode = UndefinedMode;

  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    mode = ProcessEvent( new TGetDisplayModeEvent( sobject->GetID().c_str(), viewUid ) );

  return mode;
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TSetDisplayModeEvent: public SALOME_Event
{
public:
  const char* myUid;
  DisplayMode myMode;
  int myViewUid;

  TSetDisplayModeEvent( const char* uid, DisplayMode mode, int viewUid )
    : myUid( uid ), myMode( mode), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    SMESH_Actor* actor = actorFromView( view, myUid );
    if ( actor && myMode != UndefinedMode )
    {
      actor->SetRepresentation( myMode );
      if ( view )
        view->Repaint();
    }
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Set mesh object's display mode.
/// \param uid Mesh object's study UID or IOR.
/// \param mode Display mode.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::setDisplayMode(const char* uid, DisplayMode mode, int viewUid)
{
  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ProcessVoidEvent( new TSetDisplayModeEvent( sobject->GetID().c_str(), mode, viewUid ) );
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TGetShrinkModeEvent: public SALOME_Event
{
public:
  typedef bool TResult;
  TResult myResult;
  const char* myUid;
  int myViewUid;

  TGetShrinkModeEvent( const char* uid, int viewUid )
    : myResult( false ), myUid( uid ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    SMESH_Actor* actor = actorFromView( view, myUid );
    if ( actor )
      myResult = actor->IsShrunk();
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Check if shrink mode is switched on.
/// \param uid Mesh object's study UID or IOR.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/// \return \c true if shrink mode is switched on;
///         \c false otherwise.
/////////////////////////////////////////////////////////////////
bool SMESH_Swig::shrinkMode(const char* uid, int viewUid)
{
  bool shrinkMode = false;

  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    shrinkMode = ProcessEvent( new TGetShrinkModeEvent( sobject->GetID().c_str(), viewUid ) );

  return shrinkMode;
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TSetShrinkModeEvent: public SALOME_Event
{
public:
  const char* myUid;
  bool myShrink;
  int myViewUid;

  TSetShrinkModeEvent( const char* uid, bool shrink, int viewUid )
    : myUid( uid ), myShrink( shrink ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    SMESH_Actor* actor = actorFromView( view, myUid );
    if ( actor )
    {
      if ( myShrink )
        actor->SetShrink();
      else
        actor->UnShrink();
      if ( view )
        view->Repaint();
    }
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Switch shrink mode on/off.
/// \param uid Mesh object's study UID or IOR.
/// \param shrink \c true to switch shrink mode on;
///               \c false otherwise.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::setShrinkMode(const char* uid, bool shrink, int viewUid)
{
  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ProcessVoidEvent( new TSetShrinkModeEvent( sobject->GetID().c_str(), shrink, viewUid ) );
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TGetOpacityEvent: public SALOME_Event
{
public:
  typedef double TResult;
  TResult myResult;
  const char* myUid;
  int myViewUid;

  TGetOpacityEvent( const char* uid, int viewUid )
    : myResult( 0 ), myUid( uid ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    SMESH_Actor* actor = actorFromView( view, myUid );
    if ( actor )
      myResult = actor->GetOpacity();
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Get mesh object's opacity.
/// \param uid Mesh object's study UID or IOR.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/// \return Opacity value.
/////////////////////////////////////////////////////////////////
double SMESH_Swig::opacity(const char* uid, int viewUid)
{
  double opacity = 0;

  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    opacity = ProcessEvent( new TGetOpacityEvent( sobject->GetID().c_str(), viewUid ) );

  return opacity;
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TSetOpacityEvent: public SALOME_Event
{
public:
  const char* myUid;
  double myOpacity;
  int myViewUid;

  TSetOpacityEvent( const char* uid, double opacity, int viewUid )
    : myUid( uid ), myOpacity( opacity ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    SMESH_Actor* actor = actorFromView( view, myUid );
    if ( actor )
    {
      actor->SetOpacity( myOpacity );
      if ( view )
        view->Repaint();
    }
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Set mesh object's opacity.
/// \param uid Mesh object's study UID or IOR.
/// \param opacity Opacity value.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::setOpacity(const char* uid, double opacity, int viewUid)
{
  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ProcessVoidEvent( new TSetOpacityEvent( sobject->GetID().c_str(), opacity, viewUid ) );
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TGetOrientationEvent: public SALOME_Event
{
public:
  typedef bool TResult;
  TResult myResult;
  const char* myUid;
  int myViewUid;

  TGetOrientationEvent( const char* uid, int viewUid )
    : myResult( false ), myUid( uid ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    SMESH_Actor* actor = actorFromView( view, myUid );
    if ( actor )
      myResult = actor->GetFacesOriented();
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Check if faces orientation vectors are shown.
/// \param uid Mesh object's study UID or IOR.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/// \return \c true if faces orientation vectors are shown;
///         \c false otherwise.
/////////////////////////////////////////////////////////////////
bool SMESH_Swig::isOrientationShown(const char* uid, int viewUid)
{
  bool shown = false;

  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    shown = ProcessEvent( new TGetOrientationEvent( sobject->GetID().c_str(), viewUid ) );

  return shown;
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TSetOrientationEvent: public SALOME_Event
{
public:
  const char* myUid;
  bool myShown;
  int myViewUid;

  TSetOrientationEvent( const char* uid, bool shown, int viewUid )
    : myUid( uid ), myShown( shown ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    SMESH_Actor* actor = actorFromView( view, myUid );
    if ( actor )
    {
      actor->SetFacesOriented( myShown );
      if ( view )
        view->Repaint();
    }
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Show/hide faces orientation vectors.
/// \param uid Mesh object's study UID or IOR.
/// \param shown \c true to show faces orientation vectors;
///              \c false otherwise.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::setOrientationShown(const char* uid, bool shown, int viewUid)
{
  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ProcessVoidEvent( new TSetOrientationEvent( sobject->GetID().c_str(), shown, viewUid ) );
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TGetEntitiesEvent: public SALOME_Event
{
public:
  typedef int TResult;
  TResult myResult;
  const char* myUid;
  int myViewUid;

  TGetEntitiesEvent( const char* uid, int viewUid )
    : myResult( EntityNone ), myUid( uid ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    SMESH_Actor* actor = actorFromView( view, myUid );
    if ( actor )
      myResult = actor->GetEntityMode();
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Get mesh object's visible entities.
/// \param uid Mesh object's study UID or IOR.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/// \return Enumerator describing entities being visible.
/////////////////////////////////////////////////////////////////
int SMESH_Swig::entitiesShown(const char* uid, int viewUid)
{
  int entities = EntityNone;

  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    entities = ProcessEvent( new TGetEntitiesEvent( sobject->GetID().c_str(), viewUid ) );

  return entities;
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TSetEntitiesEvent: public SALOME_Event
{
public:
  const char* myUid;
  int myEntities;
  int myViewUid;

  TSetEntitiesEvent( const char* uid, int entities, int viewUid )
    : myUid( uid ), myEntities( entities ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SALOME_View* view = uid2wnd( myViewUid );
    SMESH_Actor* actor = actorFromView( view, myUid );
    if ( actor )
    {
      actor->SetEntityMode( myEntities );
      if ( view )
        view->Repaint();
    }
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Set mesh object's visible entities.
/// \param uid Mesh object's study UID or IOR.
/// \param entities Enumerator describing entities to be shown.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::setEntitiesShown(const char* uid, int entities, int viewUid)
{
  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ProcessVoidEvent( new TSetEntitiesEvent( sobject->GetID().c_str(), entities, viewUid ) );
}

/////////////////////////////////////////////////////////////////
/// \brief Check if given mesh object's entity is shown.
/// \param uid Mesh object's study UID or IOR.
/// \param entity Mesh entity.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/// \return \c true if entity is shown; \c false otherwise.
/////////////////////////////////////////////////////////////////
bool SMESH_Swig::isEntityShown(const char* uid, EntityMode entity, int viewUid)
{
  bool shown = false;

  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
  {
    int entities = ProcessEvent( new TGetEntitiesEvent( sobject->GetID().c_str(), viewUid ) );
    shown = (bool)( entities & entity );
  }

  return shown;
}

/////////////////////////////////////////////////////////////////
/// \brief Show/hide entity for given mesh object.
/// \param uid Mesh object's study UID or IOR.
/// \param entity Mesh entity.
/// \param show Visibility status.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::setEntityShown(const char* uid, EntityMode entity, bool show, int viewUid)
{
  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
  {
    int entities = ProcessEvent( new TGetEntitiesEvent( sobject->GetID().c_str(), viewUid ) );
    if ( show )
      entities |= entity;
    else
      entities &= ~entity;
    ProcessVoidEvent( new TSetEntitiesEvent( sobject->GetID().c_str(), entities, viewUid ) );
  }
}

/////////////////////////////////////////////////////////////////
/// \brief Initialize %SMESH GUI Python interface.
/// \deprecated Interface is initialized automatically.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::Init()
{
  deprecated("SMESH_Swig::Init");
  // does nothing; initialization is done automatically.
}

/////////////////////////////////////////////////////////////////
/// \brief Publish mesh in the active study.
/// \deprecated Publishing is done automatically.
/// \param ior IOR of the mesh.
/// \param name Name of the mesh (optional).
/// \return UID of the data object.
/////////////////////////////////////////////////////////////////
const char* SMESH_Swig::AddNewMesh(const char* ior, const char* name)
{
  deprecated("SMESH_Swig::AddNewMesh", "SMESH_Swig::publish");
  return publish( ior, name );
}

/////////////////////////////////////////////////////////////////
/// \brief Publish hypothesis in the active study.
/// \deprecated Publishing is done automatically.
/// \param ior IOR of the hypothesis.
/// \param name Name of the hypothesis (optional).
/// \return UID of the data object.
/////////////////////////////////////////////////////////////////
const char* SMESH_Swig::AddNewHypothesis(const char* ior, const char* name)
{
  deprecated("SMESH_Swig::AddNewHypothesis", "SMESH_Swig::publish");
  return publish( ior, name );
}

/////////////////////////////////////////////////////////////////
/// \brief Publish algorithm in the active study.
/// \deprecated Publishing is done automatically.
/// \param ior IOR of the algorithm.
/// \param name Name of the algorithm (optional).
/// \return UID of the data object.
/////////////////////////////////////////////////////////////////
const char* SMESH_Swig::AddNewAlgorithm(const char* ior, const char* name)
{
  deprecated("SMESH_Swig::AddNewAlgorithm", "SMESH_Swig::publish");
  return publish( ior, name );
}

/////////////////////////////////////////////////////////////////
/// \deprecated Publishing is done automatically.
/// \deprecated Synonym of AddNewAlgorithm().
/// \param ior IOR of the algorithm.
/// \param name Name of the algorithm (optional).
/// \return UID of the data object.
/////////////////////////////////////////////////////////////////
const char* SMESH_Swig::AddNewAlgorithms(const char* ior, const char* name)
{
  deprecated("SMESH_Swig::AddNewAlgorithms", "SMESH_Swig::publish");
  return publish( ior, name );
}

/////////////////////////////////////////////////////////////////
/// \brief Add reference on a shape for mesh in a study.
/// \deprecated Publishing is done automatically.
/// \param shapeUid GEOM shape's study UID (not used).
/// \param meshUid Mesh's study UID (not used).
/////////////////////////////////////////////////////////////////
void SMESH_Swig::SetShape(const char* /*shapeUid*/, const char* /*meshUid*/)
{
  deprecated("SMESH_Swig::SetShape", "SMESH_Swig::publish");
  // does nothing: publishing is done automatically
}

/////////////////////////////////////////////////////////////////
/// \brief Assign hypothesis to mesh or sub-mesh.
/// \deprecated Publishing is done automatically.
/// \param meshUid Mesh's or sub-mesh's study UID (not used).
/// \param hypoUID Hypothesis's study UID (not used).
/////////////////////////////////////////////////////////////////
void SMESH_Swig::SetHypothesis(const char* /*meshUid*/, const char* /*hypoUID*/)
{
  deprecated("SMESH_Swig::SetHypothesis", "SMESH_Swig::publish");
  // does nothing: publishing is done automatically
}

/////////////////////////////////////////////////////////////////
/// \brief Assign algorithm to mesh or sub-mesh.
/// \deprecated Publishing is done automatically.
/// \param meshUid Mesh's or sub-mesh's study UID (not used).
/// \param algoUID Algorithm's study UID (not used).
/////////////////////////////////////////////////////////////////
void SMESH_Swig::SetAlgorithms(const char* /*meshUid*/, const char* /*algoUID*/)
{
  deprecated("SMESH_Swig::SetAlgorithms", "SMESH_Swig::publish");
  // does nothing: publishing is done automatically
}

/////////////////////////////////////////////////////////////////
/// \brief Anassign hypothesis or algorithm from mesh or sub-mesh.
/// \deprecated Unpublishing is done automatically.
/// \param uid Hypothesis's or algorithm's study UID (not used).
/////////////////////////////////////////////////////////////////
void SMESH_Swig::UnSetHypothesis(const char* /*uid*/)
{
  deprecated("SMESH_Swig::UnSetHypothesis");
  // does nothing: unpublishing is done automatically
}

/////////////////////////////////////////////////////////////////
/// \brief Publish sub-mesh in the active study.
/// \deprecated Publishing is done automatically.
/// \param meshUid Parent mesh's study UID (not used).
/// \param ior IOR of the sub-mesh.
/// \param shapeType GEOM shape's type (not used).
/// \param name Name of the sub-mesh (optional).
/// \return UID of the data object.
/////////////////////////////////////////////////////////////////
const char* SMESH_Swig::AddSubMesh(const char* /*meshUid*/,
                                   const char* ior,
                                   int /*shapeType*/,
                                   const char* name)
{
  deprecated("SMESH_Swig::AddSubMesh", "SMESH_Swig::publish");
  return publish( ior, name );
}

/////////////////////////////////////////////////////////////////
/// \brief Publish sub-mesh in the active study.
/// \deprecated Publishing is done automatically.
/// \param meshUid Parent mesh's study UID (not used).
/// \param shapeUid GEOM shape's study UID (not used).
/// \param ior IOR of the sub-mesh.
/// \param shapeType GEOM shape's type (not used).
/// \param name Name of the sub-mesh (optional).
/// \return UID of the data object.
/////////////////////////////////////////////////////////////////
const char* SMESH_Swig::AddSubMeshOnShape(const char* /*meshUid*/,
                                          const char* /*shapeUid*/,
                                          const char* ior,
                                          int /*shapeType*/,
                                          const char* name)
{
  deprecated("SMESH_Swig::AddSubMeshOnShape", "SMESH_Swig::publish");
  return publish( ior, name );
}

/////////////////////////////////////////////////////////////////
/// \brief Set new study name of given object.
/// \deprecated Use rename() method.
/// \param uid Object's study UID or IOR.
/// \param name New name of the object.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::SetName(const char* uid, const char* name)
{
  deprecated("SMESH_Swig::SetName", "SMESH_Swig::rename");
  rename( uid, name );
}

/////////////////////////////////////////////////////////////////
/// \brief Set mesh icon according to compute status
/// \deprecated Publishing is done automatically.
/// \param meshUid Mesh's study UID (not used).
/// \param isComputed Flag pointing that mesh is computed or no
///        (not used).
/// \param isEmpty Flag pointing that mesh is empty or no
///        (not used).
/////////////////////////////////////////////////////////////////
void SMESH_Swig::SetMeshIcon(const char* /*meshUid*/,
                             const bool /*isComputed*/,
                             const bool /*isEmpty*/)
{
  deprecated("SMESH_Swig::SetMeshIcon", "SMESH_Swig::publish");
  // does nothing: publishing is done automatically
}

/////////////////////////////////////////////////////////////////
/// Display mesh in the currently active view window.
/// \deprecated Use display() method.
/// \param meshUid Mesh's study UID.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::CreateAndDisplayActor(const char* meshUid)
{
  deprecated("SMESH_Swig::CreateAndDisplayActor", "SMESH_Swig::display");
  display( meshUid );
}

/////////////////////////////////////////////////////////////////
/// Erase mesh in the view window(s).
/// \deprecated Use erase() method.
/// \param meshUid Mesh's study UID.
/// \param allViewers If \c true, mesh is removed from all views.
///                   Default: \c false.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::EraseActor(const char* meshUid, const bool allViewers)
{
  deprecated("SMESH_Swig::EraseActor", "SMESH_Swig::erase");
  erase( meshUid, allViewers ? -1 : 0 );
}

/////////////////////////////////////////////////////////////////
/// Update mesh object.
/// \deprecated Use update() method.
/// \param meshUid Mesh's study UID.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::UpdateActor(const char* meshUid)
{
  deprecated("SMESH_Swig::UpdateActor", "SMESH_Swig::update");
  update( meshUid );
}

/////////////////////////////////////////////////////////////////
/// Get mesh object's properties.
/// \deprecated Use properties() method.
/// \param meshUid Mesh's study UID.
/// \param viewUid View window UID (0 means currently active view
///                window). Default: 0.
/////////////////////////////////////////////////////////////////
actorAspect SMESH_Swig::GetActorAspect(const char* meshUid, int viewUid)
{
  deprecated("SMESH_Swig::GetActorAspect", "SMESH_Swig::properties");
  return properties( meshUid, viewUid );
}

/////////////////////////////////////////////////////////////////
/// Set mesh object's properties.
/// \deprecated Use setProperties() method.
/// \param meshUid Mesh's study UID.
/// \param viewUid View window UID (0 means currently active view
///                window). Default: 0.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::SetActorAspect(const actorAspect& aspect, const char* meshUid, int viewUid)
{
  deprecated("SMESH_Swig::SetActorAspect", "SMESH_Swig::setProperties");
  setProperties( meshUid, aspect, viewUid );
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TSelectListEvent: public SALOME_Event
{
  const char* myUid;
  std::vector<int> myIds;
  bool myIsAppend;

public:
  TSelectListEvent(const char* uid, std::vector<int> ids, bool append)
    : myUid( uid ), myIds( ids ), myIsAppend( append )
  {}

  virtual void Execute()
  {
    LightApp_SelectionMgr* selMgr = 0;
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( SUIT_Session::session()->activeApplication() );
    if ( app )
      selMgr = dynamic_cast<LightApp_SelectionMgr*>( app->selectionMgr() );

    if ( !selMgr )
      return;

    selMgr->clearFilters();

    SVTK_ViewWindow* vw = SMESH::GetViewWindow();
    if ( !vw )
      return;

    SMESH_Actor* actor = SMESH::FindActorByEntry( myUid );

    if ( !actor || !actor->hasIO() )
      return;

    Handle(SALOME_InteractiveObject) io = actor->getIO();
    SALOME_ListIO ios;
    ios.Append( io );
    selMgr->setSelectedObjects( ios, false );

    if ( vw->SelectionMode() == ActorSelection )
      return;

    SVTK_TVtkIDsMap idMap;
    std::vector<int>::const_iterator it;
    for ( it = myIds.begin(); it != myIds.end(); ++it )
    {
      idMap.Add( *it );
    }

    // Set new selection
    SVTK_Selector* selector = vw->GetSelector();
    selector->AddOrRemoveIndex( io, idMap, myIsAppend );
    vw->highlight( io, true, true );
    vw->GetInteractor()->onEmitSelectionChanged();
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Select elements of the mesh, sub-mesh or group.
/// \param uid Mesh object's study UID or IOR.
/// \param ids List of mesh elements.
/// \param append If \c true, elements are added to current
///               selection; otherwise, previous selection is
///               cleared.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::select(const char* uid, std::vector<int> ids, bool append)
{
  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ProcessVoidEvent( new TSelectListEvent( sobject->GetID().c_str(), ids, append ) );
}

/////////////////////////////////////////////////////////////////
/// \brief Select element of the mesh, sub-mesh or group.
/// \param uid Mesh object's study UID or IOR.
/// \param id Mesh element.
/// \param append If \c true, element is added to current
///               selection; otherwise, previous selection is
///               cleared.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::select(const char* uid, int id, bool append)
{
  init();

  std::vector<int> ids;
  ids.push_back( id );

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ProcessVoidEvent( new TSelectListEvent( sobject->GetID().c_str(), ids, append ) );
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TSelectListOfPairEvent: public SALOME_Event
{
  const char* myUid;
  std::vector<std::pair<int, int> > myIds;
  bool myIsAppend;

public:
  TSelectListOfPairEvent(const char* uid, std::vector<std::pair<int, int> > ids, bool append)
    : myUid( uid ), myIds( ids ), myIsAppend( append )
  {}

  virtual void Execute()
  {
    LightApp_SelectionMgr* selMgr = 0;
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( SUIT_Session::session()->activeApplication() );
    if ( app )
      selMgr = dynamic_cast<LightApp_SelectionMgr*>( app->selectionMgr() );

    if ( !selMgr )
      return;

    selMgr->clearFilters();

    SVTK_ViewWindow* vw = SMESH::GetViewWindow();
    if ( !vw )
      return;

    SMESH_Actor* actor = SMESH::FindActorByEntry( myUid );

    if ( !actor || !actor->hasIO() )
      return;

    Handle(SALOME_InteractiveObject) io = actor->getIO();
    SALOME_ListIO ios;
    ios.Append( io );
    selMgr->setSelectedObjects( ios, false );

    if ( vw->SelectionMode() != EdgeOfCellSelection )
      return;

    SVTK_IndexedMapOfVtkIds idMap;
    std::vector<std::pair<int, int> >::const_iterator it;
    for ( it = myIds.begin(); it != myIds.end(); ++it )
    {
      std::vector<vtkIdType> pair;
      pair.push_back( (*it).first );
      pair.push_back( (*it).second );
      idMap.Add( pair );
    }

    // Set new selection
    SVTK_Selector* selector = vw->GetSelector();
    selector->AddOrRemoveCompositeIndex( io, idMap, myIsAppend );
    vw->highlight( io, true, true );
    vw->GetInteractor()->onEmitSelectionChanged();
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Select pseudo-edges (specified by two nodes)
///        of the mesh, sub-mesh or group.
/// \param uid Mesh object's study UID or IOR.
/// \param ids List of pairs containing two nodes IDs.
/// \param append If \c true, pseudo-edges are added to current
///               selection; otherwise, previous selection is
///               cleared.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::select(const char* uid, std::vector<std::pair<int,int> > ids, bool append)
{
  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ProcessVoidEvent( new TSelectListOfPairEvent( sobject->GetID().c_str(), ids, append ) );
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TGetSelectionModeEvent: public SALOME_Event
{
public:
  typedef SelectionMode TResult;
  TResult myResult;
  int myViewUid;

  TGetSelectionModeEvent( int viewUid ) :
    myResult( Undefined ), myViewUid( viewUid ) {}

  virtual void Execute()
  {
    SVTK_Viewer* model = dynamic_cast<SVTK_Viewer*>( uid2wnd( myViewUid ) );
    if ( model )
    {
      SVTK_ViewWindow* vw = dynamic_cast<SVTK_ViewWindow*>( model->getViewManager()->getActiveView() );
      if ( vw )
        myResult = (SelectionMode)vw->SelectionMode();
    }
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Get selection mode of view window.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/// \return Current selection mode.
/////////////////////////////////////////////////////////////////
SelectionMode SMESH_Swig::getSelectionMode(int viewUid)
{
  init();
  return ProcessEvent( new TGetSelectionModeEvent( viewUid ) );
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TSetSelectionModeEvent: public SALOME_Event
{
  SelectionMode mySelectionMode;
  int myViewUid;

public:
  TSetSelectionModeEvent( const SelectionMode selectionMode, int viewUid )
    : mySelectionMode( selectionMode ), myViewUid( viewUid )
  {}

  virtual void Execute()
  {
    SVTK_Viewer* model = dynamic_cast<SVTK_Viewer*>( uid2wnd( myViewUid ) );
    if ( model )
    {
      SVTK_ViewWindow* vw = dynamic_cast<SVTK_ViewWindow*>( model->getViewManager()->getActiveView() );
      if ( vw )
      {
        SelectionMode previousMode = (SelectionMode)vw->SelectionMode();
        bool switchPointMode = ( previousMode == Node && mySelectionMode != Node ) ||
          ( previousMode != Node && mySelectionMode == Node );
        if ( switchPointMode )
        {
          vtkRenderer* renderer = vw->getRenderer();
          VTK::ActorCollectionCopy actors( renderer->GetActors() );
          vtkActorCollection* collection = actors.GetActors();
          collection->InitTraversal();
          while ( vtkActor* vtkActor = collection->GetNextActor() )
          {
            if ( SMESH_Actor* actor = dynamic_cast<SMESH_Actor*>( vtkActor ) )
            {
              if ( actor->GetVisibility() )
                actor->SetPointRepresentation( mySelectionMode == Node );
            }
          }
        }
        vw->SetSelectionMode( mySelectionMode );
      }
    }
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Set selection mode to view window.
/// \param viewUid View window UID (0 means currently active view
///                window); Default: 0.
/// \param mode Selection mode.
/////////////////////////////////////////////////////////////////
void SMESH_Swig::setSelectionMode(SelectionMode mode, int viewUid)
{
  init();
  ProcessVoidEvent( new TSetSelectionModeEvent( mode, viewUid ) );
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TGetSelectedEvent: public SALOME_Event
{
public:
  typedef std::vector<int> TResult;
  TResult myResult;
  const char* myUid;

  TGetSelectedEvent( const char* uid )
    : myUid( uid ) {}

  virtual void Execute()
  {
    SVTK_ViewWindow* vw = SMESH::GetViewWindow();
    if ( !vw )
      return;

    SVTK_Selector* selector = vw->GetSelector();
    if ( !selector )
      return;

    SMESH_Actor* actor = SMESH::FindActorByEntry( myUid );
    if ( !actor || !actor->hasIO() )
      return;

    SVTK_TIndexedMapOfVtkId idMap;
    selector->GetIndex( actor->getIO(), idMap );

    for ( int i = 1; i <= idMap.Extent(); i++ )
      myResult.push_back( (int)idMap( i ) );
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Get selected elements of the mesh, sub-mesh or group.
/// \param uid Mesh object's study UID or IOR.
/// \return List of selected mesh elements.
/////////////////////////////////////////////////////////////////
std::vector<int> SMESH_Swig::getSelected(const char* uid)
{
  std::vector<int> ids;

  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    ids = ProcessEvent( new TGetSelectedEvent( sobject->GetID().c_str() ) );

  return ids;
}

/////////////////////////////////////////////////////////////////
/// \internal
/////////////////////////////////////////////////////////////////
class TGetSelectedPairEvent : public SALOME_Event
{
public:
  typedef std::vector<std::pair<int, int> > TResult;
  TResult myResult;
  const char* myUid;

  TGetSelectedPairEvent( const char* uid )
    :  myUid( uid ) {}

  virtual void Execute()
  {
    SVTK_ViewWindow* vw = SMESH::GetViewWindow();
    if ( !vw )
      return;

    if ( vw->SelectionMode() != EdgeOfCellSelection )
      return;

    SVTK_Selector* selector = vw->GetSelector();
    if ( !selector )
      return;

    SMESH_Actor* actor = SMESH::FindActorByEntry( myUid );
    if ( !actor || !actor->hasIO() )
      return;

    SVTK_IndexedMapOfVtkIds idMap;
    selector->GetCompositeIndex( actor->getIO(), idMap );

    for ( int i = 1; i <= idMap.Extent(); i++ ) {
      myResult.push_back( std::make_pair( (int)idMap(i)[0], (int)idMap(i)[1]) );
    }
  }
};

/////////////////////////////////////////////////////////////////
/// \brief Get selected pseudo-edges (specified by two nodes)
///        of the mesh, sub-mesh or group.
/// \param uid Mesh object's study UID or IOR.
/// \param ids List of pairs containing two nodes IDs.
/// \param append If \c true, pseudo-edges are added to current
///               selection; otherwise, previous selection is
///               cleared.
/////////////////////////////////////////////////////////////////
std::vector<std::pair<int,int> > SMESH_Swig::getSelectedEdgeOfCell(const char* uid)
{
  std::vector<std::pair<int,int> > pairs;

  init();

  _PTR(SObject) sobject = uid2object( uid );
  if ( sobject )
    pairs = ProcessEvent( new TGetSelectedPairEvent( sobject->GetID().c_str() ) );

  return pairs;
}
