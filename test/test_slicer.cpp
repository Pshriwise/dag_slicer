
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

//test functions
void get_sets_by_category_test();

int main( int /* argc */, char** /* argv */) 
{ 

  MBErrorCode result = mbi->load_mesh("cyl.h5m");
  ERR_CHECK(result);
  
  //get_sets_by_category_test( mbi );
  int failed_tests = 0; 
  failed_tests += RUN_TEST(get_sets_by_category_test);

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
  result = get_sets_by_category(mbi, sets, category2);
 
  CHECK_EQUAL(3, (int)sets.size() );  

}  
  




  
