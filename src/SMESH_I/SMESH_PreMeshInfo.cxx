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
// File      : SMESH_PreMeshInfo.cxx
// Created   : Fri Feb 10 17:36:39 2012
// Author    : Edward AGAPOV (eap)
//

#include "SMESH_PreMeshInfo.hxx"

#include "DriverMED.hxx"
#include "DriverMED_R_SMESHDS_Mesh.h"
#include "MED_Factory.hxx"
#include "SMDS_EdgePosition.hxx"
#include "SMDS_FacePosition.hxx"
#include "SMDS_SpacePosition.hxx"
#include "SMDS_VertexPosition.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESHDS_GroupOnFilter.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Group_i.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_subMesh_i.hxx"

#include <HDFarray.hxx>
#include <HDFdataset.hxx>
#include <HDFfile.hxx>
#include <HDFgroup.hxx>
#include <SALOMEDS_Tool.hxx>
#include <SALOMEDS_wrap.hxx>

#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>

#include "SMESH_TryCatch.hxx"

#include CORBA_SERVER_HEADER(SALOME_Session)

using namespace std;

#define MYDEBUGOUT(msg) //std::cout << msg << std::endl;

namespace
{
  enum {  GroupOnFilter_OutOfDate = -1 };

  // a map to count not yet loaded meshes 
  static std::map< int, int > theStudyIDToMeshCounter;

  //================================================================================
  /*!
   * \brief Counts not fully loaded meshes
   */
  //================================================================================

  void meshInfoLoaded( SMESH_Mesh_i* mesh )
  {
    std::map< int, int >::iterator id2counter =
      theStudyIDToMeshCounter.insert( std::make_pair( (int) mesh->GetStudyId(), 0 )).first;
    id2counter->second++;
  }
  //================================================================================
  /*!
   * \brief Removes temporary files if none of meshes needs them
   */
  //================================================================================

  void filesNoMoreNeeded(SMESH_Mesh_i* mesh,
                         std::string   medFile,
                         std::string   hdfFile)
  {
    if ( --theStudyIDToMeshCounter[ (int) mesh->GetStudyId() ] == 0 )
    {
      std::string tmpDir = SALOMEDS_Tool::GetDirFromPath( hdfFile );

      SALOMEDS::ListOfFileNames_var aFiles = new SALOMEDS::ListOfFileNames;
      aFiles->length(2);
      medFile = SALOMEDS_Tool::GetNameFromPath( medFile ) + ".med";
      hdfFile = SALOMEDS_Tool::GetNameFromPath( hdfFile ) + ".hdf";
      aFiles[0] = medFile.c_str();
      aFiles[1] = hdfFile.c_str();

      SALOMEDS_Tool::RemoveTemporaryFiles( tmpDir.c_str(), aFiles.in(), true );
    }
  }

  //=============================================================================
  /*!
   * \brief Class sending signals on start and finish of loading
   */
  //=============================================================================

  class SignalToGUI
  {
    std::string         _messagePrefix;
    SALOME::Session_var _session;
  public:
    SignalToGUI( SMESH_Mesh_i* mesh )
    {
      SMESH_Gen_i* gen = SMESH_Gen_i::GetSMESHGen();
      SALOMEDS::Study_var study = gen->GetCurrentStudy();
      if ( !study->_is_nil() && study->StudyId() == mesh->GetStudyId() )
      {
        SALOMEDS::SObject_wrap meshSO = gen->ObjectToSObject(study, mesh->_this() );
        CORBA::Object_var        obj = gen->GetNS()->Resolve( "/Kernel/Session" );
        _session = SALOME::Session::_narrow( obj );
        if ( !meshSO->_is_nil() && !_session->_is_nil() )
        {
          CORBA::String_var meshEntry = meshSO->GetID();
          _messagePrefix = "SMESH/mesh_loading/";
          _messagePrefix += meshEntry.in();

          std::string msgToGUI = _messagePrefix + "/";
          msgToGUI += SMESH_Comment( mesh->NbNodes() );
          msgToGUI += "/";
          msgToGUI += SMESH_Comment( mesh->NbElements() );

          _session->emitMessageOneWay( msgToGUI.c_str());
        }
      }
    }
    void sendStop()
    {
      if ( !_messagePrefix.empty() )
      {
        std::string msgToGUI = _messagePrefix + "/stop";
        _session->emitMessageOneWay( msgToGUI.c_str());
        _messagePrefix.clear();
      }
    }
    ~SignalToGUI() { sendStop(); }
  };

  //=============================================================================
  /*!
   * \brief Creates SMDS_Position according to shape type
   */
  //=============================================================================

  class PositionCreator {
  public:
    SMDS_PositionPtr MakePosition(const TopAbs_ShapeEnum type) {
      return (this->*myFuncTable[ type ])();
    }
    PositionCreator() {
      myFuncTable.resize( (size_t) TopAbs_SHAPE, & PositionCreator::defaultPosition );
      myFuncTable[ TopAbs_SOLID  ] = & PositionCreator::volumePosition;
      myFuncTable[ TopAbs_FACE   ] = & PositionCreator::facePosition;
      myFuncTable[ TopAbs_EDGE   ] = & PositionCreator::edgePosition;
      myFuncTable[ TopAbs_VERTEX ] = & PositionCreator::vertexPosition;
    }
  private:
    SMDS_PositionPtr edgePosition()    const { return SMDS_PositionPtr( new SMDS_EdgePosition  ); }
    SMDS_PositionPtr facePosition()    const { return SMDS_PositionPtr( new SMDS_FacePosition  ); }
    SMDS_PositionPtr volumePosition()  const { return SMDS_PositionPtr( new SMDS_SpacePosition ); }
    SMDS_PositionPtr vertexPosition()  const { return SMDS_PositionPtr( new SMDS_VertexPosition); }
    SMDS_PositionPtr defaultPosition() const { return SMDS_SpacePosition::originSpacePosition();  }
    typedef SMDS_PositionPtr (PositionCreator:: * FmakePos)() const;
    std::vector<FmakePos> myFuncTable;
  };

  //================================================================================
  /*!
   * \brief Returns ids of simple shapes composing a complex one
   */
  //================================================================================

  std::vector<int> getSimpleSubMeshIds( SMESHDS_Mesh* meshDS, int shapeId )
  {
    std::vector<int> ids;

    std::list<TopoDS_Shape> shapeQueue( 1, meshDS->IndexToShape( shapeId ));
    std::list<TopoDS_Shape>::iterator shape = shapeQueue.begin();
    for ( ; shape != shapeQueue.end(); ++shape )
    {
      if ( shape->IsNull() ) continue;
      if ( shape->ShapeType() == TopAbs_COMPOUND ||
           shape->ShapeType() == TopAbs_COMPSOLID )
      {
        for ( TopoDS_Iterator it( *shape ); it.More(); it.Next() )
          shapeQueue.push_back( it.Value() );
      }
      else
      {
        ids.push_back( meshDS->ShapeToIndex( *shape ));
      }
    }
    return ids;
  }

  //================================================================================
  /*!
   * \brief Return EEntiteMaillage by EGeometrieElement
   */
  //================================================================================

  MED::EEntiteMaillage entityByGeom(const MED::EGeometrieElement geom )
  {
    return geom == MED::eBALL ? MED::eSTRUCT_ELEMENT : MED::eMAILLE;
  }

  //================================================================================
  /*!
   * \brief Return a map< EGeometrieElement, SMDSAbs_EntityType >
   */
  //================================================================================

  typedef std::map< MED::EGeometrieElement, SMDSAbs_EntityType > Tmed2smeshElemTypeMap;
  const Tmed2smeshElemTypeMap& med2smeshElemTypeMap()
  {
    static Tmed2smeshElemTypeMap med2smeshTypes;
    if ( med2smeshTypes.empty() )
    {
      for  ( int iG = 0; iG < SMDSEntity_Last; ++iG )
      {
        SMDSAbs_EntityType    smdsType = (SMDSAbs_EntityType) iG;
        MED::EGeometrieElement medType =
          (MED::EGeometrieElement) DriverMED::GetMedGeoType( smdsType );
        med2smeshTypes.insert( std::make_pair( medType, smdsType ));
      }
    }
    return med2smeshTypes;
  }

  //================================================================================
  /*!
   * \brief Writes meshInfo into a HDF file
   */
  //================================================================================

  void meshInfo2hdf( SMESH::long_array_var meshInfo,
                     const std::string&    name,
                     HDFgroup*             hdfGroup)
  {
    // we use med identification of element (MED::EGeometrieElement) types
    // but not enum SMDSAbs_EntityType because values of SMDSAbs_EntityType may
    // change at insertion of new items in the middle.
    //const vector<MED::EGeometrieElement>& medTypes = mesh2medElemType();

    std::vector<int> data;

    for ( size_t i = 0; i < meshInfo->length(); ++i )
      if ( meshInfo[i] > 0 )
      {
        data.push_back( DriverMED::GetMedGeoType( SMDSAbs_EntityType( i ))); //medTypes[ i ] );
        data.push_back( meshInfo[ i ] );
      }

    if ( !data.empty() )
    {
      hdf_size datasetSize[] = { data.size() };
      HDFarray* anArray = new HDFarray(0, HDF_INT32, 1, datasetSize);
      anArray->CreateOnDisk();
      datasetSize[0] = 1;
      HDFdataset* dataset = new HDFdataset( name.c_str(), hdfGroup, HDF_ARRAY, datasetSize, 1 );
      dataset->SetArrayId(anArray->GetId());
      dataset->CreateOnDisk();
      dataset->WriteOnDisk( & data[0]  );
      dataset->CloseOnDisk();
      anArray->CloseOnDisk();
    }
  }
}

//================================================================================
/*!
 * \brief Reads meshInfo from a HDF file
 */
//================================================================================

void SMESH_PreMeshInfo::hdf2meshInfo( const std::string& name,
                                      HDFgroup*          hdfGroup)
{
  if ( hdfGroup->ExistInternalObject( name.c_str()) )
  {
    HDFdataset* dataset = new HDFdataset( name.c_str(), hdfGroup );
    dataset->OpenOnDisk();

    // // hdf_size datasetSize[ 1 ];
    // // HDFarray *array = new HDFarray(dataset);
    // // array->GetDim( datasetSize );
    // int size = dataset->GetSize();

    std::vector<int> info( SMDSEntity_Last * 2, 0 );
    dataset->ReadFromDisk( &info[0] );
    dataset->CloseOnDisk();

    const Tmed2smeshElemTypeMap& med2smesh = med2smeshElemTypeMap();
    Tmed2smeshElemTypeMap::const_iterator me2sme, me2smeEnd = med2smesh.end();
    for ( size_t i = 0; i < info.size(); )
    {
      int medType = info[i++];
      int nbElems = info[i++];
      if ( !nbElems ) break;
      me2sme = med2smesh.find( (MED::EGeometrieElement) medType );
      if ( me2sme != me2smeEnd )
        setNb( me2sme->second, nbElems );
    }
  }
  _isInfoOk = true;

  if ( NbNodes() == GroupOnFilter_OutOfDate ) // case of !SMESHDS_GroupOnFilter::IsUpToDate()
  {
    _isInfoOk = false;
    setNb( SMDSEntity_Node, 0 );
  }
}

//================================================================================
/*!
 * \brief Constructor callable by SMESH_PreMeshInfo only
 */
//================================================================================

SMESH_PreMeshInfo::SMESH_PreMeshInfo(SMESH_Mesh_i*      mesh,
                                     const int          meshID,
                                     const std::string& medFile,
                                     const std::string& hdfFile)
  : _medFileName( medFile ),
    _hdfFileName( hdfFile ),
    _toRemoveFiles( false ),
    _meshID( meshID ),
    _mesh( mesh ),
    _isInfoOk( false ),
    _elemCounter( 0 )
{
}

//================================================================================
/*!
 * \brief Release temporary files
 */
//================================================================================

SMESH_PreMeshInfo::~SMESH_PreMeshInfo()
{
  if ( _toRemoveFiles ) // it can be true only for SMESH_PreMeshInfo of the mesh
    filesNoMoreNeeded( _mesh, _medFileName, _hdfFileName );

  _toRemoveFiles = false;
}

//================================================================================
/*!
 * \brief fills SMESH_PreMeshInfo field of all objects of mesh
 */
//================================================================================

void SMESH_PreMeshInfo::LoadFromFile( SMESH_Mesh_i*      mesh,
                                      const int          meshID,
                                      const std::string& medFile,
                                      const std::string& hdfFile,
                                      const bool         toRemoveFiles)
{
  SMESH_TRY;

  SMESH_PreMeshInfo* meshPreInfo = new SMESH_PreMeshInfo( mesh,meshID,medFile,hdfFile );
  mesh->changePreMeshInfo() = meshPreInfo;

  meshPreInfo->_toRemoveFiles = toRemoveFiles;
  if ( toRemoveFiles )
    meshInfoLoaded( mesh );

  if ( meshPreInfo->readPreInfoFromHDF() )
    // all SMESH_PreMeshInfo's are stored in HDF file (written after
    // implementing SMESH_PreMeshInfo)
    return;

  // try to read SMESH_PreMeshInfo from med file (as study is older than SMESH_PreMeshInfo)
  if ( meshPreInfo->readMeshInfo() )
  {
    meshPreInfo->readGroupInfo();
    meshPreInfo->readSubMeshInfo();
  }
  else
  {
    meshPreInfo->FullLoadFromFile();
  }
  SMESH_CATCH( SMESH::doNothing );
}

//================================================================================
/*!
 * \brief Tries to read all SMESH_PreMeshInfo from a HDF file
 *  \retval bool - true if succeeded
 *
 * This method is symmetrical to SaveToFile()
 */
//================================================================================

bool SMESH_PreMeshInfo::readPreInfoFromHDF()
{
  HDFfile* aFile = new HDFfile( (char*) _hdfFileName.c_str() );
  aFile->OpenOnDisk( HDF_RDONLY );

  SMESH_Comment hdfGroupName("SMESH_PreMeshInfo"); hdfGroupName << _meshID;
  const bool infoAvailable = aFile->ExistInternalObject( hdfGroupName );
  if ( infoAvailable )
  {
    HDFgroup* infoHdfGroup = new HDFgroup( hdfGroupName, aFile );
    infoHdfGroup->OpenOnDisk();

    _mesh->changePreMeshInfo()->hdf2meshInfo( "Mesh", infoHdfGroup );

    // read SMESH_PreMeshInfo of groups
    map<int, SMESH::SMESH_GroupBase_ptr>::const_iterator i2group = _mesh->_mapGroups.begin();
    for ( ; i2group != _mesh->_mapGroups.end(); ++i2group )
    {
      if ( SMESH_GroupBase_i* group_i =
           SMESH::DownCast<SMESH_GroupBase_i*>( i2group->second ))
      {
        group_i->changePreMeshInfo() = newInstance();
        if ( SMESHDS_GroupBase* group = group_i->GetGroupDS() )
        {
          const std::string name = group->GetStoreName();
          group_i->changePreMeshInfo()->hdf2meshInfo( name, infoHdfGroup );
        }
      }
    }

    // read SMESH_PreMeshInfo of sub-meshes
    map<int, SMESH::SMESH_subMesh_ptr>::iterator id2sm = _mesh->_mapSubMeshIor.begin();
    for ( ; id2sm != _mesh->_mapSubMeshIor.end(); ++id2sm )
    {
      if ( SMESH_subMesh_i* sm = SMESH::DownCast<SMESH_subMesh_i*>( id2sm->second ))
      {
        sm->changePreMeshInfo() = newInstance();
        sm->changePreMeshInfo()->hdf2meshInfo( SMESH_Comment( sm->GetId()), infoHdfGroup );
      }
    }
  }

  aFile->CloseOnDisk();
  delete aFile;

  return infoAvailable;
}

//================================================================================
/*!
 * \brief Reads mesh info of mesh from the med file
 */
//================================================================================

bool SMESH_PreMeshInfo::readMeshInfo()
{
  _isInfoOk = true;

  MED::PWrapper aMed = MED::CrWrapper(_medFileName,true);
  // if ( aMed->GetVersion() != MED::eV2_2 )
  //   return false;

  MED::PMeshInfo medMeshInfo = aMed->CrMeshInfo(3,3,SMESH_Comment( _meshID ));

  // read nb nodes
  int nbNodes = std::max( 0, aMed->GetNbNodes( medMeshInfo ));
  if ( nbNodes > 0 )
  {
    setNb( SMDSEntity_Node, nbNodes);

    // read nb of elements
    Tmed2smeshElemTypeMap::const_iterator me2sme    = med2smeshElemTypeMap().begin();
    Tmed2smeshElemTypeMap::const_iterator me2smeEnd = med2smeshElemTypeMap().end();
    for ( ; me2sme != me2smeEnd; ++me2sme )
    {
      int nbElems = aMed->GetNbCells( medMeshInfo, entityByGeom(me2sme->first), me2sme->first );
      if ( nbElems > 0 )
        setNb( me2sme->second, nbElems );
    }
  }
  return true;
}

//================================================================================
/*!
 * \brief Reads info of groups from the med file
 */
//================================================================================

void SMESH_PreMeshInfo::readGroupInfo()
{
  if ( _mesh->_mapGroups.empty() ) return;

  // make SMESH_PreMeshInfo of groups
  map< std::string, SMESH_PreMeshInfo* > name2GroupInfo;
  map<int, SMESH::SMESH_GroupBase_ptr>::const_iterator i2group = _mesh->_mapGroups.begin();
  for ( ; i2group != _mesh->_mapGroups.end(); ++i2group )
  {
    if ( SMESH_GroupBase_i* group_i =
         SMESH::DownCast<SMESH_GroupBase_i*>( i2group->second ))
    {
      SMESH_PreMeshInfo* info = newInstance();
      group_i->changePreMeshInfo() = info;
      if ( SMESHDS_Group* group = dynamic_cast< SMESHDS_Group* >( group_i->GetGroupDS() ))
      {
        std::string name = group->GetStoreName();
        name2GroupInfo.insert( std::make_pair( name, info ));
        info->_isInfoOk = true;
      }
    }
  }

  map< int, vector< SMESH_PreMeshInfo* > > famId2grInfo;

  MED::PWrapper aMed = MED::CrWrapper(_medFileName,false);
  MED::PMeshInfo medMeshInfo = aMed->CrMeshInfo(3,3,SMESH_Comment( _meshID ));

  // read families to fill in famId2grInfo
  int nbFams = aMed->GetNbFamilies( medMeshInfo );
  if ( nbFams <= 1 ) return; // zero family is always present
  for ( int iF = 0; iF <= nbFams; ++iF )
  {
    int nbGroups = aMed->GetNbFamGroup( iF, medMeshInfo );
    if ( nbGroups < 1 ) continue;
    MED::PFamilyInfo medFamInfo = aMed->CrFamilyInfo( medMeshInfo, nbGroups, nbGroups );
    aMed->GetFamilyInfo( iF, medFamInfo ); // read groups of a family
    vector< SMESH_PreMeshInfo* >& grInfoVec = famId2grInfo[ medFamInfo->GetId() ];
    for ( int iG = 0; iG < nbGroups; ++iG )
    {
      const std::string grName = medFamInfo->GetGroupName( iG );
      map< std::string, SMESH_PreMeshInfo* >::iterator n2i = name2GroupInfo.find( grName );
      if ( n2i != name2GroupInfo.end() )
        grInfoVec.push_back( n2i->second );
    }
  }

  // read family numbers of elements
  Tmed2smeshElemTypeMap::const_iterator me2sme    = med2smeshElemTypeMap().begin();
  Tmed2smeshElemTypeMap::const_iterator me2smeEnd = med2smeshElemTypeMap().end();
  MED::PElemInfo medElemInfo = aMed->CrElemInfo( medMeshInfo, 0 );
  MED::TIntVector& famNums = medElemInfo->myFamNum;
  for ( ; me2sme != me2smeEnd; ++me2sme ) // loop on elem types
  {
    famNums.resize( NbEntities( me2sme->second ));
    if ( famNums.empty() ) continue;
    aMed->GetFamilies( medElemInfo, famNums.size(), entityByGeom(me2sme->first), me2sme->first );
    // distribute elements of a type among groups
    map< int, vector< SMESH_PreMeshInfo* > >::iterator f2infos = famId2grInfo.begin();
    for ( size_t i = 0; i < famNums.size(); ++i )
    {
      if ( famNums[i] != f2infos->first )
      {
        f2infos = famId2grInfo.find( famNums[i] );
        if ( f2infos == famId2grInfo.end() )
          f2infos = famId2grInfo.insert
            ( std::make_pair( famNums[i], vector< SMESH_PreMeshInfo*>())).first;
      }
      vector< SMESH_PreMeshInfo* >& infoVec = f2infos->second ;
      for ( size_t j = 0; j < infoVec.size(); ++j )
        infoVec[j]->_elemCounter++;
    }
    // pass _elemCounter to a real elem type
    map< std::string, SMESH_PreMeshInfo* >::iterator n2i = name2GroupInfo.begin();
    for ( ; n2i != name2GroupInfo.end(); ++n2i )
    {
      SMESH_PreMeshInfo* info = n2i->second;
      info->setNb( me2sme->second, info->_elemCounter );
      info->_elemCounter = 0;
    }
  }
}

//================================================================================
/*!
 * \brief Reads info of sub-meshes from hdf file of old study
 */
//================================================================================

void SMESH_PreMeshInfo::readSubMeshInfo()
{
  if ( _mesh->_mapSubMeshIor.empty() ) return;

  // create SMESH_PreMeshInfo of sub-meshes
  map<int, SMESH::SMESH_subMesh_ptr>::iterator id2sm = _mesh->_mapSubMeshIor.begin();
  for ( ; id2sm != _mesh->_mapSubMeshIor.end(); ++id2sm )
  {
    if ( SMESH_subMesh_i* sm = SMESH::DownCast<SMESH_subMesh_i*>( id2sm->second ))
    {
      sm->changePreMeshInfo() = newInstance();
      sm->changePreMeshInfo()->_isInfoOk = true;
    }
  }

  // try to read 
  HDFfile* aFile = new HDFfile( (char*) _hdfFileName.c_str() );
  aFile->OpenOnDisk( HDF_RDONLY );

  char meshGrpName[ 30 ];
  sprintf( meshGrpName, "Mesh %d", _meshID );
  if ( aFile->ExistInternalObject( meshGrpName ) )
  {
    HDFgroup* aTopGroup = new HDFgroup( meshGrpName, aFile );
    aTopGroup->OpenOnDisk();
    if ( aTopGroup->ExistInternalObject( "Submeshes" ))
    {
      HDFgroup* aGroup = new HDFgroup( "Submeshes", aTopGroup );
      aGroup->OpenOnDisk();

      SMESHDS_Mesh* meshDS = _mesh->GetImpl().GetMeshDS();
      int maxSmId = Max( meshDS->MaxSubMeshIndex(), meshDS->MaxShapeIndex() );

      for ( int isNode = 0; isNode < 2; ++isNode )
      {
        std::string aDSName( isNode ? "Node Submeshes" : "Element Submeshes");
        if ( aGroup->ExistInternalObject( (char*) aDSName.c_str() ))
        {
          // read sub-mesh id of all nodes or elems
          HDFdataset* aDataset = new HDFdataset( (char*) aDSName.c_str(), aGroup );
          aDataset->OpenOnDisk();
          int nbElems = aDataset->GetSize();
          int* smIDs = new int [ nbElems ];
          aDataset->ReadFromDisk( smIDs );
          aDataset->CloseOnDisk();
          // count nb elems in each sub-mesh
          vector<int> nbBySubmeshId( maxSmId + 1, 0 );
          for ( int i = 0; i < nbElems; ++i )
          {
            const int smID = smIDs[ i ];
            if ( smID < (int) nbBySubmeshId.size() )
              nbBySubmeshId[ smID ]++;
          }
          delete [] smIDs;

          // store nb elems in SMESH_PreMeshInfo of sub-meshes
          map<int, SMESH::SMESH_subMesh_ptr>::iterator id2sm = _mesh->_mapSubMeshIor.begin();
          for ( ; id2sm != _mesh->_mapSubMeshIor.end(); ++id2sm )
          {
            if ( SMESH_subMesh_i* sm = SMESH::DownCast<SMESH_subMesh_i*>( id2sm->second ))
            {
              SMESH_PreMeshInfo* & info = sm->changePreMeshInfo();

              vector<int> smIds = getSimpleSubMeshIds( meshDS, id2sm->first );
              for ( size_t i = 0; i < smIds.size(); ++i )
                info->_elemCounter += nbBySubmeshId[ smIds[i] ];

              SMDSAbs_EntityType elemType;
              if ( isNode )
              {
                elemType = SMDSEntity_Node;
              }
              else
              {
                bool koElemType = false;
                const TopoDS_Shape& shape = meshDS->IndexToShape( smIds[0] );
                elemType = getElemType( shape.ShapeType(), info->_elemCounter, koElemType );
                info->_isInfoOk = !koElemType;
              }
              info->setNb( elemType, info->_elemCounter );
            }
          }
        } // if ( aGroup->ExistInternalObject( aDSName ))
      } // for ( int isNode = 0; isNode < 2; ++isNode )

      aGroup->CloseOnDisk();
    } // if ( aTopGroup->ExistInternalObject( "Submeshes" ))

    aTopGroup->CloseOnDisk();
  } // if ( aFile->ExistInternalObject( meshGrpName ) )

  aFile->CloseOnDisk();
  delete aFile;
}

//================================================================================
/*!
 * \brief Return type of element for sub-mesh on a shape of given type
 */
//================================================================================

SMDSAbs_EntityType SMESH_PreMeshInfo::getElemType( const TopAbs_ShapeEnum shapeType,
                                                   const int              nbElemsInSubMesh,
                                                   bool&                  isKoType) const
{
  isKoType = false;
  int type, typeEnd;
  SMESH_PreMeshInfo* meshInfo = _mesh->changePreMeshInfo();

  switch ( shapeType )
  {
  case TopAbs_SOLID:
    type = SMDSEntity_Tetra;
    typeEnd = SMDSEntity_Last;
    isKoType = ( meshInfo->NbVolumes() != nbElemsInSubMesh );
    break;
  case TopAbs_FACE:
  case TopAbs_SHELL:  
    type = SMDSEntity_Triangle;
    typeEnd = SMDSEntity_Tetra;
    isKoType = ( meshInfo->NbFaces() != nbElemsInSubMesh );
    break;
  case TopAbs_WIRE:
  case TopAbs_EDGE:   return SMDSEntity_Edge;
  case TopAbs_VERTEX: return SMDSEntity_0D;
  default:            return SMDSEntity_Last;
  }

  if ( !isKoType )
  {
    for ( int t = type; t < typeEnd; ++t )
      if ( nbElemsInSubMesh == meshInfo->NbEntities( SMDSAbs_EntityType( t )))
        return SMDSAbs_EntityType( t );
  }
  isKoType = true;
  return SMDSAbs_EntityType( type );
}

//================================================================================
/*!
 * \brief Saves SMESH_PreMeshInfo to the study file
 */
//================================================================================

void SMESH_PreMeshInfo::SaveToFile( SMESH_Mesh_i* mesh,
                                    const int     meshID,
                                    HDFfile*      hdfFile)
{
  // create a HDF group for SMESH_PreMeshInfo of this mesh
  SMESH_Comment hdfGroupName("SMESH_PreMeshInfo"); hdfGroupName << meshID;
  HDFgroup* infoHdfGroup = new HDFgroup( hdfGroupName, hdfFile );
  infoHdfGroup->CreateOnDisk();

  SMESH_TRY;

  // info of mesh
  meshInfo2hdf( mesh->GetMeshInfo(), "Mesh", infoHdfGroup );
  
  // info of groups
  SMESH_PreMeshInfo incompleteInfo( 0,0,"","");
  incompleteInfo.setNb( SMDSEntity_Node, GroupOnFilter_OutOfDate );
  SMESHDS_Mesh* meshDS = mesh->GetImpl().GetMeshDS();

  map<int, SMESH::SMESH_GroupBase_ptr>::const_iterator i2group = mesh->_mapGroups.begin();
  for ( ; i2group != mesh->_mapGroups.end(); ++i2group )
  {
    if ( SMESH_GroupBase_i* group_i = SMESH::DownCast<SMESH_GroupBase_i*>( i2group->second ))
    {
      SMESHDS_GroupBase * group = group_i->GetGroupDS();
      if ( SMESHDS_GroupOnFilter* gof = dynamic_cast<SMESHDS_GroupOnFilter*>(group))
      {
        // prevent too long storage time due to applying filter to many elements
        if ( !gof->IsUpToDate() && meshDS->GetMeshInfo().NbElements( gof->GetType() ) > 1e5 )
        {
          meshInfo2hdf( incompleteInfo.GetMeshInfo(),
                        group->GetStoreName(),
                        infoHdfGroup);
          continue;
        }
      }
      meshInfo2hdf( group_i->GetMeshInfo(), group->GetStoreName(), infoHdfGroup);
    }
  }

  // info of sub-meshes
  map<int, SMESH::SMESH_subMesh_ptr>::iterator id2sm = mesh->_mapSubMeshIor.begin();
  for ( ; id2sm != mesh->_mapSubMeshIor.end(); ++id2sm )
  {
    if ( SMESH_subMesh_i* sm = SMESH::DownCast<SMESH_subMesh_i*>( id2sm->second ))
    {
      meshInfo2hdf( sm->GetMeshInfo(),
                    SMESH_Comment( sm->GetId() ),
                    infoHdfGroup);
    }
  }

  SMESH_CATCH( SMESH::doNothing );

  infoHdfGroup->CloseOnDisk();
}

//================================================================================
/*!
 * \brief Reads all data and remove all SMESH_PreMeshInfo fields from objects
 */
//================================================================================

void SMESH_PreMeshInfo::FullLoadFromFile() const
{
  SignalToGUI signalOnLoading( _mesh );

  SMESH_PreMeshInfo* meshInfo = _mesh->changePreMeshInfo();
  _mesh->changePreMeshInfo() = NULL; // to allow GUI accessing to real info

  ::SMESH_Mesh&   mesh = _mesh->GetImpl();
  SMESHDS_Mesh* meshDS = mesh.GetMeshDS();

  SMESH_TRY;

  MYDEBUGOUT( "BEG FullLoadFromFile() " << _meshID );

  // load mesh
  DriverMED_R_SMESHDS_Mesh myReader;
  myReader.SetFile( _medFileName.c_str() );
  myReader.SetMesh( meshDS );
  myReader.SetMeshId( _meshID );
  myReader.Perform();

  // load groups
  const set<SMESHDS_GroupBase*>& groups = meshDS->GetGroups();
  set<SMESHDS_GroupBase*>::const_iterator groupIt = groups.begin();
  for ( ; groupIt != groups.end(); ++groupIt )
    if ( SMESHDS_Group* aGrp = dynamic_cast<SMESHDS_Group*>( *groupIt ))
      myReader.GetGroup( aGrp );

  // load sub-meshes
  readSubMeshes( &myReader );

  SMESH_CATCH( SMESH::doNothing );

  _mesh->changePreMeshInfo() = meshInfo;

  ForgetAllData();

  signalOnLoading.sendStop();

  meshDS->Modified();

  // load dependent meshes referring/referred via hypotheses
  mesh.GetSubMesh( mesh.GetShapeToMesh() )->
    ComputeStateEngine (SMESH_subMesh::SUBMESH_LOADED);

  MYDEBUGOUT( "END FullLoadFromFile()" );
}

//================================================================================
/*!
 * \brief Reads full data of sub-meshes
 */
//================================================================================

void SMESH_PreMeshInfo::readSubMeshes(DriverMED_R_SMESHDS_Mesh* reader) const
{
  HDFfile* aFile = new HDFfile( (char*) _hdfFileName.c_str() );
  aFile->OpenOnDisk( HDF_RDONLY );

  char meshGrpName[ 30 ];
  sprintf( meshGrpName, "Mesh %d", _meshID );
  if ( aFile->ExistInternalObject( meshGrpName ) )
  {
    HDFgroup* aTopGroup = new HDFgroup( meshGrpName, aFile );
    aTopGroup->OpenOnDisk();

    SMESHDS_Mesh* meshDS = _mesh->GetImpl().GetMeshDS();

    // issue 0020693. Restore _isModified flag
    if ( aTopGroup->ExistInternalObject( "_isModified" ))
    {
      HDFdataset* aDataset = new HDFdataset( "_isModified", aTopGroup );
      aDataset->OpenOnDisk();
      hdf_size size = aDataset->GetSize();
      int* isModified = new int[ size ];
      aDataset->ReadFromDisk( isModified );
      aDataset->CloseOnDisk();
      _mesh->GetImpl().SetIsModified( bool(*isModified));
      delete [] isModified;
    }

    bool submeshesInFamilies = ( ! aTopGroup->ExistInternalObject( "Submeshes" ));
    if ( submeshesInFamilies ) // from MED
    {
      // old way working before fix of PAL 12992
      reader->CreateAllSubMeshes();
    }
    else
    {
      // open a group
      HDFgroup* aGroup = new HDFgroup( "Submeshes", aTopGroup );
      aGroup->OpenOnDisk();

      int maxID = Max( meshDS->MaxSubMeshIndex(), meshDS->MaxShapeIndex() );
      vector< SMESHDS_SubMesh * > subMeshes( maxID + 1, (SMESHDS_SubMesh*) 0 );
      vector< TopAbs_ShapeEnum  > smType   ( maxID + 1, TopAbs_SHAPE );

      PositionCreator aPositionCreator;

      SMDS_NodeIteratorPtr nIt = meshDS->nodesIterator();
      SMDS_ElemIteratorPtr eIt = meshDS->elementsIterator();
      for ( int isNode = 0; isNode < 2; ++isNode )
      {
        std::string aDSName( isNode ? "Node Submeshes" : "Element Submeshes");
        if ( aGroup->ExistInternalObject( (char*) aDSName.c_str() ))
        {
          HDFdataset* aDataset = new HDFdataset( (char*) aDSName.c_str(), aGroup );
          aDataset->OpenOnDisk();
          // read submesh IDs for all elements sorted by ID
          size_t nbElems = aDataset->GetSize();
          int* smIDs = new int [ nbElems ];
          aDataset->ReadFromDisk( smIDs );
          aDataset->CloseOnDisk();

          // get elements sorted by ID
          TIDSortedElemSet elemSet;
          if ( isNode )
            while ( nIt->more() ) elemSet.insert( elemSet.end(), nIt->next() );
          else
            while ( eIt->more() ) elemSet.insert( elemSet.end(), eIt->next() );
          //ASSERT( elemSet.size() == nbElems ); -- issue 20182
          // -- Most probably a bad study was saved when there were
          // not fixed bugs in SMDS_MeshInfo
          if ( elemSet.size() < nbElems ) {
#ifdef _DEBUG_
            cout << "SMESH_Gen_i::Load(), warning: Node position data is invalid" << endl;
#endif
            nbElems = elemSet.size();
          }
          // add elements to sub-meshes
          TIDSortedElemSet::iterator iE = elemSet.begin();
          for ( size_t i = 0; i < nbElems; ++i, ++iE )
          {
            int smID = smIDs[ i ];
            if ( smID == 0 ) continue;
            const SMDS_MeshElement* elem = *iE;
            if ( smID > maxID ) {
              // corresponding subshape no longer exists: maybe geom group has been edited
              if ( _mesh->GetImpl().HasShapeToMesh() )
                meshDS->RemoveElement( elem );
              continue;
            }
            // get or create submesh
            SMESHDS_SubMesh* & sm = subMeshes[ smID ];
            if ( ! sm ) {
              sm = meshDS->NewSubMesh( smID );
              smType[ smID ] = meshDS->IndexToShape( smID ).ShapeType();
            }
            // add
            if ( isNode ) {
              SMDS_PositionPtr pos = aPositionCreator.MakePosition( smType[ smID ]);
              SMDS_MeshNode* node = const_cast<SMDS_MeshNode*>( static_cast<const SMDS_MeshNode*>( elem ));
              node->SetPosition( pos );
              sm->AddNode( node );
            } else {
              sm->AddElement( elem );
            }
          }
          delete [] smIDs;
        }
      }
    } // end reading submeshes

    // Read node positions on sub-shapes (SMDS_Position)

    if ( aTopGroup->ExistInternalObject( "Node Positions" ))
    {
      // There are 5 datasets to read:
      // "Nodes on Edges" - ID of node on edge
      // "Edge positions" - U parameter on node on edge
      // "Nodes on Faces" - ID of node on face
      // "Face U positions" - U parameter of node on face
      // "Face V positions" - V parameter of node on face
      const char* aEid_DSName = "Nodes on Edges";
      const char* aEu_DSName  = "Edge positions";
      const char* aFu_DSName  = "Face U positions";
      //char* aFid_DSName = "Nodes on Faces";
      //char* aFv_DSName  = "Face V positions";

      // data to retrieve
      int nbEids = 0, nbFids = 0;
      int *aEids = 0, *aFids  = 0;
      double *aEpos = 0, *aFupos = 0, *aFvpos = 0;

      // open a group
      HDFgroup* aGroup = new HDFgroup( "Node Positions", aTopGroup );
      aGroup->OpenOnDisk();

      // loop on 5 data sets
      int aNbObjects = aGroup->nInternalObjects();
      for ( int i = 0; i < aNbObjects; i++ )
      {
        // identify dataset
        char aDSName[ HDF_NAME_MAX_LEN+1 ];
        aGroup->InternalObjectIndentify( i, aDSName );
        // read data
        HDFdataset* aDataset = new HDFdataset( aDSName, aGroup );
        aDataset->OpenOnDisk();
        if ( aDataset->GetType() == HDF_FLOAT64 ) // Positions
        {
          double* pos = new double [ aDataset->GetSize() ];
          aDataset->ReadFromDisk( pos );
          // which one?
          if ( strncmp( aDSName, aEu_DSName, strlen( aEu_DSName )) == 0 )
            aEpos = pos;
          else if ( strncmp( aDSName, aFu_DSName, strlen( aFu_DSName )) == 0 )
            aFupos = pos;
          else
            aFvpos = pos;
        }
        else // NODE IDS
        {
          int aSize = aDataset->GetSize();

          // for reading files, created from 18.07.2005 till 10.10.2005
          if (aDataset->GetType() == HDF_STRING)
            aSize /= sizeof(int);

          int* ids = new int [aSize];
          aDataset->ReadFromDisk( ids );
          // on face or nodes?
          if ( strncmp( aDSName, aEid_DSName, strlen( aEid_DSName )) == 0 ) {
            aEids = ids;
            nbEids = aSize;
          }
          else {
            aFids = ids;
            nbFids = aSize;
          }
        }
        aDataset->CloseOnDisk();
      } // loop on 5 datasets

      // Set node positions on edges or faces
      for ( int onFace = 0; onFace < 2; onFace++ )
      {
        int nbNodes = ( onFace ? nbFids : nbEids );
        if ( nbNodes == 0 ) continue;
        int* aNodeIDs = ( onFace ? aFids : aEids );
        double* aUPos = ( onFace ? aFupos : aEpos );
        double* aVPos = ( onFace ? aFvpos : 0 );
        // loop on node IDs
        for ( int iNode = 0; iNode < nbNodes; iNode++ )
        {
          const SMDS_MeshNode* node = meshDS->FindNode( aNodeIDs[ iNode ]);
          if ( !node ) continue; // maybe removed while Loading() if geometry changed
          SMDS_PositionPtr aPos = node->GetPosition();
          ASSERT( aPos );
          if ( onFace ) {
            // ASSERT( aPos->GetTypeOfPosition() == SMDS_TOP_FACE );-- issue 20182
            // -- Most probably a bad study was saved when there were
            // not fixed bugs in SMDS_MeshInfo
            if ( aPos->GetTypeOfPosition() == SMDS_TOP_FACE ) {
              SMDS_FacePosition* fPos = const_cast<SMDS_FacePosition*>
                ( static_cast<const SMDS_FacePosition*>( aPos ));
              fPos->SetUParameter( aUPos[ iNode ]);
              fPos->SetVParameter( aVPos[ iNode ]);
            }
          }
          else {
            // ASSERT( aPos->GetTypeOfPosition() == SMDS_TOP_EDGE );-- issue 20182
            if ( aPos->GetTypeOfPosition() == SMDS_TOP_EDGE ) {
              SMDS_EdgePosition* fPos = const_cast<SMDS_EdgePosition*>
                ( static_cast<const SMDS_EdgePosition*>( aPos ));
              fPos->SetUParameter( aUPos[ iNode ]);
            }
          }
        }
      }
      if ( aEids ) delete [] aEids;
      if ( aFids ) delete [] aFids;
      if ( aEpos ) delete [] aEpos;
      if ( aFupos ) delete [] aFupos;
      if ( aFvpos ) delete [] aFvpos;

      aGroup->CloseOnDisk();

    } // if ( aTopGroup->ExistInternalObject( "Node Positions" ) )

    aTopGroup->CloseOnDisk();
  } // if ( aFile->ExistInternalObject( meshGrpName ) )
  
  aFile->CloseOnDisk();
  delete aFile;
}

//================================================================================
/*!
 * \brief Remove all SMESH_PreMeshInfo fields from objects w/o data loading
 */
//================================================================================

void SMESH_PreMeshInfo::ForgetAllData() const
{
  SMESH_TRY;

  if ( _mesh->changePreMeshInfo() != this )
    return _mesh->changePreMeshInfo()->ForgetAllData();

  // remove SMESH_PreMeshInfo from groups
  map<int, SMESH::SMESH_GroupBase_ptr>::const_iterator i2group = _mesh->_mapGroups.begin();
  for ( ; i2group != _mesh->_mapGroups.end(); ++i2group )
  {
    if ( SMESH_GroupBase_i* group_i =
         SMESH::DownCast<SMESH_GroupBase_i*>( i2group->second ))
    {
      SMESH_PreMeshInfo* & info = group_i->changePreMeshInfo();
      delete info;
      info = NULL;
    }
  }
  // remove SMESH_PreMeshInfo from sub-meshes
  map<int, SMESH::SMESH_subMesh_ptr>::iterator id2sm = _mesh->_mapSubMeshIor.begin();
  for ( ; id2sm != _mesh->_mapSubMeshIor.end(); ++id2sm )
  {
    if ( SMESH_subMesh_i* sm_i = SMESH::DownCast<SMESH_subMesh_i*>( id2sm->second ))
    {
      SMESH_PreMeshInfo* & info = sm_i->changePreMeshInfo();
      delete info;
      info = NULL;
    }
  }
  // remove SMESH_PreMeshInfo from the mesh
  _mesh->changePreMeshInfo() = NULL;
  delete this;

  SMESH_CATCH( SMESH::doNothing );


  // Finalize loading

  // SMESH_TRY;

  // ::SMESH_Mesh& mesh = _mesh->GetImpl();

  // // update hyps needing full mesh data restored (issue 20918)
  // // map<int, SMESH::SMESH_Hypothesis_ptr>::iterator id2hyp= _mesh->_mapHypo.begin();
  // // for ( ; id2hyp != _mesh->_mapHypo.end(); ++id2hyp )
  // //   if ( SMESH_Hypothesis_i* hyp = SMESH::DownCast<SMESH_Hypothesis_i*>( id2hyp->second ))
  // //     hyp->UpdateAsMeshesRestored();


  // SMESH_CATCH( SMESH::doNothing );
}

//================================================================================
/*!
 * \brief remove all SMESH_PreMeshInfo fields from mesh and its child objects w/o data loading
 */
//================================================================================

void SMESH_PreMeshInfo::ForgetAllData( SMESH_Mesh_i* mesh )
{
  if ( mesh && mesh->changePreMeshInfo() )
    mesh->changePreMeshInfo()->ForgetAllData();
}

//================================================================================
/*!
 * \brief Calls either FullLoadFromFile() or ForgetAllData() depending on preferences
 */
//================================================================================

void SMESH_PreMeshInfo::ForgetOrLoad() const
{
  if ( SMESH_Gen_i::GetSMESHGen()->ToForgetMeshDataOnHypModif() &&
       _mesh->HasShapeToMesh())
    ForgetAllData();
  else
    FullLoadFromFile();
}

//================================================================================
/*!
 * \brief Method of SMESH_IDSource interface
 */
//================================================================================

SMESH::array_of_ElementType* SMESH_PreMeshInfo::GetTypes() const
{
  SMESH::array_of_ElementType_var types = new SMESH::array_of_ElementType;

  types->length( 4 );
  int nbTypes = 0;
  if (NbEdges())      types[nbTypes++] = SMESH::EDGE;
  if (NbFaces())      types[nbTypes++] = SMESH::FACE;
  if (NbVolumes())    types[nbTypes++] = SMESH::VOLUME;
  if (Nb0DElements()) types[nbTypes++] = SMESH::ELEM0D;
  if (NbBalls())      types[nbTypes++] = SMESH::BALL;
  types->length( nbTypes );

  return types._retn();
}

//================================================================================
/*!
 * \brief Method of SMESH_IDSource interface returning nb elements by element type
 */
//================================================================================

SMESH::long_array* SMESH_PreMeshInfo::GetMeshInfo() const
{
  SMESH::long_array_var aRes = new SMESH::long_array();
  aRes->length(SMESH::Entity_Last);
  for (int i = SMESH::Entity_Node; i < SMESH::Entity_Last; i++)
    aRes[i] = 0;

  for (int i = SMESH::Entity_Node; i < SMESH::Entity_Last; i++)
    aRes[i] = NbEntities((SMDSAbs_EntityType)i);
  return aRes._retn();
}

//================================================================================
/*!
 * Returns false if GetMeshInfo() returns incorrect information that may
 * happen if mesh data is not yet fully loaded from the file of study.
 */
//================================================================================

bool SMESH_PreMeshInfo::IsMeshInfoCorrect() const
{
  return _isInfoOk;
}

//================================================================================
/*!
 * \brief TEMPORARY method to remove study files on closing study;
 * RIGHT WAY: study files are remove automatically when meshes are destroyed
 */
//================================================================================

void SMESH_PreMeshInfo::RemoveStudyFiles_TMP_METHOD(SALOMEDS::SComponent_ptr smeshComp)
{
  SALOMEDS::Study_var study = smeshComp->GetStudy();
  if ( theStudyIDToMeshCounter[ (int) study->StudyId() ] > 0 )
  {
    SALOMEDS::ChildIterator_wrap itBig = study->NewChildIterator( smeshComp );
    for ( ; itBig->More(); itBig->Next() ) {
      SALOMEDS::SObject_wrap gotBranch = itBig->Value();
      CORBA::Object_var       anObject = SMESH_Gen_i::SObjectToObject( gotBranch );
      if ( SMESH_Mesh_i* mesh = SMESH::DownCast<SMESH_Mesh_i*>( anObject ))
      {
        if ( mesh->changePreMeshInfo() )
        {
          mesh->changePreMeshInfo()->ForgetAllData();
        }
      }
    }
  }
}
