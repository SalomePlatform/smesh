#ifndef _INCLUDE_MESH_READER
#define _INCLUDE_MESH_READER

#include <string>
#include "Handle_SMDS_Mesh.hxx"

class Mesh_Reader {

  public :
    virtual void Add() =0;
    virtual void Read() =0;
    virtual void SetMesh(Handle(SMDS_Mesh)&) =0;
    virtual void SetMeshId(int) =0;
    virtual void SetFile(string) =0;

};
#endif
