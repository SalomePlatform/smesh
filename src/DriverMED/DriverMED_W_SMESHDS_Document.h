#ifndef _INCLUDE_DRIVERMED_W_SMESHDS_DOCUMENT
#define _INCLUDE_DRIVERMED_W_SMESHDS_DOCUMENT

#include "SMESHDS_Document.hxx"
#include "Document_Writer.h"

#include <string>

extern "C"
{
#include <med.h>
}

class DriverMED_W_SMESHDS_Document : public Document_Writer {
  
public :
  DriverMED_W_SMESHDS_Document();
  ~DriverMED_W_SMESHDS_Document();
 
  void Write();
  //void SetFile(string);
  //void SetDocument(Handle(SMESHDS_Document)&);

private :
  //Handle_SMESHDS_Document myDocument;
  //string myFile; 

};
#endif
