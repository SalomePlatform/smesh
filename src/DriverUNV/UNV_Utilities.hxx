//  Copyright (C) 2003  CEA/DEN, EDF R&D
//
//
//
//  File   : VISU_DatConvertor.hxx
//  Author : Alexey PETROV
//  Module : VISU

#ifndef MED_Utilities_HeaderFile
#define MED_Utilities_HeaderFile

#include <iostream>	
#include <sstream>	
#include <string>
#include <stdexcept>
#include <cassert>

namespace UNV{
  using namespace std;

  class PrefixPrinter{
    static int myCounter;
  public:
    PrefixPrinter();
    ~PrefixPrinter();

    static string GetPrefix();
  };

  /**
   * @returns \p false when error occured, \p true otherwise.
   * Adjusts the \p in_stream to the beginning of the
   * dataset \p ds_name.
   */
  inline bool beginning_of_dataset(std::istream& in_file, const std::string& ds_name)
  {
    assert (in_file.good());
    assert (!ds_name.empty());
    
    std::string olds, news;
    
    while(true){
      in_file >> olds >> news;
      /*
       * a "-1" followed by a number means the beginning of a dataset
       * stop combing at the end of the file
       */
      while( ((olds != "-1") || (news == "-1") ) && !in_file.eof() ){	  
	olds = news;
	in_file >> news;
      }
      if(in_file.eof())
	return false;
      if (news == ds_name)
	return true;
    }
    // should never end up here
    return false;
  }

  /**
   * Method for converting exponential notation
   * from "D" to "e", for example
   * \p 3.141592654D+00 \p --> \p 3.141592654e+00
   * in order to make it readable for C++.
   */
  inline double D_to_e(std::string& number)
  {
    /* find "D" in string, start looking at 
     * 6th element, to improve speed.
     * We dont expect a "D" earlier
     */
    const int position = number.find("D",6);
    if(position != std::string::npos){
      number.replace(position, 1, "e"); 
    }
    return atof (number.c_str());
  }

};


#ifndef MESSAGE

#define MESSAGE(msg) std::cout<<__FILE__<<"["<<__LINE__<<"]::"<<msg<<endl;

#define BEGMSG(msg) std::cout<<UNV::PrefixPrinter::GetPrefix()<<msg

#define ADDMSG(msg) std::cout<<msg

#endif


#ifndef EXCEPTION

#define EXCEPTION(TYPE, MSG) {\
  std::ostringstream aStream;\
  aStream<<__FILE__<<"["<<__LINE__<<"]::"<<MSG;\
  throw TYPE(aStream.str());\
}

#endif

#endif
