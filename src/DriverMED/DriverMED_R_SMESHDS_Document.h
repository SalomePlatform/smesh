#ifndef _INCLUDE_DRIVERMED_R_SMESHDS_DOCUMENT
#define _INCLUDE_DRIVERMED_R_SMESHDS_DOCUMENT

#include "SMESHDS_Document.hxx"
#include "Document_Reader.h"

extern "C"
{
#include <med.h>
}

class DriverMED_R_SMESHDS_Document : public Document_Reader {
  
public :
  DriverMED_R_SMESHDS_Document();
  ~DriverMED_R_SMESHDS_Document();
 
  void Read();
  //void SetFile(string);

private :
  //Handle_SMESHDS_Document myDocument;
  //string myFile; 

};
#endif
