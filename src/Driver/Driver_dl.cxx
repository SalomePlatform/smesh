using namespace std;
#include "Driver.h"

#include <dlfcn.h>
#include <stdio.h>
#include <utilities.h>

#include "DriverDAT_R_SMESHDS_Document.h"
#include "Test.h"

/*!  extern "C"
{
  void test() {

                  void *handle;
                  double (*cosine)(double);
                  char *error;

                  handle = dlopen ("/usr/lib/libm.so", RTLD_LAZY);
                  if (!handle) {
                      fputs (dlerror(), stderr);
                      exit(1);
                  }

                  cosine = dlsym(handle, "cos");
                  if ((error = dlerror()) != NULL)  {
                      fprintf (stderr, "%s\n", error);
                      exit(1);
                  }

                  printf ("%f\n", (*cosine)(2.0));
                  dlclose(handle);

  char* error;
  string Extension=string("DAT");
  string Class=string("SMESHDS_Document");
  string myLibrary = string("/home/barberou/barberou/SALOME_3105/build/lib/libMeshDriver")+Extension+string(".so");
  SCRUTE(myLibrary);
  //Document_Reader* myDriver;//a caster ???
  DriverDAT_R_SMESHDS_Document* myDriver;

  string myClass = string("Driver")+Extension+string("_R_")+Class;
  SCRUTE(myClass);
  
  void* handle = dlopen (myLibrary.c_str(), RTLD_LAZY);
  if (!handle) {
    fputs (dlerror(), stderr);
    exit(1);
  }
  MESSAGE("Open ok");

  //int* res = (int*)dlsym(handle, "getOne");
  //SCRUTE(res);
  SCRUTE(dlsym(handle, "getOne"));
  //int res2= (*res)();
  myDriver = (DriverDAT_R_SMESHDS_Document*) dlsym(handle, myClass.c_str());
  MESSAGE("Reading 1");
  SCRUTE(myDriver);
  if ((error = dlerror()) != NULL)  {
    fprintf (stderr, "%s\n", error);
    exit(1);
  }
  MESSAGE("Reading 2");
    
  dlclose(handle);
  MESSAGE("after close");
		  
  }
  }*/

Document_Reader* Driver::GetDocumentReader(string Extension, string Class) {
  test();
  //p-e extern C ?
  /*!
  char* error;
  string myLibrary = string("/home/barberou/barberou/SALOME_3105/build/lib/libMeshDriver")+Extension+string(".so");
  SCRUTE(myLibrary);
  //Document_Reader* myDriver;//a caster ???
  DriverDAT_R_SMESHDS_Document* myDriver;

  string myClass = string("Driver")+Extension+string("_R_")+Class;
  SCRUTE(myClass);
  
  void* handle = dlopen (myLibrary.c_str(), RTLD_LAZY);
  if (!handle) {
    fputs (dlerror(), stderr);
    exit(1);
  }
  MESSAGE("Open ok");
  //myDriver = (Document_Reader*) dlsym(handle, myClass.c_str());
  int* res = (int*) dlsym(handle, "getOne");
  SCRUTE(res);
  myDriver = (DriverDAT_R_SMESHDS_Document*) dlsym(handle, myClass.c_str());
  MESSAGE("Reading 1");
  SCRUTE(myDriver);
  if ((error = dlerror()) != NULL)  {
    fprintf (stderr, "%s\n", error);
    exit(1);
  }
  MESSAGE("Reading 2");
    
  dlclose(handle);
  MESSAGE("after close");
  return (myDriver);
  */

}

Document_Writer* Driver::GetDocumentWriter(string Extension, string Class) {

  char* error;
  string myLibrary = string("libMeshDriver")+Extension+string(".so");
  Document_Writer* myDriver;//a caster ???
  string myClass = string("Driver")+Extension+string("_W_")+Class;
  
  void* handle = dlopen (myLibrary.c_str(), RTLD_LAZY);
  if (!handle) {
    fputs (dlerror(), stderr);
    exit(1);
  }

  myDriver = (Document_Writer*) dlsym(handle, myClass.c_str());
  if ((error = dlerror()) != NULL)  {
    fprintf (stderr, "%s\n", error);
    exit(1);
  }
    
  dlclose(handle);
  
  return (myDriver);

}

Mesh_Reader* Driver::GetMeshReader(string Extension, string Class) {

  char* error;
  string myLibrary = string("libMeshDriver")+Extension+string(".so");
  Mesh_Reader* myDriver;//a caster ???
  string myClass = string("Driver")+Extension+string("_R_")+Class;
  
  void* handle = dlopen (myLibrary.c_str(), RTLD_LAZY);
  if (!handle) {
    fputs (dlerror(), stderr);
    exit(1);
  }

  myDriver = (Mesh_Reader*) dlsym(handle, myClass.c_str());
  if ((error = dlerror()) != NULL)  {
    fprintf (stderr, "%s\n", error);
    exit(1);
  }
    
  dlclose(handle);
  
  return (myDriver);

}

Mesh_Writer* Driver::GetMeshWriter(string Extension, string Class) {

  char* error;
  string myLibrary = string("libMeshDriver")+Extension+string(".so");
  Mesh_Writer* myDriver;//a caster ???
  string myClass = string("Driver")+Extension+string("_W_")+Class;
  
  void* handle = dlopen (myLibrary.c_str(), RTLD_LAZY);
  if (!handle) {
    fputs (dlerror(), stderr);
    exit(1);
  }

  myDriver = (Mesh_Writer*) dlsym(handle, myClass.c_str());
  if ((error = dlerror()) != NULL)  {
    fprintf (stderr, "%s\n", error);
    exit(1);
  }
    
  dlclose(handle);
  
  return (myDriver);

}

