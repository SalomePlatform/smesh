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
//  File   : SMESHGUI_StudyAPI.cxx
//  Author : Yves FRICAUD 
//  Module : SMESH
//  $Header: 

using namespace std;
#include "SMESHGUI_StudyAPI.h"
#include "utilities.h"
#include "Utils_ORB_INIT.hxx"
#include "Utils_SINGLETON.hxx"

#include "QAD_MessageBox.h"
#include "QAD_Desktop.h"
#include "QAD_Application.h"

// QT Includes
#include <qobject.h>

// Open CASCADE Includes
#include "Standard_CString.hxx"

static CORBA::ORB_var _orb;

// Tags definition 
long Tag_HypothesisRoot  = 1;
long Tag_AlgorithmsRoot  = 2;

long Tag_RefOnShape      = 1;
long Tag_RefOnAppliedHypothesis = 2;
long Tag_RefOnAppliedAlgorithms = 3;

long Tag_SubMeshOnVertex = 4;
long Tag_SubMeshOnEdge = 5;
long Tag_SubMeshOnFace = 6;
long Tag_SubMeshOnSolid = 7;
long Tag_SubMeshOnCompound = 8;



static char* IORToString (CORBA::Object_ptr obj)
{
  //  MESSAGE ( " IORToString " << _orb->object_to_string(obj) )
  return _orb->object_to_string(obj);
}

CORBA::Object_ptr SMESHGUI_StudyAPI::StringToIOR (const char* IOR)
{
  //  MESSAGE ( " StringToIOR " << _orb->string_to_object(IOR) )
  return _orb->string_to_object(IOR);
}
//=======================================================================
// function :
// purpose  : Constructor
//=======================================================================
SMESHGUI_StudyAPI::SMESHGUI_StudyAPI ()
{
}

SMESHGUI_StudyAPI::SMESHGUI_StudyAPI ( SALOMEDS::Study_ptr aStudy,
				       SMESH::SMESH_Gen_ptr CompMesh)
{
  myStudy = aStudy;
  myStudyBuilder = aStudy->NewBuilder();

  setOrb();

  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
//    QAD_MessageBox::warn1 ( (QWidget*)QAD_Application::getDesktop(),
//			    QObject::tr("WARNING"), 
//			    QObject::tr("WRN_STUDY_LOCKED"),
//			    QObject::tr("BUT_OK") );
    
    return;
  }
  // NRI

  SALOMEDS::SComponent_var father = aStudy->FindComponent("MESH");
  /*SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeName_var    aName;
  SALOMEDS::AttributePixMap_var  aPixmap;
  
  if (father->_is_nil()) {
    father = myStudyBuilder->NewComponent("MESH");
  }
  anAttr = myStudyBuilder->FindOrCreateAttribute(father, "AttributeName");
  aName = SALOMEDS::AttributeName::_narrow(anAttr);
  //NRI    aName->SetValue(QObject::tr("SMESH_MEN_COMPONENT"));
  aName->SetValue( QAD_Application::getDesktop()->getComponentUserName( "SMESH" ) );
  anAttr = myStudyBuilder->FindOrCreateAttribute(father, "AttributePixMap");
  aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
  aPixmap->SetPixMap( "ICON_OBJBROWSER_SMESH" );
  
  myStudyBuilder->DefineComponentInstance(father, CompMesh );
  mySComponentMesh = SALOMEDS::SComponent::_narrow( father );*/
}

//=======================================================================
// function :
// purpose  : Destructor
//=======================================================================
SMESHGUI_StudyAPI::~SMESHGUI_StudyAPI ()
{
}

//=======================================================================
// function :
// purpose  : Update
//=======================================================================
void SMESHGUI_StudyAPI::Update(SMESH::SMESH_Gen_ptr CompMesh)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return;
  }
  // NRI

  SALOMEDS::SComponent_var father = myStudy->FindComponent("MESH");
  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeName_var    aName;
  SALOMEDS::AttributePixMap_var  aPixmap;
  
  if (father->_is_nil()) {
    father = myStudyBuilder->NewComponent("MESH");
    anAttr = myStudyBuilder->FindOrCreateAttribute(father, "AttributeName");
    aName = SALOMEDS::AttributeName::_narrow(anAttr);
    //NRI    aName->SetValue(QObject::tr("SMESH_MEN_COMPONENT"));
    aName->SetValue( QAD_Application::getDesktop()->getComponentUserName( "SMESH" ) );
    anAttr = myStudyBuilder->FindOrCreateAttribute(father, "AttributePixMap");
    aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
    aPixmap->SetPixMap( "ICON_OBJBROWSER_SMESH" );
  }
  myStudyBuilder->DefineComponentInstance(father, CompMesh );
  mySComponentMesh = SALOMEDS::SComponent::_narrow( father );  
}

//=======================================================================
// function : AddNewMesh 
// purpose  :
//=======================================================================

SALOMEDS::SObject_ptr SMESHGUI_StudyAPI::AddNewMesh (SMESH::SMESH_Mesh_ptr M)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return SALOMEDS::SObject::_nil();
  }
  // NRI

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
  anIOR->SetValue(IORToString(M));
  return SALOMEDS::SObject::_narrow( newMesh );
}


//=======================================================================
// function : SetShape
// purpose  :
//=======================================================================
void SMESHGUI_StudyAPI::SetShape (SALOMEDS::SObject_ptr SO_MorSM, 
				  SALOMEDS::SObject_ptr SO_GeomShape )
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return;
  }
  // NRI
  SALOMEDS::SObject_var SO = myStudyBuilder->NewObjectToTag (SO_MorSM, Tag_RefOnShape);
  myStudyBuilder->Addreference (SO,SO_GeomShape);
}
  
//=======================================================================
// function : GetShapeOnMeshOrSubMesh
// purpose  :
//=======================================================================
GEOM::GEOM_Shape_ptr SMESHGUI_StudyAPI::GetShapeOnMeshOrSubMesh(SALOMEDS::SObject_ptr SO_Mesh_Or_SubMesh)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return GEOM::GEOM_Shape::_nil();
  }
  // NRI

  GEOM::GEOM_Shape_var Shape;
  SALOMEDS::SObject_var aSO, aGeom;
  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeIOR_var     anIOR;
  if ( SO_Mesh_Or_SubMesh->FindSubObject( Tag_RefOnShape, aSO ) ) {
    if ( aSO->ReferencedObject(aGeom) ) {
      if (aGeom->FindAttribute(anAttr, "AttributeIOR")) {
        anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	return GEOM::GEOM_Shape::_narrow( _orb->string_to_object(anIOR->Value()) );
      }
    }
  }
  return GEOM::GEOM_Shape::_nil();
}

//=======================================================================
// function : AddNewHypothesis 
// purpose  :
//=======================================================================
SALOMEDS::SObject_ptr SMESHGUI_StudyAPI::AddNewHypothesis  (SMESH::SMESH_Hypothesis_ptr H)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return SALOMEDS::SObject::_nil();
  }
  // NRI

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
  QString aType = H->GetName();
  MESSAGE ( " aType " << aType )
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
  anIOR->SetValue(IORToString(H));
  return SALOMEDS::SObject::_narrow(newHypo);
}  

//=======================================================================
// function : AddNewAlgorithms 
// purpose  :
//=======================================================================
SALOMEDS::SObject_ptr SMESHGUI_StudyAPI::AddNewAlgorithms  (SMESH::SMESH_Hypothesis_ptr H)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return SALOMEDS::SObject::_nil();
  }
  // NRI

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
  QString aType = H->GetName();
  //  if ( aType.compare("Regular_1D") == 0 )
  aPixmap->SetPixMap( "ICON_SMESH_TREE_ALGO_" + aType );
  //      aPixmap->SetPixMap( "ICON_SMESH_TREE_ALGO_REGULAR" );
  //    else if ( aType.compare("MEFISTO_2D") == 0 )
  //      aPixmap->SetPixMap( "ICON_SMESH_TREE_ALGO_MEFISTO" );
  //    else if ( aType.compare("Quadrangle_2D") == 0 )
  //      aPixmap->SetPixMap( "ICON_SMESH_TREE_ALGO_QUAD" );
  //    else if ( aType.compare("Hexa_3D") == 0 )
  //      aPixmap->SetPixMap( "ICON_SMESH_TREE_ALGO_HEXA" );
  anAttr = myStudyBuilder->FindOrCreateAttribute(newHypo, "AttributeIOR");
  anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
  anIOR->SetValue(IORToString(H));
  return SALOMEDS::SObject::_narrow(newHypo);
}  


//=======================================================================
// function : AddSubMeshOnShape
// purpose  :
//=======================================================================
SALOMEDS::SObject_ptr SMESHGUI_StudyAPI::AddSubMeshOnShape (SALOMEDS::SObject_ptr SO_Mesh, 
							    SMESH::SMESH_subMesh_ptr SM, 
							    GEOM::shape_type ST)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return SALOMEDS::SObject::_nil();
  }
  // NRI

  long Tag_Shape ;
  Standard_CString Name;

  if      (ST == GEOM::SOLID) {Tag_Shape = Tag_SubMeshOnSolid;    Name = strdup(QObject::tr("SMESH_MEN_SubMeshesOnSolid"));}
  else if (ST == GEOM::FACE)  {Tag_Shape = Tag_SubMeshOnFace;     Name = strdup(QObject::tr("SMESH_MEN_SubMeshesOnFace"));}
  else if (ST == GEOM::EDGE)  {Tag_Shape = Tag_SubMeshOnEdge;     Name = strdup(QObject::tr("SMESH_MEN_SubMeshesOnEdge"));}
  else if (ST == GEOM::VERTEX){Tag_Shape = Tag_SubMeshOnVertex;   Name = strdup(QObject::tr("SMESH_MEN_SubMeshesOnVertex"));}
  else {
    Tag_Shape = Tag_SubMeshOnCompound; Name = strdup(QObject::tr("SMESH_MEN_SubMeshesOnCompound"));
  }

  SALOMEDS::SObject_var              SubmeshesRoot;
  SALOMEDS::GenericAttribute_var     anAttr;
  SALOMEDS::AttributeName_var        aName;
  SALOMEDS::AttributeIOR_var         anIOR;
  SALOMEDS::AttributeSelectable_var  aSelAttr;

  if (!SO_Mesh->FindSubObject (Tag_Shape,SubmeshesRoot )) {
    SubmeshesRoot = myStudyBuilder->NewObjectToTag (SO_Mesh, Tag_Shape);
    anAttr = myStudyBuilder->FindOrCreateAttribute(SubmeshesRoot, "AttributeName");
    aName = SALOMEDS::AttributeName::_narrow(anAttr);
    aName->SetValue(Name);
    anAttr = myStudyBuilder->FindOrCreateAttribute(SubmeshesRoot, "AttributeSelectable");
    aSelAttr = SALOMEDS::AttributeSelectable::_narrow(anAttr);
    aSelAttr->SetSelectable(false);
  }
  SALOMEDS::SObject_var SO = myStudyBuilder->NewObject (SubmeshesRoot);
  anAttr = myStudyBuilder->FindOrCreateAttribute(SO, "AttributeIOR");
  anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
  anIOR->SetValue(IORToString(SM));
  return SALOMEDS::SObject::_narrow( SO );
}

//=======================================================================
// function : AddSubMeshOnShape 
// purpose  :
//=======================================================================
SALOMEDS::SObject_ptr SMESHGUI_StudyAPI::AddSubMeshOnShape (SALOMEDS::SObject_ptr SO_Mesh, 
							    SALOMEDS::SObject_ptr SO_GeomShape, 
							    SMESH::SMESH_subMesh_ptr SM,
							    GEOM::shape_type ST)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return SALOMEDS::SObject::_nil();
  }
  // NRI

  SALOMEDS::SObject_var SO_SM = AddSubMeshOnShape (SO_Mesh,SM,ST);
  SetShape (SO_SM,SO_GeomShape);
  //  SetShapeOnSubMesh (SO_SM,SO_GeomShape);
  return SALOMEDS::SObject::_narrow( SO_SM );
}


//=======================================================================
// function : SetHypothesis 
// purpose  : 
//=======================================================================
void SMESHGUI_StudyAPI::SetHypothesis     (SALOMEDS::SObject_ptr SO_MorSM, 
					   SALOMEDS::SObject_ptr SO_Hypothesis)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return;
  }
  // NRI

  //Find or Create Applied Hypothesis root
  SALOMEDS::SObject_var             AHR;
  SALOMEDS::GenericAttribute_var    anAttr;
  SALOMEDS::AttributeName_var       aName;
  SALOMEDS::AttributeSelectable_var aSelAttr;
  SALOMEDS::AttributePixMap_var     aPixmap;

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

//=======================================================================
// function : SetAlgorithms 
// purpose  : 
//=======================================================================
void SMESHGUI_StudyAPI::SetAlgorithms     (SALOMEDS::SObject_ptr SO_MorSM, 
					   SALOMEDS::SObject_ptr SO_Algorithms)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return;
  }
  // NRI

  //Find or Create Applied Algorithms root
  SALOMEDS::SObject_var             AHR;
  SALOMEDS::GenericAttribute_var    anAttr;
  SALOMEDS::AttributeName_var       aName;
  SALOMEDS::AttributeSelectable_var aSelAttr;
  SALOMEDS::AttributePixMap_var     aPixmap;

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


//=======================================================================
// function :
// purpose  : 
//=======================================================================
SALOMEDS::SObject_ptr SMESHGUI_StudyAPI::FindMesh       (SMESH::SMESH_Mesh_ptr M)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return SALOMEDS::SObject::_nil();
  }
  // NRI
  return SALOMEDS::SObject::_narrow( myStudy->FindObjectIOR (IORToString(M)) );
}   

//=======================================================================
// function :
// purpose  : 
//=======================================================================
SALOMEDS::SObject_ptr SMESHGUI_StudyAPI::FindHypothesisOrAlgorithms (SMESH::SMESH_Hypothesis_ptr H)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return SALOMEDS::SObject::_nil();
  }
  // NRI
  return SALOMEDS::SObject::_narrow( myStudy->FindObjectIOR (IORToString(H)) );
} 
  

//=======================================================================
// function :
// purpose  : 
//=======================================================================
SALOMEDS::SObject_ptr SMESHGUI_StudyAPI::FindSubMesh    (SMESH::SMESH_subMesh_ptr SM)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return SALOMEDS::SObject::_nil();
  }
  // NRI
  return SALOMEDS::SObject::_narrow( myStudy->FindObjectIOR (IORToString(SM)) );
}   
 

void SMESHGUI_StudyAPI::setOrb()
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

//=======================================================================
// function :
// purpose  : 
//=======================================================================
void SMESHGUI_StudyAPI::SetTagHypothesisRoot()
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return;
  }
  // NRI
  SALOMEDS::ChildIterator_var it = myStudy->NewChildIterator(mySComponentMesh);
  int i = 0;
  for (; it->More();it->Next()) {
  i++;
  }
  Tag_HypothesisRoot = i++;
}

//=======================================================================
// function :
// purpose  : 
//=======================================================================
void SMESHGUI_StudyAPI::SetName( SALOMEDS::SObject_ptr SO, const char* Name )
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return;
  }
  // NRI

  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeName_var    aName;
  anAttr = myStudyBuilder->FindOrCreateAttribute(SO, "AttributeName");
  aName = SALOMEDS::AttributeName::_narrow(anAttr);
  aName->SetValue(Name);
}

//=======================================================================
// function :
// purpose  : 
//=======================================================================
void SMESHGUI_StudyAPI::UnSetHypothesis    (SALOMEDS::SObject_ptr SO_Applied_Hypothesis)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return;
  }
  // NRI
  myStudyBuilder->RemoveObject(SO_Applied_Hypothesis);
}

//=======================================================================
// function :
// purpose  : 
//=======================================================================
void SMESHGUI_StudyAPI::UnSetAlgorithm      (SALOMEDS::SObject_ptr SO_Applied_Algorithm)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return;
  }
  // NRI
  myStudyBuilder->RemoveObject(SO_Applied_Algorithm);
}

//=======================================================================
// function :
// purpose  : 
//=======================================================================
SALOMEDS::SObject_ptr SMESHGUI_StudyAPI::GetMeshOrSubmesh  (SALOMEDS::SObject_ptr SO)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return SALOMEDS::SObject::_nil();
  }
  // NRI

  SALOMEDS::SObject_var MorSM,SB;
  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeName_var    aName;
  SALOMEDS::AttributeIOR_var     anIOR;
  
  if (SO->FindAttribute(anAttr, "AttributeIOR")) {
    anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
    GEOM::GEOM_Shape_var Shape = GEOM::GEOM_Shape::_narrow( _orb->string_to_object(anIOR->Value()) );
    if (!Shape->_is_nil()) {
      //It s a shape
      MorSM = SO->GetFather();
      //      MESSAGE ( " MorSM  shape " << MorSM->GetID() )
      return SALOMEDS::SObject::_narrow( MorSM);
    }
  }
  
  if (SO->ReferencedObject(SB)) {
    //It's Reference on Hypothesis or Algorithm
    MorSM = SO->GetFather()->GetFather();
    //    MESSAGE ( " MorSM reference " << MorSM->GetID() )
    return SALOMEDS::SObject::_narrow( MorSM);
  }
  // It's  SMESH_MEN_APPLIED_HYPOTHESIS or SMESH_MEN_APPLIED_ALGORITHMS or SubMeshesOnFace etc...
  MorSM = SO->GetFather();
  //  MESSAGE ( " MorSM  banniere " << MorSM->GetID() )
  return SALOMEDS::SObject::_narrow( MorSM);
}

void SMESHGUI_StudyAPI::ModifiedMesh( SALOMEDS::SObject_ptr MorSM, bool right)
{
  // NRI : Temporary added
  if ( myStudy->GetProperties()->IsLocked() ) {
    return ;
  }
  // NRI

  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributePixMap_var  aPixmap;
  
  anAttr = myStudyBuilder->FindOrCreateAttribute(MorSM, "AttributePixMap");
  aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
  if (right) {
    aPixmap->SetPixMap( "ICON_SMESH_TREE_MESH" );
  } else {
    aPixmap->SetPixMap( "ICON_SMESH_TREE_MESH_WARN" );
  }
  
  int i = 1;
  SALOMEDS::ChildIterator_var it = myStudy->NewChildIterator(MorSM);
  for (; it->More();it->Next()) {
    SALOMEDS::SObject_var Obj = it->Value();
    if ( i >= 4 ) {
      int j = 1;
      SALOMEDS::ChildIterator_var it1 = myStudy->NewChildIterator(Obj);
      for (; it1->More();it1->Next()) {
	SALOMEDS::SObject_var Obj1 = it1->Value();
	anAttr = myStudyBuilder->FindOrCreateAttribute(Obj1, "AttributePixMap");
	aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
	if (right) {
	  aPixmap->SetPixMap( "ICON_SMESH_TREE_MESH" );
	} else {
	  aPixmap->SetPixMap( "ICON_SMESH_TREE_MESH_WARN" );
	}
      }
    }
    i++;
  }
}
