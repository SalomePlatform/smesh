//  SMESH OBJECT : interactive object for SMESH visualization
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
//  File   : SMESH_Actor.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

using namespace std;
/*!
  \class SMESH_Actor SMESH_Actor.h
  \brief ...
*/

#include "SMESH_Actor.h"
#include "SMESH_Grid.h"
#include "utilities.h"

// VTK Includes
#include <vtkObjectFactory.h>
#include <vtkMergePoints.h>
#include <vtkDataSetMapper.h>
#include <vtkFeatureEdges.h>
#include <vtkGeometryFilter.h>

//-------------------------------------------------------------
// Main methods
//-------------------------------------------------------------

SMESH_Actor* SMESH_Actor::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("SMESH_Actor");
  if(ret)
    {
      return (SMESH_Actor*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new SMESH_Actor;
}


SMESH_Actor::SMESH_Actor()
{
  this->Device = vtkActor::New();

  this->EdgeDevice = vtkActor::New();
  EdgeDevice->VisibilityOff();
  EdgeDevice->PickableOff();

  this->EdgeShrinkDevice = vtkActor::New();
  EdgeShrinkDevice->VisibilityOff();
  EdgeShrinkDevice->PickableOff();

  DataSource = NULL;

  myIO = NULL;
  myName = "";
  myDisplayMode = 0;

  ishighlighted = false;
  ispreselected = false;

  edgeColor.r = 0.;
  edgeColor.g = 0.;
  edgeColor.b = 0.;
  
  edgeHighlightColor.r = 1.;
  edgeHighlightColor.g = 1.;
  edgeHighlightColor.b = 1.;

  edgePreselectedColor.r = 0.;
  edgePreselectedColor.g = 1.;
  edgePreselectedColor.b = 1.;

  actorColor.r = 1.;
  actorColor.g = 1.;
  actorColor.b = 0.;

  actorHighlightColor.r = 1.;
  actorHighlightColor.g = 1.;
  actorHighlightColor.b = 1.;

  actorPreselectedColor.r = 0.;
  actorPreselectedColor.g = 1.;
  actorPreselectedColor.b = 1.;

  actorNodeColor.r = 1.;
  actorNodeColor.g = 1.;
  actorNodeColor.b = 0.;

  actorNodeSize = 2 ;
  
}

SMESH_Actor::~SMESH_Actor()
{
  this->EdgeDevice->Delete();
  this->EdgeShrinkDevice->Delete();
}

void SMESH_Actor::setReader(vtkUnstructuredGridReader* r) {
  myReader=r;
}

vtkUnstructuredGridReader* SMESH_Actor::getReader() {
  return (myReader);
}

vtkMapper* SMESH_Actor::getMapper() {
  return (this->Mapper);
}

void SMESH_Actor::ShallowCopy(vtkProp *prop)
{
  SMESH_Actor *f = SMESH_Actor::SafeDownCast(prop);
  if ( f != NULL )
    {
      this->setName( f->getName() );
      if ( f->hasIO() )
	this->setIO( f->getIO() );
      this->setDisplayMode( f->getDisplayMode() );

      // Copy devices
      vtkActor* tempDev = vtkActor::New();
      tempDev->ShallowCopy(f->Device);
      vtkProperty* prp = vtkProperty::New();
      prp->DeepCopy(f->Device->GetProperty());
      tempDev->SetProperty(prp);
      prp = vtkProperty::New();
      prp->DeepCopy(f->Device->GetBackfaceProperty());
      tempDev->SetBackfaceProperty(prp);
      this->Device = tempDev;
      
      tempDev = vtkActor::New();
      tempDev->ShallowCopy(f->EdgeDevice);
      prp = vtkProperty::New();
      prp->DeepCopy(f->EdgeDevice->GetProperty());
      tempDev->SetProperty(prp);
      prp = vtkProperty::New();
      prp->DeepCopy(f->EdgeDevice->GetBackfaceProperty());
      tempDev->SetBackfaceProperty(prp);
      this->EdgeDevice = tempDev;

      tempDev = vtkActor::New();
      tempDev->ShallowCopy(f->EdgeShrinkDevice);
      prp = vtkProperty::New();
      prp->DeepCopy(f->EdgeShrinkDevice->GetProperty());
      tempDev->SetProperty(prp);
      prp = vtkProperty::New();
      prp->DeepCopy(f->EdgeShrinkDevice->GetBackfaceProperty());
      tempDev->SetBackfaceProperty(prp);
      this->EdgeShrinkDevice = tempDev;

      // Copy data source
      this->DataSource = f->DataSource;

      this->myReader   = f->myReader;
    }

  // Now do superclass
  this->SALOME_Actor::ShallowCopy(prop);

  // Here we need to modify default ShallowCopy() results
  // Create copies of properties
  if ( f != NULL ) {
    vtkProperty* prp = vtkProperty::New();
    prp->DeepCopy(f->GetProperty());
    this->SetProperty(prp);

    prp = vtkProperty::New();
    prp->DeepCopy(f->GetBackfaceProperty());
    this->SetBackfaceProperty(prp);

    // Copy the mapper
    vtkDataSetMapper* mpr = vtkDataSetMapper::New();
    mpr->ShallowCopy(f->GetMapper());
    mpr->SetInput(f->DataSource);
    this->SetMapper(mpr);
  }
}

void SMESH_Actor::Render(vtkRenderer *ren, vtkMapper *Mapper )
{
   if (this->Mapper == NULL) {
    MESSAGE ("No mapper for actor.")
    return;
  }

   if ( myDisplayMode == 1 ) {
     EdgeDevice->VisibilityOn();
     EdgeShrinkDevice->VisibilityOff();
   } else if ( myDisplayMode == 2 ) {
     EdgeShrinkDevice->VisibilityOn();
     EdgeDevice->VisibilityOff();
   } else {
     EdgeShrinkDevice->VisibilityOff();
     EdgeDevice->VisibilityOff();
   }
     

  vtkMapper *bestMapper;
  bestMapper = this->Mapper;

  /* render the property */
  if (!this->Property) {
    // force creation of a property
    this->GetProperty();
  }

  if ( ishighlighted ) {
   if ( myDisplayMode == 1 ) {
     EdgeDevice->GetProperty()->SetColor(edgeHighlightColor.r,edgeHighlightColor.g,edgeHighlightColor.b);
     this->GetProperty()->SetColor(actorColor.r,actorColor.g,actorColor.b);
   } else if ( myDisplayMode == 2 ) {
     EdgeShrinkDevice->GetProperty()->SetColor(edgeHighlightColor.r,edgeHighlightColor.g,edgeHighlightColor.b);
   } else {
     this->GetProperty()->SetColor(actorHighlightColor.r,actorHighlightColor.g,actorHighlightColor.b);
   }
  } else if (! ispreselected ) {
    if ( myDisplayMode == 1 ) {
      EdgeDevice->GetProperty()->SetColor(edgeColor.r,edgeColor.g,edgeColor.b);
      this->GetProperty()->SetColor(actorColor.r,actorColor.g,actorColor.b);
    }
    else if ( myDisplayMode == 2 ) 
      EdgeShrinkDevice->GetProperty()->SetColor(edgeColor.r,edgeColor.g,edgeColor.b);
    else
      this->GetProperty()->SetColor(actorColor.r,actorColor.g,actorColor.b);
  }
  else {
    if ( myDisplayMode == 1 )
      EdgeDevice->GetProperty()->SetColor(edgePreselectedColor.r,edgePreselectedColor.g,edgePreselectedColor.b);
    else if ( myDisplayMode == 2 ) 
      EdgeShrinkDevice->GetProperty()->SetColor(edgePreselectedColor.r,edgePreselectedColor.g,edgePreselectedColor.b);
    else
      this->GetProperty()->SetColor(actorPreselectedColor.r,actorPreselectedColor.g,actorPreselectedColor.b);
  }

  this->Property->Render(this, ren);
  if (this->BackfaceProperty) {
    this->BackfaceProperty->BackfaceRender(this, ren);
    this->Device->SetBackfaceProperty(this->BackfaceProperty);
  }
  this->Device->SetProperty(this->Property);
  
  /* render the texture */
  if (this->Texture) {
    this->Texture->Render(ren);
  }
  
  
  // Store information on time it takes to render.
  // We might want to estimate time from the number of polygons in mapper.
  this->Device->Render(ren,bestMapper);
  this->EstimatedRenderTime = bestMapper->GetTimeToDraw();
}

int SMESH_Actor::RenderOpaqueGeometry(vtkViewport *vp)
{
  int          renderedSomething = 0; 
  vtkRenderer  *ren = (vtkRenderer *)vp;
  
  if ( ! this->Mapper ) {
    return 0;
  }
  
  // make sure we have a property
  if (!this->Property) {
    // force creation of a property
    this->GetProperty();
  }
  
  if ( ishighlighted ) {
   if ( myDisplayMode == 1 ) {
     EdgeDevice->GetProperty()->SetColor(edgeHighlightColor.r,edgeHighlightColor.g,edgeHighlightColor.b);
   } else if ( myDisplayMode == 2 ) {
     EdgeShrinkDevice->GetProperty()->SetColor(edgeHighlightColor.r,edgeHighlightColor.g,edgeHighlightColor.b);
   } else {
     this->GetProperty()->SetColor(actorHighlightColor.r,actorHighlightColor.g,actorHighlightColor.b);
   }
  } else if (! ispreselected ) {
    if ( myDisplayMode == 1 )
      EdgeDevice->GetProperty()->SetColor(edgeColor.r,edgeColor.g,edgeColor.b);
    else if ( myDisplayMode == 2 ) 
      EdgeShrinkDevice->GetProperty()->SetColor(edgeColor.r,edgeColor.g,edgeColor.b);
    else
      this->GetProperty()->SetColor(actorColor.r,actorColor.g,actorColor.b);
  }
  else {
    if ( myDisplayMode == 1 )
      EdgeDevice->GetProperty()->SetColor(edgePreselectedColor.r,edgePreselectedColor.g,edgePreselectedColor.b);
    else if ( myDisplayMode == 2 ) 
      EdgeShrinkDevice->GetProperty()->SetColor(edgePreselectedColor.r,edgePreselectedColor.g,edgePreselectedColor.b);
    else
      this->GetProperty()->SetColor(actorPreselectedColor.r,actorPreselectedColor.g,actorPreselectedColor.b);    
  }

  // is this actor opaque ?
  if (this->GetIsOpaque()) {
    this->Property->Render(this, ren);
    
    // render the backface property
    if (this->BackfaceProperty) {
      this->BackfaceProperty->BackfaceRender(this, ren);
    }
    
    // render the texture 
    if (this->Texture) {
      this->Texture->Render(ren);
    }
    this->Render(ren,this->Mapper);
    
    renderedSomething = 1;
  }
  
  return renderedSomething; 
}


void SMESH_Actor::SetColor(float r,float g,float b)
{
  actorColor.r = r;
  actorColor.g = g;
  actorColor.b = b;
}

void SMESH_Actor::GetColor(float& r,float& g,float& b)
{
  r = actorColor.r;
  g = actorColor.g;
  b = actorColor.b;
}

void SMESH_Actor::SetPreselectedColor(float r,float g,float b)
{
  actorPreselectedColor.r = r;
  actorPreselectedColor.g = g;
  actorPreselectedColor.b = b;
}

void SMESH_Actor::GetPreselectedColor(float& r,float& g,float& b)
{
  r = actorPreselectedColor.r;
  g = actorPreselectedColor.g;
  b = actorPreselectedColor.b;
}

void SMESH_Actor::SetHighlightColor(float r,float g,float b)
{
  actorHighlightColor.r = r;
  actorHighlightColor.g = g;
  actorHighlightColor.b = b;
}

void SMESH_Actor::GetHighlightColor(float& r,float& g,float& b)
{
  r = actorHighlightColor.r;
  g = actorHighlightColor.g;
  b = actorHighlightColor.b;
}

void SMESH_Actor::SetEdgeColor(float r,float g,float b)
{
  edgeColor.r = r;
  edgeColor.g = g;
  edgeColor.b = b;
}

void SMESH_Actor::GetEdgeColor(float& r,float& g,float& b)
{
  r = edgeColor.r;
  g = edgeColor.g;
  b = edgeColor.b;
}

void SMESH_Actor::SetEdgeHighlightColor(float r,float g,float b)
{
  edgeHighlightColor.r = r;
  edgeHighlightColor.g = g;
  edgeHighlightColor.b = b;
}

void SMESH_Actor::GetEdgeHighlightColor(float& r,float& g,float& b)
{
  r = edgeHighlightColor.r;
  g = edgeHighlightColor.g;
  b = edgeHighlightColor.b;
}

void SMESH_Actor::SetEdgePreselectedColor(float r,float g,float b)
{
  edgePreselectedColor.r = r;
  edgePreselectedColor.g = g;
  edgePreselectedColor.b = b;
}

void SMESH_Actor::GetEdgePreselectedColor(float& r,float& g,float& b)
{
  r = edgePreselectedColor.r;
  g = edgePreselectedColor.g;
  b = edgePreselectedColor.b;
}


void SMESH_Actor::SetNodeColor(float r,float g,float b)
{ 
  actorNodeColor.r = r ;
  actorNodeColor.g = g ;
  actorNodeColor.b = b ;
}

void SMESH_Actor::GetNodeColor(float& r,float& g,float& b)
{ 
  r = actorNodeColor.r ;
  g = actorNodeColor.g ;
  b = actorNodeColor.b ;
}

void SMESH_Actor::SetNodeSize(int size)
{
  actorNodeSize = size ;
}

int SMESH_Actor::GetNodeSize()
{
  return actorNodeSize ;
}


void SMESH_Actor::AddNode(int idSMESHDSnode,int idVTKnode)
{
  if (DataSource->IsA("SMESH_Grid")) {
    ((SMESH_Grid*)DataSource)->AddNode(idSMESHDSnode, idVTKnode);
  } else 
    MESSAGE("AddNode() method has been moved to SMESH_Grid class");
}
void SMESH_Actor::AddElement(int idSMESHDSelement, int idVTKelement)
{
  if (DataSource->IsA("SMESH_Grid")) {
    ((SMESH_Grid*)DataSource)->AddElement(idSMESHDSelement, idVTKelement);
  } else 
    MESSAGE("AddElement() method has been moved to SMESH_Grid class");
}

void SMESH_Actor::SetIdsVTKNode(const TColStd_DataMapOfIntegerInteger& mapVTK)
{
  if (DataSource->IsA("SMESH_Grid")) {
    ((SMESH_Grid*)DataSource)->SetIdsVTKNode(mapVTK);
  } else 
    MESSAGE("SetIdsVTKNode() method has been moved to SMESH_Grid class");
}
void SMESH_Actor::SetIdsSMESHDSNode(const TColStd_DataMapOfIntegerInteger& mapSMESHDS)
{
  if (DataSource->IsA("SMESH_Grid")) {
    ((SMESH_Grid*)DataSource)->SetIdsSMESHDSNode(mapSMESHDS);
  } else 
    MESSAGE("SetIdsSMESHDSNode() method has been moved to SMESH_Grid class");
}

void SMESH_Actor::SetIdsVTKElement(const TColStd_DataMapOfIntegerInteger& mapVTK)
{
  if (DataSource->IsA("SMESH_Grid")) {
    ((SMESH_Grid*)DataSource)->SetIdsVTKElement(mapVTK);
  } else 
    MESSAGE("SetIdsVTKElement() method has been moved to SMESH_Grid class");
}
void SMESH_Actor::SetIdsSMESHDSElement(const TColStd_DataMapOfIntegerInteger& mapSMESHDS)
{
  if (DataSource->IsA("SMESH_Grid")) {
    ((SMESH_Grid*)DataSource)->SetIdsSMESHDSElement(mapSMESHDS);
  } else 
    MESSAGE("SetIdsSMESHDSElement() method has been moved to SMESH_Grid class");
}

int SMESH_Actor::GetIdVTKNode(int idSMESHDSnode)
{
  if (DataSource->IsA("SMESH_Grid")) {
    return ((SMESH_Grid*)DataSource)->GetIdVTKNode(idSMESHDSnode);
  } else {
    MESSAGE("GetIdVTKNode() method has been moved to SMESH_Grid class");
    return -1;
  }
}
int SMESH_Actor::GetIdVTKElement(int idSMESHDSelement)
{
  if (DataSource->IsA("SMESH_Grid")) {
    return ((SMESH_Grid*)DataSource)->GetIdVTKElement(idSMESHDSelement);
  } else {
    MESSAGE("GetIdVTKElement() method has been moved to SMESH_Grid class");
    return -1;
  }

}

int SMESH_Actor::GetIdSMESHDSNode(int idVTKnode)
{
  if (DataSource->IsA("SMESH_Grid")) {
    return ((SMESH_Grid*)DataSource)->GetIdSMESHDSNode(idVTKnode);
  } else {
    MESSAGE("GetIdSMESHDSNode() method has been moved to SMESH_Grid class");
    return -1;
  }
}

int SMESH_Actor::GetIdSMESHDSElement(int idVTKelement)
{
  if (DataSource->IsA("SMESH_Grid")) {
    return ((SMESH_Grid*)DataSource)->GetIdSMESHDSElement(idVTKelement);
  } else {
    MESSAGE("AddNode() method has been moved to SMESH_Grid class");
    return -1;
  }
}

void SMESH_Actor::ClearNode()
{
  if (DataSource->IsA("SMESH_Grid")) {
    ((SMESH_Grid*)DataSource)->ClearNode();
  } else 
    MESSAGE("ClearNode() method has been moved to SMESH_Grid class");
}

void SMESH_Actor::ClearElement()
{
  if (DataSource->IsA("SMESH_Grid")) {
    ((SMESH_Grid*)DataSource)->ClearElement();
  } else 
    MESSAGE("ClearElement() method has been moved to SMESH_Grid class");
}

void SMESH_Actor::RemoveNode(int id)
{
  if (DataSource->IsA("SMESH_Grid")) {
    ((SMESH_Grid*)DataSource)->RemoveNode(id);
  } else 
    MESSAGE("RemoveNode() method has been moved to SMESH_Grid class");
}
void SMESH_Actor::RemoveElement(int id)
{
  if (DataSource->IsA("SMESH_Grid")) {
    ((SMESH_Grid*)DataSource)->RemoveElement(id);
  } else 
    MESSAGE("RemoveElement() method has been moved to SMESH_Grid class");
}

void SMESH_Actor::setDisplayMode(int thenewmode) {
  myDisplayMode = thenewmode;
  if ( myDisplayMode == 1 ) {
    EdgeDevice->VisibilityOn();
    EdgeShrinkDevice->VisibilityOff();
  } else if ( myDisplayMode == 2 ) {
    EdgeDevice->VisibilityOff();
    EdgeShrinkDevice->VisibilityOn();
  } else {
    EdgeDevice->VisibilityOff();
    EdgeShrinkDevice->VisibilityOff();
  }
}

float SMESH_Actor::GetShrinkFactor()
{
  return myShrinkFactor;
}

void SMESH_Actor::SetShrinkFactor(float value )
{
  if ( value <= 0.1 ) 
    value = 0.8;

  myShrinkFactor = value;
}

void SMESH_Actor::GetChildActors(vtkActorCollection* actors)
{
  actors->AddItem(EdgeDevice);
  actors->AddItem(EdgeShrinkDevice);
}

void SMESH_Actor::SetVisibility(bool visibility) 
{
  if ( visibility ) {
    this->VisibilityOn();
    if ( myDisplayMode == 1 ) {
      EdgeDevice->VisibilityOn();
      EdgeShrinkDevice->VisibilityOff();
    } else if ( myDisplayMode == 2 ) {
      EdgeDevice->VisibilityOff();
      EdgeShrinkDevice->VisibilityOn();
    } else {
      EdgeDevice->VisibilityOff();
      EdgeShrinkDevice->VisibilityOff();
    }
  } else {
    this->VisibilityOff();
    EdgeDevice->VisibilityOff();
    EdgeShrinkDevice->VisibilityOff();    
  }
}

