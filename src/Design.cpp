#include "Design.h"
using namespace std;
using namespace bLib;


Design::Design()
{
  m_input       = "input.ascii";
  m_output      = "output.tcl";
}


bool
Design::parseParameters(int argc, char** argv)
//{{{
{
  argc--, argv++; // parse "main"
#ifdef _DEBUG_BEI
  cout << "Parameters: ";
	for (int i=0; i<argc; i++) cout << argv[i] << " ";
	cout << endl;
#endif

  while (argc)
  {
    if ( strcmp(*argv, "-in") == 0 )
    {
      argc--, argv++;
      m_input = (*argv);
      argc--, argv++;
      continue;
    }
    if ( strcmp(*argv, "-out") == 0 )
    {
      argc--, argv++;
      m_output = (*argv);
      argc--, argv++;
      continue;
    }

		cout << "ERROR| can NOT parse: " << *argv << endl;
		return false;
  }
  return true;
}
//}}}


// ==== output tcl file
void
Design::outputTcl()
//{{{
{
  string sname = bLib::trimSuffix(m_output);
  string tcl_name = sname + ".tcl";
  string gds_name = sname + ".gds";
  m_tclFileNames.insert(tcl_name);

  ofstream out;
  out.open(tcl_name.c_str());
  out << "set L [layout create]" << endl;
  out << "$L create cell debug" << endl;

  map<int,int>::iterator mitr;
  for (mitr=m_layer2Id.begin(); mitr!=m_layer2Id.end(); mitr++)
  {
    int lid = mitr->first;
    outputTcl(lid, out);
  }

  out<< "$L gdsout "<<gds_name<<" \n" << endl;
  out.close();
}
//}}}


void
Design::outputTcl(int lid, ofstream& out)
//{{{
{
  int id = m_layer2Id[lid];
  if (id<0 || id>=m_Metals.size()) return;
  out<<"$L create layer "<<lid<<endl;

  for (int i=0; i<m_Metals[id].size(); i++)
  {
    myShape* pmyshape = m_Metals[id][i];
    if (pmyshape->getPointNum() <= 0) continue;
    out<<"$L create polygon debug "<<lid<<" ";
    for (int j=0; j<pmyshape->getPointNum(); j++)
    {
      out<<pmyshape->getPointX(j)<<" "<<pmyshape->getPointY(j)<<" ";
    }
    out<<endl;
  }
}
//}}}


// ==== output ascii 
void
Design::outputAscii()
//{{{
{
#ifdef _DEBUG_BEI
  cout << "DEBUG| OutputASCII()" << endl;
#endif
  string file = m_output;
  if (file.length() <= 0) file = "wireMerge_output.ascii";

  ofstream out;  out.open( file.c_str() );

  // Step 1: output header
  out<<"HEADER: 600"<<endl;
  out<<"BGNLIB: 2012, 9, 20, 2, 21, 41, 2012, 9, 20, 2, 21, 41"<<endl;
  out<<"LIBNAME: \"drc.db\""<<endl;
  out<<"UNITS: 0.001, 1e-9"<<endl;
  out<<"BGNSTR: 2012, 9, 20, 2, 21, 41, 2012, 9, 20, 2, 21, 41"<<endl;
  out<<"STRNAME: \"TOP\""<<endl;

  // Step 2: output each layers
  map<int,int>::iterator mitr;
  for (mitr=m_layer2Id.begin(); mitr!=m_layer2Id.end(); mitr++)
  {
    int lid = mitr->first;
    outputAscii(lid, out);
  }
  
  out<<"ENDSTR"<<endl;
  out<<"ENDLIB"<<endl;
	out.close();

  cout << "STAT| finish output to " << file << endl;
}
//}}}


void
Design::outputAscii(int lid, ofstream& out)
//{{{
{
  int id = m_layer2Id[lid];
  if (id<0 || id>=m_Metals.size()) return;

  for (int i=0; i<m_Metals[id].size(); i++)
  {
    myShape* pmyshape = m_Metals[id][i];
    if (pmyshape->getPointNum() <= 0) continue;
    out<<"BOX"<<endl;
    out<<"LAYER: "<<lid<<endl;
    out<<"BOXTYPE: 0"<<endl;
    out<<"XY: ";
    for (int j=0; j<pmyshape->getPointNum(); j++)
    {
      out << pmyshape->getPointX(j) << ", " << pmyshape->getPointY(j) << ", ";
    }
    out << pmyshape->getPointX(0) << ", " << pmyshape->getPointY(0) << endl;
    out<<"ENDEL"<<endl;
  }
}
//}}}


// ====================================================
//               Reading Functions
// ====================================================
void
Design::readAll()
//{{{
{
  size_t found = m_input.rfind( ".ascii" );
  if (found == string::npos)
  {
    cout<<"ERROR| input should be a ascii file"<<endl;  exit(0);
  }

  readAscii(1);
  m_Metals.  clear();  m_Metals.  resize(m_layer2Id.size());
  readAscii(2);

  // ==== debug
#if 0
  map<int,int>::iterator mitr;
  for (mitr=m_layer2Id.begin(); mitr!=m_layer2Id.end(); mitr++)
  {
    cout<<(*mitr).first<<" "<<(*mitr).second<<endl;
  } // for mitr
#endif
}
//}}}

double
Design::readBlock(int round, ifstream& in)
//{{{
{
  // parse information for block
  string str;
  if (false==readSearchUntil(in, str, "BOX", "BOUNDARY"))     return false;
  int layer = readLayerNum(in, "LAYER");       if (layer < 0) return false;
  if (false==readSearchUntil(in, str, "BOXTYPE", "DATATYPE")) return false;

  if (1>=round)
  {
    int size = m_layer2Id.size();
    if (m_layer2Id.find(layer)==m_layer2Id.end()) m_layer2Id[layer]=size;
    return true;
  }

  // read vpoints
  vector<myPoint> vpoints;  vpoints.clear();
  int xl = INT_MAX, yl = INT_MAX;
  int xh = INT_MIN, yh = INT_MIN;
  getline(in, str);  if (str.size() <= 0) return false;
  char* pch, chs[20000]; strcpy(chs, str.c_str());
  pch = strtok (chs, " :,");
  while (pch != NULL)
  {
    if (0 != isdigit(pch[0]) || '-' == pch[0])
    {
      int x = atoi( pch );
      pch = strtok (NULL, " :,");
      if (NULL == pch)
      {
        cout << "ERROR| in Design::ReadBlock()" << endl;
        cout << "chs = " << chs << endl;
        exit(-1);
      }
      int y = atoi( pch );
      
      if (m_ratio > 1.0 + 1e-4)
      {
        x = (int) (x / m_ratio);
        y = (int) (y / m_ratio);
      }
      vpoints.push_back( myPoint(x, y) );  // New point
      if (xl > x) xl = x;
      if (yl > y) yl = y;
      if (xh < x) xh = x;
      if (yh < y) yh = y;
    }
    pch = strtok (NULL, " :,");
  }
  if (vpoints.size() == 0) {return true;}
  vpoints.resize( vpoints.size()-1, true );

  myShape* pmyshape = new myShape(xl, yl, xh, yh);
  pmyshape->setPoints( vpoints );
  int lid = m_layer2Id[layer];
  pmyshape->setId( m_Metals[lid].size() );
  m_Metals[ lid ].push_back(pmyshape);

  if (false == readSearchUntil( in, str, "ENDEL" )) return false;
  return true;
}
//}}}

void
Design::readAscii(int round)
//{{{
{
  ifstream in;  in.open(m_input.c_str());
  if (false == in.is_open())
  {
    cout<<"ERROR| cannot open file: "<<m_input<<endl; exit(0);
  }

  string str;
  readSearchUntil(in, str, "HEADER");
  readSearchUntil(in, str, "BGNLIB");
  readSearchUntil(in, str, "LIBNAME");
  double units = readUnitsValue(in, "UNITS");
  m_ratio = 0.001 / units;
  readSearchUntil(in, str, "BGNSTR");
  readSearchUntil(in, str, "STRNAME");

  bool bb = true;
  while (bb) bb = readBlock(round, in);
}
//}}}


double
Design::readUnitsValue( ifstream& in, const string& to_compare )
//{{{
{
  size_t found = string::npos;  string str;
  while (!in.eof())
  {
    getline( in, str );   if (in.eof()) return -1.0;
    found = str.find(to_compare);
    if (found != string::npos)
    {
      char* pch, chs[100];
      strcpy(chs, str.c_str());
      pch = strtok(chs,  " :,");
      pch = strtok(NULL, " :,");
      assert( isdigit( pch[0] ) != 0 );
      return atof( pch );
    } // find "UNITS"
  }
  return -1.0;
}
//}}}

int
Design::readLayerNum( ifstream& in, const string& to_compare )
//{{{
{
  size_t found = string::npos;
  string str;
  while ( ! in.eof() )
  {
    getline( in, str );  if ( in.eof() ) return -1;
    found = str.find( to_compare );
    if (found != string::npos)
    {
      char* pch, chs[100];
      strcpy(chs, str.c_str());
      pch = strtok(chs,  " :,");
      pch = strtok(NULL, " :,");
      assert( isdigit( pch[0] ) != 0 );
      return atoi( pch );
    } // find "LAYER"
  }
  return -1;
}
//}}}


