
#include "slicer.hpp"

bool point_match( MBCartVect pnt1, MBCartVect pnt2) 
{
  bool ans = false; 
  double tolerance = 1e-4; 

  MBCartVect diff = pnt2-pnt1; 

  if (diff.length() < tolerance) ans = true; 

  return ans;
}


MBErrorCode get_sets_by_category( MBInterface *mbi, MBRange &entsets, char* category)
{
  
  MBErrorCode result = MB_SUCCESS;

  //get the name tag 
  MBTag category_tag; 
  result = mbi->tag_get_handle(CATEGORY_TAG_NAME, category_tag); 
  ERR_CHECK(result); 

  const void *dum = &(category[0]);

  result = mbi->get_entities_by_type_and_tag(0, MBENTITYSET, &category_tag, &dum, 1, entsets);
  ERR_CHECK(result);
  
  return result;

}

MBErrorCode get_surfaces( MBInterface* mbi, MBRange &surfs)
{

  MBErrorCode result = MB_SUCCESS;

  char category[CATEGORY_TAG_SIZE] = "Surface";
  
  result = get_sets_by_category( mbi, surfs, category);

  std::cout << "There are " << surfs.size() << " surfaces in this model." << std::endl;

  return result;  

}

MBErrorCode get_all_volumes( MBInterface *mbi, MBRange &vols)
{ 

  MBErrorCode result = MB_SUCCESS;

  char category[CATEGORY_TAG_SIZE] = "Volume";
  
  result = get_sets_by_category( mbi, vols, category);
  
  std::cout << "There are " << vols.size() << " volumes in this model." << std::endl;

  return result; 
}

MBErrorCode slice_faceted_model_out( std::string filename, int axis, double coord, std::vector< std::vector<double> > &x_pnts, std::vector< std::vector<double> > &y_pnts, std::vector< std::vector<int> > &codings, bool by_group)
{
  
  std::vector< std::vector<xypnt> > paths;
  MBErrorCode result = slice_faceted_model( filename, axis, coord, paths, codings, by_group);

    std::vector< std::vector<xypnt> >::iterator path;

  for( path = paths.begin(); path != paths.end(); path++)
    {
      std::vector<double> path_xs, path_ys;
      for( unsigned int i = 0; i < (*path).size(); i++)
	{
	  path_xs.push_back((*path)[i].x);
	  path_ys.push_back((*path)[i].y);
	}
      x_pnts.push_back(path_xs);
      y_pnts.push_back(path_ys);
    }
  
  return result;
}

MBErrorCode slice_faceted_model( std::string filename, int axis, double coord, std::vector< std::vector<xypnt> > &paths, std::vector< std::vector<int> > &codings, bool by_group)
{

  MBInterface *mbi = new MBCore();
  
  MBErrorCode result; 
  std::map<MBEntityHandle,std::vector<Loop> > intersection_map;

  result = mbi->load_file( filename.c_str() );
  ERR_CHECK(result);
  
  MBRange surfaces; 
  result = get_surfaces( mbi, surfaces );
  ERR_CHECK(result);

  result = create_surface_intersections( mbi, surfaces, axis, coord, intersection_map);
  ERR_CHECK(result);

  //path container for a volume
  std::vector< std::vector<Loop> > all_paths;
  
  if (by_group)
    {
      std::map< std::string, MBRange > group_mapping;
      std::vector<std::string> group_names;
      result = get_volumes_by_group( mbi, group_mapping, group_names);
      ERR_CHECK(result);
      
      std::cout << "Size of group map: " << group_mapping.size() << std::endl;
      std::cout << "Size of group names: " << group_names.size() << std::endl;
      std::vector<std::string>::iterator group_name; 
      for( group_name = group_names.begin(); group_name != group_names.end();)
	{
	  std::vector< std::vector<Loop> > all_group_paths;
	  result = get_volume_paths( mbi, group_mapping[ *group_name ], axis, intersection_map, all_group_paths);
	  ERR_CHECK(result);

	  if (0 == all_group_paths.size())
	    {
	      std::cout << "Erasing group: " << *group_name << std::endl;
	      group_name = group_names.erase(group_name); //erase and set iterator to next item
	      continue;
	    }
	  std::cout << "Getting slice for group:" << *group_name << std::endl;
	  std::vector<xypnt> group_path; 
	  std::vector<int> group_coding;

	  std::vector< std::vector<Loop> >::iterator path;

	  for (path = all_group_paths.begin(); path != all_group_paths.end(); path++)
	    {
	      std::vector<xypnt> vol_path;
	      std::vector<int> vol_coding;
	      create_patch(axis, *path, vol_path, vol_coding);
	      
	      //insert this path and coding into the group path and group coding
	      group_path.insert(group_path.end(), vol_path.begin(), vol_path.end());
	      group_coding.insert(group_coding.end(), vol_coding.begin(), vol_coding.end());
	      (*path).clear();

	    }
	  //when we're done with this group, push the path and coding into the main set of paths 
	  codings.push_back(group_coding);
	  paths.push_back(group_path);
	  all_group_paths.clear();
	  group_name++;
	}

    }
  else
    {
      MBRange volumes; 
      result = get_all_volumes( mbi, volumes );
      ERR_CHECK(result);


      result = get_volume_paths( mbi, volumes, axis, intersection_map, all_paths);
      ERR_CHECK(result);

      std::vector< std::vector<Loop> >::iterator i;

      //std::vector< std::vector<xypnt> > paths;
      //std::vector< std::vector<int> > codings;
      for( i = all_paths.begin(); i != all_paths.end(); i++)
	{
	  std::vector<xypnt> path;
	  std::vector<int> coding;
	  create_patch(axis, *i, path, coding);
	  codings.push_back(coding);
	  paths.push_back(path);
	}
      
    }
  

  return MB_SUCCESS;

}

MBErrorCode get_volumes_by_group( MBInterface *mbi, std::map< std::string, MBRange > &group_map, std::vector<std::string> &group_names )
{
  MBErrorCode result;

  //get all meshsets in the model 
  std::vector<MBEntityHandle> all_entsets;
  result = mbi->get_entities_by_type(0, MBENTITYSET, all_entsets);
  ERR_CHECK(result);

  //get the category tag
  MBTag name_tag; 
  result = mbi->tag_get_handle(NAME_TAG_NAME, name_tag);
  ERR_CHECK(result);

  std::vector<MBEntityHandle>::iterator i;
  for( i = all_entsets.begin(); i != all_entsets.end(); ++i)
    {
      
      //get all the tags on this entity set
      std::vector<MBTag> ent_tags;
      result = mbi->tag_get_tags_on_entity( *i, ent_tags);
      ERR_CHECK(result);

      //check if this entity has a name_tag (is a group)
      if ( std::find( ent_tags.begin(), ent_tags.end(), name_tag) != ent_tags.end() )
	{
	  std::string ent_name;
	  ent_name.resize(NAME_TAG_SIZE);
	  void *dum = &(ent_name[0]);
	  //get the tag data on this entity set
	  result = mbi->tag_get_data( name_tag, &(*i), 1, dum);
	  ERR_CHECK(result);
	  
	  std::cout << ent_name << std::endl; 
	  
	  //get this group's children
	  MBRange group_contents;
	  result = mbi->get_entities_by_type( *i, MBENTITYSET, group_contents);
	  ERR_CHECK(result); 

	  //add this group to the list of group names
	  group_names.push_back( ent_name ); 
	  
	  //add this set of children to the map
	  group_map[ent_name] = group_contents;
	  group_contents.clear();
	}
      
      ent_tags.clear();

    }
  
  


  return MB_SUCCESS;
}

MBErrorCode get_volume_paths( MBInterface *mbi, MBRange volumes, int axis, std::map<MBEntityHandle, std::vector<Loop> > intersection_dict,   std::vector< std::vector<Loop> > &all_vol_paths ) 
{
  
  MBErrorCode result; 
  
  MBRange::iterator i; 
  for( i = volumes.begin(); i != volumes.end(); i++)
    {
      std::vector<Loop> this_vol_intersections;
      result = get_volume_intersections( mbi, *i, intersection_dict, this_vol_intersections );
      ERR_CHECK(result);

      if ( 0 == this_vol_intersections.size() ) continue; 

      std::cout << "Retrieved " << this_vol_intersections.size() << " intersections for this volume." << std::endl;

      std::vector<Loop> vol_paths;
      stitch( this_vol_intersections, vol_paths );
      all_vol_paths.push_back(vol_paths);
      
    }
  


  return MB_SUCCESS;

}

void stitch( std::vector<Loop> loops, std::vector<Loop> &paths )
{


  int i = 0; 

  while ( i < loops.size() )

    {

      //check for complete loops first 
      
      //start with arbitrary
      Loop this_intersection = loops[i];

      // if we find a complete loop, add it to the volume paths
      if (point_match( this_intersection.points.front(), this_intersection.points.back() ) && this_intersection.points.size() > 2 )
	{
	  paths.push_back(this_intersection);
	  loops.erase( loops.begin() + i );
	  i = 0;
	} 
      // if this is a line of negligible length, remove it from loops
      else if ( point_match( this_intersection.points.front(), this_intersection.points.back() ) && 2 == this_intersection.points.size() )
	{
	  loops.erase( loops.begin() + i );
	  i = 0;
	}
      
      i += 1;

    }
  

  //if there are no point collections left, we're done
  if ( 0 == loops.size() ) return;

  
  //now we'll start stitching loops together
  i = 0;

  //start by adding an arbitrary surface intersection to the paths
  paths.push_back( loops[i] );
  loops.erase( loops.begin() + i );


  while ( 0 != loops.size() )
    {
      
      //if we have a complete loop, then move on to a new starting point
      if ( point_match( paths.back().points.front() , paths.back().points.back() ) )
	{
	  paths.push_back( loops.front() );
	  loops.erase( loops.begin() );
	}	   
      else
	{
	 
	  i = 0;
	  
	  while ( i < loops.size() )
	    {

	      Loop this_intersection = loops[i];

	      if (point_match( paths.back().points.front(), this_intersection.points.front() ) )
		{
		  //reverse this_intersection and attach to the front of paths.back()
		  std::reverse( this_intersection.points.begin(), this_intersection.points.end() );
		  paths.back().points.insert( paths.back().points.begin(), this_intersection.points.begin(), this_intersection.points.end() );
		  loops.erase( loops.begin() + i );
		  i = 0;
		}
	      else if (point_match( paths.back().points.front(), this_intersection.points.back() ) ) 
		{
		  // attach to the front of paths.back()
		  paths.back().points.insert( paths.back().points.begin(), this_intersection.points.begin(), this_intersection.points.end() );
		  loops.erase( loops.begin() + i );
		  i = 0;		  
		}
	      else if (point_match( paths.back().points.back(), this_intersection.points.front() ) ) 
		{
		  //attach to the back of paths.back()
		  paths.back().points.insert( paths.back().points.end(), this_intersection.points.begin(), this_intersection.points.end() );
		  loops.erase( loops.begin() + i );
		  i = 0;		  
		}
	      else if (point_match( paths.back().points.back(), this_intersection.points.back() ) )
		{
		  //reverse intersection and attach to the back of paths.back()
		  std::reverse( this_intersection.points.begin(), this_intersection.points.end() );
		  paths.back().points.insert( paths.back().points.end(), this_intersection.points.begin(), this_intersection.points.end() );
		  loops.erase( loops.begin() + i );
		  i = 0;
		} 

	      //if no match is found, move on to the next intersection 
	      i++;

	    } //end inner while
 
	} // end if

    } // end outer while
  std::cout << "Created " << paths.size() << "paths for this volume." << std::endl; 
  return;

} // end stitch




MBErrorCode get_volume_intersections( MBInterface *mbi, MBEntityHandle volume, std::map<MBEntityHandle, std::vector<Loop> > intersection_dict,   std::vector<Loop> &volume_intersections )
{

  MBErrorCode result; 
  std::vector<MBEntityHandle> chld_surfaces;
  result = mbi->get_child_meshsets( volume, chld_surfaces );
  ERR_CHECK( result );


  
  std::vector<MBEntityHandle>::iterator i; 
  for( i = chld_surfaces.begin(); i != chld_surfaces.end(); i++) 
    {
      
      std::vector<Loop> this_set = intersection_dict[*i];

      volume_intersections.insert( volume_intersections.end(), this_set.begin(), this_set.end() );

    }


  return MB_SUCCESS;

}

MBErrorCode create_surface_intersections( MBInterface *mbi, MBRange surfs, int axis, double coord, std::map<MBEntityHandle,std::vector<Loop> > &intersection_map)
{

  MBErrorCode result; 

  MBRange::iterator i; 
  for ( i = surfs.begin(); i !=surfs.end(); i++)
    {
      //get the surface triangles
      std::vector<MBEntityHandle> surf_tris; 
      result = mbi->get_entities_by_type( *i, MBTRI, surf_tris);
      ERR_CHECK(result); 
      
      //now create surface intersection
      std::vector<Loop> surf_intersections;
      result = surface_intersections( mbi, surf_tris, axis, coord, surf_intersections);
      ERR_CHECK(result);

      intersection_map[*i] = surf_intersections;
      surf_intersections.clear();

    }

  return result; 
}


MBErrorCode surface_intersections(MBInterface *mbi, std::vector<MBEntityHandle> tris, int axis, double coord, std::vector<Loop> &surf_intersections)
{

  MBErrorCode result; 

  std::vector<Line> intersect_lines; 

  std::vector<MBEntityHandle>::iterator i; 
  for ( i = tris.begin(); i != tris.end(); i++)
    { 
      Line line; bool intersect;
      result = intersection( mbi, axis, coord, *i, line, intersect);
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
    curr_loop.points.push_back( intersect_lines.back().begin );
    curr_loop.points.push_back( intersect_lines.back().end );
    intersect_lines.pop_back();

    while (index < intersect_lines.size() )
      {

	Line this_line = intersect_lines[index];

	if ( point_match( this_line.begin, curr_loop.points.front() ) )
	  {
	    //insert the line into the current loop
	    curr_loop.points.insert(curr_loop.points.begin(), this_line.end );
	    //delete the current matched line from intersect_lines  
	    intersect_lines.erase(intersect_lines.begin()+index);
	    index = 0;
	  }
	else if ( point_match( this_line.begin, curr_loop.points.back() ) )
	  {
	    curr_loop.points.push_back( this_line.end );
	    intersect_lines.erase(intersect_lines.begin()+index);
	    index = 0;
	  }
	else if ( point_match( this_line.end, curr_loop.points.front() ) )
	  {
	    curr_loop.points.insert(curr_loop.points.begin(), this_line.begin );
	    intersect_lines.erase(intersect_lines.begin()+index);
	    index = 0;
	  }
	else if ( point_match( this_line.end, curr_loop.points.back() ) )
	  {
	    curr_loop.points.push_back( this_line.begin );
	    intersect_lines.erase(intersect_lines.begin()+index);
	    index = 0;
	  }
	else
	  {
	    index++;
	  }
      }

    all_surf_intersections.push_back(curr_loop);

  }

  surf_intersections = all_surf_intersections;

  return result; 

}


MBErrorCode intersection( MBInterface *mbi,  int axis, double coord, MBEntityHandle tri, Line &tri_intersection, bool &intersect)
{
  MBErrorCode result;
  //get the triangle vertices
  std::vector<MBEntityHandle> verts;
  
  result = mbi->get_adjacencies( &tri, 1, 0, false, verts);
  ERR_CHECK(result);
  
  MBCartVect tri_coords[3];
  result = mbi->get_coords( &(verts[0]), 1, tri_coords[0].array() );
  ERR_CHECK(result);
  result = mbi->get_coords( &(verts[1]), 1, tri_coords[1].array() );
  ERR_CHECK(result);
  result = mbi->get_coords( &(verts[2]), 1, tri_coords[2].array() );
  ERR_CHECK(result);

  triangle_plane_intersect(axis, coord, tri_coords, tri_intersection);

  intersect = tri_intersection.full;

  return result; 

}



void triangle_plane_intersect( int axis, double coord, MBCartVect *coords, Line &line_out)
{
  
  //check to see how many triangle edges cross the coordinate
  int count = 0; int nlines = 0; 
  MBCartVect p0,p1,p2,p3;

  //expecting three points for the triangle
  for( unsigned int i = 0 ; i < 3; i++) 
    p3[i] = copysign(1.0, coords[i][axis] - coord);
  /*
  if (p3[0] * p3[1] < 0) count += 1;
  if (p3[1] * p3[2] < 0) count += 1;
  if (p3[2] * p3[0] < 0) count += 1;

  //count up how many line points we'll get from this triangle
  if (count ==2)
    nlines += 1;
  else if (count == 3)
    nlines += 2; 
  */

  if (p3[0] * p3[1] < 0 )
    {
      p0 = coords[0];
      p1 = coords[1];
      get_intersection(p0, p1, axis, coord, line_out);
    } 
  if (p3[1] * p3[2] < 0 )
    {
      p0 = coords[1];
      p1 = coords[2];
      get_intersection(p0, p1, axis, coord, line_out);
    }
  if (p3[2] * p3[0] < 0 )
    {
      p0 = coords[2];
      p1 = coords[0];
      get_intersection(p0, p1, axis, coord, line_out);
    }
  
}

void get_intersection( MBCartVect pnt0, MBCartVect pnt1, int axis, double coord, Line &line )
{

  MBCartVect vec = pnt1-pnt0;

  double t = (coord - pnt0[axis])/vec[axis];


  MBCartVect pnt_out = pnt0 + t*vec;

  line.add_pnt(pnt_out);

}


void convert_to_stl( std::vector< std::vector<Loop> > a, std::vector< std::vector< std::vector< std::vector<double> > > > &b)
{

  b.resize(a.size());
  for(unsigned int i = 0; i < a.size(); i++)
    {
      b[i].resize(a[i].size());
      for(unsigned int j = 0; j < a[i].size(); j++)
	{
	  b[i][j].resize(a[i][j].points.size());
	  
	  for(unsigned int k = 0; k < a[i][j].points.size(); k++)
	    {
	      b[i][j][k].push_back(a[i][j].points[k][0]);
	      b[i][j][k].push_back(a[i][j].points[k][1]);
	      b[i][j][k].push_back(a[i][j].points[k][2]);
	    }
	}
    }	      	      


}

/********************************************
Section for handling loop windings and path coding as required by matplotlib
*********************************************/


void create_patch( int axis, std::vector<Loop> input_loops, std::vector<xypnt> &path_out, std::vector<int> &coding_out)
{

  std::vector<Loop>::iterator loop; 
  for ( loop = input_loops.begin(); loop != input_loops.end(); loop++ )
    {
      (*loop).gen_xys(axis);
      //(*loop).points.clear();
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
  set_windings( windings, desired_windings, input_loops);

  //create coding and paths
  generate_patch_path( input_loops, path_out, coding_out);

}


void get_containment( std::vector<Loop> loops, std::vector< std::vector<int> > &Mat )
{
  
  Mat.resize(loops.size());
  for( unsigned int i = 0; i < loops.size(); i++ )
    {
      Mat[i].resize(loops.size());
      for( unsigned int j = 0; j < loops.size(); j++ )
	{
	  Mat[i][j] = is_poly_a_in_poly_b( loops[i], loops[j]);
	}
    }
  
}

// checks to see if loop a is in loop b
// (assuming fully nested loops for now)
bool is_poly_a_in_poly_b( Loop a, Loop b)
{

  //use the first point of a to test for now
  double x = a.xypnts[0].x; double y = a.xypnts[0].y;

  int i, j = b.xypnts.size() - 1;
  
  bool result = false; 

  for( i = 0; i < b.xypnts.size(); i++)
    {
      if( (b.xypnts[i].y < y && b.xypnts[j].y >= y
	  || b.xypnts[j].y < y && b.xypnts[i].y >= y)
	  && (b.xypnts[i].x <= x || b.xypnts[j].x <= x))
	{
	  if (b.xypnts[i].x+(y-b.xypnts[i].y)/(b.xypnts[j].y-b.xypnts[i].y)
	      *(b.xypnts[j].x-b.xypnts[i].x) < x)
	    {
	    result = true;
	    }
	}
      j=i;
    }
  return result;
}

void get_windings( std::vector<Loop> loops, std::vector<int> &windings)
{
  std::vector<Loop>::iterator loop; 
  windings.clear(); //JIC
  
  for( loop = loops.begin(); loop != loops.end(); loop++)
    {
      windings.push_back(find_winding(*loop));
    }

}

int find_winding( Loop loop )
{
  double area = 0; 
  
  int j = loop.xypnts.size() - 1; 

  for(unsigned int i = 0; i < loop.xypnts.size(); i++)
    {
      area += (loop.xypnts[j].x + loop.xypnts[i].x) * (loop.xypnts[j].y - loop.xypnts[i].y);
      j=i;
    }

  return area >= 0 ? CW : CCW;
}


void get_fill_windings( std::vector< std::vector<int> > fill_mat, std::vector<int> &windings)
{

  int a = fill_mat.size(); 
  int b = fill_mat[0].size(); 
  assert( a == b); 
  int wind;
  for( unsigned int i = 0; i < a; i++)
    {
      int dum;
      for(std::vector<int>::iterator j = fill_mat[i].begin();
	  j != fill_mat[i].end(); j++)
	dum += *j;
      wind = dum%2 == 0 ? CCW : CW;
      windings.push_back(wind);
    }


}

void set_windings( std::vector<int> current_windings, std::vector<int> desired_windings, std::vector<Loop> loops)
{

  assert ( current_windings.size() == desired_windings.size() );

  for( unsigned int i = 0; i < current_windings.size(); i++)
    {
      if (current_windings[i] != desired_windings[i])
	{
	  Loop this_loop = loops[i];
	  std::reverse( this_loop.points.begin(), this_loop.points.end()); //JIC
	  std::reverse( this_loop.xypnts.begin(), this_loop.xypnts.end());
	}
    }

} 


void generate_patch_path( std::vector<Loop> loops, std::vector<xypnt> &path, std::vector<int> &coding)
{

  std::vector<Loop>::iterator loop;
  
  for( loop = loops.begin(); loop != loops.end(); loop++)
    {
      for(unsigned int i = 0; i < (*loop).xypnts.size(); i++)
	{
	  path.push_back((*loop).xypnts[i]);
	  i == 0 ? coding.push_back(1) : coding.push_back(2);
	}

    }



}
