#ifndef _INCLUDE_MESH_WRITER
#define _INCLUDE_MESH_WRITER

#include <string>
#include "Handle_SMDS_Mesh.hxx"

class Mesh_Writer {

  public :
    virtual void Add() =0;
    virtual void Write() =0;
    virtual void SetMesh(Handle(SMDS_Mesh)&) =0;
    virtual void SetFile(string) =0;
    virtual void SetMeshId(int) =0;

};
#endif
