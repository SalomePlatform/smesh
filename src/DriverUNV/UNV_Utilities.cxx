//  Copyright (C) 2003  CEA/DEN, EDF R&D
//
//
//
//  File   : VISU_DatConvertor.cxx
//  Author : Alexey PETROV
//  Module : VISU

#include "UNV_Utilities.hxx"

using namespace std;

#ifdef _DEBUG_
static int MYDEBUG = 1;
#else
static int MYDEBUG = 0;
#endif


int UNV::PrefixPrinter::myCounter = 0;

UNV::PrefixPrinter::PrefixPrinter()
{
  myCounter++;
}

UNV::PrefixPrinter::~PrefixPrinter()
{
  myCounter--;
}

string UNV::PrefixPrinter::GetPrefix()
{
  if(myCounter)
    return string(myCounter*2,' ');
  return "";
}
