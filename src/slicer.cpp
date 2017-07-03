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
  MB_CHK_SET_ERR(result, "Failed to get category tag handle"); 

  //create void pointer for tag data match
  const void *dum = &(category[0]);

  // retrieve sets with the category value requested
  result = mbi()->get_entities_by_type_and_tag(0, moab::MBENTITYSET, &category_tag, &dum, 1, entsets);
  MB_CHK_SET_ERR(result, "Failed to get entities tagged with the category value");
  
  return result;

}

moab::ErrorCode get_all_surfaces(moab::Range &surfs) {

  moab::ErrorCode result = moab::MB_SUCCESS;

  char category[CATEGORY_TAG_SIZE] = "Surface";
  
  result = get_sets_by_category(surfs, category);
  MB_CHK_SET_ERR(result, "Failed to get surfaces using the category tag");
  
  if (OPT_VERBOSE) std::cout << "There are " << surfs.size() << " surfaces in this model." << std::endl;

  return result;

}

moab::ErrorCode get_all_volumes(moab::Range &vols) {

  moab::ErrorCode result = moab::MB_SUCCESS;

  char category[CATEGORY_TAG_SIZE] = "Volume";
  
  result = get_sets_by_category(vols, category);
  MB_CHK_SET_ERR(result, "Failed to get volumes using the category tag");
  
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
					bool verbose,
					bool debug,
					bool ca) {
  // set options for output
  opts.verbose = verbose;
  opts.debug = debug;
  // datastructure for path data
  std::vector< std::vector<xypnt> > paths;
  moab::ErrorCode result = slice_faceted_model(filename, axis, coord, paths, codings, group_names, group_ids, by_group, ca);

  // seperate the x and y values of the points
  std::vector< std::vector<xypnt> >::iterator path;
  for (path = paths.begin(); path != paths.end(); path++) {
    std::vector<double> path_xs, path_ys;
    for (unsigned int i = 0; i < (*path).size(); i++)	{
      // set the values for this path
      path_xs.push_back((*path)[i].x);
      path_ys.push_back((*path)[i].y);
    }
    // push paths back onto data structure
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
				    bool by_group,
				    bool ca) {
  
  moab::ErrorCode result;
  // data structure for mapping surfaces to their slice loop
  std::map<moab::EntityHandle,std::vector<Loop> > intersection_map;
  moab::Tag aabb_tag;

  // if this is a different filename than the one that was previously loaded,
  // clean out the MOAB instance, AABB data, and load the new file
  if(is_new_filename(filename))
    {
      //remove all old mesh content
      result = mbi()->delete_mesh();
      MB_CHK_SET_ERR(result, "Failed to delete mesh");

      // get the AABB tag handle
      result = mbi()->tag_get_handle("AABB", 6, moab::MB_TYPE_DOUBLE, aabb_tag, moab::MB_TAG_DENSE);
      // delete the tag (and all of its data)
      if (result == moab::MB_SUCCESS) {
	result = mbi()->tag_delete(aabb_tag);
	MB_CHK_SET_ERR(result, "Failed to delete the AABB tag");
      }
      
      //load the new file
      std::cout << "Loading new file..." << std::endl;
      result = mbi()->load_file(filename.c_str());
      if (moab::MB_FILE_DOES_NOT_EXIST == result) {
	// some extra output if the file cannot be found
	std::cout << "Could not open specified file." << std::endl;
	return result;
      }
      else {
	MB_CHK_SET_ERR(result, "Failed to load file");
      }

      // get the filename_tag
      moab::Tag filename_tag;
      result = mbi()->tag_get_handle(FILENAME_TAG_NAME, 50, moab::MB_TYPE_OPAQUE, filename_tag, moab::MB_TAG_CREAT|moab::MB_TAG_SPARSE);
      MB_CHK_SET_ERR(result, "Failed to get the filename tag handle");

      // tag the root set with the filename
      moab::EntityHandle rs = mbi()->get_root_set();
      result = mbi()->tag_set_data(filename_tag,&rs,1,(void*)filename.c_str());
      MB_CHK_SET_ERR(result, "Failed to set the filename tag data");
    }

  // retrieve all surfaces from the file
  moab::Range surfaces;
  result = get_all_surfaces(surfaces);
  MB_CHK_SET_ERR(result, "Failed to get all surfaces");

  
  // check for the AABB tag in the instance
  result = mbi()->tag_get_handle("AABB", 6, moab::MB_TYPE_DOUBLE, aabb_tag, moab::MB_TAG_DENSE);
  // if it is found, filter out surfaces whose AABBs don't intersect with the slice plane
  if(result == moab::MB_SUCCESS) {
    std::cout << "Filtering surfaces." << std::endl;
    result = filter_surfaces(surfaces, aabb_tag, axis, coord);
    std::cout << "Got here" << std::endl;
    MB_CHK_SET_ERR(result, "Failed to filter surfaces using the axis aligned bounding coordinates");
  }

  // now create intersections for each surface and add it to the intersection map
  result = create_surface_intersections(surfaces, axis, coord, intersection_map);
  MB_CHK_SET_ERR(result, "Failed to create surface intersections");

  //path container for a volume
  std::vector< std::vector<Loop> > all_paths;

  ////////////////////
  // SLICE BY GROUP //
  ////////////////////
  if (by_group) {
    // a mapping of groupname to group volumes
    std::map< std::string, moab::Range > group_mapping;
    result = get_volumes_by_group(group_mapping, group_names, group_ids);
    MB_CHK_SET_ERR(result, "Failed to get the volumes by group");
    
    if (OPT_VERBOSE) std::cout << "Size of group map: " << group_mapping.size() << std::endl;
    if (OPT_VERBOSE) std::cout << "Size of group names: " << group_names.size() << std::endl;
    std::vector<std::string>::iterator group_name;

    // for each group, generate the path loops for each set of volumes in the group
    for (group_name = group_names.begin(); group_name != group_names.end();) {

      // generate the loops of line segments that compose intersection paths with this group's volumes
      std::vector< std::vector<Loop> > all_group_paths;
      result = get_volume_paths(group_mapping[*group_name], intersection_map, all_group_paths, ca);
      MB_CHK_SET_ERR(result, "Failed to get the volume paths for a group");

      // if there are no paths generated for this group's volumes,
      // remove the group from the list of group names and ids
      if (0 == all_group_paths.size()) {
	if (OPT_VERBOSE) std::cout << "Erasing group: " << *group_name << std::endl;
	int val = group_name-group_names.begin();
	std::vector<int>::iterator group_id = group_ids.begin()+val;
	group_ids.erase(group_id);
	group_names.erase(group_name); //erase and set iterator to next item
	continue;
      }

      
      if (OPT_DEBUG) std::cout << "Getting slice for group:" << *group_name << std::endl;

      // concatenate the paths for this group into a single, compound  path
      // and generate coding to inticate where closed paths begin and end
      std::vector<xypnt> group_path;
      std::vector<int> group_coding;
      std::vector< std::vector<Loop> >::iterator path;
      for (path = all_group_paths.begin(); path != all_group_paths.end(); path++) {
	
	std::vector<xypnt> vol_path;
	std::vector<int> vol_coding;
	// create the matplotlib path coding for 
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
  /////////////////////
  // SLICE BY VOLUME //
  /////////////////////
  else {

    // get all volumes from the instance
    moab::Range volumes;
    result = get_all_volumes(volumes);
    MB_CHK_SET_ERR(result, "Failed to get all volumes");

    // get the paths for each volume
    result = get_volume_paths(volumes, intersection_map, all_paths, ca);
    MB_CHK_SET_ERR(result, "Failed to get all volume paths");

    //generate coding for each path
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

  //clear out old data if present
  group_map.clear();
  group_names.clear();
  group_ids.clear();
  
  //retrieve the tags needed for this operation
  moab::Tag category_tag, name_tag, global_id_tag;
  result = mbi()->tag_get_handle(CATEGORY_TAG_NAME, category_tag);
  MB_CHK_SET_ERR(result, "Failed to get the category tag handle");
  result = mbi()->tag_get_handle(NAME_TAG_NAME, name_tag);
  MB_CHK_SET_ERR(result, "Failed to get the name tag handle");
  result = mbi()->tag_get_handle(GLOBAL_ID_TAG_NAME, global_id_tag);
  MB_CHK_SET_ERR(result, "Failed to get the global id tag handle");
  
  //get all groups in the model (defined by having a name tag and category tag)  
  moab::Tag ths[2] = {category_tag,name_tag};
  moab::Range group_sets;
  result = mbi()->get_entities_by_type_and_tag(0, moab::MBENTITYSET, &ths[0], NULL, 2, group_sets);
  MB_CHK_SET_ERR(result, "Failed to get all entities with a category tag and name tag (groups)");

  // for each group set in the instance, get its name and global ID
  moab::Range::iterator i;
  for (i = group_sets.begin(); i != group_sets.end(); ++i) {

    // get the group's name
    std::string group_name;
    group_name.resize(NAME_TAG_SIZE);
    result = mbi()->tag_get_data(name_tag, &(*i), 1, (void *)group_name.c_str());
    MB_CHK_SET_ERR(result, "Failed to get the name tag data for a group");

    // get the group's global id
    int group_global_id;
    result = mbi()->tag_get_data(global_id_tag, &(*i), 1, (void *)&group_global_id);
    MB_CHK_SET_ERR(result, "Failed to get the global id of a group");
    //add this group name to the vector of group names
    group_names.push_back(group_name);

    // get the contents of the group (should be volumes)
    moab::Range group_contents;
    result = mbi()->get_entities_by_type(*i, moab::MBENTITYSET, group_contents);
    MB_CHK_SET_ERR(result, "Failed to get the contained entity sets of a group");
    //add this id to the list of group ids
    group_ids.push_back(group_global_id);
   
    // add this group to the mapping
    group_map[group_name] = group_contents;
    group_contents.clear();
      
  }

  // this function should return the same number of names and ids
  if (group_names.size() != group_ids.size()) {
    std::cout << "Ineqaul number of group names and ids found." << std::endl;
    return moab::MB_FAILURE;
  }
  
  return moab::MB_SUCCESS;
}

moab::ErrorCode get_volume_paths(moab::Range volumes,
				 std::map<moab::EntityHandle, 
				 std::vector<Loop> > intersection_dict,
				 std::vector< std::vector<Loop> > &all_vol_paths,
				 bool roam) {
  moab::ErrorCode result;
  // for each volume, create the paths that make up its intersection
  moab::Range::iterator i; 
  for (i = volumes.begin(); i != volumes.end(); i++) {
    // retrieve this volume's surface intersections from the intersection dict
    std::vector<Loop> this_vol_intersections;
    result = get_volume_intersections(*i, intersection_dict, this_vol_intersections);
    MB_CHK_SET_ERR(result, "Failed to create volume intersections");

    // if no intersections were found, move on to the next volume
    if (0 == this_vol_intersections.size()) continue; 

    if (OPT_VERBOSE) std::cout << "Retrieved " << this_vol_intersections.size()
			       << " intersections for this volume." << std::endl;
    
    // stitch the surface intersections of this volume into closed paths
    std::vector<Loop> vol_paths;
    stitch(this_vol_intersections, vol_paths, roam);
    // add volume's path(s) to the total set
    all_vol_paths.push_back(vol_paths);
  }
  
  return moab::MB_SUCCESS;
}

void stitch(std::vector<Loop> loops, std::vector<Loop> &paths, bool roam) {

  // stitch the loops into closed paths
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
  

  //if there are no point collections left,
  // then they all came in as closed loops and we're done
  if (0 == loops.size()) return;
  
  //now we'll start stitching unclosed loops together
  i = 0;

  // start by adding an arbitrary surface intersection to the paths
  // as seeds 
  paths.push_back(loops[i]);
  loops.erase(loops.begin()+i);

  // tolerance for considering points to be coincident
  double pt_match_tol = MATCH_TOL; 
  // used to track the number of loops that have been placed in a path  
  int prev_loops_size = (int) loops.size();
// indicator as to whether or not this is the first search  
  bool first_search = true; 

  // continue until all loops have been placed
  while (0 != loops.size()) {

    // if no match for this unclosed loop was found in the first pass,
    // and roaming is enabled, then increase the point matching tolerance
    // slightly
    if ( prev_loops_size == (int) loops.size() && !first_search && roam) {
      pt_match_tol*=1.05;
    }
    // if not roaming, or this is the first pass through the loops
    // set the point matching tolerance to the original value
    else {
      pt_match_tol = MATCH_TOL;
    }

    // at this point, the search will begin
    first_search = false;
    
    // if we have a complete loop, then grab a new seed loop at random
    if (point_match(paths.back().points.front(), paths.back().points.back())) {
      paths.push_back(loops.front());
      loops.erase(loops.begin());
      // in this case the point matching tolerance should be rese
      pt_match_tol = MATCH_TOL; 
    }
    // if the loop is unclosed, then start the search for a matching loop
    else {
      
      i = 0;
      prev_loops_size = (int) loops.size(); // mark how many unmatched loops exist before the search
      while ( i < loops.size() ) {
	// use the next intersection in the list of loops
	Loop this_intersection = loops[i];

	// checkfor a point match - front of current path to front of this loop
	if (point_match( paths.back().points.front(), this_intersection.points.front(),pt_match_tol)) {
	  //reverse this_intersection and attach to the front of paths.back()
	  std::reverse( this_intersection.points.begin(), this_intersection.points.end());
	  paths.back().points.insert(paths.back().points.begin(), this_intersection.points.begin(), this_intersection.points.end());
	  loops.erase(loops.begin()+i);
	  i = 0;
	}
	// check for a point match - front of current path to the back of this loop
	else if (point_match( paths.back().points.front(), this_intersection.points.back(), pt_match_tol)) {
	  // attach to the front of paths.back()
	  paths.back().points.insert(paths.back().points.begin(), this_intersection.points.begin(), this_intersection.points.end());
	  loops.erase(loops.begin()+i);
	  i = 0;		  
	}
	// check for a point match - back of current path to the front of this loop
	else if (point_match( paths.back().points.back(), this_intersection.points.front(), pt_match_tol)) {
	  //attach to the back of paths.back()
	  paths.back().points.insert(paths.back().points.end(), this_intersection.points.begin(), this_intersection.points.end());
	  loops.erase(loops.begin()+i);
	  i = 0;		  
	}
	// check for a point match - back of current path to back of this loop
	else if (point_match(paths.back().points.back(), this_intersection.points.back(), pt_match_tol)) {
	  //reverse intersection and attach to the back of paths.back()
	  std::reverse(this_intersection.points.begin(), this_intersection.points.end());
	  paths.back().points.insert(paths.back().points.end(), this_intersection.points.begin(), this_intersection.points.end());
	  loops.erase(loops.begin()+i);
	  i = 0;
	}
	// if no match is found, move on to the next intersection 
	i++;
      } //end inner while
      
    } // end if-else for complete loop check

  } // end outer while
  
  if (OPT_VERBOSE) std::cout << "Created " << paths.size() << "paths for this volume." << std::endl;
  return;
  
} // end stitch

moab::ErrorCode get_volume_intersections(moab::EntityHandle volume,
					 std::map<moab::EntityHandle, 
					 std::vector<Loop> > intersection_dict,
					 std::vector<Loop> &volume_intersections) {
  moab::ErrorCode result;
  // get the children of this volume
  std::vector<moab::EntityHandle> chld_surfaces;
  result = mbi()->get_child_meshsets(volume, chld_surfaces);
  MB_CHK_SET_ERR(result, "Failed to get the child meshsets (surfaces) of a volume");

  // for each child surface, insert the set of intersections from the intersection dict
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

  // check for the AABB tag
  bool bound_surfs;
  moab::Tag aabb_tag;
  result = mbi()->tag_get_handle("AABB", 6, moab::MB_TYPE_DOUBLE, aabb_tag, moab::MB_TAG_DENSE);
  // if present, there is no need to setup AABBs for the surfaces, they should already be there
  if(moab::MB_SUCCESS == result) {
    bound_surfs = false;
  }
  // if the tag is not found, then create the tag, and indicate that AABBs should be built for each surface
  else if (moab::MB_TAG_NOT_FOUND == result) {
    result = mbi()->tag_get_handle("AABB", 6, moab::MB_TYPE_DOUBLE, aabb_tag, moab::MB_TAG_CREAT|moab::MB_TAG_DENSE);
    MB_CHK_SET_ERR(result, "Failed to create the AABB tag handle");
    bound_surfs = true;
  }
  // if some error other than MB_TAG_NOT_FOUND is returned, then return a failure
  else {
    MB_CHK_SET_ERR(moab::MB_FAILURE, "Failed to get the AABB tag handle");
  }

  // for evvery surface, create an intersection
  moab::Range::iterator i; 
  for (i = surfs.begin(); i != surfs.end(); i++) {
    
    //get the surface triangles
    std::vector<moab::EntityHandle> surf_tris; 
    result = mbi()->get_entities_by_type(*i, moab::MBTRI, surf_tris);
    MB_CHK_SET_ERR(result, "Failed to get the triangles of a surface"); 
      
    //now create surface intersection
    std::vector<Loop> surf_intersections;
    result = surface_intersections(*i, aabb_tag, surf_tris, axis, coord, surf_intersections, bound_surfs);
    MB_CHK_SET_ERR(result, "Failed to generate a surface's intersections");

    // add this surface's intersection to the intersection mapping
    intersection_map[*i] = surf_intersections;
    surf_intersections.clear();
  }

  return result; 
}


moab::ErrorCode surface_intersections(moab::EntityHandle surf,
				      moab::Tag aabb_tag,
				      std::vector<moab::EntityHandle> tris,
				      int axis,
				      double coord,
				      std::vector<Loop> &surf_intersections,
				      bool bound) {
  moab::ErrorCode result; 
  std::vector<Line> intersect_lines;
  // initalize max/min values for the AABB of this surface
  double myarr[6] = {1.e38,-1.e38,1.e38,-1.e38,1.e38,-1.e38};
  std::vector<double> bounds(myarr, myarr+6);

  // loop over triangles, detecting intersections
  std::vector<moab::EntityHandle>::iterator i; 
  for ( i = tris.begin(); i != tris.end(); i++) {

    // check triangle for intersection with slice plane
    // also update the axis-aligned bounds of the surface
    Line line; bool intersect;
    result = intersection(axis, coord, *i, line, intersect, bounds, bound);
    MB_CHK_SET_ERR(result, "Failed to intersect the slicing plane with a triangle");

    // if an intersection was found, add it to the lintersection lines
    if(intersect) intersect_lines.push_back(line);
      
  } 

  //tag surface handle with bounds if requested
  if (bound) {
    result = mbi()->tag_set_data(aabb_tag, &surf, 1, (void *)(&bounds[0]));
    MB_CHK_SET_ERR(result, "Failed to set the AABB tag data on a surface");
  }
  
  // now the line segments should be ordered
  unsigned int index = 0; //index for intersection lines
  //arbitrarily start a new, empty intersection loop
  Loop curr_loop;

  // loop over all intersections until they are placed in a loop
  std::vector<Loop> all_surf_intersections;
  while (intersect_lines.size() != 0) {

    // initialize the current loop with a pair of intersection points
    curr_loop.points.clear();
    curr_loop.points.push_back( intersect_lines.back().begin);
    curr_loop.points.push_back( intersect_lines.back().end);
    // remove this intersection from the list of intersections
    intersect_lines.pop_back();

    // now loop over all remaining intersections
    while (index < intersect_lines.size()) {

      // get the next intersection
      Line this_line = intersect_lines[index];

      // check for a match - front of this triangle intersection with the current loop's front
      if (point_match(this_line.begin, curr_loop.points.front())) {
	//insert the line into the current loop
	curr_loop.points.insert(curr_loop.points.begin(), this_line.end );
	//delete the current matched line from intersect_lines  
	intersect_lines.erase(intersect_lines.begin()+index);
	index = 0;
      }
      // check for a match - front of this triangle intersection with the current loop's back
      else if ( point_match( this_line.begin, curr_loop.points.back() ) ) {
	curr_loop.points.push_back( this_line.end );
	intersect_lines.erase(intersect_lines.begin()+index);
	index = 0;
      }
      // check for a match - back of this triangle intersection with the current loop's front
      else if ( point_match( this_line.end, curr_loop.points.front() ) ) {
	curr_loop.points.insert(curr_loop.points.begin(), this_line.begin );
	intersect_lines.erase(intersect_lines.begin()+index);
	index = 0;
      }
      // check for a match - back of this triangle intersection with the current loop's back      
      else if ( point_match( this_line.end, curr_loop.points.back() ) ) {
	curr_loop.points.push_back( this_line.begin );
	intersect_lines.erase(intersect_lines.begin()+index);
	index = 0;
      }
      // if no match was found, move on to the next triangle intersection
      else {
	index++;
      }
    }
    all_surf_intersections.push_back(curr_loop);
  }

  // set the return variable
  surf_intersections = all_surf_intersections;

  return result; 
}


moab::ErrorCode intersection(int axis,
			     double coord,
			     moab::EntityHandle tri,
			     Line &tri_intersection,
			     bool &intersect){
  std::vector<double> dum_bounds(6);
  moab::ErrorCode result = intersection(axis,coord,tri,tri_intersection,intersect,dum_bounds,false);
  MB_CHK_SET_ERR(result, "Failed to intersect slice plane with triangle");
  return result;
					
}

moab::ErrorCode intersection(int axis,
			     double coord,
			     moab::EntityHandle tri,
			     Line &tri_intersection,
			     bool &intersect,
			     std::vector<double> &bounds,
			     bool bound) {
  moab::ErrorCode result;
  //get the triangle vertices
  std::vector<moab::EntityHandle> verts;  
  result = mbi()->get_adjacencies(&tri, 1, 0, false, verts);
  MB_CHK_SET_ERR(result, "Failed to get the adjacencies of a triangle");

  // get the triangle coordinates
  moab::CartVect tri_coords[3];
  result = mbi()->get_coords(&(verts[0]), 1, tri_coords[0].array());
  MB_CHK_SET_ERR(result, "Failed to get coordinates of triangle vertex");
  result = mbi()->get_coords(&(verts[1]), 1, tri_coords[1].array());
  MB_CHK_SET_ERR(result, "Failed to get coordinates of triangle vertex");
  result = mbi()->get_coords(&(verts[2]), 1, tri_coords[2].array());
  MB_CHK_SET_ERR(result, "Failed to get coordinates of triangle vertex");

  // check the triangle for an intersection with the slice plane
  triangle_plane_intersect(axis, coord, tri_coords, tri_intersection);

  // check that an intersection was found
  intersect = tri_intersection.full;

  // if updated bounds are requested,
  // update the bounds using this triangle's vertex coordinates
  if (bound) {
    for(unsigned int i = 0; i < 3; i++) {
      for(unsigned int j = 0; j < 3; j++) {
	if (tri_coords[i][j] < bounds[2*j]) bounds[2*j] = tri_coords[i][j];
	if (tri_coords[i][j] > bounds[(2*j)+1]) bounds[(2*j)+1] = tri_coords[i][j];
      }
    }
  }
  
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

  // generate the explicit x,y values for each point in the loop
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
  // create a square matrix size NxN, where N is the number of loops
  Mat.resize(loops.size());
  // set each entry (diagonals are always true)
  for (unsigned int i = 0; i < loops.size(); i++) {
    Mat[i].resize(loops.size());
    for (unsigned int j = 0; j < loops.size(); j++) {
      if ( i == j)
	Mat[i][j] = true; //polygons contain themselves
      else
	// set entry based on whether or not loop a is in loop b or not
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
  // get the winding for each loop
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
  // if the area is zero or positive, then the winding is clockwise, otherwise it is counter-clockwise
  return area >= 0 ? CW : CCW;
}

void get_fill_windings(std::vector< std::vector<int> > fill_mat,
		       std::vector<int> &windings) {
  unsigned int a = fill_mat.size();
  unsigned int b = fill_mat[0].size();

  // verify that fill_mat is a squre data structure
  if( a != b ) { MB_CHK_SET_ERR_RET(moab::MB_FAILURE, "Fill matrix is not square"); }

  // for each entry in the matrix, determine the desired winding based on
  // the sum of the current row
  int wind;
  for (unsigned int i = 0; i < a; i++) {
    int dum = 0;
    // sum along this row
    for (std::vector<int>::iterator j = fill_mat[i].begin();
	 j != fill_mat[i].end(); j++) {
      dum += *j;
    }
    // if sum of containment values for this row is even, the
    // desired winding is clockwise
    // if odd, counter-clockwise
    wind = dum%2 == 0 ? CW : CCW;
    // add this winding value to the set of desired windings
    windings.push_back(wind);
  }
}

void set_windings(std::vector<int> current_windings,
		  std::vector<int> desired_windings,
		  std::vector<Loop> &loops) {
  
  assert(current_windings.size() == desired_windings.size());

  // if the winding of a loop is incorrect, then reverse it
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
  // coding values
  int MOVE_TO = 1;
  int LINE_TO = 2;
  // loop over vectors, at the beginning of each new loop
  // place a "MOVE_TO" code value, otherwise place a "LINE_TO" value
  std::vector<Loop>::iterator loop;
  for (loop = loops.begin(); loop != loops.end(); loop++) {
      for(unsigned int i = 0; i < (*loop).xypnts.size(); i++) {
	  path.push_back((*loop).xypnts[i]);
	  i == 0 ? coding.push_back(MOVE_TO) : coding.push_back(LINE_TO);
	}
    }
}

void rename_group_out(int group_global_id, std::string new_name) {

  moab::ErrorCode result;

  //get the category tag
  moab::Tag category_tag;
  result = mbi()->tag_get_handle(CATEGORY_TAG_NAME, category_tag);
  MB_CHK_SET_ERR_RET(result, "Failed to get the category tag handle");
  //get the global id tag 
  moab::Tag global_id_tag;
  result = mbi()->tag_get_handle(GLOBAL_ID_TAG_NAME, global_id_tag);
  MB_CHK_SET_ERR_RET(result, "Failed to get the global id tag handle");
  // setup query data for the group in question
  std::vector<moab::Tag> tags;
  tags.push_back(category_tag);
  tags.push_back(global_id_tag);
  moab::Range entsets;
  char grp[CATEGORY_TAG_SIZE] = "Group";
  void *grp_ptr = &grp;
  void *id_ptr = &group_global_id;
  void *vals[2] = {grp_ptr,id_ptr};
  // query for the group using the provided group global_id and the category tag with value "Group"
  result = mbi()->get_entities_by_type_and_tag(0, moab::MBENTITYSET, &tags[0], &(vals[0]), 2, entsets, moab::Interface::INTERSECT, true);
  MB_CHK_SET_ERR_RET(result, "Failed to get entity sets with a category tag value of 'Group' and a global id tag value " << group_global_id);

  // ensure that exactly one group was found
  if(1 != entsets.size() ) { 
    std::cout << "Invalid group id." << std::endl;
    MB_CHK_SET_ERR_RET(moab::MB_FAILURE, "Group ID is invalid");
  }

  // get the handle of the group to modify
  moab::EntityHandle group_to_mod = entsets[0];
  //get the name tag, because this global id should indicate a group with this tag
  moab::Tag name_tag; 
  result = mbi()->tag_get_handle(NAME_TAG_NAME,name_tag);
  MB_CHK_SET_ERR_RET(result, "Failed to get the name tag handle");
  //give warning about truncated name
  if(NAME_TAG_SIZE < new_name.size()) {
    std::cout << "Warning: size of name exceed standard group name size. It will be trucnated." << std::endl;
      }
  //now set the new name
  new_name.resize(NAME_TAG_SIZE);
  result = mbi()->tag_set_data(name_tag, &group_to_mod, 1, (void*)new_name.c_str());
  MB_CHK_SET_ERR_RET(result, "Failed to get the name tag data for a group set");

  return;
}

void write_file_out(std::string new_filename) {
  moab::ErrorCode result;
  // if the provided filename is the same as the current filename tagged on the instance,
  // print a warning
  if (!is_new_filename(new_filename)) {
    std::cout << "Warning! This filename is identical to the current filename. Originaly file will be over-written." << std::endl;
      }

  // write the file
  result = mbi()->write_file(new_filename.c_str());
  MB_CHK_SET_ERR_RET(result, "Failed to write file");
}

bool is_new_filename(std::string name) {

  moab::ErrorCode result;
  //get the filename_tag
  moab::Tag filename_tag;
  result = mbi()->tag_get_handle( FILENAME_TAG_NAME, 50, moab::MB_TYPE_OPAQUE, filename_tag, moab::MB_TAG_CREAT|moab::MB_TAG_SPARSE);
  MB_CHK_SET_ERR(result, "Failed to get or create the filename tag handle");

  //check the root_set for this tag
  moab::EntityHandle root_set = mbi()->get_root_set();
  std::vector<moab::Tag> root_set_tags;
  result = mbi()->tag_get_tags_on_entity(root_set, root_set_tags);
  MB_CHK_SET_ERR_CONT(result, "Failed to get all tag handles on the root set");
  // search for the tag
  std::vector<moab::Tag>::iterator val  = std::find(root_set_tags.begin(),root_set_tags.end(),filename_tag);

  
  // if the tag is found, then check the current filename vs the provided name
  if(val != root_set_tags.end()) { 
    //check that filename is the same
    std::string current_filename;
    current_filename.resize(50);
    result = mbi()->tag_get_data(filename_tag, &root_set, 1, (void*)current_filename.c_str());
    MB_CHK_SET_ERR(result, "Failed to get the filename tag data stored on the root set");
    current_filename.resize(name.size());
    if(current_filename == name) return false;
  }
  // if the tag was not found on the root set, then assume this is a new filename
  return true;
}

moab::ErrorCode filter_surfaces(moab::Range &surfs, moab::Tag aabb_tag, int axis, double coord) {

  std::cout << "Number of initial surfaces before filter: " << surfs.size() << std::endl;
  moab::ErrorCode result;
  std::vector<moab::EntityHandle> surfs_to_skip;
  moab::Range::iterator i;
  // for each surface, check for intersection with the slice plane
  for(i = surfs.begin(); i != surfs.end(); i++) {
    //first get the box data
    double box[6];
    result = mbi()->tag_get_data(aabb_tag, &(*i), 1, (void *)box);
    MB_CHK_SET_ERR(result, "Failed to get the AABB tag data for a surface");
    //now check to see if this slice plane intersects with the surface
    double min = box[2*axis], max = box[(axis*2)+1];
    //if not, remove the surface from the range
    if(coord > max || coord < min) {
      surfs_to_skip.push_back(*i);
    }
  }

  // remove the surfaces to skip from the input list of surface
  for(std::vector<moab::EntityHandle>::iterator s = surfs_to_skip.begin();
      s != surfs_to_skip.end(); s++) surfs.erase(*s);
  
  std::cout << "Number of surfaces after filtering: " << surfs.size() << std::endl;
  
  return result;
}
