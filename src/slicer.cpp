
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

MBErrorCode slice_faceted_model( MBInterface *mbi, std::string filename, int axis, double coord )
{
  MBErrorCode result; 
  std::map<MBEntityHandle,std::vector<Loop> > intersection_map;

  result = mbi->load_file( filename.c_str() );
  ERR_CHECK(result);
  
  MBRange surfaces; 
  result = get_surfaces( mbi, surfaces );
  ERR_CHECK(result);

  result = create_surface_intersections( mbi, surfaces, axis, coord, intersection_map);
  ERR_CHECK(result);

  MBRange volumes; 
  result = get_all_volumes( mbi, volumes );
  ERR_CHECK(result);

  std::vector < std::vector<Loop> > all_paths;
  result = get_volume_paths( mbi, volumes, axis, intersection_map, all_paths);
  ERR_CHECK(result);
  
  
  
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
