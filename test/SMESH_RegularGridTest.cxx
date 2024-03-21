// Copyright (C) 2016-2024  CEA, EDF
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
// File      : SMESH_RegularGridTest.cxx (unit test)
// Author    : Cesar Conopoima (cce)

// std
#include<iostream>
#include <memory>

// smesh
#include "SMESH_RegularGridTemplate.hxx"
#include "SMDS_MeshNode.hxx"

bool testConstructor()
{
  auto regularGrid2D = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(0,5,4));
  if ( regularGrid2D->Size() != 5*4 ) throw std::runtime_error("2D Grid size not match in testConstructor()\n");
  auto regularGrid3D = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(0,5,4,20));
  if ( regularGrid3D->Size() != 5*4*20 ) throw std::runtime_error("3D Grid size not match in testConstructor()\n");
  return true;
}

bool testSetGetNode()
{
  auto regularGrid = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(0,5,4));
  double x,y;

  for (size_t j = 0; j < 4; j++)
    for (size_t i = 0; i < 5; i++)
    {
      x = i * 0.1;
      y = j * 0.1;
      auto node  = std::make_shared<gp_Pnt>( x, y, 0. );
      regularGrid->SetNode( node, i, j );
    }
  for (size_t j = 0; j < 4; j++)
    for (size_t i = 0; i < 5; i++)
    {
      x = i * 0.1;
      y = j * 0.1;
      auto node = regularGrid->GetNode( i, j, 0 );
      if ( node == nullptr ) throw std::runtime_error("error getting node in testSetGetNode()\n");
      if ( node->X() != x ||
            node->Y() != y ||
              node->Z() != 0. ) throw std::runtime_error("error for node not in the correct position in testSetGetNode()\n");
    }  
    
  return true;
}

bool testGetCommontInterface1D()
{   
  int mnx = 5, mny = 4;
  auto regularGrid0 = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(0,mnx,mny));
  auto regularGrid1 = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(1,mnx,mny));    
  double x,y;
  
  /*side by side grid with 1to1 interface*/
  /* o-o-o-o-oxo-o-o-o-o */
  /* o-oG0-o-oxo-o-G1o-o */
  /* o-o-o-o-oxo-o-o-o-o */
  /* o-o-o-o-oxo-o-o-o-o */

  for (size_t j = 0; j < mny; j++)
    for (size_t i = 0; i < mnx; i++)
    {
      x = i * 0.1;
      y = j * 0.1;
      auto node0  = std::make_shared<gp_Pnt>(x,y,0.);
      regularGrid0->SetNode( node0, i, j );

      x = i * 0.1 + 0.4;
      y = j * 0.1;   
      auto node1  = std::make_shared<gp_Pnt>(x,y,0.);
      regularGrid1->SetNode( node1, i, j );
    }

  std::vector<int> interface; /* interface at right  */
  regularGrid0->GetEdgeInterfaces( regularGrid1.get(), interface );
  std::vector<int> expectedRange        = regularGrid0->getEdgeIndexLimits(SMESHUtils::SMESH_RegularGrid::RIGHT);
  std::vector<int> expectedDonorRange   = regularGrid1->getEdgeIndexLimitsInverted(SMESHUtils::SMESH_RegularGrid::LEFT);

  if ( interface.size() != 1+mny*2+2 ) 
    throw std::runtime_error("1to1 edge interface not found in testGetCommontInterface1D()\n");

  if ( interface[0] != int(SMESHUtils::SMESH_RegularGrid::RIGHT) ) 
    throw std::runtime_error("edge found in interface is incorrect testGetCommontInterface1D()\n");
  
  for (size_t i = 0; i < 4; i++)
    if ( interface[i+1] != expectedRange[i])
        throw std::runtime_error("interface range index is incorrect in testGetCommontInterface1D()\n");
    
  for (size_t i = 0; i < 4; i++)
    if ( interface[i+5] != expectedDonorRange[i])
        throw std::runtime_error("interface donor index is incorrect in testGetCommontInterface1D()\n");

  return true;
}

bool testGetCommontInterface2D()
{
  int mnx = 5, mny = 4, mnz = 2;
  auto regularGrid0 = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(0,mnx,mny,mnz));
  auto regularGrid1 = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(1,mnx,mny,mnz));
  auto regularGrid2 = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(2,mnx,mny,mnz));
  auto regularGrid3 = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(3,mnx,mny,mnz));
  auto regularGrid4 = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(4,mnx,mny,mnz));
  auto regularGrid5 = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(5,mnx,mny,mnz));
  auto regularGrid6 = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(6,mnx,mny,mnz));
  double x,y,z;

  for (size_t k = 0; k < mnz; k++)
    for (size_t j = 0; j < mny; j++)
      for (size_t i = 0; i < mnx; i++)
      {
        x = i * 0.1;
        y = j * 0.1;
        z = k * 0.1;
        auto node0  = std::make_shared<gp_Pnt>(x,y,z);
        regularGrid0->SetNode( node0, i, j, k );

        // Grid at botton
        x = i * 0.1;
        y = j * 0.1;   
        z = k * 0.1-0.1;
        auto node1 = std::make_shared<gp_Pnt>(x,y,z);
        regularGrid1->SetNode( node1, i, j, k );
            
        // Grid at right
        x = i * 0.1+0.4;
        y = j * 0.1;   
        z = k * 0.1;
        auto node2  = std::make_shared<gp_Pnt>(x,y,z);
        regularGrid2->SetNode( node2, i, j, k );

        // Grid at back
        x = i * 0.1;
        y = j * 0.1+0.3;   
        z = k * 0.1;
        auto node3  = std::make_shared<gp_Pnt>(x,y,z);
        regularGrid3->SetNode( node3, i, j, k );

        // Grid at left
        x = i * 0.1-0.4;
        y = j * 0.1;   
        z = k * 0.1;
        auto node4 = std::make_shared<gp_Pnt>(x,y,z);
        regularGrid4->SetNode( node4, i, j, k );

        // Grid at front
        x = i * 0.1;
        y = j * 0.1-0.3;   
        z = k * 0.1;
        auto node5 = std::make_shared<gp_Pnt>(x,y,z);
        regularGrid5->SetNode( node5, i, j, k );

        // Grid on top
        x = i * 0.1;
        y = j * 0.1;   
        z = k * 0.1+0.1;
        auto node6 = std::make_shared<gp_Pnt>(x,y,z);
        regularGrid6->SetNode( node6, i, j, k );
      }

  std::vector<std::vector<int>> interface(6);
  std::vector<std::vector<int>> expectedRange(6);
  std::vector<std::vector<int>> expectedDonorRange(6);
  
  regularGrid0->GetFaceInterfaces( regularGrid1.get(), interface[0] );
  regularGrid0->GetFaceInterfaces( regularGrid2.get(), interface[1] );
  regularGrid0->GetFaceInterfaces( regularGrid3.get(), interface[2] );
  regularGrid0->GetFaceInterfaces( regularGrid4.get(), interface[3] );
  regularGrid0->GetFaceInterfaces( regularGrid5.get(), interface[4] );
  regularGrid0->GetFaceInterfaces( regularGrid6.get(), interface[5] );

  expectedRange[0] = regularGrid0->getFaceIndexLimits(SMESHUtils::SMESH_RegularGrid::B_BOTTOM);
  expectedRange[1] = regularGrid0->getFaceIndexLimits(SMESHUtils::SMESH_RegularGrid::B_RIGHT);
  expectedRange[2] = regularGrid0->getFaceIndexLimits(SMESHUtils::SMESH_RegularGrid::B_BACK);
  expectedRange[3] = regularGrid0->getFaceIndexLimits(SMESHUtils::SMESH_RegularGrid::B_LEFT);
  expectedRange[4] = regularGrid0->getFaceIndexLimits(SMESHUtils::SMESH_RegularGrid::B_FRONT);
  expectedRange[5] = regularGrid0->getFaceIndexLimits(SMESHUtils::SMESH_RegularGrid::B_TOP);

  expectedDonorRange[0] = regularGrid1->getFaceIndexLimits(SMESHUtils::SMESH_RegularGrid::B_TOP);
  expectedDonorRange[1] = regularGrid2->getFaceIndexLimits(SMESHUtils::SMESH_RegularGrid::B_LEFT);
  expectedDonorRange[2] = regularGrid3->getFaceIndexLimits(SMESHUtils::SMESH_RegularGrid::B_FRONT );
  expectedDonorRange[3] = regularGrid4->getFaceIndexLimits(SMESHUtils::SMESH_RegularGrid::B_RIGHT);
  expectedDonorRange[4] = regularGrid5->getFaceIndexLimits(SMESHUtils::SMESH_RegularGrid::B_BACK);
  expectedDonorRange[5] = regularGrid6->getFaceIndexLimits(SMESHUtils::SMESH_RegularGrid::B_BOTTOM);
  
  int interfaceSize = 1+6*2+3;
  for ( auto itf : interface )
  {
    if ( itf.size() != interfaceSize ) 
      throw std::runtime_error("1to1 face interface not found in testGetCommontInterface2D()\n");  
  }

  int count = 0;
  for ( auto itf : interface )
  {
    if ( itf[0] != count /*face id of the interface*/) 
      throw std::runtime_error("face found in interface is incorrect testGetCommontInterface2D()\n");

    for (size_t i = 0; i < 6; i++)
      if ( itf[i+1] != expectedRange[count][i])
        throw std::runtime_error("interface range index is incorrect in testGetCommontInterface2D()\n");
    
    for (size_t i = 0; i < 6; i++)
      if ( itf[i+7] != expectedDonorRange[count][i])
        throw std::runtime_error("interface donor index is incorrect in testGetCommontInterface2D()\n");
    count++;        
  }

  return true;
}

bool testGetPartialInterface2D()
{
  int mnx = 5, mny = 4, mnz = 3;
  int mnxf = 2, mnyf = 2, mnzf = 2;
  int mnxl = 2, mnyl = 3, mnzl = 2;
  auto regularGrid0 = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(0,mnx,mny,mnz));
  auto regularGrid1 = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(1,mnxf,mnyf,mnzf));
  auto regularGrid2 = std::unique_ptr<SMESHUtils::SMESH_RegularGrid>( new SMESHUtils::SMESH_RegularGrid(2,mnxl,mnyl,mnzl));
  double x,y,z;

  for (size_t k = 0; k < mnz; k++)
    for (size_t j = 0; j < mny; j++)
      for (size_t i = 0; i < mnx; i++)
      {
        x = i * 0.1;
        y = j * 0.1;
        z = k * 0.1;
        auto node0  = std::make_shared<gp_Pnt>(x,y,z);
        regularGrid0->SetNode( node0, i, j, k );                   
      }

  for (size_t k = 0; k < mnzf; k++)
    for (size_t j = 0; j < mnyf; j++)
      for (size_t i = 0; i < mnxf; i++)
      {
        // Grid at front
        x =  i * 0.1+0.3;
        y = -j * 0.1;   
        z =  k * 0.1+0.1;
        auto node1 = std::make_shared<gp_Pnt>(x,y,z);
        regularGrid1->SetNode( node1, i, j, k );   
      }   

  for (size_t k = 0; k < mnzl; k++)
    for (size_t j = 0; j < mnyl; j++)
      for (size_t i = 0; i < mnxl; i++)
      {
        // Grid at left
        x = -i * 0.1;
        y =  j * 0.1+0.1;   
        z =  k * 0.1;
        auto node2  = std::make_shared<gp_Pnt>(x,y,z);
        regularGrid2->SetNode( node2, i, j, k );
      }

  std::vector<std::vector<int>> interface(2);
  std::vector<int> faceId = {SMESHUtils::SMESH_RegularGrid::B_FRONT,SMESHUtils::SMESH_RegularGrid::B_LEFT}; /*faces where interfaces are*/
  std::vector<std::vector<int>> expectedRange(2);
  std::vector<std::vector<int>> expectedDonorRange(2);
  
  regularGrid0->GetFaceInterfaces( regularGrid1.get(), interface[0] );
  regularGrid0->GetFaceInterfaces( regularGrid2.get(), interface[1] );

  expectedRange[0] = std::vector<int>({4,1,2,5,1,3});
  expectedRange[1] = std::vector<int>({1,4,1,1,2,2});

  expectedDonorRange[0] = regularGrid1->getFaceIndexLimits(SMESHUtils::SMESH_RegularGrid::B_FRONT);
  expectedDonorRange[1] = regularGrid2->getFaceIndexLimits(SMESHUtils::SMESH_RegularGrid::B_LEFT);
  
  int interfaceSize = 1+6*2+3;
  for ( auto itf : interface )
  {
    if ( itf.size() != interfaceSize ) 
      throw std::runtime_error("1to1 face interface not found in testGetPartialInterface2D()\n");  
  }

  int count = 0;
  for ( auto itf : interface )
  {
    if ( itf[0] != faceId[count] /*face id of the interface*/) 
      throw std::runtime_error("face found in interface is incorrect testGetPartialInterface2D()\n");
    
    for (size_t i = 0; i < 6; i++)
      if ( itf[i+1] != expectedRange[count][i])
        throw std::runtime_error("interface range index is incorrect in testGetPartialInterface2D()\n");
          
    for (size_t i = 0; i < 6; i++)
      if ( itf[i+7] != expectedDonorRange[count][i])
        throw std::runtime_error("interface donor index is incorrect in testGetPartialInterface2D()\n");
    count++;        
  }

  return true;
}

int main()
{
  if ( !testConstructor() || !testSetGetNode() || 
        !testGetCommontInterface1D() || !testGetCommontInterface2D() ||
         !testGetPartialInterface2D() )
    return 1;
  else 
    return 0;

}