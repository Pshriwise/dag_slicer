

//c++ includes
#include <assert.h>
#include <math.h>
#include <iostream> 
#include <string>

//SIGMA includes
#include "MBCartVect.hpp"
#include "MBCore.hpp"
#include "MBTagConventions.hpp"

#define CCW = 1
#define CW = -1

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
  Line() : full(false) {}
  MBCartVect begin; 
  MBCartVect end; 
  bool full; 
  void check_cap ()
  {
    if (begin[0] != NULL && end[0] != NULL) full = true;
  }
};

struct Loop{
  Loop() : num_pnts(0), closed(false) {}
  std::vector<MBCartVect> points;
  int num_pnts; 
  bool closed;

};


bool point_match( MBCartVect pnt1, MBCartVect pnt2);

MBErrorCode get_sets_by_category( MBInterface *mbi, MBRange &entsets, char* category);

MBErrorCode get_surfaces( MBInterface* mbi, MBRange &surfs);

MBErrorCode get_all_volumes( MBInterface *mbi, MBRange &vols);

MBErrorCode surface_intersections(MBInterface *mbi, std::vector<MBEntityHandle> tris, int axis, double coord, std::vector<Loop> &surf_intersections);

MBErrorCode intersection( MBInterface *mbi,  int axis, double coord, MBEntityHandle tri, Line &tri_intersection, bool &intersect);

void triangle_plane_intersect( int axis, double coord, MBCartVect *coords, Line &line_out);

void get_intersection( MBCartVect pnt0, MBCartVect pnt1, int axis, double coord, Line &line );

