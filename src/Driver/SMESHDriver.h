#ifndef _INCLUDE_SMESHDRIVER
#define _INCLUDE_SMESHDRIVER

#include "Document_Reader.h"
#include "Document_Writer.h"
#include "Mesh_Reader.h"
#include "Mesh_Writer.h"

class SMESHDriver {

  public :
    static Document_Reader* GetDocumentReader(string Extension, string Class);
    static Document_Writer* GetDocumentWriter(string Extension, string Class);

    static Mesh_Reader* GetMeshReader(string Extension, string Class);
    static Mesh_Writer* GetMeshWriter(string Extension, string Class);

};
#endif
