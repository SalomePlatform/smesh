#ifndef _INCLUDE_DOCUMENT_READER
#define _INCLUDE_DOCUMENT_READER

#include "SMESHDS_Document.hxx"
#include <string>

class Document_Reader {
  
  public :
    virtual void Read() =0;
    void SetFile(string);
    void SetDocument(Handle(SMESHDS_Document)&);

  protected :
    Handle_SMESHDS_Document myDocument;
    string myFile; 
};
#endif
