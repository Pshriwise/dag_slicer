

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

bool point_match( MBCartVect pnt1, MBCartVect pnt2);

MBErrorCode get_sets_by_category( MBInterface *mbi, MBRange &entsets, char* category);

MBErrorCode get_surfaces( MBInterface* mbi, MBRange &surfs);

MBErrorCode get_all_volumes( MBInterface *mbi, MBRange &vols);


MBErrorCode intersection( MBInterface *mbi,  int axis, double coord, MBEntityHandle tri, MBCartVect *line, bool &intersect);

