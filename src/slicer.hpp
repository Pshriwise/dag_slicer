
//c++ includes
#include <assert.h>
#include <math.h>
#include <iostream> 
#include <string>

//SIGMA includes
#include "moab/CartVect.hpp"
#include "moab/Core.hpp"
#include "MBTagConventions.hpp"

// std includes
#include <vector>

//GLOBAL Defines
#define CCW 1
#define CW -1

#define MATCH_TOL 1e-7
#define FILENAME_TAG_NAME "FILENAME"

inline void ERR_CHECK(moab::ErrorCode rval) {
  if (rval) {
    assert(false);
    std::cout << "ERROR" << std::endl; 
    exit(1);
  }
}

#ifndef SLICER_H
#define SLICER_H

moab::Interface* mbi();

#endif
  
//special containers for our intersection data
struct Line {
  Line() : started(false), full(false) {}
  moab::CartVect begin;
  moab::CartVect end;
  bool started;
  bool full;
  void add_pnt(moab::CartVect a) {
    
    if (full) {
      std::cout << "Line is full. Point not added." << std::endl;
    }
    
    if(!started) {
      begin = a;
      started = true; 
    }
    else {
	end = a;
	full = true;
    }
  }
};

struct xypnt {
  double x; 
  double y; 
};

struct Loop {
  Loop() : num_pnts(0), closed(false) { }
  ~Loop() { }
  std::vector<moab::CartVect> points;
  std::vector<xypnt> xypnts;
  int num_pnts;
  bool closed;

  void gen_xys(int axis) {
    
    xypnts.clear(); //clear out vector as precaution
    int x = 0 , y = 0;
    
    //set indices for x an y based on slice plane (provided externally)
    switch(axis) {
      case 0:
	x = 1; y = 2; break;
      case 1:
	x = 0; y = 2; break;
      case 2:
	x = 0; y = 1; break;
      }

    //this shouldn't happen
    if ( x == 0 && y == 0 ) {
      ERR_CHECK(moab::MB_FAILURE);
    }
    
    xypnts.resize(points.size());
    for (unsigned int i = 0; i < points.size(); i++) {
      xypnts[i].x = points[i][x];
      xypnts[i].y = points[i][y];
    }
  }
};

bool point_match(moab::CartVect pnt1,
		 moab::CartVect pnt2,
		 double tolerance = MATCH_TOL);

moab::ErrorCode get_sets_by_category(moab::Range &entsets,
				     char* category);

moab::ErrorCode get_surfaces(moab::Interface* mbi,
			     moab::Range &surfs);

moab::ErrorCode get_all_volumes(moab::Range &vols);

moab::ErrorCode slice_faceted_model_out(std::string filename,
					int axis,
					double coord,
					std::vector< std::vector<double> > &x_pnts,
					std::vector< std::vector<double> > &y_pnts,
					std::vector< std::vector<int> > &codings,
					std::vector<std::string> &group_names,
					bool by_group = false,
					bool verbose = false,
					bool debug = false);

moab::ErrorCode slice_faceted_model(std::string filename,
				    int axis,
				    double coord,
				    std::vector< std::vector<xypnt> > &paths,
				    std::vector< std::vector<int> > &codings,
				    std::vector<std::string> &group_names,
				    bool by_group = false);

moab::ErrorCode get_volumes_by_group(std::map< std::string, moab::Range > &group_map,
				     std::vector<std::string> &group_names);

moab::ErrorCode create_surface_intersections(moab::Range surfs,
					     int axis,
					     double coord,
					     std::map<moab::EntityHandle, std::vector<Loop> > &intersection_map);

moab::ErrorCode get_volume_intersections(moab::EntityHandle volume,
					  std::map<moab::EntityHandle, std::vector<Loop> > intersection_dict,
					  std::vector<Loop> &volume_intersections);

moab::ErrorCode get_volume_paths(moab::Range volumes,
				 std::map<moab::EntityHandle, std::vector<Loop> > intersection_dict,
				 std::vector< std::vector<Loop> > &all_vol_paths);

void stitch(std::vector<Loop> loops, std::vector<Loop> &paths);

moab::ErrorCode surface_intersections(std::vector<moab::EntityHandle> tris,
				      int axis,
				      double coord,
				      std::vector<Loop> &surf_intersections);

moab::ErrorCode intersection(int axis,
			     double coord,
			     moab::EntityHandle tri,
			     Line &tri_intersection,
			     bool &intersect);

void triangle_plane_intersect(int axis,
			      double coord,
			      moab::CartVect *coords,
			      Line &line_out);

void get_intersection(moab::CartVect pnt0,
		      moab::CartVect pnt1,
		      int axis,
		      double coord,
		      Line &line);

void convert_to_stl(std::vector< std::vector<Loop> > a,
		    std::vector< std::vector< std::vector< std::vector<double> > > > &b);

void create_patch(int axis,
		  std::vector<Loop> input_loops,
		  std::vector<xypnt> &path_out,
		  std::vector<int> &coding_out);

void get_containment(std::vector<Loop> loops,
		     std::vector< std::vector<int> > &Mat);

bool is_poly_a_in_poly_b(Loop a, Loop b);

void get_windings(std::vector<Loop> loops, std::vector<int> &windings);

int find_winding(Loop loop);

void get_fill_windings(std::vector< std::vector<int> > fill_mat, std::vector<int> &windings);

void set_windings(std::vector<int> current_windings,
		  std::vector<int> desired_windings,
		  std::vector<Loop> &loops);

void generate_patch_path(std::vector<Loop> loops,
			 std::vector<xypnt> &path,
			 std::vector<int> &coding);


void rename_group_out(int group_global_id, std::string new_name);
