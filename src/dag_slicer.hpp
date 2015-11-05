#ifndef DAG_SLICER
#define DAG_SLICER

#include "slicer.hpp"
#include <string>
#include <vector>

class Dag_Slicer{
  
public:
  Dag_Slicer(std::string file_to_slice, int ax, double coord, bool by_grp = false);
  ~Dag_Slicer();
  std::string filename; 
  int axis; 
  double coord; 
  std::vector<std::string> group_names;
  std::vector<double> dum_pnts;
  std::vector< std::vector<double> > slice_x_pnts;
  std::vector< std::vector<double> > slice_y_pnts; 
  std::vector<int> dum_ints;
  std::vector< std::vector<int> > path_coding;
  void create_slice();
  bool by_group;
  bool verbose; 
  bool debug;
};

#endif
