#ifndef DAG_SLICER
#define DAG_SLICER

#include "slicer.hpp"
#include <string>
#include <vector>

#define ROAM_WARNING "WARNING: Roaming for unmatched stitch points has been \
                      enabled. This feature is intended for use as an attempt to slice \
                      unsealed meshes and isn't garaunteed to produce a valid results.";
		      
class Dag_Slicer{
  
public:
  // consructor
  Dag_Slicer(std::string file_to_slice, // filename of the modle to slice
	     int ax, // slice axis
	     double coord, // coordinate of the slice
	     bool by_grp = false, // slice by group (material) if true, by volume if false
	     bool ca = false); // 
  ~Dag_Slicer();

  // slice creation method
  int create_slice();
  
  //modify methods
  void rename_group(int group_global_id, std::string new_name);
  void write_file(std::string new_filename);
  
  // output data
  std::vector<std::string> group_names;
  std::vector<double> dum_pnts;
  std::vector< std::vector<double> > slice_x_pnts;
  std::vector< std::vector<double> > slice_y_pnts; 
  std::vector<int> group_ids;
  std::vector< std::vector<int> > path_coding;

  // slice parameters
  std::string filename;
  int axis;
  double coord;
  bool by_group;
  bool verbose;
  bool debug;
  bool roam;

  //other
  std::string _roam_warning;
};

#endif
