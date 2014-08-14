#ifndef _DESIGN_H_
#define _DESIGN_H_

#include <boost/algorithm/string.hpp>
#include "bLib/bLibBase.h"
#include "bLib/bLibShape.h"
#include "main.h"

class Design
{
public:
  Design();
  ~Design(){}

  bool   parseParameters(int argc, char** argv);
  void   readAll();
  void   outputTcl();                    // dump out tcl file
  void   outputTcl(int, std::ofstream&);
  void   outputAscii();                  // dump out ascii file
  void   outputAscii(int, std::ofstream&);

protected:
  std::string             m_input;          // input file name
  std::string             m_output;         // output file name
  std::set<std::string>   m_tclFileNames;   // all output tcl filenames
  double                  m_ratio;

  std::vector< std::vector<bLib::myShape*> >     m_Metals;    // input myShape
  std::map<int, int>                             m_layer2Id;

  // local functions
  void   readAscii(int);
  double readUnitsValue(std::ifstream&, const std::string&);
  int    readLayerNum(std::ifstream&, const std::string&);
  double readBlock(int, std::ifstream&);
};

#endif


