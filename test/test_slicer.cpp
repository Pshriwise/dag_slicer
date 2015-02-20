
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
void stitch_test();
void get_containment_test(); 
void is_poly_a_in_poly_b_test();

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
  failed_tests += RUN_TEST(stitch_test);
  failed_tests += RUN_TEST(get_containment_test);
  failed_tests += RUN_TEST(is_poly_a_in_poly_b_test);

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


Loop create_circle_loop( double radius, unsigned int intervals )
{

  Loop circle_loop; 

  double x,y,z; 
  double theta; 

  //add a complete unit circle
  for( unsigned int i = 0; i <= intervals; i++)
    {

      theta = 2*M_PI * (double) i * (1/(double)intervals);
      x = radius*cos(theta); y = radius*sin(theta); z = 1.0; 
      MBCartVect pnt( x, y, z); 

      circle_loop.points.push_back(pnt); 

    }

  return circle_loop;

}


void stitch_test()
{

  std::vector<Loop> dummy_intersections; 
  std::vector<Loop> returned_paths;

  Loop dummy_loop1 = create_circle_loop( 1.0, 10 ); 

  dummy_intersections.push_back(dummy_loop1);

  //this should return the circle
  stitch( dummy_intersections, returned_paths); 

  CHECK ( 1 == (int)returned_paths.size() );
  
  //clear some data
  returned_paths.clear(); 
  dummy_loop1.points.clear();

  Loop dummy_loop2 = create_circle_loop( 2.0 , 10 );
  
  dummy_intersections.push_back(dummy_loop2);

  stitch( dummy_intersections, returned_paths ); 
  // there should now be two paths: the unit circle and the larger circle of rad 2
  CHECK ( 2 == (int)returned_paths.size() );

  //clear everything
  returned_paths.clear();
  dummy_intersections.clear(); 
  dummy_loop1.points.clear();
  dummy_loop2.points.clear();

  //now write half the points of a circle to one loop and half to the other
  dummy_loop1 = create_circle_loop( 1.0, 10 ); 

  //give half of dummy_loop1's points to dummy_loop2
  dummy_loop2.points.insert(  dummy_loop2.points.begin(), dummy_loop1.points.begin()+3, dummy_loop1.points.end() );
  dummy_loop1.points.erase( dummy_loop1.points.begin()+4, dummy_loop1.points.end() ); 
    
  dummy_intersections.push_back(dummy_loop1);
  dummy_intersections.push_back(dummy_loop2); 
  
  //the function should find these two sections and stitch them without a problem
  stitch( dummy_intersections, returned_paths );


  CHECK( 1 == (int)returned_paths.size() );
    
  //check that our end and beginning points are correct

  //if these loops stitch correctly the beginning of the path will be the beginning of dummy_loop1
  // the end of the path will be the end of dummy_loop2

  //BEFORE *- coincident point (beginning and end of circle)
  // dummy_loop1 *-----------#
  // dummy_loop2 #-----------*

  //AFTER # - test points
  // returned_path #-----------**-----------#
  
  MBCartVect begin_test_pnt = dummy_loop2.points.front(); 
  MBCartVect end_test_pnt = dummy_loop1.points.back(); 

  CHECK( returned_paths[0].points.front() == begin_test_pnt );
  CHECK( returned_paths[0].points.back() == end_test_pnt );

  //this function should always return a closed loop (i.e. # == # in above diagram )
  CHECK( returned_paths[0].points.front() == returned_paths[0].points.back() );
 

}

void get_containment_test() 
{

  std::vector<Loop> test_loops;
  //create some loops for testing containment (concentric circles)
  test_loops.push_back( create_circle_loop( 1.0, 10 ) ); 
  test_loops.push_back( create_circle_loop( 2.0, 10 ) ); 
  test_loops.push_back( create_circle_loop( 3.0, 10 ) ); 
  test_loops.push_back( create_circle_loop( 4.0, 10 ) ); 

  //generate the xy values for each loop
  for( unsigned int i = 0 ; i < test_loops.size(); i++) 
    test_loops[i].gen_xys(2);

  //now get the containment matrix for these loops
  std::vector< std::vector<int> > returned_mat, test_mat; 

  get_containment(test_loops, returned_mat); 


  //what we expect our matrix to look like
  std::vector<int> row(4); 
  row[0] = 0; row[1] = 1; row[2] = 1; row[3] = 1;
  test_mat.push_back(row); 
  row[0] = 0; row[1] = 0; row[2] = 1; row[3] = 1;
  test_mat.push_back(row); 
  row[0] = 0; row[1] = 0; row[2] = 0; row[3] = 1;
  test_mat.push_back(row); 
  row[0] = 0; row[1] = 0; row[2] = 0; row[3] = 0;
  test_mat.push_back(row); 

  //the matrices should be of the same size
  CHECK_EQUAL( test_mat.size(), returned_mat.size() ); 

  for( unsigned int i = 0; i < test_mat.size(); i ++) 
    {
      CHECK_EQUAL( test_mat[i].size(),returned_mat[i].size() );
      //the matrices should be square
      CHECK( returned_mat.size() == returned_mat[i].size() );
      CHECK( test_mat.size() == test_mat[i].size() );
      
      for( unsigned int j = 0; j < test_mat[i].size(); j++)
	{

	  //check that the values match in our test matrix
	  CHECK_EQUAL(  test_mat[i][j], returned_mat[i][j] ); 


	}
      
    }

}

void is_poly_a_in_poly_b_test()
{
  //create a few test loops

  Loop test_loop1, test_loop2, test_loop3;
  
  double base_radius = 2.0; unsigned int intervals = 20; 
  int axis = 2; //expecting a simulated slice along the z axis
  test_loop1 = create_circle_loop( base_radius, intervals ); 
  test_loop1.gen_xys(axis);

  //any polygon should be contained by itself
  CHECK( is_poly_a_in_poly_b( test_loop1, test_loop1 ) );


  double tolerances[6] = {1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6};

  for( unsigned int i = 0; i < 6; i++)
    {

      double tol = tolerances[i];
      double larger_rad = base_radius + tol;
      double smaller_rad = base_radius - tol;

      test_loop2 = create_circle_loop( larger_rad, intervals+3 ); // one just on the outside
      test_loop3 = create_circle_loop( smaller_rad, intervals+3 ); // one just on the inside
      test_loop2.gen_xys(axis); test_loop3.gen_xys(axis);

      CHECK( is_poly_a_in_poly_b( test_loop1, test_loop2 ) );
      CHECK( !is_poly_a_in_poly_b( test_loop2, test_loop1 ) );

      CHECK( !is_poly_a_in_poly_b( test_loop1, test_loop3 ) );
      CHECK( is_poly_a_in_poly_b( test_loop3, test_loop1 ) );

      CHECK( is_poly_a_in_poly_b( test_loop3, test_loop2 ) );
      CHECK( !is_poly_a_in_poly_b( test_loop2, test_loop3 ) );

      test_loop2.points.clear(); test_loop2.xypnts.clear();
      test_loop3.points.clear(); test_loop3.xypnts.clear();

    }
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
