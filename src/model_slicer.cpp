
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

  std::vector< std::vector< std::vector< std::vector<double> > > > paths;
  result = slice_faceted_model( filename, axis, coord, paths );
  ERR_CHECK(result);
  std::vector< std::vector< std::vector< std::vector<double> > > >::iterator i; 
  std::vector< std::vector< std::vector<double> > >::iterator j; 

  std::ofstream output; 
  output.precision(6);
  //output.set(std::ios::fixed);
  //output.set(std::ios::showpoint);

 


  output.open("slicepnts.txt");

  for( i = paths.begin(); i != paths.end(); i++ )
    {
      for ( j = (*i).begin(); j != (*i).end(); j++ )
	{
	  for( unsigned int k = 0; k < (*j).size(); k++)
	    {
	    for( unsigned int ax = 0; ax <= 2; ax++)
	      {
		if ( ax != axis )
		  output << (*j)[k][ax] << std::fixed << " ";
	      }
	    output << std::endl;
	    }
	  output << std::endl;
	}
    }

  output.close();

}
