using namespace std;
// File:	SMDS_MeshElement.cxx
// Created:	Wed Jan 23 16:49:11 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_MeshElement.ixx"

//=======================================================================
//function : SMDS_MeshElement
//purpose  : 
//=======================================================================

SMDS_MeshElement::SMDS_MeshElement(const Standard_Integer ID, const Standard_Integer nb,const SMDSAbs_ElementType Type) 
  :myID(ID),myNbNodes(nb),myType(Type)
{
}

//=======================================================================
//function : GetConnections
//purpose  : 
//=======================================================================

Standard_Address SMDS_MeshElement::GetConnections() const
{
  return (Standard_Address)&myID;
}


//=======================================================================
//function : GetConnection
//purpose  : 
//=======================================================================

Standard_Integer SMDS_MeshElement::GetConnection(const Standard_Integer rank) const
{
  return myID;
}


//=======================================================================
//function : InverseElements
//purpose  : 
//=======================================================================

const SMDS_ListOfMeshElement& SMDS_MeshElement::InverseElements() const
{
  static SMDS_ListOfMeshElement empty;
  return empty;
}

//=======================================================================
//function : ClearInverseElements
//purpose  : 
//=======================================================================

void SMDS_MeshElement::ClearInverseElements()
{
}

//=======================================================================
//function : AddInverseElement
//purpose  : 
//=======================================================================

void SMDS_MeshElement::AddInverseElement(const Handle(SMDS_MeshElement)& elem)
{
}

//=======================================================================
//function : NbEdges
//purpose  : 
//=======================================================================

Standard_Integer SMDS_MeshElement::NbEdges() const
{
  return 0;
}

//=======================================================================
//function : NbFaces
//purpose  : 
//=======================================================================

Standard_Integer SMDS_MeshElement::NbFaces() const
{
  return 0;
}


//=======================================================================
//function : GetEdgeDefinedByNodes
//purpose  : 
//=======================================================================

void SMDS_MeshElement::GetEdgeDefinedByNodes(const Standard_Integer rank,
					     Standard_Integer& idnode1,
					     Standard_Integer& idnode2) const
{
  idnode1 = 0;
  idnode2 = 0;
}

//=======================================================================
//function : GetFaceDefinedByNodes
//purpose  : 
//=======================================================================

void SMDS_MeshElement::GetFaceDefinedByNodes(const Standard_Integer rank,
					     const Standard_Address idnode,
					     Standard_Integer& nb) const
{
  nb = 0;
}

//=======================================================================
//function : SetNormal
//purpose  : 
//=======================================================================

void SMDS_MeshElement::SetNormal(const Standard_Integer rank,
				 const Standard_Real vx,
				 const Standard_Real vy,
				 const Standard_Real vz)

{
  if (myNormals.IsNull()) {
    myNormals = new TColgp_HArray1OfDir(1,NbNodes());
  }
  myNormals->SetValue(rank, gp_Dir(vx,vy,vz));
}

//=======================================================================
//function : SetNormal
//purpose  : 
//=======================================================================

void SMDS_MeshElement::SetNormal(const Standard_Integer rank,
				 const gp_Vec& V)
{
  if (myNormals.IsNull()) {
    myNormals = new TColgp_HArray1OfDir(1,NbNodes());
  }
  myNormals->SetValue(rank, gp_Dir(V));
}

//=======================================================================
//function : GetNormal
//purpose  : 
//=======================================================================

gp_Dir SMDS_MeshElement::GetNormal(const Standard_Integer rank) 
{
  if (myNormals.IsNull()) {
    myNormals = new TColgp_HArray1OfDir(1,NbNodes());
  }
  return myNormals->Value(rank);
}

//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void SMDS_MeshElement::Print(Standard_OStream& OS) const
{
  OS << "dump of mesh element" << endl;
}


Standard_OStream& operator << (Standard_OStream& OS
			      ,const Handle(SMDS_MeshElement)& ME) 
{
  ME->Print(OS);
  return OS;
}
