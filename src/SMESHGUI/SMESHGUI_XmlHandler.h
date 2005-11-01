//  SMESH SMESHGUI : reading of xml file with list of available hypotheses and algorithms
//
//  Copyright (C) 2003  CEA
//
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org
//
//
//
//  File   : SMESHGUI_XmlHandler.cxx
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_XmlHandler_HeaderFile
#define SMESHGUI_XmlHandler_HeaderFile

#include "SMESHGUI_Hypotheses.h"

#include <qxml.h>
#include <map>

class SMESHGUI_XmlHandler : public QXmlDefaultHandler
{
 public:
  SMESHGUI_XmlHandler();
  virtual ~SMESHGUI_XmlHandler();

  bool startDocument();
  bool startElement( const QString& namespaceURI, const QString& localName, 
		     const QString& qName, const QXmlAttributes& atts );
  bool endElement( const QString& namespaceURI, const QString& localName, const QString& qName );
  bool characters( const QString& ch );

  QString errorString();
  QString errorProtocol();
  bool fatalError   (const QXmlParseException& exception);

 public:
  std::map<std::string, HypothesisData*> myHypothesesMap;
  std::map<std::string, HypothesisData*> myAlgorithmsMap;

  std::list<HypothesesSet*> myListOfHypothesesSets;

 private:
  QString myErrorProt;
  QString myPluginName;
  QString myServerLib;
  QString myClientLib;
};


#endif
