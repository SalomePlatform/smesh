//  SMESH Driver : implementation of driver for reading and writing	
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 

#include "SMESHDriver.h"

#include <dlfcn.h>
#include <utilities.h>

Document_Reader* SMESHDriver::GetDocumentReader(string Extension)
{
	// if there is not document reader in the driver create a default
	// one with the mesh reader.
	Document_Reader * docReader=
		(Document_Reader*)getMeshDocumentDriver(Extension);

	if(docReader==NULL)
	{
		Mesh_Reader * reader=GetMeshReader(Extension);
		if(reader==NULL)
		{
	    	MESSAGE("No driver known for this extension");
			return NULL;
		}
		return new Document_Reader(reader);
	}
	else return docReader;
}

Document_Writer* SMESHDriver::GetDocumentWriter(string Extension)
{
	Mesh_Writer * writer=GetMeshWriter(Extension);
	if(writer==NULL)
	{
    	MESSAGE("No driver known for this extension");
		return NULL;
	}
	return new Document_Writer(writer);
}

Mesh_Reader* SMESHDriver::GetMeshReader(string extension)
{
	void * driver = getMeshDriver(extension, string("Reader"));
	return (Mesh_Reader*)driver;
}

Mesh_Writer* SMESHDriver::GetMeshWriter(string extension)
{
	void * driver = getMeshDriver(extension, string("Writer"));
	return (Mesh_Writer*)driver;
}

void * SMESHDriver::getMeshDriver(string extension, string type)
{
	string libName = string("libMeshDriver")+extension+string(".so");
	void * handle = dlopen(libName.c_str(), RTLD_LAZY);
	if(!handle)
	{
		fputs (dlerror(), stderr);
		return NULL;
	}
	else
	{
		void * (*factory)();
		string symbol = string("SMESH_create")+extension+string("Mesh")+type;
		factory = (void * (*)()) dlsym(handle, symbol.c_str());
		if(factory==NULL)
		{
			fputs (dlerror(), stderr);
			return NULL;
		}
		else return factory();
	}
}

void * SMESHDriver::getMeshDocumentDriver(string extension)
{
	string libName = string("libMeshDriver")+extension+string(".so");
	void * handle = dlopen(libName.c_str(), RTLD_LAZY);
	if(!handle)
	{
		fputs (dlerror(), stderr);
		return NULL;
	}
	else
	{
		void * (*factory)();
		string symbol = string("SMESH_create")+extension+string("DocumentReader");
		factory = (void * (*)()) dlsym(handle, symbol.c_str());
		if(factory==NULL)
		{
			fputs (dlerror(), stderr);
			return NULL;
		}
		else return factory();
	}
}
