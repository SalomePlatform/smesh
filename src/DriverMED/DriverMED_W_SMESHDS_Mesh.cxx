// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

//  SMESH DriverMED : driver to read and write 'med' files
//  File   : DriverMED_W_SMESHDS_Mesh.cxx
//  Module : SMESH
//
#include <sstream>

#include "DriverMED_W_SMESHDS_Mesh.h"
#include "DriverMED_Family.h"

#include "SMESHDS_Mesh.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_PolyhedralVolumeOfNodes.hxx"

#include "utilities.h"

#include "MED_Utilities.hxx"

#define _EDF_NODE_IDS_
//#define _ELEMENTS_BY_DIM_

using namespace std;
using namespace MED;


DriverMED_W_SMESHDS_Mesh::DriverMED_W_SMESHDS_Mesh():
  myAllSubMeshes (false),
  myDoGroupOfNodes (false),
  myDoGroupOfEdges (false),
  myDoGroupOfFaces (false),
  myDoGroupOfVolumes (false)
{}

void DriverMED_W_SMESHDS_Mesh::SetFile(const std::string& theFileName, 
                                       MED::EVersion theId)
{
  myMed = CrWrapper(theFileName,theId);
  Driver_SMESHDS_Mesh::SetFile(theFileName);
}

void DriverMED_W_SMESHDS_Mesh::SetFile(const std::string& theFileName)
{
  return SetFile(theFileName,MED::eV2_2);
}

string DriverMED_W_SMESHDS_Mesh::GetVersionString(const MED::EVersion theVersion, int theNbDigits)
{
  TInt majeur, mineur, release;
  majeur =  mineur = release = 0;
//   if ( theVersion == eV2_1 )
//     MED::GetVersionRelease<eV2_1>(majeur, mineur, release);
//   else
    MED::GetVersionRelease<eV2_2>(majeur, mineur, release);
  ostringstream name;
  if ( theNbDigits > 0 )
    name << majeur;
  if ( theNbDigits > 1 )
    name << "." << mineur;
  if ( theNbDigits > 2 )
    name << "." << release;
  return name.str();
}

void DriverMED_W_SMESHDS_Mesh::AddGroup(SMESHDS_GroupBase* theGroup)
{
  myGroups.push_back(theGroup);
}

void DriverMED_W_SMESHDS_Mesh::AddAllSubMeshes()
{
  myAllSubMeshes = true;
}

void DriverMED_W_SMESHDS_Mesh::AddSubMesh(SMESHDS_SubMesh* theSubMesh, int theID)
{
  mySubMeshes[theID] = theSubMesh;
}

void DriverMED_W_SMESHDS_Mesh::AddGroupOfNodes()
{
  myDoGroupOfNodes = true;
}

void DriverMED_W_SMESHDS_Mesh::AddGroupOfEdges()
{
  myDoGroupOfEdges = true;
}

void DriverMED_W_SMESHDS_Mesh::AddGroupOfFaces()
{
  myDoGroupOfFaces = true;
}

void DriverMED_W_SMESHDS_Mesh::AddGroupOfVolumes()
{
  myDoGroupOfVolumes = true;
}

namespace{
  typedef double (SMDS_MeshNode::* TGetCoord)() const;
  typedef const char* TName;
  typedef const char* TUnit;

  // name length in a mesh must be equal to 16 :
  //         1234567890123456
  TName M = "m               ";
  TName X = "x               ";
  TName Y = "y               ";
  TName Z = "z               ";

  TUnit aUnit[3] = {M,M,M};

  // 3 dim
  TGetCoord aXYZGetCoord[3] = {
    &SMDS_MeshNode::X, 
    &SMDS_MeshNode::Y, 
    &SMDS_MeshNode::Z
  };
  TName aXYZName[3] = {X,Y,Z};
  
  // 2 dim
  TGetCoord aXYGetCoord[2] = {
    &SMDS_MeshNode::X, 
    &SMDS_MeshNode::Y
  };
  TName aXYName[2] = {X,Y};

  TGetCoord aYZGetCoord[2] = {
    &SMDS_MeshNode::Y, 
    &SMDS_MeshNode::Z
  };
  TName aYZName[2] = {Y,Z};

  TGetCoord aXZGetCoord[2] = {
    &SMDS_MeshNode::X, 
    &SMDS_MeshNode::Z
  };
  TName aXZName[2] = {X,Z};

  // 1 dim
  TGetCoord aXGetCoord[1] = {
    &SMDS_MeshNode::X
  };
  TName aXName[1] = {X};

  TGetCoord aYGetCoord[1] = {
    &SMDS_MeshNode::Y
  };
  TName aYName[1] = {Y};

  TGetCoord aZGetCoord[1] = {
    &SMDS_MeshNode::Z
  };
  TName aZName[1] = {Z};


  class TCoordHelper{
    SMDS_NodeIteratorPtr myNodeIter;
    const SMDS_MeshNode* myCurrentNode;
    TGetCoord* myGetCoord;
    TName* myName;
    TUnit* myUnit;
  public:
    TCoordHelper(const SMDS_NodeIteratorPtr& theNodeIter,
                 TGetCoord* theGetCoord,
                 TName* theName,
                 TUnit* theUnit = aUnit):
      myNodeIter(theNodeIter),
      myGetCoord(theGetCoord),
      myName(theName),
      myUnit(theUnit)
    {}
    virtual ~TCoordHelper(){}
    bool Next(){ 
      return myNodeIter->more() && 
        (myCurrentNode = myNodeIter->next());
    }
    const SMDS_MeshNode* GetNode(){
      return myCurrentNode;
    }
    MED::TIntVector::value_type GetID(){
      return myCurrentNode->GetID();
    }
    MED::TFloatVector::value_type GetCoord(TInt theCoodId){
      return (myCurrentNode->*myGetCoord[theCoodId])();
    }
    MED::TStringVector::value_type GetName(TInt theDimId){
      return myName[theDimId];
    }
    MED::TStringVector::value_type GetUnit(TInt theDimId){
      return myUnit[theDimId];
    }
  };
  typedef boost::shared_ptr<TCoordHelper> TCoordHelperPtr;

  //-------------------------------------------------------
  /*!
   * \brief Structure describing element type
   */
  //-------------------------------------------------------
  struct TElemTypeData
  {
    EEntiteMaillage     _entity;
    EGeometrieElement   _geomType;
    TInt                _nbElems;
    SMDSAbs_ElementType _smdsType;

    TElemTypeData (EEntiteMaillage entity, EGeometrieElement geom, TInt nb, SMDSAbs_ElementType type)
      : _entity(entity), _geomType(geom), _nbElems( nb ), _smdsType( type ) {}
  };


  typedef NCollection_DataMap< Standard_Address, int > TElemFamilyMap;

  //================================================================================
  /*!
   * \brief Fills element to famaly ID map for element type.
   * Removes all families of anElemType
   */
  //================================================================================

  void fillElemFamilyMap( TElemFamilyMap &            anElemFamMap,
                          list<DriverMED_FamilyPtr> & aFamilies,
                          const SMDSAbs_ElementType   anElemType)
  {
    anElemFamMap.Clear();
    //anElemFamMap.clear();
    list<DriverMED_FamilyPtr>::iterator aFamsIter = aFamilies.begin();
    while ( aFamsIter != aFamilies.end() )
    {
      if ((*aFamsIter)->GetType() != anElemType) {
        aFamsIter++;
      }
      else {
        int aFamId = (*aFamsIter)->GetId();
        const set<const SMDS_MeshElement *>& anElems = (*aFamsIter)->GetElements();
        set<const SMDS_MeshElement *>::const_iterator anElemsIter = anElems.begin();
        for (; anElemsIter != anElems.end(); anElemsIter++)
        {
          anElemFamMap.Bind( (Standard_Address)*anElemsIter, aFamId );
          //anElemFamMap[*anElemsIter] = aFamId;
        }
        // remove a family from the list
        aFamilies.erase( aFamsIter++ );
      }
    }
  }

  //================================================================================
  /*!
   * \brief For an element, return family ID found in the map or a default one
   */
  //================================================================================

  int getFamilyId( const TElemFamilyMap &  anElemFamMap,
                   const SMDS_MeshElement* anElement,
                   const int               aDefaultFamilyId)
  {
    if ( anElemFamMap.IsBound( (Standard_Address) anElement ))
      return anElemFamMap( (Standard_Address) anElement );
//     TElemFamilyMap::iterator elem_famNum = anElemFamMap.find( anElement );
//     if ( elem_famNum != anElemFamMap.end() )
//       return elem_famNum->second;
    return aDefaultFamilyId;
  }
}

Driver_Mesh::Status DriverMED_W_SMESHDS_Mesh::Perform()
{
  Status aResult = DRS_OK;
  if (myMesh->hasConstructionEdges() || myMesh->hasConstructionFaces()) {
    INFOS("SMDS_MESH with hasConstructionEdges() or hasConstructionFaces() do not supports!!!");
    return DRS_FAIL;
  }
  try {
    MESSAGE("Perform - myFile : "<<myFile);

    // Creating the MED mesh for corresponding SMDS structure
    //-------------------------------------------------------
    string aMeshName;
    if (myMeshId != -1) {
      ostringstream aMeshNameStr;
      aMeshNameStr<<myMeshId;
      aMeshName = aMeshNameStr.str();
    } else {
      aMeshName = myMeshName;
    }

    // Mesh dimension definition
    TInt aSpaceDimension;
    TCoordHelperPtr aCoordHelperPtr;
    {  
      bool anIsXDimension = false;
      bool anIsYDimension = false;
      bool anIsZDimension = false;
      {
        SMDS_NodeIteratorPtr aNodesIter = myMesh->nodesIterator();
        double aBounds[6];
        if(aNodesIter->more()){
          const SMDS_MeshNode* aNode = aNodesIter->next();
          aBounds[0] = aBounds[1] = aNode->X();
          aBounds[2] = aBounds[3] = aNode->Y();
          aBounds[4] = aBounds[5] = aNode->Z();
        }
        while(aNodesIter->more()){
          const SMDS_MeshNode* aNode = aNodesIter->next();
          aBounds[0] = min(aBounds[0],aNode->X());
          aBounds[1] = max(aBounds[1],aNode->X());
          
          aBounds[2] = min(aBounds[2],aNode->Y());
          aBounds[3] = max(aBounds[3],aNode->Y());
          
          aBounds[4] = min(aBounds[4],aNode->Z());
          aBounds[5] = max(aBounds[5],aNode->Z());
        }

        double EPS = 1.0E-7;
        anIsXDimension = (aBounds[1] - aBounds[0]) + abs(aBounds[1]) + abs(aBounds[0]) > EPS;
        anIsYDimension = (aBounds[3] - aBounds[2]) + abs(aBounds[3]) + abs(aBounds[2]) > EPS;
        anIsZDimension = (aBounds[5] - aBounds[4]) + abs(aBounds[5]) + abs(aBounds[4]) > EPS;
        aSpaceDimension = anIsXDimension + anIsYDimension + anIsZDimension;
        if(!aSpaceDimension)
          aSpaceDimension = 3;
        // PAL16857(SMESH not conform to the MED convention):
        if ( aSpaceDimension == 2 && anIsZDimension ) // 2D only if mesh is in XOY plane
          aSpaceDimension = 3;
        // PAL18941(a saved study with a mesh belong Z is opened and the mesh is belong X)
        if ( aSpaceDimension == 1 && !anIsXDimension ) {// 1D only if mesh is along OX
          if ( anIsYDimension ) {
            aSpaceDimension = 2;
            anIsXDimension = true;
          } else {
            aSpaceDimension = 3;
          }
        }
      }

      SMDS_NodeIteratorPtr aNodesIter = myMesh->nodesIterator(/*idInceasingOrder=*/true);
      switch(aSpaceDimension){
      case 3:
        aCoordHelperPtr.reset(new TCoordHelper(aNodesIter,aXYZGetCoord,aXYZName));
        break;
      case 2:
        if(anIsXDimension && anIsYDimension)
          aCoordHelperPtr.reset(new TCoordHelper(aNodesIter,aXYGetCoord,aXYName));
        if(anIsYDimension && anIsZDimension)
          aCoordHelperPtr.reset(new TCoordHelper(aNodesIter,aYZGetCoord,aYZName));
        if(anIsXDimension && anIsZDimension)
          aCoordHelperPtr.reset(new TCoordHelper(aNodesIter,aXZGetCoord,aXZName));
        break;
      case 1:
        if(anIsXDimension)
          aCoordHelperPtr.reset(new TCoordHelper(aNodesIter,aXGetCoord,aXName));
        if(anIsYDimension)
          aCoordHelperPtr.reset(new TCoordHelper(aNodesIter,aYGetCoord,aYName));
        if(anIsZDimension)
          aCoordHelperPtr.reset(new TCoordHelper(aNodesIter,aZGetCoord,aZName));
        break;
      }
    }
    TInt aMeshDimension = 0;
    if ( myMesh->NbEdges() > 0 )
      aMeshDimension = 1;
    if ( myMesh->NbFaces() > 0 )
      aMeshDimension = 2;
    if ( myMesh->NbVolumes() > 0 )
      aMeshDimension = 3;
    
    PMeshInfo aMeshInfo = myMed->CrMeshInfo(aMeshDimension,aSpaceDimension,aMeshName);
    MESSAGE("Add - aMeshName : "<<aMeshName<<"; "<<aMeshInfo->GetName());
    myMed->SetMeshInfo(aMeshInfo);

    // Storing SMDS groups and sub-meshes as med families
    //----------------------------------------------------
    int myNodesDefaultFamilyId   = 0;
    int my0DElementsDefaultFamilyId = 0;
    int myEdgesDefaultFamilyId   = 0;
    int myFacesDefaultFamilyId   = 0;
    int myVolumesDefaultFamilyId = 0;
    int nbNodes   = myMesh->NbNodes();
    //int nb0DElements = myMesh->Nb0DElements();
    int nbEdges   = myMesh->NbEdges();
    int nbFaces   = myMesh->NbFaces();
    int nbVolumes = myMesh->NbVolumes();
    if (myDoGroupOfNodes && nbNodes)
      myNodesDefaultFamilyId = REST_NODES_FAMILY;
    if (myDoGroupOfEdges && nbEdges)
      myEdgesDefaultFamilyId = REST_EDGES_FAMILY;
    if (myDoGroupOfFaces && nbFaces)
      myFacesDefaultFamilyId = REST_FACES_FAMILY;
    if (myDoGroupOfVolumes && nbVolumes)
      myVolumesDefaultFamilyId = REST_VOLUMES_FAMILY;

    MESSAGE("Perform - aFamilyInfo");
    //cout << " DriverMED_Family::MakeFamilies() " << endl;
    list<DriverMED_FamilyPtr> aFamilies;
    if (myAllSubMeshes) {
      aFamilies = DriverMED_Family::MakeFamilies
        (myMesh->SubMeshes(), myGroups,
         myDoGroupOfNodes   && nbNodes,
         myDoGroupOfEdges   && nbEdges,
         myDoGroupOfFaces   && nbFaces,
         myDoGroupOfVolumes && nbVolumes);
    } else {
      aFamilies = DriverMED_Family::MakeFamilies
        (mySubMeshes, myGroups,
         myDoGroupOfNodes   && nbNodes,
         myDoGroupOfEdges   && nbEdges,
         myDoGroupOfFaces   && nbFaces,
         myDoGroupOfVolumes && nbVolumes);
    }
    //cout << " myMed->SetFamilyInfo() " << endl;
    list<DriverMED_FamilyPtr>::iterator aFamsIter;
    for (aFamsIter = aFamilies.begin(); aFamsIter != aFamilies.end(); aFamsIter++)
    {
      PFamilyInfo aFamilyInfo = (*aFamsIter)->GetFamilyInfo(myMed,aMeshInfo);
      myMed->SetFamilyInfo(aFamilyInfo);
    }

    // Storing SMDS nodes to the MED file for the MED mesh
    //----------------------------------------------------
#ifdef _EDF_NODE_IDS_
    typedef map<TInt,TInt> TNodeIdMap;
    TNodeIdMap aNodeIdMap;
#endif
    const EModeSwitch   theMode        = eFULL_INTERLACE;
    const ERepere       theSystem      = eCART;
    const EBooleen      theIsElemNum   = eVRAI;
    const EBooleen      theIsElemNames = eFAUX;
    const EConnectivite theConnMode    = eNOD;

    TInt aNbNodes = myMesh->NbNodes();
    //cout << " myMed->CrNodeInfo() aNbNodes = " << aNbNodes << endl;
    PNodeInfo aNodeInfo = myMed->CrNodeInfo(aMeshInfo, aNbNodes,
                                            theMode, theSystem, theIsElemNum, theIsElemNames);

    //cout << " fillElemFamilyMap( SMDSAbs_Node )" << endl;
    // find family numbers for nodes
    TElemFamilyMap anElemFamMap;
    fillElemFamilyMap( anElemFamMap, aFamilies, SMDSAbs_Node );

    for (TInt iNode = 0; aCoordHelperPtr->Next(); iNode++)
    {
      // coordinates
      TCoordSlice aTCoordSlice = aNodeInfo->GetCoordSlice( iNode );
      for(TInt iCoord = 0; iCoord < aSpaceDimension; iCoord++){
        aTCoordSlice[iCoord] = aCoordHelperPtr->GetCoord(iCoord);
      }
      // node number
      int aNodeID = aCoordHelperPtr->GetID();
      aNodeInfo->SetElemNum( iNode, aNodeID );
#ifdef _EDF_NODE_IDS_
      aNodeIdMap[aNodeID] = iNode+1;
#endif
      // family number
      const SMDS_MeshNode* aNode = aCoordHelperPtr->GetNode();
      int famNum = getFamilyId( anElemFamMap, aNode, myNodesDefaultFamilyId );
      aNodeInfo->SetFamNum( iNode, famNum );
    }
    anElemFamMap.Clear();

    // coordinate names and units
    for (TInt iCoord = 0; iCoord < aSpaceDimension; iCoord++) {
      aNodeInfo->SetCoordName( iCoord, aCoordHelperPtr->GetName(iCoord));
      aNodeInfo->SetCoordUnit( iCoord, aCoordHelperPtr->GetUnit(iCoord));
    }

    //cout << " SetNodeInfo(aNodeInfo)" << endl;
    MESSAGE("Perform - aNodeInfo->GetNbElem() = "<<aNbNodes);
    myMed->SetNodeInfo(aNodeInfo);
    aNodeInfo.reset(); // free memory used for arrays


    // Storing SMDS elements to the MED file for the MED mesh
    //-------------------------------------------------------
    // Write one element type at once in order to minimize memory usage (PAL19276)

    const SMDS_MeshInfo& nbElemInfo = myMesh->GetMeshInfo();

    // poly elements are not supported by med-2.1
    bool polyTypesSupported = myMed->CrPolygoneInfo(aMeshInfo,eMAILLE,ePOLYGONE,0,0);
    TInt nbPolygonNodes = 0, nbPolyhedronNodes = 0, nbPolyhedronFaces = 0;

    // collect info on all geom types

    list< TElemTypeData > aTElemTypeDatas;

    EEntiteMaillage anEntity = eMAILLE;
#ifdef _ELEMENTS_BY_DIM_
    anEntity = eNOEUD_ELEMENT;
#endif
    aTElemTypeDatas.push_back(TElemTypeData(anEntity,
                                            ePOINT1,
                                            nbElemInfo.Nb0DElements(),
                                            SMDSAbs_0DElement));
#ifdef _ELEMENTS_BY_DIM_
    anEntity = eARETE;
#endif
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                            eSEG2,
                                            nbElemInfo.NbEdges( ORDER_LINEAR ),
                                            SMDSAbs_Edge));
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                            eSEG3,
                                            nbElemInfo.NbEdges( ORDER_QUADRATIC ),
                                            SMDSAbs_Edge));
#ifdef _ELEMENTS_BY_DIM_
    anEntity = eFACE;
#endif
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                            eTRIA3,
                                            nbElemInfo.NbTriangles( ORDER_LINEAR ),
                                            SMDSAbs_Face));
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                            eTRIA6,
                                            nbElemInfo.NbTriangles( ORDER_QUADRATIC ),
                                            SMDSAbs_Face));
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                            eQUAD4,
                                            nbElemInfo.NbQuadrangles( ORDER_LINEAR ),
                                            SMDSAbs_Face));
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                             eQUAD8,
                                             nbElemInfo.NbQuadrangles( ORDER_QUADRATIC ) -
                                             nbElemInfo.NbBiQuadQuadrangles(),
                                             SMDSAbs_Face));
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                             eQUAD9,
                                             nbElemInfo.NbBiQuadQuadrangles(),
                                             SMDSAbs_Face));
    if ( polyTypesSupported ) {
      aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                               ePOLYGONE,
                                               nbElemInfo.NbPolygons(),
                                               SMDSAbs_Face));
      // we need one more loop on poly elements to count nb of their nodes
      aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                               ePOLYGONE,
                                               nbElemInfo.NbPolygons(),
                                               SMDSAbs_Face));
    }
#ifdef _ELEMENTS_BY_DIM_
    anEntity = eMAILLE;
#endif
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                            eTETRA4,
                                            nbElemInfo.NbTetras( ORDER_LINEAR ),
                                            SMDSAbs_Volume));
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                            eTETRA10,
                                            nbElemInfo.NbTetras( ORDER_QUADRATIC ),
                                            SMDSAbs_Volume));
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                            ePYRA5,
                                            nbElemInfo.NbPyramids( ORDER_LINEAR ),
                                            SMDSAbs_Volume));
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                            ePYRA13,
                                            nbElemInfo.NbPyramids( ORDER_QUADRATIC ),
                                            SMDSAbs_Volume));
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                            ePENTA6,
                                            nbElemInfo.NbPrisms( ORDER_LINEAR ),
                                            SMDSAbs_Volume));
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                             ePENTA15,
                                             nbElemInfo.NbPrisms( ORDER_QUADRATIC ),
                                             SMDSAbs_Volume));
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                             eHEXA8,
                                             nbElemInfo.NbHexas( ORDER_LINEAR ),
                                             SMDSAbs_Volume));
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                             eHEXA20,
                                             nbElemInfo.NbHexas( ORDER_QUADRATIC )-
                                             nbElemInfo.NbTriQuadHexas(),
                                             SMDSAbs_Volume));
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                             eHEXA27,
                                             nbElemInfo.NbTriQuadHexas(),
                                             SMDSAbs_Volume));
    aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                             eOCTA12,
                                             nbElemInfo.NbHexPrisms(),
                                             SMDSAbs_Volume));
    if ( polyTypesSupported ) {
      aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                               ePOLYEDRE,
                                               nbElemInfo.NbPolyhedrons(),
                                               SMDSAbs_Volume));
      // we need one more loop on poly elements to count nb of their nodes
      aTElemTypeDatas.push_back( TElemTypeData(anEntity,
                                               ePOLYEDRE,
                                               nbElemInfo.NbPolyhedrons(),
                                               SMDSAbs_Volume));
    }

    vector< bool > isElemFamMapBuilt( SMDSAbs_NbElementTypes, false );

    // loop on all geom types of elements

    list< TElemTypeData >::iterator aElemTypeData = aTElemTypeDatas.begin();
    for ( ; aElemTypeData != aTElemTypeDatas.end(); ++aElemTypeData )
    {
      if ( aElemTypeData->_nbElems == 0 )
        continue;

      int defaultFamilyId = 0;
      switch ( aElemTypeData->_smdsType ) {
      case SMDSAbs_0DElement:
        defaultFamilyId = my0DElementsDefaultFamilyId;
        break;
      case SMDSAbs_Edge:
        defaultFamilyId = myEdgesDefaultFamilyId;
        break;
      case SMDSAbs_Face:
        defaultFamilyId = myFacesDefaultFamilyId;
        break;
      case SMDSAbs_Volume:
        defaultFamilyId = myVolumesDefaultFamilyId;
        break;
      default:
        continue;
      }

      // iterator on elements of a current type
      SMDS_ElemIteratorPtr elemIterator = myMesh->elementsIterator( aElemTypeData->_smdsType );
      if ( !elemIterator->more()) continue;
      int iElem = 0;

      // Treat POLYGONs
      // ---------------
      if ( aElemTypeData->_geomType == ePOLYGONE )
      {
        if ( nbPolygonNodes == 0 ) {
          // Count nb of nodes
          while ( const SMDS_MeshElement* anElem = elemIterator->next() ) {
            if ( anElem->IsPoly() ) {
              nbPolygonNodes += anElem->NbNodes();
              if ( ++iElem == aElemTypeData->_nbElems )
                break;
            }
          }
        }
        else {
          // Store in med file
          PPolygoneInfo aPolygoneInfo = myMed->CrPolygoneInfo(aMeshInfo,
                                                              aElemTypeData->_entity,
                                                              aElemTypeData->_geomType,
                                                              aElemTypeData->_nbElems,
                                                              nbPolygonNodes,
                                                              theConnMode, theIsElemNum,
                                                              theIsElemNames);
          TElemNum & index = *(aPolygoneInfo->myIndex.get());
          index[0] = 1;

          while ( const SMDS_MeshElement* anElem = elemIterator->next() )
          {
            if ( !anElem->IsPoly() )
              continue;

            // index
            TInt aNbNodes = anElem->NbNodes();
            index[ iElem+1 ] = index[ iElem ] + aNbNodes;

            // connectivity
            TConnSlice aTConnSlice = aPolygoneInfo->GetConnSlice( iElem );
            for(TInt iNode = 0; iNode < aNbNodes; iNode++) {
              const SMDS_MeshElement* aNode = anElem->GetNode( iNode );
#ifdef _EDF_NODE_IDS_
              aTConnSlice[ iNode ] = aNodeIdMap[aNode->GetID()];
#else
              aTConnSlice[ iNode ] = aNode->GetID();
#endif
            }
            // element number
            aPolygoneInfo->SetElemNum( iElem, anElem->GetID() );

            // family number
            int famNum = getFamilyId( anElemFamMap, anElem, defaultFamilyId );
            aPolygoneInfo->SetFamNum( iElem, famNum );

            if ( ++iElem == aPolygoneInfo->GetNbElem() )
              break;
          }
          myMed->SetPolygoneInfo(aPolygoneInfo);
        }

      } 

      // Treat POLYEDREs
      // ----------------
      else if (aElemTypeData->_geomType == ePOLYEDRE )
      {
        if ( nbPolyhedronNodes == 0 ) {
          // Count nb of nodes
          while ( const SMDS_MeshElement* anElem = elemIterator->next() ) {
              const SMDS_VtkVolume *aPolyedre = dynamic_cast<const SMDS_VtkVolume*>(anElem);
              if ( aPolyedre && aPolyedre->IsPoly()) {
              nbPolyhedronNodes += aPolyedre->NbNodes();
              nbPolyhedronFaces += aPolyedre->NbFaces();
              if ( ++iElem == aElemTypeData->_nbElems )
                break;
            }
          }
        }
        else {
          // Store in med file
          PPolyedreInfo aPolyhInfo = myMed->CrPolyedreInfo(aMeshInfo,
                                                           aElemTypeData->_entity,
                                                           aElemTypeData->_geomType,
                                                           aElemTypeData->_nbElems,
                                                           nbPolyhedronFaces+1,
                                                           nbPolyhedronNodes,
                                                           theConnMode,
                                                           theIsElemNum,
                                                           theIsElemNames);
          TElemNum & index = *(aPolyhInfo->myIndex.get());
          TElemNum & faces = *(aPolyhInfo->myFaces.get());
          TElemNum & conn  = *(aPolyhInfo->myConn.get());
          index[0] = 1;
          faces[0] = 1;

          TInt iFace = 0, iNode = 0;
          while ( const SMDS_MeshElement* anElem = elemIterator->next() )
          {
            const SMDS_VtkVolume *aPolyedre = dynamic_cast<const SMDS_VtkVolume*>(anElem);
            if ( !aPolyedre )
              continue;
            if ( !aPolyedre->IsPoly() )
              continue;
            // index
            TInt aNbFaces = aPolyedre->NbFaces();
            index[ iElem+1 ] = index[ iElem ] + aNbFaces;

            // face index
            for (TInt f = 1; f <= aNbFaces; ++f, ++iFace ) {
              int aNbFaceNodes = aPolyedre->NbFaceNodes( f );
              faces[ iFace+1 ] = faces[ iFace ] + aNbFaceNodes;
            }
            // connectivity
            SMDS_ElemIteratorPtr nodeIt = anElem->nodesIterator();
            while ( nodeIt->more() ) {
              const SMDS_MeshElement* aNode = nodeIt->next();
#ifdef _EDF_NODE_IDS_
              conn[ iNode ] = aNodeIdMap[aNode->GetID()];
#else
              conn[ iNode ] = aNode->GetID();
#endif
              ++iNode;
            }
            // element number
            aPolyhInfo->SetElemNum( iElem, anElem->GetID() );

            // family number
            int famNum = getFamilyId( anElemFamMap, anElem, defaultFamilyId );
            aPolyhInfo->SetFamNum( iElem, famNum );

            if ( ++iElem == aPolyhInfo->GetNbElem() )
              break;
          }
          myMed->SetPolyedreInfo(aPolyhInfo);
        }
      } // if (aElemTypeData->_geomType == ePOLYEDRE )

      else
      {
        // Treat standard types
        // ---------------------

        // allocate data arrays
        PCellInfo aCellInfo = myMed->CrCellInfo( aMeshInfo,
                                                 aElemTypeData->_entity,
                                                 aElemTypeData->_geomType,
                                                 aElemTypeData->_nbElems,
                                                 theConnMode,
                                                 theIsElemNum,
                                                 theIsElemNames);
        // build map of family numbers for this type
        if ( !isElemFamMapBuilt[ aElemTypeData->_smdsType ])
        {
          //cout << " fillElemFamilyMap()" << endl;
          fillElemFamilyMap( anElemFamMap, aFamilies, aElemTypeData->_smdsType );
          isElemFamMapBuilt[ aElemTypeData->_smdsType ] = true;
        }

        TInt aNbNodes = MED::GetNbNodes(aElemTypeData->_geomType);
        while ( const SMDS_MeshElement* anElem = elemIterator->next() )
        {
          if ( anElem->NbNodes() != aNbNodes || anElem->IsPoly() )
            continue; // other geometry

          // connectivity
          TConnSlice aTConnSlice = aCellInfo->GetConnSlice( iElem );
          for (TInt iNode = 0; iNode < aNbNodes; iNode++) {
            const SMDS_MeshElement* aNode = anElem->GetNode( iNode );
#ifdef _EDF_NODE_IDS_
            aTConnSlice[ iNode ] = aNodeIdMap[aNode->GetID()];
#else
            aTConnSlice[ iNode ] = aNode->GetID();
#endif
          }
          // element number
          aCellInfo->SetElemNum( iElem, anElem->GetID() );

          // family number
          int famNum = getFamilyId( anElemFamMap, anElem, defaultFamilyId );
          aCellInfo->SetFamNum( iElem, famNum );

          if ( ++iElem == aCellInfo->GetNbElem() )
            break;
        }
        // store data in a file
        myMed->SetCellInfo(aCellInfo);
      }

    } // loop on geom types


  }
  catch(const std::exception& exc) {
    INFOS("The following exception was caught:\n\t"<<exc.what());
    throw;
  }
  catch(...) {
    INFOS("Unknown exception was caught !!!");
    throw;
  }

  myMeshId = -1;
  myGroups.clear();
  mySubMeshes.clear();
  return aResult;
}
