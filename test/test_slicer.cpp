
//c++ includes
#include <assert.h>
#include <iostream> 

//slicer includes
#include "slicer.hpp"

//SIGMA includes
#include "MBCore.hpp"
#include "MBInterface.hpp"

#include "testutils.hpp"

using namespace moab;

//global moab instance
MBInterface *mbi = new MBCore(); 

//structure functions
void line_struct_test();
//test functions
void create_surface_intersections_test();
void intersection_test();
void triangle_plane_intersect_test();
void get_intersection_test();
void get_sets_by_category_test();
void get_surfaces_test();
void get_all_volumes_test();
void test_point_match();
void get_volume_intersections_test(); 


int main( int /* argc */, char** /* argv */) 
{ 

  MBErrorCode result = mbi->load_mesh("cube.h5m");
  ERR_CHECK(result);
  
  //get_sets_by_category_test( mbi );
  int failed_tests = 0; 

  failed_tests += RUN_TEST(line_struct_test);
  failed_tests += RUN_TEST(get_sets_by_category_test);
  failed_tests += RUN_TEST(get_surfaces_test);
  failed_tests += RUN_TEST(get_all_volumes_test);
  failed_tests += RUN_TEST(test_point_match);
  failed_tests += RUN_TEST(create_surface_intersections_test);
  failed_tests += RUN_TEST(get_intersection_test);
  failed_tests += RUN_TEST(triangle_plane_intersect_test);
  failed_tests += RUN_TEST(get_volume_intersections_test); 

}

void line_struct_test()
{

  Line test_line; 
  
  MBCartVect point(0,1,0);

  CHECK( !test_line.started );
  CHECK( !test_line.full );
  
  test_line.add_pnt(point);
  
  CHECK( test_line.started );
  CHECK( !test_line.full );

  test_line.add_pnt(point);
  
  CHECK( test_line.started );
  CHECK( test_line.full );
  
  CHECK( test_line.begin == test_line.end );

}

void create_surface_intersections_test()
{

  MBRange surfs;
  get_surfaces(mbi, surfs);

  std::map<MBEntityHandle, std::vector<Loop> >int_map;
  MBErrorCode result = create_surface_intersections( mbi, surfs, 0, 0, int_map);
  ERR_CHECK(result);

  CHECK( (int)int_map.size() == 6); // the cube should have 6 surfaces
  
  //check for surfaces w/ intersections
  int num_intersections = 0;
  std::map<MBEntityHandle, std::vector<Loop> >::iterator i; 
  for( i = int_map.begin(); i != int_map.end() ; i++)
    if ( (i->second).size() != 0 ) num_intersections++;

  // the axis and coordinate given should slice through four surfaces
  CHECK( 4 == num_intersections );

}

void intersection_test()
{
  MBErrorCode result; 
  
  //create a new triangle in the moab instance 
  MBCartVect coords[3];
  coords[0][0] = 1; coords[0][1] = 0; coords[0][2] = 0;
  coords[1][0] = 0; coords[1][1] = 1; coords[1][2] = 0;
  coords[2][0] = 0; coords[2][1] = 0; coords[2][2] = 1;
  
  MBEntityHandle v0,v1,v2,tri;
 
  result = mbi->create_vertex( coords[0].array(), v0); 
  ERR_CHECK(result);
  result = mbi->create_vertex( coords[1].array(), v1); 
  ERR_CHECK(result);
  result = mbi->create_vertex( coords[2].array(), v2); 
  ERR_CHECK(result);

  MBEntityHandle verts[] = {v0,v1,v2};
  result = mbi->create_element(MBTRI, verts, 3, tri);
  ERR_CHECK(result);

  Line test_line; bool intersect;
  result = intersection( mbi, 2, 0.5, tri, test_line, intersect);
  ERR_CHECK(result);

  CHECK(intersect);

  //make sure the values we get back from this function are 
  //correct
  CHECK( test_line.begin[0] == 0 );
  CHECK( test_line.begin[1] == 0.5 );
  CHECK( test_line.begin[2] == 0.5 );

  CHECK( test_line.end[0] == 0.5 );
  CHECK( test_line.end[1] == 0 );
  CHECK( test_line.end[2] == 0.5 );

  Line test_line2;
  //this plane should not intersect our triangle
  result = intersection( mbi, 2, 6, tri, test_line2, intersect);
  ERR_CHECK(result);

  CHECK(!intersect);
  
}

void triangle_plane_intersect_test()
{
  
  MBCartVect coords[3];
  coords[0][0] = 1; coords[0][1] = 0; coords[0][2] = 0;
  coords[1][0] = 0; coords[1][1] = 1; coords[1][2] = 0;
  coords[2][0] = 0; coords[2][1] = 0; coords[2][2] = 1;

  Line test_line; 

  CHECK( !test_line.full );
  triangle_plane_intersect( 2, 0.5, coords, test_line); 

  CHECK( test_line.started );
  CHECK( test_line.full );

  //make sure the values we get back from this function are 
  //correct
  CHECK( test_line.begin[0] == 0 );
  CHECK( test_line.begin[1] == 0.5 );
  CHECK( test_line.begin[2] == 0.5 );

  CHECK( test_line.end[0] == 0.5 );
  CHECK( test_line.end[1] == 0 );
  CHECK( test_line.end[2] == 0.5 );

  Line test_line2;
  // this plane should not intersect with the triangle
  triangle_plane_intersect( 2, 6.0, coords, test_line2 );
  
  CHECK( !test_line2.started );
  CHECK( !test_line2.full );

}

void get_intersection_test()
{

  MBCartVect point0( 1, 1, 0 );
  MBCartVect point1(-1, 1, 0 );

  Line test_line;

  get_intersection( point0, point1, 0, 0.0, test_line);

  CHECK( test_line.begin[0] == 0.0 );
  CHECK( test_line.begin[1] == 1.0 ); 
  CHECK( test_line.begin[2] == 0.0 );

  point1[0] = 1; point1[1] = -1; point1[2] = 0; 

  test_line.started = true; 

  get_intersection( point0, point1, 1 , 0.0, test_line ); 

  CHECK( test_line.end[0] == 1.0 );
  CHECK( test_line.end[1] == 0.0 );
  CHECK( test_line.end[2] == 0.0 );

  point1[0] = 1; point1[1] = 1; point1[2] = 2; 

  Line test_line2; 

  get_intersection( point0, point1, 2 , 1.0, test_line2 ); 
  
  CHECK( test_line2.begin[0] = 1.0 );
  CHECK( test_line2.begin[1] = 1.0 ); 
  CHECK( test_line2.begin[2] = 1.0 );

  //this function expects an intersection to exist
  //an intentional failure case will not be created
}

void get_volume_intersections_test()
{
  //get the volume from the instance
  MBRange sets;
  char category[CATEGORY_TAG_SIZE] = "Volume";
  MBErrorCode result = get_sets_by_category( mbi, sets, category);
  ERR_CHECK(result);
  
  MBEntityHandle cube_vol = sets[0]; //there should only be one volume in the test model

  //create surface intersections for the model
  char category1[CATEGORY_TAG_SIZE] = "Surface";
  sets.clear(); 
  result = get_sets_by_category( mbi, sets, category1);
  ERR_CHECK(result);

  //create a fake map for this volume. 

  std::map< MBEntityHandle, std::vector<Loop> > fake_map; 
  
  std::vector<Loop> dummy_loop;
  dummy_loop.resize(2); 

  fake_map[sets[0]] = dummy_loop; 

  //check that we get this loop back
  std::vector<Loop> intersections;
  get_volume_intersections( mbi, cube_vol, fake_map, intersections); 
  
  //we should get back our dummy loop once 
  //and insert it into the intersections vector
  CHECK( 2 == intersections.size() );

  //add a new dummy to the map
  fake_map[sets[1]] = dummy_loop; 

  intersections.clear(); 
  get_volume_intersections( mbi, cube_vol, fake_map, intersections); 

  //with this map we should get 4 values in the intersections vector
  CHECK( 4 == intersections.size() );


}
void get_sets_by_category_test()
{

  MBRange sets; 
  char category1[CATEGORY_TAG_SIZE] = "Volume";
  MBErrorCode result = get_sets_by_category( mbi, sets, category1); 
  ERR_CHECK(result); 
 
  CHECK_EQUAL( 1 , (int)sets.size() ); 

  sets.clear();
  char category2[CATEGORY_TAG_SIZE] = "Surface";
  result = get_sets_by_category( mbi, sets, category2 );
 
  CHECK_EQUAL(6, (int)sets.size() );

}  
  

void get_surfaces_test()
{

  MBRange surfaces; 
  MBErrorCode result = get_surfaces( mbi, surfaces );
  ERR_CHECK(result);
  //test file is a cube and should have 6 surfaces
  CHECK_EQUAL( 6, (int)surfaces.size() );

}

void get_all_volumes_test()
{

  MBRange volumes; 
  MBErrorCode result = get_all_volumes( mbi, volumes );
  ERR_CHECK(result);
  //test file is a lone cube and should have 1 volume
  CHECK_EQUAL( 1, (int)volumes.size() ); 

}

void test_point_match()
{

  //setup two cart vectors that will represent points 
  MBCartVect a,b;
 
  //initialize
  a[0] = 1.0; a[1] = 1.0; a[2] = 1.0-1e-7;
  
  //make b equal to a
  b=a; 

  //These should be recognized as coincident for this tolerance
  CHECK( point_match(a,b, 1e-6) );

  //Now alter b a little bit
  b[2] = 1.0;

  //These should still be recognized as coincident
  CHECK( point_match(a,b,1e-6) );

  // This point should not be recognized as coincident
  CHECK( !point_match(a,b,1e-8) );

  //Now alter b a lot (relatively)
  b[2] = 2.0;

  //This should not be recognized as coincident
  CHECK( !point_match(a,b) );

  
}
