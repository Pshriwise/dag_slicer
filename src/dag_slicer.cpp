#include "dag_slicer.hpp"

  Dag_Slicer::Dag_Slicer( std::string file_to_slice, int ax, double coordinate )
  : axis(ax), filename(file_to_slice), coord(coordinate){}

  Dag_Slicer::~Dag_Slicer()
  {
  }
  
  void Dag_Slicer::create_slice(bool by_groups)
  {
    slice_x_pnts.clear(); 
    slice_y_pnts.clear(); 
    path_coding.clear();
    slice_faceted_model_out( filename, axis, coord, slice_x_pnts, slice_y_pnts, path_coding, by_groups);
    
  }  
  
