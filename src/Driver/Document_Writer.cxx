using namespace std;
#include "Document_Writer.h"

void Document_Writer::SetFile(string aFile) {
  myFile = aFile;
}

void Document_Writer::SetDocument(Handle(SMESHDS_Document)& aDoc) {
  myDocument = aDoc;
}

