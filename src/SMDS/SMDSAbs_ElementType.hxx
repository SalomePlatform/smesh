//  SMESH SMDS : implementaion of Salome mesh data structure
//
//  Copyright (C) 2003  OPEN CASCADE
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
//  See http://www.opencascade.org or email : webmaster@opencascade.org 
//
//
//
//  File   : SMDSAbs_ElementType.hxx
//  Module : SMESH

#ifndef _SMDSAbs_ElementType_HeaderFile
#define _SMDSAbs_ElementType_HeaderFile

///////////////////////////////////////////////////////////////////////////////
/// Type (node, edge, face or volume) of elements
///////////////////////////////////////////////////////////////////////////////
enum SMDSAbs_ElementType
{
	SMDSAbs_All,
	SMDSAbs_Node,
	SMDSAbs_Edge,
	SMDSAbs_Face,
	SMDSAbs_Volume
};

#endif
