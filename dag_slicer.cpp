#include "dag_slicer.hpp"

Dag_Slicer::Dag_Slicer( std::string file_to_slice, int ax, double coordinate, bool by_grp, bool ca)
  : axis(ax), filename(file_to_slice), coord(coordinate), by_group(by_grp), verbose(false), debug(false), roam(false) { }

  Dag_Slicer::~Dag_Slicer() { }

  int Dag_Slicer::create_slice() {
    slice_x_pnts.clear(); 
    slice_y_pnts.clear(); 
    path_coding.clear();
    int result = slice_faceted_model_out(filename,
					 axis,
					 coord,
					 slice_x_pnts,
					 slice_y_pnts,
					 path_coding,
					 group_names,
					 group_ids,
					 by_group,
					 verbose,
					 debug,
					 roam);
    return result;
  }  
  
void Dag_Slicer::rename_group(int group_global_id, std::string new_name) {
  rename_group_out(group_global_id, new_name);
}
     
void Dag_Slicer::write_file(std::string new_filename) {
  write_file_out(new_filename);
}
