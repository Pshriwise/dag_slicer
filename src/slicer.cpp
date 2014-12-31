
#include "slicer.hpp"

bool point_match( MBCartVect pnt1, MBCartVect pnt2) 
{
  bool ans = false; 
  double tolerance = 1e-6; 

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

MBErrorCode create_surface_intersections( MBInterface *mbi, MBRange surfs, int axis, double coord)
{

  MBErrorCode result; 

  std::map<MBEntityHandle,std::vector< std::vector<MBCartVect> > > intersection_dict;

  MBRange::iterator i; 
  for ( i = surfs.begin(); i !=surfs.end(); i++)
    {
      //get the surface triangles
      std::vector<MBEntityHandle> surf_tris; 
      result = mbi->get_entities_by_type( *i, MBTRI, surf_tris);
      ERR_CHECK(result); 
      
      //now create surface intersection

    }

  return result; 
}


MBErrorCode surface_intersections(MBInterface *mbi, std::vector<MBEntityHandle> tris, int axis, double coord)
{

  MBErrorCode result; 

  std::vector< MBCartVect[2] > lines; 

  std::vector<MBEntityHandle>::iterator i; 
  for ( i = tris.begin(); i != tris.end(); i++)
    { 
      MBCartVect line[2]; bool intersect;
      result = intersection( mbi, axis, coord, *i, line, intersect);
      ERR_CHECK(result);
    } 

  return result; 

}


MBErrorCode intersection( MBInterface *mbi,  int axis, double coord, MBEntityHandle tri, MBCartVect *line, bool &intersect)
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
  
  return result; 
}

void triangle_plane_intersect( int axis, double coord, MBCartVect *coords)
{
  
  //check to see how many triangle edges cross the coordinate
  int count = 0; int nlines = 0; 
  MBCartVect p0,p1,p2,p3;

  for( unsigned int i = 0 ; i < 3; i++) 
    p3[i] = copysign(1.0, coords[i][axis] - coord);

  if (p3[0] * p3[1] < 0) count += 1;
  if (p3[1] * p3[2] < 0) count += 1;
  if (p3[2] * p3[0] < 0) count += 1;

  //count up how many line points we'll get from this triangle
  if (count ==2)
    nlines += 1;
  else if (count == 3)
    nlines += 2; 


  if (p3[0] * p3[1] < 0 )
    {
      for(unsigned int i = 0; i < 3; i++)
	{
	  p0[i] = coords[0][i];
	  p1[i] = coords[1][i];
	  //get_intersection(p0, p1, ax, coord, points)
	}
    } 
  if (p3[1] * p3[2] < 0 )
    {
      for(unsigned int i = 0; i < 3; i++)
	{
	  p0[i] = coords[1][i];
	  p1[i] = coords[2][i];
	  //get_intersection(p0, p1, ax, coord, points)
	}
    }
  if (p3[2] * p3[0] < 0 )
    {
      for(unsigned int i = 0; i < 3; i++)
	{
	  p0[i] = coords[2][i];
	  p1[i] = coords[0][i];
	  //get_intersection(p0, p1, ax, coord, points)
	}
    }



}
