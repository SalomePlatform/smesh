using namespace std;
#include "SMESHDriver.h"

#include <dlfcn.h>
#include <utilities.h>

//A enlever
#include "DriverMED_R_SMESHDS_Document.h"
#include "DriverMED_R_SMESHDS_Mesh.h"
#include "DriverMED_R_SMDS_Mesh.h"
#include "DriverMED_W_SMESHDS_Document.h"
#include "DriverMED_W_SMESHDS_Mesh.h"
#include "DriverMED_W_SMDS_Mesh.h"

#include "DriverDAT_R_SMESHDS_Document.h"
#include "DriverDAT_R_SMESHDS_Mesh.h"
#include "DriverDAT_R_SMDS_Mesh.h"
#include "DriverDAT_W_SMESHDS_Document.h"
#include "DriverDAT_W_SMESHDS_Mesh.h"
#include "DriverDAT_W_SMDS_Mesh.h"
//

Document_Reader* SMESHDriver::GetDocumentReader(string Extension, string Class) {
  if (Extension==string("MED")) {
    DriverMED_R_SMESHDS_Document* myDriver = new DriverMED_R_SMESHDS_Document();
    return (myDriver);
  }
  else if (Extension==string("DAT")) {
    DriverDAT_R_SMESHDS_Document* myDriver = new DriverDAT_R_SMESHDS_Document();
    return (myDriver);
  }
  else {
    MESSAGE("No driver known for this extension");
    return (Document_Reader*)NULL;
  }


}

Document_Writer* SMESHDriver::GetDocumentWriter(string Extension, string Class) {
  if (Extension==string("MED")) {
    DriverMED_W_SMESHDS_Document* myDriver = new DriverMED_W_SMESHDS_Document();
    return (myDriver);
  }    
  else if (Extension==string("DAT")) {
    DriverDAT_W_SMESHDS_Document* myDriver = new DriverDAT_W_SMESHDS_Document();
    return (myDriver);
  }    
  else {
    MESSAGE("No driver known for this extension");
    return (Document_Writer*)NULL;
  }


}

Mesh_Reader* SMESHDriver::GetMeshReader(string Extension, string Class) {
  if (Extension==string("MED")) {

  if (strcmp(Class.c_str(),"SMESHDS_Mesh")==0) {
    DriverMED_R_SMESHDS_Mesh* myDriver = new DriverMED_R_SMESHDS_Mesh();
    return (myDriver);
  }
  else if (strcmp(Class.c_str(),"SMDS_Mesh")==0) {
    DriverMED_R_SMDS_Mesh* myDriver = new DriverMED_R_SMDS_Mesh();
    return (myDriver);
  }

  }
  else if (Extension==string("DAT")) {

  if (strcmp(Class.c_str(),"SMESHDS_Mesh")==0) {
    DriverDAT_R_SMESHDS_Mesh* myDriver = new DriverDAT_R_SMESHDS_Mesh();
    return (myDriver);
  }
  else if (strcmp(Class.c_str(),"SMDS_Mesh")==0) {
    DriverDAT_R_SMDS_Mesh* myDriver = new DriverDAT_R_SMDS_Mesh();
    return (myDriver);
  }

  }


}

Mesh_Writer* SMESHDriver::GetMeshWriter(string Extension, string Class) {
  if (Extension==string("MED")) {

  if (strcmp(Class.c_str(),"SMESHDS_Mesh")==0) {
    DriverMED_W_SMESHDS_Mesh* myDriver = new DriverMED_W_SMESHDS_Mesh();
    return (myDriver);
  }
  else if (strcmp(Class.c_str(),"SMDS_Mesh")==0) {
    DriverMED_W_SMDS_Mesh* myDriver = new DriverMED_W_SMDS_Mesh();
    return (myDriver);
  }

  }
  else if (Extension==string("DAT")) {

  if (strcmp(Class.c_str(),"SMESHDS_Mesh")==0) {
    DriverDAT_W_SMESHDS_Mesh* myDriver = new DriverDAT_W_SMESHDS_Mesh();
    return (myDriver);
  }
  else if (strcmp(Class.c_str(),"SMDS_Mesh")==0) {
    DriverDAT_W_SMDS_Mesh* myDriver = new DriverDAT_W_SMDS_Mesh();
    return (myDriver);
  }

  }

}

