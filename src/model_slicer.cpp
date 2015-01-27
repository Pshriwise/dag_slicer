
#include <fstream>
#include <iomanip>

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

  po.addOpt<double>( "coord", "Coordinate for the slice. Default is 0.0", &coord);

  po.addOpt<void>( "w", "Write slice points to file slicepnts.txt.", &write_to_file);

  po.addOpt<void>( "p", "Plot the slice using gnuplot streamer.", &plot);

  po.parseCommandLine( argc, argv ); 



  MBErrorCode result; 

  std::vector< std::vector<xypnt> > paths;
  std::vector< std::vector<int> > codings;
  result = slice_faceted_model( filename, axis, coord, paths, codings );
  ERR_CHECK(result);

  std::ofstream output, coding; 
  output.precision(6);
  //output.set(std::ios::fixed);
  //output.set(std::ios::showpoint);

 

  std::vector< std::vector<xypnt> >::iterator path;
  std::vector< std::vector<int> >::iterator code;
  output.open("slicepnts.txt");
  coding.open("coding.txt");

  for( path = paths.begin(), code = codings.begin(); path != paths.end(); path++, code++)
    {
      for( unsigned int i = 0; i < (*path).size(); i++)
	{
	  //write x point
	  output << (*path)[i].x << std::fixed << " ";
	  output << (*path)[i].y << std::fixed << " ";
	  output << std::endl;
	  coding << (*code)[i] << std::endl;
	}
      output << std::endl;
      coding << std::endl;
    }

  output.close();
  coding.close();
}
