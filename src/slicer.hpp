

//c++ includes
#include <assert.h>
#include <math.h>
#include <iostream> 
#include <string>

//SIGMA includes
#include "MBCartVect.hpp"
#include "MBCore.hpp"
#include "MBTagConventions.hpp"

#define CCW 1
#define CW -1

inline void ERR_CHECK( moab::ErrorCode rval )
{
  if (rval)
    {
      assert(false);
      std::cout << "ERROR" << std::endl; 
      exit(1);
    }
}

//special containers for our intersection data
struct Line {
  Line() : started(false), full(false) {}
  MBCartVect begin; 
  MBCartVect end; 
  bool started;
  bool full; 
  void add_pnt( MBCartVect a )
  {
    
    if (full) std::cout << "Line is full. Point not added." << std::endl;

    if(!started)
      {
	begin = a;
	started = true; 
      }
    else
      {
	end = a;
	full = true;
      }

  }

};

struct xypnt{
  double x; 
  double y; 
};

struct Loop{
  Loop() : num_pnts(0), closed(false) {}
  std::vector<MBCartVect> points;
  std::vector<xypnt> xypnts; 
  int num_pnts; 
  bool closed;

  void gen_xys(int axis)
  {
    
    xypnts.clear(); //clear out vector as precaution
    int x,y;
    
    //set indices for x an y based on slice plane (provided externally)
    switch(axis)
      {
      case 0:
	x = 1; y = 2; break;
      case 1:
	x = 0; y = 2; break;
      case 2:
	x = 0; y = 1; break;
      }

    xypnts.resize(points.size());
    for( unsigned int i = 0; i < points.size(); i++)
      {
	xypnts[i].x = points[i][x];
	xypnts[i].y = points[i][y];
      }
  }
  
};



bool point_match( MBCartVect pnt1, MBCartVect pnt2);

MBErrorCode get_sets_by_category( MBInterface *mbi, MBRange &entsets, char* category);

MBErrorCode get_surfaces( MBInterface* mbi, MBRange &surfs);

MBErrorCode get_all_volumes( MBInterface *mbi, MBRange &vols);


MBErrorCode slice_faceted_model( std::string filename, int axis, double coord, std::vector< std::vector<Loop> > &all_paths );

MBErrorCode slice_faceted_model( std::string filename, int axis, double coord,   std::vector< std::vector< std::vector< std::vector<double> > > > &valid_paths );

MBErrorCode create_surface_intersections( MBInterface *mbi, MBRange surfs, int axis, double coord,   std::map<MBEntityHandle,std::vector<Loop> > &intersection_map);

MBErrorCode get_volume_intersections( MBInterface *mbi, MBEntityHandle volume, std::map<MBEntityHandle, std::vector<Loop> > intersection_dict,   std::vector<Loop> &volume_intersections );

MBErrorCode get_volume_paths( MBInterface *mbi, MBRange volumes, int axis, std::map<MBEntityHandle, std::vector<Loop> > intersection_dict,   std::vector< std::vector<Loop> > &all_vol_paths );

void stitch( std::vector<Loop> loops, std::vector<Loop> &paths );

MBErrorCode surface_intersections(MBInterface *mbi, std::vector<MBEntityHandle> tris, int axis, double coord, std::vector<Loop> &surf_intersections);

MBErrorCode intersection( MBInterface *mbi,  int axis, double coord, MBEntityHandle tri, Line &tri_intersection, bool &intersect);

void triangle_plane_intersect( int axis, double coord, MBCartVect *coords, Line &line_out);

void get_intersection( MBCartVect pnt0, MBCartVect pnt1, int axis, double coord, Line &line );

void convert_to_stl( std::vector< std::vector<Loop> > a, std::vector< std::vector< std::vector< std::vector<double> > > > &b);
