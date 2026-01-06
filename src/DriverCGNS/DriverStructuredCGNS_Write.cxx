// Copyright (C) 2016-2026  CEA, EDF
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// File      : DriverStructuredCGNS_Write.cxx
// Created   : Tuesday March 19 2024
// Author    : Cesar Conopoima (cce)

#include "DriverStructuredCGNS_Write.hxx"

#include "SMDS_IteratorOnIterators.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMESHDS_GroupBase.hxx"
#include "SMESHDS_GroupOnGeom.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESH_Block.hxx"
#include "SMESH_Comment.hxx"

#include <TopoDS_Solid.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_DataMapOfShapeReal.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

//CAS
#include <BRep_Tool.hxx>

// CGNS
#include <cgnslib.h>

#if CGNS_VERSION < 3100
# define cgsize_t int
#endif


std::string DriverStructuredCGNS_Write::GetGroupName( const int shapeToIndex, int dim )
{
  auto groupBase = myMesh->GetGroups();
  for( auto grp : groupBase )
  {
    /*associate the group based on geometry*/
    if ( dynamic_cast<SMESHDS_GroupOnGeom*>( grp ))
    {
      SMESHDS_GroupOnGeom * geomGrp = dynamic_cast<SMESHDS_GroupOnGeom*>( grp );
      TopoDS_Shape shape = geomGrp->GetShape();
      if( myMesh->ShapeToIndex(shape) == shapeToIndex  )
        return std::string(geomGrp->GetStoreName());
      
      if ( dim==3 && (shape.ShapeType() == TopAbs_COMPSOLID || shape.ShapeType() == TopAbs_COMPOUND ))
      {
        for ( TopExp_Explorer fEx( shape, TopAbs_SOLID ); fEx.More(); fEx.Next() )
        {
          if( myMesh->ShapeToIndex(fEx.Current()) == shapeToIndex  )
            return std::string(geomGrp->GetStoreName());
        }
      }

      if ( dim==2 && (shape.ShapeType() == TopAbs_SHELL || shape.ShapeType() == TopAbs_COMPOUND) )
      {
        for ( TopExp_Explorer fEx( shape, TopAbs_FACE, TopAbs_SOLID ); fEx.More(); fEx.Next() )
        {
          if( myMesh->ShapeToIndex(fEx.Current()) == shapeToIndex  )
            return std::string(geomGrp->GetStoreName());
        }
      }
    }
  }
  // else by convention, create a group called ZONESOLID or ZONEFACE
  std::string emptyZoneName("ZONE");
  if (dim == 3)
    emptyZoneName += "SOLID";
  else
    emptyZoneName += "FACE";
  return emptyZoneName;
}

void DriverStructuredCGNS_Write::GetGroupNamesMap( const TopoDS_Shape& shape, int dim, std::map<size_t, std::string>& groupNameMapById )
{
  // get all zone names
  // get a unique name for each zone name
  std::map<std::string, size_t> countGroupName;
  std::map<size_t, std::string> originalGroupNameMapById;
  TopAbs_ShapeEnum exploreType;
  if (dim == 3)
    exploreType = TopAbs_SOLID;
  else
    exploreType = TopAbs_FACE;
  size_t i = 0;
  for ( TopExp_Explorer fEx( shape, exploreType ); fEx.More(); fEx.Next(), ++i )
  {
    size_t solidId;
    if (dim == 3)
    {
      auto currentSolid = TopoDS::Solid(fEx.Current()); 
      solidId = myMesh->ShapeToIndex(currentSolid);
    }
    else
    {
      auto currentSolid = TopoDS::Face(fEx.Current()); 
      solidId = myMesh->ShapeToIndex(currentSolid);
    }
    std::string zoneName = GetGroupName(solidId, dim);
    // store the original group name
    originalGroupNameMapById[solidId] = zoneName;
    // update the frequency count
    countGroupName[zoneName]++;
    // append frequency count to end of the string
    size_t count = countGroupName[zoneName];
    zoneName += "_"+std::to_string(count);
    // store the name with suffix
    groupNameMapById[solidId] = zoneName;
  }
  // don't use the group with suffix if there is only one solid with this name
  for (auto const& pair : originalGroupNameMapById)
  {
    size_t solidId = pair.first;
    std::string zoneName = pair.second;
    size_t count = countGroupName[zoneName];
    if (count == 1)
      groupNameMapById[solidId] = zoneName;
  }
}

void DriverStructuredCGNS_Write::CheckForGroupNameOnFaceInterfaces( const SMESHUtils::SMESH_RegularGrid* grid, std::vector<std::string>& boundaryNames )
{
  auto groupBase = myMesh->GetGroups();
  for( auto grp : groupBase )  
  {
    if ( dynamic_cast<SMESHDS_GroupOnGeom*>( grp ) )
    {
      SMESHDS_GroupOnGeom * geomGrp = dynamic_cast<SMESHDS_GroupOnGeom*>( grp );
      TopoDS_Shape shape = geomGrp->GetShape();
      if( shape.ShapeType() == TopAbs_FACE  )
      {
        SMESHUtils::SMESH_RegularGrid::FaceType f = grid->getFaceTypeByGeomFace( shape );      
        if ( f != SMESHUtils::SMESH_RegularGrid::B_NONE ) 
          boundaryNames[ int(f) ] = geomGrp->GetStoreName();
      }

      if ( shape.ShapeType() == TopAbs_SHELL || shape.ShapeType() == TopAbs_COMPOUND )
      {
        for ( TopExp_Explorer fEx( shape, TopAbs_FACE, TopAbs_SOLID ); fEx.More(); fEx.Next() )
        {
          SMESHUtils::SMESH_RegularGrid::FaceType f = grid->getFaceTypeByGeomFace( fEx.Current() ); 
          if ( f != SMESHUtils::SMESH_RegularGrid::B_NONE )
            boundaryNames[ int(f) ] = geomGrp->GetStoreName(); 
        }
      }      
    }
  }
}

void DriverStructuredCGNS_Write::CheckForGroupNameOnEdgeInterfaces( const SMESHUtils::SMESH_RegularGrid* grid, std::vector<std::string>& boundaryNames )
{
  auto groupBase = myMesh->GetGroups();
  for( auto grp : groupBase )  
  {
    if ( dynamic_cast<SMESHDS_GroupOnGeom*>( grp ) )
    {
      SMESHDS_GroupOnGeom * geomGrp = dynamic_cast<SMESHDS_GroupOnGeom*>( grp );
      TopoDS_Shape shape = geomGrp->GetShape();
      if( shape.ShapeType() == TopAbs_EDGE  )
      {
        SMESHUtils::SMESH_RegularGrid::EdgeType e = grid->getEdgeTypeByGeomEdge( shape );      
        if ( e != SMESHUtils::SMESH_RegularGrid::NONE ) 
          boundaryNames[ int(e) ] = geomGrp->GetStoreName();
      }

      if ( shape.ShapeType() == TopAbs_COMPOUND )
      {
        for ( TopExp_Explorer fEx( shape, TopAbs_EDGE ); fEx.More(); fEx.Next() )
        {
          SMESHUtils::SMESH_RegularGrid::EdgeType e = grid->getEdgeTypeByGeomEdge( fEx.Current() ); 
          if ( e != SMESHUtils::SMESH_RegularGrid::NONE )
            boundaryNames[ int(e) ] = geomGrp->GetStoreName(); 
        }
      }
    }
  }
}

//================================================================================
/*!
 * \brief Write the mesh into the CGNS file
 */
//================================================================================

Driver_Mesh::Status DriverStructuredCGNS_Write::Perform()
{
  myErrorMessages.clear();

  if ( !myMesh || myMesh->GetMeshInfo().NbElements() < 1 )
    return addMessage( !myMesh ? "NULL mesh" : "Empty mesh (no elements)", /*fatal = */true );

  if ( Driver_Mesh::IsMeshTooLarge< cgsize_t >( myMesh, /*checkIDs =*/ false))
    return DRS_TOO_LARGE_MESH;

  // open the file
  if ( cg_open(myFile.c_str(), CG_MODE_MODIFY, &_fn) != CG_OK &&
       cg_open(myFile.c_str(), CG_MODE_WRITE,  &_fn) != CG_OK )
    return addMessage( cg_get_error(), /*fatal = */true );

  // create a Base
  // --------------

  const int spaceDim = 3;
  int        meshDim = 1;
  if ( myMesh->NbFaces()   > 0 ) meshDim = 2;
  if ( myMesh->NbVolumes() > 0 ) meshDim = 3;

  if ( myMeshName.empty() )
  {
    int nbases = 0;
    if ( cg_nbases( _fn, &nbases) == CG_OK )
      myMeshName = ( SMESH_Comment("Base_") << nbases+1 );
    else
      myMeshName = "Base_0";
  }
  int iBase;
  if ( cg_base_write( _fn, myMeshName.c_str(), meshDim, spaceDim, &iBase ) != CG_OK )
    return addMessage( cg_get_error(), /*fatal = */true );

  if ( cg_goto(_fn, iBase, "end") != CG_OK )
    return addMessage( cg_get_error(), /*fatal = */true );

  if ( cg_descriptor_write("About", "Created by SMESH") != CG_OK )
    return addMessage( cg_get_error(), /*fatal = */true );

  // create a structured Zone
  // --------------

  TopoDS_Shape shape = myMesh->ShapeToMesh();

  if ( meshDim == 3 )
  {
    // get a map of unique names for each solid
    std::map<size_t, std::string> groupNameMapById;
    GetGroupNamesMap(shape, meshDim, groupNameMapById);
    // Fill the cgns info for each solid
    for ( TopExp_Explorer fEx( shape, TopAbs_SOLID ); fEx.More(); fEx.Next() )
    {
      TopoDS_Solid currentSolid = TopoDS::Solid(fEx.Current()); 

      if ( myMesh->HasStructuredGridFilled(currentSolid) )
      {
        auto grid = myMesh->GetTheGrid(currentSolid).get();
        int imax = grid->nx();
        int jmax = grid->ny();
        int kmax = grid->nz();

        cgsize_t size[9] = {imax, jmax, kmax, imax - 1, jmax - 1, kmax - 1, 0, 0, 0};

        size_t currentSolidId = myMesh->ShapeToIndex(currentSolid);
        std::string zoneName = groupNameMapById[currentSolidId];
        // write Zone
        int iZone;
        if(cg_zone_write(_fn, iBase, zoneName.c_str(), size,
                      CGNS_ENUMV(Structured), &iZone) != CG_OK)
          return addMessage( cg_get_error(), /*fatal = */true );    

        // write Grid
        int iGrid=0;
        if(cg_grid_write(_fn, iBase, iZone, "GridCoordinates", &iGrid) != CG_OK)
          return addMessage( cg_get_error(), /*fatal = */true );

        // write the Coordinates      
        std::vector< double > coords( grid->Size() /*nx*ny*nz*/);
        // write X-Coordinates
        auto coordIter = grid->CoordinateBegin();
        for ( int i = 0; i < grid->Size(); coordIter.Next(), i++ ) 
          coords[i] = coordIter.Value()->X();

        if ( cg_coord_write( _fn, iBase, iZone, CGNS_ENUMV(RealDouble),
                              "CoordinateX", &coords[0], &iGrid) != CG_OK )
          return addMessage( cg_get_error(), /*fatal = */true );
        
        // write Y-Coordinates
        coordIter = grid->CoordinateBegin();
        for ( int i = 0; i < grid->Size(); coordIter.Next(), i++ ) 
          coords[i] = coordIter.Value()->Y();
        
        if ( cg_coord_write( _fn, iBase, iZone, CGNS_ENUMV(RealDouble),
                              "CoordinateY", &coords[0], &iGrid) != CG_OK )
          return addMessage( cg_get_error(), /*fatal = */true );
        
        // write Z-Coordinates
        coordIter = grid->CoordinateBegin();
        for ( int i = 0; i < grid->Size(); coordIter.Next(), i++ ) 
          coords[i] = coordIter.Value()->Z();

        if ( cg_coord_write( _fn, iBase, iZone, CGNS_ENUMV(RealDouble),
                              "CoordinateZ", &coords[0], &iGrid) != CG_OK )
          return addMessage( cg_get_error(), /*fatal = */true );        
        //end write Coordinates
        
        // Write Boundary condition for the grid faces
        std::vector<std::vector<cgsize_t>> allRanges;
        std::vector<std::string> boundaryNames(6,"");
        grid->getAllFaceIndexLimits( allRanges );
        CheckForGroupNameOnFaceInterfaces(grid,boundaryNames);
  
        int faceId = 0;
        std::set<std::string> bNames;
        for (auto pRange : allRanges)
        {
          if(pRange[3] < pRange[0]) {std::swap(pRange[0],pRange[3]);}
          if(pRange[4] < pRange[1]) {std::swap(pRange[1],pRange[4]);}
          if(pRange[5] < pRange[2]) {std::swap(pRange[2],pRange[5]);}          
          int cgIndexBoco = 0;              
          std::string boundaryName = boundaryNames[faceId];
          std::string boundaryNameOrig(boundaryName);
          bool isInGroup = false;
          if (boundaryName.empty())
            boundaryName = zoneName + "_" + std::to_string(faceId);
          else
            isInGroup = true;
          if ( bNames.count(boundaryName)!=0 )
            boundaryName = boundaryName + "_" + std::to_string(faceId);
          bNames.insert( boundaryName );
          if(cg_boco_write(_fn, iBase, iZone, boundaryName.c_str(), CGNS_ENUMV(BCTypeNull), 
                            CGNS_ENUMV(PointRange), 2, &pRange[0], &cgIndexBoco) != CG_OK) return addMessage(cg_get_error(), /*fatal = */true);
          // write also the group name as FamilyName in the same node
          if (isInGroup)
          {
            // go to the BC node
            if(cg_goto(_fn, iBase, "Zone_t", iZone, "ZoneBC_t", 1,
                  "BC_t", cgIndexBoco, "end") != CG_OK)
              return addMessage(cg_get_error(), /*fatal = */true);
            // write the family name
            if(cg_famname_write(boundaryNameOrig.c_str()) != CG_OK)
              return addMessage(cg_get_error(), /*fatal = */true);
          }
          faceId++;
        }
        // End write boundary
        
        // Write Interfaces
        for ( TopExp_Explorer fEx( shape, TopAbs_SOLID ); fEx.More(); fEx.Next() )
        {
          TopoDS_Solid neighbourSolid = TopoDS::Solid(fEx.Current()); 
          if ( !currentSolid.IsSame( neighbourSolid ) && myMesh->HasStructuredGridFilled(neighbourSolid))
          {
            std::vector<int> interface;
            grid->GetFaceInterfaces(myMesh->GetTheGrid(neighbourSolid).get(), interface);
            if ( !interface.empty() )
            {
              std::vector<cgsize_t> interfacecgns( 12 );
              for (size_t i = 0; i < 12; i++)
                interfacecgns[i] = cgsize_t(interface[i+1]);
                
              int iConn;  
              size_t neighbourSolidId = myMesh->ShapeToIndex(neighbourSolid);
              std::string neigbourZoneName = groupNameMapById[neighbourSolidId];
              // interfaceName must be unique (it should be by construction)
              std::string interfaceName     = zoneName + "_" + neigbourZoneName;  
              if(cg_1to1_write(_fn, iBase, iZone, interfaceName.c_str(), neigbourZoneName.c_str(), 
                                &interfacecgns[0], &interfacecgns[6], &interface[13], &iConn) != CG_OK) return addMessage(cg_get_error(), /*fatal = */true);
            }

          }
        }
      }
    } 
  }
  else if ( meshDim == 2 )
  {    
    // get a map of unique names for each face
    std::map<size_t, std::string> groupNameMapById;
    GetGroupNamesMap(shape, meshDim, groupNameMapById);
    for ( TopExp_Explorer fEx( shape, TopAbs_FACE ); fEx.More(); fEx.Next() )
    {
      TopoDS_Face currentFace = TopoDS::Face(fEx.Current());      

      if ( myMesh->HasStructuredGridFilled(currentFace) )
      {
        auto grid = myMesh->GetTheGrid(currentFace).get();
        int imax = grid->nx();
        int jmax = grid->ny();
        int iZone;
        cgsize_t size[6] = {imax, jmax, imax - 1, jmax - 1, 0, 0};

        size_t currentFaceId = myMesh->ShapeToIndex(currentFace);
        std::string zoneName = groupNameMapById[currentFaceId];
            
        // write Zone
        if(cg_zone_write(_fn, iBase, zoneName.c_str(), size,
                      CGNS_ENUMV(Structured), &iZone) != CG_OK)
          return addMessage( cg_get_error(), /*fatal = */true );    
        
        // write Grid
        int iGrid;
        if(cg_grid_write(_fn, iBase, iZone, "GridCoordinates", &iGrid) != CG_OK)
          return addMessage( cg_get_error(), /*fatal = */true );
        
        // write the Coordinates      
        std::vector< double > coords( grid->Size() /*nx*ny*nz*/);

        // write X-Coordinates
        auto coordIter = grid->CoordinateBegin();
        for ( int i = 0; i < grid->Size(); coordIter.Next(), i++ ) 
          coords[i] = coordIter.Value()->X();

        if ( cg_coord_write( _fn, iBase, iZone, CGNS_ENUMV(RealDouble),
                              "CoordinateX", &coords[0], &iGrid) != CG_OK )
          return addMessage( cg_get_error(), /*fatal = */true );
        
        // write Y-Coordinates
        coordIter = grid->CoordinateBegin();
        for ( int i = 0; i < grid->Size(); coordIter.Next(), i++ ) 
          coords[i] = coordIter.Value()->Y();
        
        if ( cg_coord_write( _fn, iBase, iZone, CGNS_ENUMV(RealDouble),
                              "CoordinateY", &coords[0], &iGrid) != CG_OK )
          return addMessage( cg_get_error(), /*fatal = */true );
        
        // write Z-Coordinates
        coordIter = grid->CoordinateBegin();
        for ( int i = 0; i < grid->Size(); coordIter.Next(), i++ ) 
          coords[i] = coordIter.Value()->Z();

        if ( cg_coord_write( _fn, iBase, iZone, CGNS_ENUMV(RealDouble),
                              "CoordinateZ", &coords[0], &iGrid) != CG_OK )
          return addMessage( cg_get_error(), /*fatal = */true );
        
        //End write Coordinates
      
        // Write Boundary condition for the grid edges
        std::vector<std::vector<cgsize_t>> allRanges;
        std::vector<std::string> boundaryNames(4,"");
        grid->getAllEdgeIndexLimits( allRanges );
        CheckForGroupNameOnEdgeInterfaces(grid,boundaryNames);
        int edgeId = 0;
        std::set<std::string> bNames;
        for (auto pRange : allRanges)
        {
          if(pRange[2] < pRange[0]) {std::swap(pRange[0],pRange[2]);}
          if(pRange[1] < pRange[3]) {std::swap(pRange[1],pRange[3]);}          
          int cgIndexBoco = 0;    
          std::string boundaryName = boundaryNames[edgeId];
          std::string boundaryNameOrig(boundaryName);
          bool isInGroup = false;
          if (boundaryName.empty())
            boundaryName = zoneName + "_" + std::to_string(edgeId);
          else
            isInGroup = true;
          if ( bNames.count(boundaryName)!=0)
            boundaryName = boundaryName + "_" + std::to_string(edgeId);
          bNames.insert( boundaryName );
          if(cg_boco_write(_fn, iBase, iZone, boundaryName.c_str(), CGNS_ENUMV(BCTypeNull), 
                            CGNS_ENUMV(PointRange), 2, &pRange[0], &cgIndexBoco) != CG_OK) return addMessage(cg_get_error(), /*fatal = */true);
          // write also the group name as FamilyName in the same node
          if (isInGroup)
          {
            // go to the BC node
            if(cg_goto(_fn, iBase, "Zone_t", iZone, "ZoneBC_t", 1,
                  "BC_t", cgIndexBoco, "end") != CG_OK)
              return addMessage(cg_get_error(), /*fatal = */true);
            // write the family name
            if(cg_famname_write(boundaryNameOrig.c_str()) != CG_OK)
              return addMessage(cg_get_error(), /*fatal = */true);
          }
          edgeId++;
        }
        // End write Boundary

        // Write Interfaces
        for ( TopExp_Explorer fEx( shape, TopAbs_FACE ); fEx.More(); fEx.Next() )
        {
          TopoDS_Face neighbourFace = TopoDS::Face(fEx.Current());
          if ( !currentFace.IsSame( neighbourFace ) && myMesh->HasStructuredGridFilled(neighbourFace) )
          {
            std::vector<int> interface;
            grid->GetEdgeInterfaces(myMesh->GetTheGrid(neighbourFace).get(), interface);
           
            if ( !interface.empty() )
            {          
              std::vector<cgsize_t> interfacecgns( 8 );
              for (size_t i = 0; i < 8; i++)
                interfacecgns[i] = cgsize_t(interface[i+1]);    

              int iConn;
              size_t neighbourFaceId = myMesh->ShapeToIndex(neighbourFace);
              std::string neigbourZoneName  = groupNameMapById[neighbourFaceId];
              std::string interfaceName = zoneName + "_" + neigbourZoneName;
              if(cg_1to1_write(_fn, iBase, iZone, interfaceName.c_str(), neigbourZoneName.c_str(), 
                                &interfacecgns[0], &interfacecgns[4], &interface[9], &iConn) != CG_OK) return addMessage(cg_get_error(), /*fatal = */true);
              
            }            
          }                     
        }              
      } /*end if grid filled*/     
    } /*foreach face*/
  }

  return DRS_OK;
}

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================

DriverStructuredCGNS_Write::DriverStructuredCGNS_Write(): _fn(0)
{
}

//================================================================================
/*!
 * \brief Close the cgns file at destruction
 */
//================================================================================

DriverStructuredCGNS_Write::~DriverStructuredCGNS_Write()
{
  if ( _fn > 0 )
    cg_close( _fn );
}
