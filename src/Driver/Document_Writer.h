#ifndef _INCLUDE_DOCUMENT_WRITER
#define _INCLUDE_DOCUMENT_WRITER

#include "SMESHDS_Document.hxx"
#include <string>

class Document_Writer {

  public :
    virtual void Write() =0;
    void SetFile(string);
    void SetDocument(Handle(SMESHDS_Document)&);

  protected :
    Handle_SMESHDS_Document myDocument;
    string myFile;

};
#endif
