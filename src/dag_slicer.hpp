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
  //modify methods
  void rename_group(int group_global_id, std::string new_name);
  void write_file(std::string new_filename);
  //data
  std::vector<std::string> group_names;
  std::vector<double> dum_pnts;
  std::vector< std::vector<double> > slice_x_pnts;
  std::vector< std::vector<double> > slice_y_pnts; 
  std::vector<int> group_ids;
  std::vector< std::vector<int> > path_coding;
};

#endif
