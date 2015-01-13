
#include <fstream>
#include <iomanip>
#include <plotter.h> //plotutils include

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



   // set a Plotter parameter
  PlotterParams params;
  //params.setplparam ("PAGESIZE", (char *)"letter");
  params.setplparam ("BITMAPSIZE", (char *)"400x400");
     
  XPlotter plotter(cin, cout, cerr, params); // declare Plotter
  if (plotter.openpl () < 0)                  // open Plotter
    {
      cerr << "Couldn't open Plotter\n";
      return 1;
    }

  //  plotter.fspace( -3, -5, 8, 5);
  plotter.flinewidth (0.001);       // line thickness in user coordinates
  plotter.fillcolorname("light blue");
  plotter.pencolorname("black");
  plotter.filltype(1);
  plotter.erase();


  //set index values
  int graph_x, graph_y;
  switch(axis){
  case 0:
    graph_x = 1;
    graph_y = 2; 
    break; 
  case 1: 
    graph_x = 0; 
    graph_y = 2; 
    break;
  case 2:
    graph_x = 0;
    graph_y = 1; 
    break;
  }

  double min_x=100000.0, max_x=-100000.0, min_y=100000.0, max_y=-100000.0;

  for( i = paths.begin(); i != paths.end(); i++ ) //path loop
    {
      for ( j = (*i).begin(); j != (*i).end(); j++ ) //subpath loop
	{
	  for( unsigned int k = 0; k < (*j).size(); k++) //points loop
	    {
	      double this_x = (*j)[k][graph_x];
	      double this_y = (*j)[k][graph_y];
	      if ( 0 == k ) plotter.fmove( this_x, this_y  );
	      else plotter.fcont( this_x, this_y );

	      if (min_x > this_x ) min_x = this_x;
	      if (max_x < this_x ) max_x = this_x;

	      if (min_y > this_y ) min_y = this_y;
	      if (max_y < this_y ) max_y = this_y;

	    }
	  plotter.endsubpath();
	}
    }

  std::cout << min_x << std::endl;
  std::cout << min_y << std::endl;
  std::cout << max_x << std::endl;
  std::cout << max_y << std::endl;

  plotter.fspace( min_x, min_y, max_x, max_y); 

  if (plotter.closepl () < 0)      // close Plotter
    {
      cerr << "Couldn't close Plotter\n";
      return 1;
    }
  return 0;


}
