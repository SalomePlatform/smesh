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
//  File   : SMESH_Swig.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESHGUI_Swig.hxx"

#include "Utils_ORB_INIT.hxx"
#include "Utils_SINGLETON.hxx"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_GEOMGenUtils.h"

// SALOME Includes
#include "SUIT_ResourceMgr.h"
#include "SUIT_Session.h"

#include "SALOMEDS_SObject.hxx"

#include "SalomeApp_Application.h"

#include "utilities.h"

// Open CASCADE Includes
#include <TopoDS.hxx>

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

static CORBA::ORB_var _orb;

static CORBA::Object_ptr StringToObject (const char* ior)
{
  return _orb->string_to_object(ior);
}

SMESH_Swig::SMESH_Swig()
{
  MESSAGE("Constructeur");
  setOrb();
}

void SMESH_Swig::Init(int studyID)
{
  MESSAGE("Init");
  SMESH::SMESH_Gen_var CompMesh = SMESHGUI::GetSMESHGen();
  GEOM::GEOM_Gen_var CompGeom = SMESH::GetGEOMGen();

  SUIT_ResourceMgr* resMgr = SMESHGUI::resourceMgr();
  if ( resMgr ) {
    resMgr->loadLanguage( QString::null, "en" );
    /*QString msg;
    if (!resMgr->loadResources( "SMESH", msg ))
      MESSAGE ( msg )*/
  }

  SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( SUIT_Session::session()->activeApplication() );
  if( !app )
    return;

  CORBA::Object_var obj = app->namingService()->Resolve("/myStudyManager");
  SALOMEDS::StudyManager_var myStudyMgr = SALOMEDS::StudyManager::_narrow(obj);
  myStudy = myStudyMgr->GetStudyByID(studyID);

  CompMesh->SetCurrentStudy( myStudy.in() ); 

  myStudyBuilder = myStudy->NewBuilder();
  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeName_var    aName;
  SALOMEDS::AttributePixMap_var  aPixmap;

  // See return value of SMESH::SMESH_Gen::ComponentDataType()
  SALOMEDS::SComponent_var father = myStudy->FindComponent("SMESH");

  if (father->_is_nil()) {
    bool aLocked = myStudy->GetProperties()->IsLocked();
    if (aLocked) myStudy->GetProperties()->SetLocked(false);
    father = myStudyBuilder->NewComponent("SMESH");
    anAttr = myStudyBuilder->FindOrCreateAttribute(father, "AttributeName");
    aName = SALOMEDS::AttributeName::_narrow(anAttr);
    //NRI    aName->SetValue(QObject::tr("SMESH_MEN_COMPONENT"));
    SMESHGUI* gui = SMESHGUI::GetSMESHGUI(); //SRN: BugID IPAL9186, load a SMESH gui if it hasn't been loaded
    if(!gui) {
      SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>(SUIT_Session::session()->activeApplication()); 
      if(app) {
        CAM_Module* module = app->module( "Mesh" );
	if(!module) module = app->loadModule("Mesh");
	gui = dynamic_cast<SMESHGUI*>( module ); 
      }
      else {
        MESSAGE("Can't find the application");
      }
    }  //SRN: BugID IPAL9186: end of a fix
    aName->SetValue( gui->moduleName() );
    anAttr = myStudyBuilder->FindOrCreateAttribute(father, "AttributePixMap");
    aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
    aPixmap->SetPixMap( "ICON_OBJBROWSER_SMESH" );
    myStudyBuilder->DefineComponentInstance(father, CompMesh );
    if (aLocked) myStudy->GetProperties()->SetLocked(true);
  }
  mySComponentMesh = SALOMEDS::SComponent::_narrow( father );

  // Tags definition 
  Tag_HypothesisRoot  = 1;
  Tag_AlgorithmsRoot  = 2;
  
  Tag_RefOnShape      = 1;
  Tag_RefOnAppliedHypothesis = 2;
  Tag_RefOnAppliedAlgorithms = 3;
  
  Tag_SubMeshOnVertex = 4;
  Tag_SubMeshOnEdge = 5;
  Tag_SubMeshOnFace = 6;
  Tag_SubMeshOnSolid = 7;
  Tag_SubMeshOnCompound = 8;
}

SMESH_Swig::~SMESH_Swig()
{
  MESSAGE("Destructeur");
}

const char* SMESH_Swig::AddNewMesh(const char* IOR)
{
  MESSAGE("AddNewMesh");

  // VSR: added temporarily - to be removed - objects are published automatically by engine
  SALOMEDS::SObject_var SO = myStudy->FindObjectIOR( IOR );
  if ( !SO->_is_nil() )
    return SO->GetID();

  //Find or Create Hypothesis root
  SALOMEDS::GenericAttribute_var    anAttr;
  SALOMEDS::AttributeName_var       aName;
  SALOMEDS::AttributeIOR_var        anIOR;
  SALOMEDS::AttributeSelectable_var aSelAttr;
  SALOMEDS::AttributePixMap_var     aPixmap;

  SALOMEDS::SObject_var HypothesisRoot;
  if (!mySComponentMesh->FindSubObject (Tag_HypothesisRoot, HypothesisRoot)) {
    HypothesisRoot = myStudyBuilder->NewObjectToTag (mySComponentMesh, Tag_HypothesisRoot);
    anAttr = myStudyBuilder->FindOrCreateAttribute(HypothesisRoot, "AttributeName");
    aName = SALOMEDS::AttributeName::_narrow(anAttr);
    aName->SetValue(QObject::tr("SMESH_MEN_HYPOTHESIS"));
    anAttr = myStudyBuilder->FindOrCreateAttribute(HypothesisRoot, "AttributePixMap");
    aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
    aPixmap->SetPixMap( "ICON_SMESH_TREE_HYPO" );
    anAttr = myStudyBuilder->FindOrCreateAttribute(HypothesisRoot, "AttributeSelectable");
    aSelAttr = SALOMEDS::AttributeSelectable::_narrow(anAttr);
    aSelAttr->SetSelectable(false);
  }

  SALOMEDS::SObject_var AlgorithmsRoot;
  if (!mySComponentMesh->FindSubObject (Tag_AlgorithmsRoot, AlgorithmsRoot)) {
    AlgorithmsRoot = myStudyBuilder->NewObjectToTag (mySComponentMesh, Tag_AlgorithmsRoot);
    anAttr = myStudyBuilder->FindOrCreateAttribute(AlgorithmsRoot, "AttributeName");
    aName = SALOMEDS::AttributeName::_narrow(anAttr);
    aName->SetValue(QObject::tr("SMESH_MEN_ALGORITHMS"));
    anAttr = myStudyBuilder->FindOrCreateAttribute(AlgorithmsRoot, "AttributePixMap");
    aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
    aPixmap->SetPixMap( "ICON_SMESH_TREE_ALGO" );
    anAttr = myStudyBuilder->FindOrCreateAttribute(AlgorithmsRoot, "AttributeSelectable");
    aSelAttr = SALOMEDS::AttributeSelectable::_narrow(anAttr);
    aSelAttr->SetSelectable(false);
  }

  // Add New Mesh
  SALOMEDS::SObject_var newMesh = myStudyBuilder->NewObject(mySComponentMesh);
  anAttr = myStudyBuilder->FindOrCreateAttribute(newMesh, "AttributePixMap");
  aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
  aPixmap->SetPixMap( "ICON_SMESH_TREE_MESH" );
  anAttr = myStudyBuilder->FindOrCreateAttribute(newMesh, "AttributeIOR");
  anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
  anIOR->SetValue(IOR);
  return SALOMEDS::SObject::_narrow( newMesh )->GetID();
}

const char* SMESH_Swig::AddNewHypothesis(const char* IOR)
{
  MESSAGE("AddNewHypothesis");

  // VSR: added temporarily - to be removed - objects are published automatically by engine
  SALOMEDS::SObject_var SO = myStudy->FindObjectIOR( IOR );
  if ( !SO->_is_nil() )
    return SO->GetID();

  //Find or Create Hypothesis root
  SALOMEDS::SObject_var             HypothesisRoot;
  SALOMEDS::GenericAttribute_var    anAttr;
  SALOMEDS::AttributeName_var       aName;
  SALOMEDS::AttributeIOR_var        anIOR;
  SALOMEDS::AttributeSelectable_var aSelAttr;
  SALOMEDS::AttributePixMap_var     aPixmap;

  if (!mySComponentMesh->FindSubObject (Tag_HypothesisRoot, HypothesisRoot)) {
    HypothesisRoot = myStudyBuilder->NewObjectToTag (mySComponentMesh, Tag_HypothesisRoot);
    anAttr = myStudyBuilder->FindOrCreateAttribute(HypothesisRoot, "AttributeName");
    aName = SALOMEDS::AttributeName::_narrow(anAttr);
    aName->SetValue(QObject::tr("SMESH_MEN_HYPOTHESIS"));
    anAttr = myStudyBuilder->FindOrCreateAttribute(HypothesisRoot, "AttributeSelectable");
    aSelAttr = SALOMEDS::AttributeSelectable::_narrow(anAttr);
    aSelAttr->SetSelectable(false);
    anAttr = myStudyBuilder->FindOrCreateAttribute(HypothesisRoot, "AttributePixMap");
    aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
    aPixmap->SetPixMap( "ICON_SMESH_TREE_HYPO" );
  }
  // Add New Hypothesis
  SALOMEDS::SObject_var newHypo = myStudyBuilder->NewObject(HypothesisRoot);
  anAttr = myStudyBuilder->FindOrCreateAttribute(newHypo, "AttributePixMap");
  aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
  SMESH::SMESH_Hypothesis_var H = SMESH::SMESH_Hypothesis::_narrow( StringToObject(IOR) );
  QString aType = H->GetName();
  aPixmap->SetPixMap( "ICON_SMESH_TREE_HYPO_" + aType );
//    if ( aType.compare("LocalLength") == 0 )
//      aPixmap->SetPixMap( "ICON_SMESH_TREE_HYPO_LENGTH" );
//    else if ( aType.compare("NumberOfSegments") == 0 )
//      aPixmap->SetPixMap( "ICON_SMESH_TREE_HYPO_SEGMENT" );
//    else if ( aType.compare("MaxElementArea") == 0 )
//      aPixmap->SetPixMap( "ICON_SMESH_TREE_HYPO_AREA" );
//    else if ( aType.compare("MaxElementVolume") == 0 )
//      aPixmap->SetPixMap( "ICON_SMESH_TREE_HYPO_VOLUME" );
  anAttr = myStudyBuilder->FindOrCreateAttribute(newHypo, "AttributeIOR");
  anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
  anIOR->SetValue(IOR);
  return SALOMEDS::SObject::_narrow(newHypo)->GetID();
}

const char* SMESH_Swig::AddNewAlgorithms(const char* IOR)
{
  MESSAGE("AddNewAlgorithms");

  // VSR: added temporarily - to be removed - objects are published automatically by engine
  SALOMEDS::SObject_var SO = myStudy->FindObjectIOR( IOR );
  if ( !SO->_is_nil() )
    return SO->GetID();

  //Find or Create Algorithms root
  SALOMEDS::SObject_var             AlgorithmsRoot;
  SALOMEDS::GenericAttribute_var    anAttr;
  SALOMEDS::AttributeName_var       aName;
  SALOMEDS::AttributeIOR_var        anIOR;
  SALOMEDS::AttributeSelectable_var aSelAttr;
  SALOMEDS::AttributePixMap_var     aPixmap;

  if (!mySComponentMesh->FindSubObject (Tag_AlgorithmsRoot, AlgorithmsRoot)) {
    AlgorithmsRoot = myStudyBuilder->NewObjectToTag (mySComponentMesh, Tag_AlgorithmsRoot);
    anAttr = myStudyBuilder->FindOrCreateAttribute(AlgorithmsRoot, "AttributeName");
    aName = SALOMEDS::AttributeName::_narrow(anAttr);
    aName->SetValue(QObject::tr("SMESH_MEN_ALGORITHMS"));
    anAttr = myStudyBuilder->FindOrCreateAttribute(AlgorithmsRoot, "AttributeSelectable");
    aSelAttr = SALOMEDS::AttributeSelectable::_narrow(anAttr);
    aSelAttr->SetSelectable(false);
    anAttr = myStudyBuilder->FindOrCreateAttribute(AlgorithmsRoot, "AttributePixMap");
    aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
    aPixmap->SetPixMap( "ICON_SMESH_TREE_ALGO" );
  }
  // Add New Algorithms
  SALOMEDS::SObject_var newHypo = myStudyBuilder->NewObject(AlgorithmsRoot);
  anAttr = myStudyBuilder->FindOrCreateAttribute(newHypo, "AttributePixMap");
  aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
  SMESH::SMESH_Hypothesis_var H = SMESH::SMESH_Hypothesis::_narrow( StringToObject(IOR) );
  QString aType = H->GetName();
  aPixmap->SetPixMap( "ICON_SMESH_TREE_ALGO_" + aType );
//    if ( aType.compare("Regular_1D") == 0 )
//      aPixmap->SetPixMap( "ICON_SMESH_TREE_ALGO_REGULAR" );
//    else if ( aType.compare("MEFISTO_2D") == 0 )
//      aPixmap->SetPixMap( "ICON_SMESH_TREE_ALGO_MEFISTO" );
//    else if ( aType.compare("Quadrangle_2D") == 0 )
//      aPixmap->SetPixMap( "ICON_SMESH_TREE_ALGO_QUAD" );
//    else if ( aType.compare("Hexa_3D") == 0 )
//      aPixmap->SetPixMap( "ICON_SMESH_TREE_ALGO_HEXA" );
  anAttr = myStudyBuilder->FindOrCreateAttribute(newHypo, "AttributeIOR");
  anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
  anIOR->SetValue(IOR);
  return SALOMEDS::SObject::_narrow(newHypo)->GetID();
}

void SMESH_Swig::SetShape(const char* ShapeEntry, const char* MeshEntry)
{
  SALOMEDS::SObject_var SO_MorSM = myStudy->FindObjectID( MeshEntry );
  SALOMEDS::SObject_var SO_GeomShape = myStudy->FindObjectID( ShapeEntry );

  if ( !SO_MorSM->_is_nil() && !SO_GeomShape->_is_nil() ) {
    SALOMEDS::SObject_var SO = myStudyBuilder->NewObjectToTag (SO_MorSM, Tag_RefOnShape);
    myStudyBuilder->Addreference (SO,SO_GeomShape);
  }
}

void SMESH_Swig::SetHypothesis(const char* Mesh_Or_SubMesh_Entry, const char* Hypothesis_Entry)
{
  SALOMEDS::SObject_var SO_MorSM = myStudy->FindObjectID( Mesh_Or_SubMesh_Entry );
  SALOMEDS::SObject_var SO_Hypothesis = myStudy->FindObjectID( Hypothesis_Entry );
  SALOMEDS::GenericAttribute_var    anAttr;
  SALOMEDS::AttributeName_var       aName;
  SALOMEDS::AttributeSelectable_var aSelAttr;
  SALOMEDS::AttributePixMap_var     aPixmap;

  if ( !SO_MorSM->_is_nil() && !SO_Hypothesis->_is_nil() ) {
    
    //Find or Create Applied Hypothesis root
    SALOMEDS::SObject_var AHR;
    if (!SO_MorSM->FindSubObject (Tag_RefOnAppliedHypothesis, AHR)) {
      AHR = myStudyBuilder->NewObjectToTag (SO_MorSM, Tag_RefOnAppliedHypothesis);
      anAttr = myStudyBuilder->FindOrCreateAttribute(AHR, "AttributeName");
      aName = SALOMEDS::AttributeName::_narrow(anAttr);
      aName->SetValue(QObject::tr("SMESH_MEN_APPLIED_HYPOTHESIS"));
      anAttr = myStudyBuilder->FindOrCreateAttribute(AHR, "AttributeSelectable");
      aSelAttr = SALOMEDS::AttributeSelectable::_narrow(anAttr);
      aSelAttr->SetSelectable(false);
      anAttr = myStudyBuilder->FindOrCreateAttribute(AHR, "AttributePixMap");
      aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
      aPixmap->SetPixMap( "ICON_SMESH_TREE_HYPO" );
    }
    SALOMEDS::SObject_var SO = myStudyBuilder->NewObject(AHR);
    myStudyBuilder->Addreference (SO,SO_Hypothesis);
  }
}

void SMESH_Swig::SetAlgorithms(const char* Mesh_Or_SubMesh_Entry, const char* Algorithms_Entry)
{
  SALOMEDS::SObject_var SO_MorSM = myStudy->FindObjectID( Mesh_Or_SubMesh_Entry );
  SALOMEDS::SObject_var SO_Algorithms = myStudy->FindObjectID( Algorithms_Entry );
  SALOMEDS::GenericAttribute_var    anAttr;
  SALOMEDS::AttributeName_var       aName;
  SALOMEDS::AttributeSelectable_var aSelAttr;
  SALOMEDS::AttributePixMap_var     aPixmap;

  if ( !SO_MorSM->_is_nil() && !SO_Algorithms->_is_nil() ) {
    //Find or Create Applied Algorithms root
    SALOMEDS::SObject_var AHR;
    if (!SO_MorSM->FindSubObject (Tag_RefOnAppliedAlgorithms, AHR)) {
      AHR = myStudyBuilder->NewObjectToTag (SO_MorSM, Tag_RefOnAppliedAlgorithms);
      anAttr = myStudyBuilder->FindOrCreateAttribute(AHR, "AttributeName");
      aName = SALOMEDS::AttributeName::_narrow(anAttr);
      aName->SetValue(QObject::tr("SMESH_MEN_APPLIED_ALGORIHTMS"));
      anAttr = myStudyBuilder->FindOrCreateAttribute(AHR, "AttributeSelectable");
      aSelAttr = SALOMEDS::AttributeSelectable::_narrow(anAttr);
      aSelAttr->SetSelectable(false);
      anAttr = myStudyBuilder->FindOrCreateAttribute(AHR, "AttributePixMap");
      aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
      aPixmap->SetPixMap( "ICON_SMESH_TREE_ALGO" );
    }
    SALOMEDS::SObject_var SO = myStudyBuilder->NewObject(AHR);
    myStudyBuilder->Addreference (SO,SO_Algorithms);
  }
}

void SMESH_Swig::UnSetHypothesis(const char* Applied_Hypothesis_Entry )
{
  SALOMEDS::SObject_var SO_Applied_Hypothesis = myStudy->FindObjectID( Applied_Hypothesis_Entry );
  if ( !SO_Applied_Hypothesis->_is_nil() )
    myStudyBuilder->RemoveObject(SO_Applied_Hypothesis);
}

const char* SMESH_Swig::AddSubMesh(const char* SO_Mesh_Entry, const char* SM_IOR, int ST)
{
  SALOMEDS::SObject_var SO_Mesh = myStudy->FindObjectID( SO_Mesh_Entry );
  if ( !SO_Mesh->_is_nil() ) {

    long Tag_Shape ;
    Standard_CString Name;
    
    if      (ST == TopAbs_SOLID) {Tag_Shape = Tag_SubMeshOnSolid;    Name = strdup(QObject::tr("SMESH_MEN_SubMeshesOnSolid"));}
    else if (ST == TopAbs_FACE)  {Tag_Shape = Tag_SubMeshOnFace;     Name = strdup(QObject::tr("SMESH_MEN_SubMeshesOnFace"));}
    else if (ST == TopAbs_EDGE)  {Tag_Shape = Tag_SubMeshOnEdge;     Name = strdup(QObject::tr("SMESH_MEN_SubMeshesOnEdge"));}
    else if (ST == TopAbs_VERTEX){Tag_Shape = Tag_SubMeshOnVertex;   Name = strdup(QObject::tr("SMESH_MEN_SubMeshesOnVertex"));}
    else {
      Tag_Shape = Tag_SubMeshOnCompound; Name = strdup(QObject::tr("SMESH_MEN_SubMeshesOnCompound"));
    }
    SALOMEDS::SObject_var SubmeshesRoot;
    SALOMEDS::GenericAttribute_var        anAttr;
    SALOMEDS::AttributeName_var           aName;
    SALOMEDS::AttributeIOR_var            anIOR;
    SALOMEDS::AttributeSelectable_var     aSelAttr;
    if (!SO_Mesh->FindSubObject (Tag_Shape,SubmeshesRoot )) {
      SubmeshesRoot = myStudyBuilder->NewObjectToTag (SO_Mesh, Tag_Shape);
      anAttr = myStudyBuilder->FindOrCreateAttribute(SubmeshesRoot, "AttributeName");
      aName = SALOMEDS::AttributeName::_narrow(anAttr);
      aName->SetValue(Name);
      anAttr = myStudyBuilder->FindOrCreateAttribute(SubmeshesRoot, "AttributeSelectable");
      aSelAttr = SALOMEDS::AttributeSelectable::_narrow(anAttr);
      aSelAttr->SetSelectable(false);
    }

    free(Name);

    SALOMEDS::SObject_var SO = myStudyBuilder->NewObject (SubmeshesRoot);
    anAttr = myStudyBuilder->FindOrCreateAttribute(SO, "AttributeIOR");
    anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
    anIOR->SetValue(SM_IOR);
    return SALOMEDS::SObject::_narrow( SO )->GetID();
  }
  return "";
}

const char* SMESH_Swig::AddSubMeshOnShape(const char* Mesh_Entry, const char* GeomShape_Entry, 
					     const char* SM_IOR, int ST)
{
  SALOMEDS::SObject_var SO_GeomShape = myStudy->FindObjectID( GeomShape_Entry );
  if ( !SO_GeomShape->_is_nil() ) {
    const char * SM_Entry = AddSubMesh (Mesh_Entry,SM_IOR,ST);
    SALOMEDS::SObject_var SO_SM = myStudy->FindObjectID( SM_Entry );
    if ( !SO_SM->_is_nil() ) {
      SetShape (GeomShape_Entry, SM_Entry);
      return SALOMEDS::SObject::_narrow( SO_SM )->GetID();
    }
  }
  return "";
}

void SMESH_Swig::CreateAndDisplayActor( const char* Mesh_Entry )
{
  //  SMESH_Actor* Mesh = smeshGUI->ReadScript(aM);
}

void SMESH_Swig::SetName(const char* Entry, const char* Name)
{
  SALOMEDS::SObject_var SO = myStudy->FindObjectID( Entry );
  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeName_var    aName;
  if ( !SO->_is_nil() )  {
    anAttr = myStudyBuilder->FindOrCreateAttribute(SO, "AttributeName");
    aName = SALOMEDS::AttributeName::_narrow(anAttr);
    aName->SetValue(Name);
  }
}

void SMESH_Swig::setOrb()
{
  try {
    ORB_INIT &init = *SINGLETON_<ORB_INIT>::Instance();
    ASSERT(SINGLETON_<ORB_INIT>::IsAlreadyExisting());
    _orb = init( 0 , 0 );
  } catch (...) {
    INFOS("internal error : orb not found");
    _orb = 0;
  }
  ASSERT(! CORBA::is_nil(_orb));
}

//================================================================================
/*!
 * \brief Set mesh icon according to compute status
  * \param Mesh_Entry - entry of a mesh
  * \param isComputed - is mesh computed or not
 */
//================================================================================

void SMESH_Swig::SetMeshIcon(const char* Mesh_Entry, const bool isComputed)
{
  SALOMEDS::SObject_var mesh_var = myStudy->FindObjectID( Mesh_Entry );
  if ( !mesh_var->_is_nil() ) {
    _PTR(SObject) mesh = _PTR(SObject)(new SALOMEDS_SObject( mesh_var ));
    if ( mesh )
      SMESH::ModifiedMesh( mesh, isComputed );
  }
}
