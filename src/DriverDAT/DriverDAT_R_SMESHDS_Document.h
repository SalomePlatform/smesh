#ifndef _INCLUDE_DRIVERDAT_R_SMESHDS_DOCUMENT
#define _INCLUDE_DRIVERDAT_R_SMESHDS_DOCUMENT

#include <stdio.h>

#include "SMESHDS_Document.hxx"
#include "Document_Reader.h"

class DriverDAT_R_SMESHDS_Document : public Document_Reader {
  
public :
  DriverDAT_R_SMESHDS_Document();
  ~DriverDAT_R_SMESHDS_Document();
 
  void Read();
  //void SetFile(string);
  //void SetDocument(Handle_SMESHDS_Document&);

private :
  //Handle_SMESHDS_Document myDocument;
  //string myFile; 

};
#endif
