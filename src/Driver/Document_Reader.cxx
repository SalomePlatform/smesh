using namespace std;
#include "Document_Reader.h"

#include "utilities.h"

void Document_Reader::SetFile(string aFile) {
  myFile = aFile;
}

void Document_Reader::SetDocument(Handle(SMESHDS_Document)& aDoc) {
  myDocument = aDoc;
}

