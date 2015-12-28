
#include "slicer.hpp"
#include "options.hpp"

moab::Interface* mbi() {
  static moab::Core instance;
  return &instance;
}

struct program_option_struct opts;

bool point_match(moab::CartVect pnt1, moab::CartVect pnt2, double tolerance) {
  moab::CartVect diff = pnt2-pnt1; 

  return (diff.length() < tolerance);
}

moab::ErrorCode get_sets_by_category(moab::Range &entsets, char* category) {
  
  moab::ErrorCode result = moab::MB_SUCCESS;

  //get the name tag from the moab instance
  moab::Tag category_tag; 
  result = mbi()->tag_get_handle(CATEGORY_TAG_NAME, category_tag); 
  ERR_CHECK(result); 

  //create void pointer for tag data match
  const void *dum = &(category[0]);

  result = mbi()->get_entities_by_type_and_tag(0, moab::MBENTITYSET, &category_tag, &dum, 1, entsets);
  ERR_CHECK(result);
  
  return result;

}

moab::ErrorCode get_surfaces(moab::Range &surfs) {

  moab::ErrorCode result = moab::MB_SUCCESS;

  char category[CATEGORY_TAG_SIZE] = "Surface";
  
  result = get_sets_by_category(surfs, category);

  if (OPT_VERBOSE) std::cout << "There are " << surfs.size() << " surfaces in this model." << std::endl;

  return result;

}

moab::ErrorCode get_all_volumes(moab::Range &vols) {

  moab::ErrorCode result = moab::MB_SUCCESS;

  char category[CATEGORY_TAG_SIZE] = "Volume";
  
  result = get_sets_by_category(vols, category);
  
  if (OPT_VERBOSE) std::cout << "There are " << vols.size() << " volumes in this model." << std::endl;

  return result; 
}

moab::ErrorCode slice_faceted_model_out(std::string filename,
					int axis,
					double coord,
					std::vector< std::vector<double> > &x_pnts,
					std::vector< std::vector<double> > &y_pnts,
					std::vector< std::vector<int> > &codings,
					std::vector<std::string> &group_names,
					std::vector<int> &group_ids,
					bool by_group,
					bool verbose, bool debug) {
  opts.verbose = verbose;
  opts.debug = debug;
  std::vector< std::vector<xypnt> > paths;
  moab::ErrorCode result = slice_faceted_model(filename, axis, coord, paths, codings, group_names, group_ids, by_group);

  std::vector< std::vector<xypnt> >::iterator path;

  for (path = paths.begin(); path != paths.end(); path++) {
    std::vector<double> path_xs, path_ys;
    for (unsigned int i = 0; i < (*path).size(); i++)	{
      path_xs.push_back((*path)[i].x);
      path_ys.push_back((*path)[i].y);
    }
    x_pnts.push_back(path_xs);
    y_pnts.push_back(path_ys);
  }
  
  return result;
}

moab::ErrorCode slice_faceted_model(std::string filename,
				    int axis,
				    double coord,
				    std::vector< std::vector<xypnt> > &paths,
				    std::vector< std::vector<int> > &codings,
				    std::vector<std::string> &group_names,
				    std::vector<int> &group_ids,
				    bool by_group) {
  
  moab::ErrorCode result;
  std::map<moab::EntityHandle,std::vector<Loop> > intersection_map;

  if(is_new_filename(filename))
    {
      //remove all old mesh content
      result = mbi()->delete_mesh();
      ERR_CHECK(result);
      //load the new file
      std::cout << "Loading new file..." << std::endl;
      result = mbi()->load_file(filename.c_str());
      if (moab::MB_FILE_DOES_NOT_EXIST == result) {
	std::cout << "Could not open specified file." << std::endl;
	return result;
      }
      else {
	ERR_CHECK(result);
      }

      //get the filename_tag
      moab::Tag filename_tag;
      result = mbi()->tag_get_handle( FILENAME_TAG_NAME, 50, moab::MB_TYPE_OPAQUE, filename_tag, moab::MB_TAG_CREAT|moab::MB_TAG_SPARSE);
      ERR_CHECK(result);

      //tag the root set with the filename
      moab::EntityHandle rs = mbi()->get_root_set();
      result = mbi()->tag_set_data(filename_tag,&rs,1,(void*)filename.c_str());
      ERR_CHECK(result);
    }
  
  moab::Range surfaces;
  result = get_surfaces(surfaces);
  ERR_CHECK(result);

  result = create_surface_intersections(surfaces, axis, coord, intersection_map);
  ERR_CHECK(result);

  //path container for a volume
  std::vector< std::vector<Loop> > all_paths;

  if (by_group) {
    std::map< std::string, moab::Range > group_mapping;
    result = get_volumes_by_group(group_mapping, group_names, group_ids);
    ERR_CHECK(result);
    assert(group_names.size() == group_ids.size());
    if (OPT_VERBOSE) std::cout << "Size of group map: " << group_mapping.size() << std::endl;
    if (OPT_VERBOSE) std::cout << "Size of group names: " << group_names.size() << std::endl;
    std::vector<std::string>::iterator group_name;
    for (group_name = group_names.begin(); group_name != group_names.end();) {
      std::vector< std::vector<Loop> > all_group_paths;
      result = get_volume_paths(group_mapping[*group_name], intersection_map, all_group_paths);
      ERR_CHECK(result);


      if (0 == all_group_paths.size()) {
	if (OPT_VERBOSE) std::cout << "Erasing group: " << *group_name << std::endl;
	int val = group_name-group_names.begin();
	std::vector<int>::iterator group_id = group_ids.begin()+val;
	group_ids.erase(group_id);
	group_names.erase(group_name); //erase and set iterator to next item
	continue;
      }

      if (OPT_DEBUG) std::cout << "Getting slice for group:" << *group_name << std::endl;
      
      std::vector<xypnt> group_path;
      std::vector<int> group_coding;
      std::vector< std::vector<Loop> >::iterator path;

      for (path = all_group_paths.begin(); path != all_group_paths.end(); path++) {
	std::vector<xypnt> vol_path;
	std::vector<int> vol_coding;
	create_patch(axis, *path, vol_path, vol_coding);
	//insert this path and coding into the group path and group coding
	group_path.insert(group_path.end(), vol_path.begin(), vol_path.end());
	group_coding.insert(group_coding.end(), vol_coding.begin(), vol_coding.end());
	(*path).clear();
      } //path loop
      //when we're done with this group, push the path and coding into the main set of paths
      codings.push_back(group_coding);
      paths.push_back(group_path);
      all_group_paths.clear();
      group_name++;
    } //group loop
  }
  else {
    moab::Range volumes;
    result = get_all_volumes(volumes);
    ERR_CHECK(result);

    result = get_volume_paths(volumes, intersection_map, all_paths);
    ERR_CHECK(result);
    
    std::vector< std::vector<Loop> >::iterator i;
    for (i = all_paths.begin(); i != all_paths.end(); i++) {
      std::vector<xypnt> path;
      std::vector<int> coding;
      create_patch(axis, *i, path, coding);
      codings.push_back(coding);
      paths.push_back(path);
    }
  }

  std::cout << "Size of all paths is: " << paths.size() << std::endl;
  
  return moab::MB_SUCCESS;
}

moab::ErrorCode get_volumes_by_group(std::map< std::string,
				     moab::Range > &group_map,
				     std::vector<std::string> &group_names,
				     std::vector<int> &group_ids) {
  moab::ErrorCode result;

  //clear out old data
  group_map.clear();
  group_names.clear();
  group_ids.clear();
  
  //get all meshsets in the model

  //get all groups in the model (defined by having a name tag and category tag)
  moab::Tag category_tag, name_tag, global_id_tag;
  result = mbi()->tag_get_handle(CATEGORY_TAG_NAME, category_tag);
  ERR_CHECK(result);
  result = mbi()->tag_get_handle(NAME_TAG_NAME, name_tag);
  ERR_CHECK(result);
  result = mbi()->tag_get_handle(GLOBAL_ID_TAG_NAME, global_id_tag);
  ERR_CHECK(result);
  moab::Tag ths[2] = {category_tag,name_tag};
  moab::Range group_sets;
  result = mbi()->get_entities_by_type_and_tag(0, moab::MBENTITYSET, &ths[0], NULL, 2, group_sets);
  ERR_CHECK(result);

  moab::Range::iterator i;
  for (i = group_sets.begin(); i != group_sets.end(); ++i) {

    //get this group's children
    std::string group_name;
    int group_global_id;
    group_name.resize(NAME_TAG_SIZE);
    result = mbi()->tag_get_data(name_tag, &(*i), 1, (void *)group_name.c_str());
    ERR_CHECK(result);

    result = mbi()->tag_get_data(global_id_tag, &(*i), 1, (void *)&group_global_id);
    ERR_CHECK(result);
    
    moab::Range group_contents;
    result = mbi()->get_entities_by_type(*i, moab::MBENTITYSET, group_contents);
    ERR_CHECK(result);

    //add this group to the list of group names
    group_names.push_back(group_name);
    //add this id to the list of group ids
    group_ids.push_back(group_global_id);
   

    //add this set of children to the map
    group_map[group_name] = group_contents;
    group_contents.clear();
      
  }

  return moab::MB_SUCCESS;
}

moab::ErrorCode get_volume_paths(moab::Range volumes,
				 std::map<moab::EntityHandle, 
				 std::vector<Loop> > intersection_dict,
				 std::vector< std::vector<Loop> > &all_vol_paths) {
  moab::ErrorCode result; 
  
  moab::Range::iterator i; 
  for (i = volumes.begin(); i != volumes.end(); i++) {
    std::vector<Loop> this_vol_intersections;
    result = get_volume_intersections(*i, intersection_dict, this_vol_intersections);
    ERR_CHECK(result);

    if (0 == this_vol_intersections.size()) continue; 

    if (OPT_VERBOSE) std::cout << "Retrieved " << this_vol_intersections.size()
			       << " intersections for this volume." << std::endl;

    std::vector<Loop> vol_paths;
    stitch( this_vol_intersections, vol_paths);
    all_vol_paths.push_back(vol_paths);
  }

  return moab::MB_SUCCESS;
}

void stitch(std::vector<Loop> loops, std::vector<Loop> &paths) {

  unsigned int i = 0; 

  while (i < loops.size()) {
    //check for complete loops first 
    //start with arbitrary loop
    Loop this_intersection = loops[i];

    // if we find a complete loop, add it to the volume paths
    if (point_match(this_intersection.points.front(), this_intersection.points.back())
	&& this_intersection.points.size() > 2) {
      paths.push_back(this_intersection);
      loops.erase( loops.begin() + i );
      i = 0;
    } 
    // if this is a line of negligible length, remove it from loops
    else if (point_match(this_intersection.points.front(), this_intersection.points.back())
	     && 2 == this_intersection.points.size())	{
      loops.erase( loops.begin() + i );
      i = 0;
    }
    i += 1;
  }
  

  //if there are no point collections left, we're done
  if (0 == loops.size()) return;
  
  //now we'll start stitching loops together
  i = 0;

  //start by adding an arbitrary surface intersection to the paths
  paths.push_back(loops[i]);
  loops.erase(loops.begin()+i);


  while (0 != loops.size()) {
      
    //if we have a complete loop, then move on to a new starting point
    if (point_match(paths.back().points.front(), paths.back().points.back())) {
      paths.push_back(loops.front());
      loops.erase(loops.begin());
    }	   
    else {
      i = 0;
	  
      while ( i < loops.size() ) {

	Loop this_intersection = loops[i];

	if (point_match( paths.back().points.front(), this_intersection.points.front())) {
	  //reverse this_intersection and attach to the front of paths.back()
	  std::reverse( this_intersection.points.begin(), this_intersection.points.end());
	  paths.back().points.insert(paths.back().points.begin(), this_intersection.points.begin(), this_intersection.points.end());
	  loops.erase(loops.begin()+i);
	  i = 0;
	}
	else if (point_match( paths.back().points.front(), this_intersection.points.back())) {
	  // attach to the front of paths.back()
	  paths.back().points.insert(paths.back().points.begin(), this_intersection.points.begin(), this_intersection.points.end());
	  loops.erase(loops.begin()+i);
	  i = 0;		  
	}
	else if (point_match( paths.back().points.back(), this_intersection.points.front())) {
	  //attach to the back of paths.back()
	  paths.back().points.insert(paths.back().points.end(), this_intersection.points.begin(), this_intersection.points.end());
	  loops.erase(loops.begin()+i);
	  i = 0;		  
	}
	else if (point_match(paths.back().points.back(), this_intersection.points.back())) {
	  //reverse intersection and attach to the back of paths.back()
	  std::reverse(this_intersection.points.begin(), this_intersection.points.end());
	  paths.back().points.insert(paths.back().points.end(), this_intersection.points.begin(), this_intersection.points.end());
	  loops.erase(loops.begin()+i);
	  i = 0;
	} 

	//if no match is found, move on to the next intersection 
	i++;

      } //end inner while
 
    } // end if

  } // end outer while
  
  if (OPT_VERBOSE) std::cout << "Created " << paths.size() << "paths for this volume." << std::endl;
  
  return;
} // end stitch

moab::ErrorCode get_volume_intersections(moab::EntityHandle volume,
					 std::map<moab::EntityHandle, 
					 std::vector<Loop> > intersection_dict,
					 std::vector<Loop> &volume_intersections) {
  moab::ErrorCode result;
  std::vector<moab::EntityHandle> chld_surfaces;
  result = mbi()->get_child_meshsets(volume, chld_surfaces);
  ERR_CHECK(result);
  
  std::vector<moab::EntityHandle>::iterator i;
  for (i = chld_surfaces.begin(); i != chld_surfaces.end(); i++) {
      std::vector<Loop> this_set = intersection_dict[*i];
      volume_intersections.insert(volume_intersections.end(), this_set.begin(), this_set.end());
    }

  return moab::MB_SUCCESS;
}

moab::ErrorCode create_surface_intersections(moab::Range surfs,
					     int axis,
					     double coord,
					     std::map<moab::EntityHandle,
					     std::vector<Loop> > &intersection_map) {
  moab::ErrorCode result; 

  moab::Range::iterator i; 
  for (i = surfs.begin(); i != surfs.end(); i++) {
    //get the surface triangles
    std::vector<moab::EntityHandle> surf_tris; 
    result = mbi()->get_entities_by_type(*i, moab::MBTRI, surf_tris);
    ERR_CHECK(result); 
      
    //now create surface intersection
    std::vector<Loop> surf_intersections;
    result = surface_intersections(surf_tris, axis, coord, surf_intersections);
    ERR_CHECK(result);

    intersection_map[*i] = surf_intersections;
    surf_intersections.clear();

  }

  return result; 
}


moab::ErrorCode surface_intersections(std::vector<moab::EntityHandle> tris,
				      int axis,
				      double coord,
				      std::vector<Loop> &surf_intersections) {
  moab::ErrorCode result; 
  std::vector<Line> intersect_lines; 

  std::vector<moab::EntityHandle>::iterator i; 
  for ( i = tris.begin(); i != tris.end(); i++) { 
    Line line; bool intersect;
    result = intersection(axis, coord, *i, line, intersect);
    ERR_CHECK(result);
      
    if(intersect) intersect_lines.push_back(line);
      
  } 

  std::vector<Loop> all_surf_intersections;
  
  //now it is time to order these 
  unsigned int index = 0; //index for intersection lines
  //arbitrarily start a new intersection loop
  Loop curr_loop; 

  while (intersect_lines.size() != 0) {

    curr_loop.points.clear();
    curr_loop.points.push_back( intersect_lines.back().begin);
    curr_loop.points.push_back( intersect_lines.back().end);
    intersect_lines.pop_back();

    while (index < intersect_lines.size()) {

      Line this_line = intersect_lines[index];

      if (point_match(this_line.begin, curr_loop.points.front())) {
	//insert the line into the current loop
	curr_loop.points.insert(curr_loop.points.begin(), this_line.end );
	//delete the current matched line from intersect_lines  
	intersect_lines.erase(intersect_lines.begin()+index);
	index = 0;
      }
      else if ( point_match( this_line.begin, curr_loop.points.back() ) ) {
	curr_loop.points.push_back( this_line.end );
	intersect_lines.erase(intersect_lines.begin()+index);
	index = 0;
      }
      else if ( point_match( this_line.end, curr_loop.points.front() ) ) {
	curr_loop.points.insert(curr_loop.points.begin(), this_line.begin );
	intersect_lines.erase(intersect_lines.begin()+index);
	index = 0;
      }
      else if ( point_match( this_line.end, curr_loop.points.back() ) ) {
	curr_loop.points.push_back( this_line.begin );
	intersect_lines.erase(intersect_lines.begin()+index);
	index = 0;
      }
      else {
	index++;
      }
    }
    all_surf_intersections.push_back(curr_loop);
  }

  //set return var
  surf_intersections = all_surf_intersections;

  return result; 
}

moab::ErrorCode intersection(int axis,
			     double coord,
			     moab::EntityHandle tri,
			     Line &tri_intersection,
			     bool &intersect) {
  moab::ErrorCode result;
  //get the triangle vertices
  std::vector<moab::EntityHandle> verts;
  
  result = mbi()->get_adjacencies(&tri, 1, 0, false, verts);
  ERR_CHECK(result);
  
  moab::CartVect tri_coords[3];
  result = mbi()->get_coords(&(verts[0]), 1, tri_coords[0].array());
  ERR_CHECK(result);
  result = mbi()->get_coords(&(verts[1]), 1, tri_coords[1].array());
  ERR_CHECK(result);
  result = mbi()->get_coords(&(verts[2]), 1, tri_coords[2].array());
  ERR_CHECK(result);

  triangle_plane_intersect(axis, coord, tri_coords, tri_intersection);

  intersect = tri_intersection.full;

  return result; 
}



void triangle_plane_intersect(int axis,
			      double coord,
			      moab::CartVect *coords,
			      Line &line_out) {
  //check to see how many triangle edges cross the coordinate
  moab::CartVect p0,p1,p2,p3;

  //expecting three points for the triangle
  for (unsigned int i = 0 ; i < 3; i++) {
    p3[i] = copysign(1.0, coords[i][axis] - coord);
  }

  if (p3[0] * p3[1] < 0) {
    p0 = coords[0];
    p1 = coords[1];
    get_intersection(p0, p1, axis, coord, line_out);
  } 
  if (p3[1] * p3[2] < 0) {
    p0 = coords[1];
    p1 = coords[2];
    get_intersection(p0, p1, axis, coord, line_out);
  }
  if (p3[2] * p3[0] < 0) {
    p0 = coords[2];
    p1 = coords[0];
    get_intersection(p0, p1, axis, coord, line_out);
  }
  
}

void get_intersection(moab::CartVect pnt0,
		      moab::CartVect pnt1,
		      int axis,
		      double coord,
		      Line &line) {
  moab::CartVect vec = pnt1-pnt0;
  double t = (coord - pnt0[axis])/vec[axis];
  moab::CartVect pnt_out = pnt0 + t*vec;
  line.add_pnt(pnt_out);
}

void convert_to_stl(std::vector< std::vector<Loop> > a,
		    std::vector< std::vector< std::vector< std::vector<double> > > > &b) {
  b.resize(a.size());
  for (unsigned int i = 0; i < a.size(); i++) {
    b[i].resize(a[i].size());
    for(unsigned int j = 0; j < a[i].size(); j++) {
      b[i][j].resize(a[i][j].points.size());
      for(unsigned int k = 0; k < a[i][j].points.size(); k++) {
	b[i][j][k].push_back(a[i][j].points[k][0]);
	b[i][j][k].push_back(a[i][j].points[k][1]);
	b[i][j][k].push_back(a[i][j].points[k][2]);
      }
    }
  }	      	      
} //end func

/********************************************
Section for handling loop windings and path coding as required by matplotlib
*********************************************/
void create_patch(int axis,
		  std::vector<Loop> input_loops,
		  std::vector<xypnt> &path_out,
		  std::vector<int> &coding_out) {

  std::vector<Loop>::iterator loop; 
  for (loop = input_loops.begin(); loop != input_loops.end(); loop++) {
      (*loop).gen_xys(axis);
    }

  //generate containment matrix for the loops
  std::vector< std::vector<int> > M;
  get_containment(input_loops, M);
  
  //get the current windings of the loops
  std::vector<int> windings;
  get_windings(input_loops, windings);

  //find desired windings from containment matrix, M
  std::vector<int> desired_windings;
  get_fill_windings(M, desired_windings);
  
  //re-orient the loops
  set_windings(windings, desired_windings, input_loops);

  //create coding and paths
  generate_patch_path(input_loops, path_out, coding_out);
  
}


void get_containment(std::vector<Loop> loops,
		     std::vector< std::vector<int> > &Mat) {
  Mat.resize(loops.size());
  for (unsigned int i = 0; i < loops.size(); i++) {
    Mat[i].resize(loops.size());
    for (unsigned int j = 0; j < loops.size(); j++) {
      if ( i == j)
	Mat[i][j] = true; //polygons contain themselves
      else
	Mat[i][j] = is_poly_a_in_poly_b( loops[i], loops[j]);
    }
  }
}

// checks to see if loop a is in loop b
// (assuming fully nested loops, no overlaps)
// does this by ray casting to the right from the test point for each polygon edge
// and checking the number of edge-intersections with the polygon
bool is_poly_a_in_poly_b(Loop a, Loop b) {
  //use the first point of a to test for now
  double x = a.xypnts[0].x; double y = a.xypnts[0].y;
  unsigned int i, j = b.xypnts.size() - 1;
  bool result = false;

  for(i = 0; i < b.xypnts.size(); i++) {
    if( ((b.xypnts[i].y < y && b.xypnts[j].y >= y)
	 || (b.xypnts[j].y < y && b.xypnts[i].y >= y))
	&& (b.xypnts[i].x <= x || b.xypnts[j].x <= x)) {
      if (b.xypnts[i].x+(y-b.xypnts[i].y)/(b.xypnts[j].y-b.xypnts[i].y)
	  *(b.xypnts[j].x-b.xypnts[i].x) < x) {
	result = !result;
      }
    }
    j=i;
  }
  
  return result;
}

void get_windings(std::vector<Loop> loops, std::vector<int> &windings)
{
  std::vector<Loop>::iterator loop;
  windings.clear(); //JIC
  
  for (loop = loops.begin(); loop != loops.end(); loop++) {
      windings.push_back(find_winding(*loop));
    }
}

int find_winding(Loop loop)
{
  double area = 0; 
  int j = loop.xypnts.size() - 1; 

  //calculate area
  for (unsigned int i = 0; i < loop.xypnts.size(); i++) {
    area += (loop.xypnts[j].x + loop.xypnts[i].x) * (loop.xypnts[j].y - loop.xypnts[i].y);
    j=i;
  }
  return area >= 0 ? CW : CCW;
}

void get_fill_windings(std::vector< std::vector<int> > fill_mat,
		       std::vector<int> &windings) {
  unsigned int a = fill_mat.size();
  unsigned int b = fill_mat[0].size();
  
  if( a != b) {
    ERR_CHECK(moab::MB_FAILURE);
      }
  
  int wind;
  for (unsigned int i = 0; i < a; i++) {
    int dum = 0;
    for (std::vector<int>::iterator j = fill_mat[i].begin();
	 j != fill_mat[i].end(); j++) {
      dum += *j;
    }
    wind = dum%2 == 0 ? CW : CCW;
    windings.push_back(wind);
  }
}

void set_windings(std::vector<int> current_windings,
		  std::vector<int> desired_windings,
		  std::vector<Loop> &loops) {
  assert(current_windings.size() == desired_windings.size());

  for (unsigned int i = 0; i < current_windings.size(); i++) {
    if (current_windings[i] != desired_windings[i]) {
      std::reverse( loops[i].points.begin(), loops[i].points.end()); //JIC
      std::reverse( loops[i].xypnts.begin(), loops[i].xypnts.end());
    }
  }
} 

void generate_patch_path(std::vector<Loop> loops,
			 std::vector<xypnt> &path,
			 std::vector<int> &coding) {
  std::vector<Loop>::iterator loop;
  for (loop = loops.begin(); loop != loops.end(); loop++) {
      for(unsigned int i = 0; i < (*loop).xypnts.size(); i++) {
	  path.push_back((*loop).xypnts[i]);
	  i == 0 ? coding.push_back(1) : coding.push_back(2);
	}
    }
}

void rename_group_out(int group_global_id, std::string new_name) {

  moab::ErrorCode result;

  //get the category tag
  moab::Tag category_tag;
  result = mbi()->tag_get_handle(CATEGORY_TAG_NAME, category_tag);
  ERR_CHECK(result);
  //get the global id tag 
  moab::Tag global_id_tag;
  result = mbi()->tag_get_handle(GLOBAL_ID_TAG_NAME, global_id_tag);
  ERR_CHECK(result);
  std::vector<moab::Tag> tags;
  tags.push_back(category_tag);
  tags.push_back(global_id_tag);
  moab::Range entsets;
  char grp[CATEGORY_TAG_SIZE] = "Group";
  void *grp_ptr = &grp;
  void *id_ptr = &group_global_id;
  void *vals[2] = {grp_ptr,id_ptr};
  result = mbi()->get_entities_by_type_and_tag(0, moab::MBENTITYSET, &tags[0], &(vals[0]), 2, entsets, moab::Interface::INTERSECT, true);
  ERR_CHECK(result);

  if(1 != entsets.size() ) { 
    std::cout << "Invalid group id." << std::endl;
    ERR_CHECK(moab::MB_FAILURE);
  }
  moab::EntityHandle group_to_mod = entsets[0];
  //get the name tag, because this global id should indicate a group with this tag
  moab::Tag name_tag; 
  result = mbi()->tag_get_handle(NAME_TAG_NAME,name_tag);
  ERR_CHECK(result);
  //give warning about truncated name
  if(NAME_TAG_SIZE < new_name.size()) {
    std::cout << "Warning: size of name exceed standard group name size. It will be trucnated." << std::endl;
      }
  //now set the new name
  new_name.resize(NAME_TAG_SIZE);
  result = mbi()->tag_set_data(name_tag, &group_to_mod, 1, (void*)new_name.c_str());
  ERR_CHECK(result);

  return;
}

void write_file_out(std::string new_filename) {

  moab::ErrorCode result;

  if (!is_new_filename(new_filename)) {
    std::cout << "Warning! This filename is identical to the current filename. Originaly file will be over-written." << std::endl;
      }

  result = mbi()->write_file(new_filename.c_str());
  ERR_CHECK(result);
}

bool is_new_filename(std::string name) {

  moab::ErrorCode result;
  //get the filename_tag
  moab::Tag filename_tag;
  result = mbi()->tag_get_handle( FILENAME_TAG_NAME, 50, moab::MB_TYPE_OPAQUE, filename_tag, moab::MB_TAG_CREAT|moab::MB_TAG_SPARSE);
  ERR_CHECK(result);


  //check the root_set for this tag
  moab::EntityHandle root_set = mbi()->get_root_set();
  std::vector<moab::Tag> root_set_tags;
  result = mbi()->tag_get_tags_on_entity(root_set, root_set_tags);
  ERR_CHECK(result);


  std::vector<moab::Tag>::iterator val  = std::find(root_set_tags.begin(),root_set_tags.end(),filename_tag);
  if(val != root_set_tags.end()) { 
    //check that filename is the same
    std::string current_filename;
    current_filename.resize(50);
    result = mbi()->tag_get_data(filename_tag, &root_set, 1, (void*)current_filename.c_str());
    ERR_CHECK(result);
    current_filename.resize(name.size());
    if(current_filename == name) return false;
  }

  return true;
}
