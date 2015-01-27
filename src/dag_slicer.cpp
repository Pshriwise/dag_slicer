

#include "dag_slicer.hpp"

Dag_Slicer::Dag_Slicer( std::string file_to_slice, int ax, double coordinate )
  : axis(ax), filename(file_to_slice), coord(coordinate){}


void Dag_Slicer::create_slice()
{

  slice_faceted_model_out( filename, axis, coord, slice_x_pnts, slice_y_pnts, path_coding);

}

Dag_Slicer::~Dag_Slicer()
{
}

