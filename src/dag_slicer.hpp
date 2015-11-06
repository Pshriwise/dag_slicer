#ifndef DAG_SLICER
#define DAG_SLICER

#include "slicer.hpp"
#include <string>
#include <vector>

class Dag_Slicer{
  
public:
  Dag_Slicer(std::string file_to_slice, int ax, double coord, bool by_grp = false);
  ~Dag_Slicer();
  //parameters
  int _axis;
  std::string _filename;
  double _coord;
  bool _by_group;
  bool _verbose;
  bool _debug;
  //creation method
  void create_slice();
  //data
  std::vector<std::string> group_names;
  std::vector<double> dum_pnts;
  std::vector< std::vector<double> > slice_x_pnts;
  std::vector< std::vector<double> > slice_y_pnts; 
  std::vector<int> dum_ints;
  std::vector< std::vector<int> > path_coding;
};

#endif
