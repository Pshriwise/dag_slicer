

#include "slicer.hpp"

class Dag_Slicer
{
public:

  Dag_Slicer( std::string file_to_slice, int ax, double coord );

  std::string filename; 
  int axis; 
  double coord; 
  std::vector< std::vector<double> > slice_x_pnts;
  std::vector< std::vector<double> > slice_y_pnts;

  ~Dag_Slicer();
  
 
  std::vector< std::vector<int> > path_coding;
  
  void create_slice();

};
