
#include <fstream>

#include "slicer.hpp"
#include "moab/ProgOptions.hpp"


int main( int argc, char ** argv )
{

  ProgOptions po( "Program for slicing a dagmc model." );

  std::string filename;
  int axis = 0;
  double coord = 0.0;
  bool write_to_file = false;
  bool plot = false;

  po.addRequiredArg<std::string>( "input_file", "Filename of the .h5m model to slice.", &filename );

  po.addOpt<int>( "ax", "Axis along which to slice the model. Default is x (0).", &axis );

  po.addOpt<double>( "coord", "Coordinate for the slice. Default is 0.0");

  po.addOpt<void>( "w", "Write slice points to file slicepnts.txt.", &write_to_file);

  po.addOpt<void>( "p", "Plot the slice using gnuplot streamer.", &plot);

  po.parseCommandLine( argc, argv ); 

  MBInterface *mbi = new MBCore();

  MBErrorCode result;
  result = mbi->load_file( filename.c_str() );
  ERR_CHECK(result);

  MBRange vols; 
  result = get_all_volumes( mbi, vols );
  ERR_CHECK(result);

  MBRange surfs; 
  result = get_surfaces( mbi, surfs ); 
  ERR_CHECK(result);

  std::cout << "Creating surface intersections..." << std::endl; 

  std::map<MBEntityHandle,std::vector<Loop> > surf_intersections;
  result = create_surface_intersections( mbi, surfs, axis, coord, surf_intersections );
  ERR_CHECK(result);

  std::cout << "Stitching intersections into paths..." << std::endl; 

  std::vector< std::vector<Loop> > paths; 
  result = get_volume_paths( mbi, vols, axis, surf_intersections, paths);
  ERR_CHECK(result);

  std::vector< std::vector<Loop> >::iterator i; 
  std::vector<Loop>::iterator j; 

  std::ofstream output; 
  output.open("slicepnts.txt");

  for( i = paths.begin(); i != paths.end(); i++ )
    {
      for ( j = (*i).begin(); j != (*i).end(); j++ )
	{
	  for( unsigned int k = 0; k < (*j).points.size(); k++)
	    {
	    for( unsigned int ax = 0; ax <= 2; ax++)
	      {
		if ( ax != axis )
		output << (*j).points[k][ax] << "\t";
	      }
	  output << std::endl;
	}
      output << std::endl;
    }
    }
}
