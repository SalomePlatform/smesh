// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

#ifndef SMESHGUI_SELECTIONPROXY_H
#define SMESHGUI_SELECTIONPROXY_H

#include "SMESH_SMESHGUI.hxx"

#include <SALOME_InteractiveObject.hxx>
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(GEOM_Gen)

#include <QColor>
#include <QList>
#include <QMap>
#include <QSet>
#include <QString>

#include <gp_XYZ.hxx>

class SMESH_Actor;

namespace SMESH
{
  class SMESHGUI_EXPORT MeshInfo
  {
    QMap<int, long> myInfo;
  public:
    MeshInfo();
    uint info( int ) const;
    uint operator[] ( int );
    uint count( int, int ) const;
  private:
    void addInfo( int, long );
    friend class SelectionProxy;
  };

  class SMESHGUI_EXPORT MedInfo
  {
    QString myFileName;
    uint mySize;
    uint myMajor, myMinor, myRelease;
  public:
    MedInfo();
    bool isValid() const;
    QString fileName() const;
    uint size() const;
    QString version() const;
  private:
    void setFileName( const QString& );
    void setSize( uint );
    void setVersion( uint, uint, uint );
    friend class SelectionProxy;
  };

  class SMESHGUI_EXPORT Position
  {
    int myShapeId;
    int myShapeType;
    double myU, myV;
    bool myHasU, myHasV;
  public:
    Position();
    bool isValid() const;
    int shapeId() const;
    int shapeType() const;
    bool hasU() const;
    double u() const;
    bool hasV() const;
    double v() const;
  private:
    void setShapeId( int );
    void setShapeType( int );
    void setU( double );
    void setV( double );
    friend class SelectionProxy;
  };

  class XYZ
  {
    double myX, myY, myZ;
  public:
    XYZ();
    XYZ( double, double, double );
    XYZ( const gp_XYZ& );
    void add( double, double, double );
    void divide( double );
    double x() const;
    double y() const;
    double z() const;
    operator gp_XYZ() const;
  };

  typedef QMap< int, QList<int> > Connectivity;

  class SMESHGUI_EXPORT SelectionProxy
  {
    Handle(SALOME_InteractiveObject) myIO;
    SMESH::SMESH_IDSource_var myObject;
    SMESH_Actor* myActor;
    bool myDirty;

  public:
    enum Type
    {
      Unknown,
      Mesh,
      Submesh,
      Group,
      GroupStd,
      GroupGeom,
      GroupFilter
    };

    // construction
    SelectionProxy();
    SelectionProxy( const Handle(SALOME_InteractiveObject)& );
    SelectionProxy( SMESH::SMESH_IDSource_ptr );
    SelectionProxy( const SelectionProxy& );

    SelectionProxy& operator= ( const SelectionProxy& );

    // comparison
    bool operator== ( const SelectionProxy& );

    // general purpose methods
    void refresh();

    bool isNull() const;
    operator bool() const;

    SMESH::SMESH_IDSource_ptr object() const;
    const Handle(SALOME_InteractiveObject)& io() const;
    SMESH_Actor* actor() const;

    bool isValid() const;
    void load();

    // methods common to all types of proxy
    QString name() const;
    Type type() const;
    MeshInfo meshInfo() const;
    SelectionProxy mesh() const;
    bool hasShapeToMesh() const;
    GEOM::GEOM_Object_ptr shape() const;
    QString shapeName() const;
    int shapeType() const;
    bool isMeshLoaded() const;

    bool hasNode( int );
    bool nodeCoordinates( int, XYZ& );
    bool nodeConnectivity( int, Connectivity& );
    bool nodePosition( int, Position& );
    QList<SelectionProxy> nodeGroups( int ) const;

    bool hasElement( int );
    SMESH::ElementType elementType( int ) const;
    int elementEntityType( int ) const;
    bool elementConnectivity( SMESH::smIdType, Connectivity& );
    bool perFaceConnectivity( int, Connectivity&, int& );
    bool elementPosition( int, Position& );
    bool elementGravityCenter( int, XYZ& );
    bool elementNormal( int, XYZ& );
    bool elementControl( int, int, double, double& ) const;
    QList<SelectionProxy> elementGroups( int ) const;

    // methods that work for mesh only
    MedInfo medFileInfo() const;
    QList<SelectionProxy> submeshes() const;
    QList<SelectionProxy> groups() const;

    // methods that work for group only
    SMESH::ElementType groupElementType() const;
    QColor color() const;
    SMESH::smIdType size( bool = false ) const;
    SMESH::smIdType nbNodes( bool = false ) const;
    QSet<uint> ids() const;

  private:
    void init();
  };
}

#endif // SMESHGUI_SELECTIONPROXY_H
