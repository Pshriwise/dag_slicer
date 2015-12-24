#include "dag_slicer.hpp"

Dag_Slicer::Dag_Slicer( std::string file_to_slice, int ax, double coordinate, bool by_grp )
  : _axis(ax), _filename(file_to_slice), _coord(coordinate), _by_group(by_grp), _verbose(false), _debug(false) { }

  Dag_Slicer::~Dag_Slicer() { }

  void Dag_Slicer::create_slice() {
    slice_x_pnts.clear(); 
    slice_y_pnts.clear(); 
    path_coding.clear();
    slice_faceted_model_out(_filename,
			    _axis,
			    _coord,
			    slice_x_pnts,
			    slice_y_pnts,
			    path_coding,
			    group_names,
			    _by_group,
			    _verbose,
			    _debug);

  }  
  
void Dag_Slicer::rename_group(int group_global_id, std::string new_name) {
  rename_group_out(group_global_id, new_name);
}
     
void Dag_Slicer::write_file(std::string new_filename) {
  write_file_out(new_filename);
}
