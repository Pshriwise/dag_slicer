//Temporary test drive for this moab tool. Eventually these unit tests will need to comply with either yt or pyne test suites so the bare minimum was brought in from moab's test driver for now.



//c++ inclucdes=
#include <stdio.h>
#  include <sys/types.h>
#  include <sys/wait.h>
#  include <unistd.h>
#  include <errno.h>


#define FLAG_ERROR exit(1)
/** Check that A is MB_SUCCESS */
#define CHECK_ERR( A )                    check_equal( MB_SUCCESS, (A), "MB_SUCCESS", #A, __LINE__, __FILE__ )
/**  Ensure that A is true */
#define CHECK( A )                        check_true( (A), #A, __LINE__, __FILE__ )
/** Check that two values are equal */
#define CHECK_EQUAL( EXP, ACT )           check_equal( (EXP), (ACT), #EXP, #ACT, __LINE__, __FILE__ )
/** Check that two real (float or double) values are equal within EPS */
#define CHECK_REAL_EQUAL( EXP, ACT, EPS ) check_equal( (EXP), (ACT), (EPS), #EXP, #ACT, __LINE__, __FILE__ )
/** Check that two arrays contain the same values in the same order */
#define CHECK_ARRAYS_EQUAL( EXP, EXP_LEN, ACT, ACT_LEN ) check_array_equal( (EXP), (EXP_LEN), (ACT), (ACT_LEN), #EXP, #ACT, __LINE__, __FILE__ )
/** Run a test
 *  Argument should be a function with the signature:  void func(void)
 *  Evaluates to zero if test is successful, one otherwise.
 */


#define RUN_TEST( FUNC )           run_test( &FUNC, #FUNC )

typedef void (*test_func)(void);
int run_test( test_func test, const char* func_name )
{
  printf("Running %s ...\n", func_name );
  
    /* For non-Windows OSs, fork() and run test in child process. */
  pid_t pid = fork();
  int status;
  
    /* Fork failed? */
  if (pid == -1) {  
    perror( "fork()" );
    abort(); /* abort all tests (can't fork child processes) */
  }
  
    /* If child process*/
  if (pid == 0) {
    (*test)();  /* call test function */
    exit(0);    /* if function returned, then it succeeded */
  }
  
    /* If here, then parent process */
    
    /* Wait until child process exits */
  waitpid( pid, &status, 0 );
  
    /* Check child exit status */
  if (WIFSIGNALED(status)) {
    if (WTERMSIG(status))
      printf("  %s: TERMINATED (signal %d)\n", func_name, (int)WTERMSIG(status) );
    if (WCOREDUMP(status))
      printf("  %s: CORE DUMP\n", func_name);
    return 1;
  }
  else if(WEXITSTATUS(status)) {
    printf( "  %s: FAILED\n", func_name );
    return 1;
  }
  else {
    return 0;
  }

}

/***************************************************************************************
 *                            Function to handle failed tests
 ***************************************************************************************/

// use a function rather than substituting FLAG_ERROR directly
// so we have a convenient place to set a break point
inline void flag_error() 
  { FLAG_ERROR; }

/***************************************************************************************
 *                            CHECK_EQUAL implementations
 ***************************************************************************************/

// Common implementatation for most types
#define EQUAL_TEST_IMPL( TEST, TYPE ) if( !(TEST) ) { \
  printf( "Equality Test Failed: %s == %s\n", sA, sB ); \
  printf( "  at line %d of '%s'\n", line, file ); \
  printf( "  Expected value: %" #TYPE "\n", A ); \
  printf( "  Actual value:   %" #TYPE "\n", B ); \
  printf( "\n" ); \
  flag_error(); \
}

void check_equal( int A, int B, const char* sA, const char* sB, int line, const char* file )
  {  EQUAL_TEST_IMPL( A == B, d ) }

void check_equal( unsigned A, unsigned B, const char* sA, const char* sB, int line, const char* file )
  {  EQUAL_TEST_IMPL( A == B, u ) }

void check_equal( long A, long B, const char* sA, const char* sB, int line, const char* file )
  {  EQUAL_TEST_IMPL( A == B, ld ) }

void check_equal( unsigned long A, unsigned long B, const char* sA, const char* sB, int line, const char* file )
  {  EQUAL_TEST_IMPL( A == B, lu ) }

void check_equal( unsigned long long A, unsigned long long B, const char* sA, const char* sB, int line, const char* file )
  {  EQUAL_TEST_IMPL( A == B, llu ) }

void check_equal( long long A, long long B, const char* sA, const char* sB, int line, const char* file )
  {  EQUAL_TEST_IMPL( A == B, lld ) }

void check_equal( void* A, void* B, const char* sA, const char* sB, int line, const char* file )
  {  EQUAL_TEST_IMPL( A == B, p ) }

void check_equal( const char* A, const char* B, const char* sA, const char* sB, int line, const char* file )
  {  EQUAL_TEST_IMPL( !strcmp((A),(B)), s ) }

void check_equal( const std::string& A, const std::string& B, const char* sA, const char* sB, int line, const char* file )
  {  check_equal( A.c_str(), B.c_str(), sA, sB, line, file); }

void check_equal( float A, float B, float eps, const char* sA, const char* sB, int line, const char* file )
  {  EQUAL_TEST_IMPL( fabsf(A - B) <= eps, f ) }

void check_equal( double A, double B, double eps, const char* sA, const char* sB, int line, const char* file )
  {  EQUAL_TEST_IMPL( fabs(A - B) <= eps, f ) }

const char* mb_error_str( moab::ErrorCode err )
{
  switch (err) {
    case moab::MB_SUCCESS                 : return "Success";
    case moab::MB_INDEX_OUT_OF_RANGE      : return "Index Out of Range";
    case moab::MB_TYPE_OUT_OF_RANGE       : return "Type Out of Range";
    case moab::MB_MEMORY_ALLOCATION_FAILED: return "Memory Alloc. Failed";
    case moab::MB_ENTITY_NOT_FOUND        : return "Entity Not Found";
    case moab::MB_MULTIPLE_ENTITIES_FOUND : return "Multiple Entities Found";
    case moab::MB_TAG_NOT_FOUND           : return "Tag Not Found";
    case moab::MB_FILE_DOES_NOT_EXIST     : return "File Not Found";
    case moab::MB_FILE_WRITE_ERROR        : return "File Write Error";
    case moab::MB_NOT_IMPLEMENTED         : return "Not Implemented";
    case moab::MB_ALREADY_ALLOCATED       : return "Already Allocated";
    case moab::MB_VARIABLE_DATA_LENGTH    : return "Variable Data Length";
    case moab::MB_INVALID_SIZE            : return "Invalid Size";
    case moab::MB_UNSUPPORTED_OPERATION   : return "Unsupported Operation";
    case moab::MB_UNHANDLED_OPTION        : return "Unhandled Option";
    case moab::MB_STRUCTURED_MESH         : return "Structured Mesh";
    case moab::MB_FAILURE                 : return "Failure";
    default                         : return "(unknown)";
  }
}


// Special case for MBErrorCode, use mb_error_str() to print the 
// string name of the error code.
void check_equal( moab::ErrorCode A, moab::ErrorCode B, const char* sA, const char* sB, int line, const char* file )
{
  if (A == B)
    return;
  
  printf( "ErrorCode Test Failed: %s == %s\n", sA, sB ); 
  printf( "  at line %d of '%s'\n", line, file ); 
  printf( "  Expected value: %s (%d)\n", mb_error_str(A), (int)A ); 
  printf( "  Actual value:   %s (%d)\n", mb_error_str(B), (int)B ); 
  printf( "\n" ); 
  flag_error(); 
}

const char* mb_type_str( moab::EntityType type )
{
  switch(type) {
    case moab::MBVERTEX    : return "Vertex";
    case moab::MBEDGE      : return "Edge";
    case moab::MBTRI       : return "Triangle";
    case moab::MBQUAD      : return "Quadrilateral";
    case moab::MBPOLYGON   : return "Polygon";
    case moab::MBTET       : return "Tetrahedron";
    case moab::MBPYRAMID   : return "Pyramid";
    case moab::MBPRISM     : return "Prism (wedge)";
    case moab::MBKNIFE     : return "Knife";
    case moab::MBHEX       : return "Hexahedron";
    case moab::MBPOLYHEDRON: return "Polyhedron";
    case moab::MBENTITYSET : return "Entity (Mesh) Set";
    case moab::MBMAXTYPE   : return "(max type)";
    default          : return "(unknown)";
  }
}



void check_true( bool cond, const char* str, int line, const char* file )
{
  if( !cond ) { 
    printf( "Test Failed: %s\n", str ); 
    printf( "  at line %d of '%s'\n", line, file ); 
    printf( "\n" ); 
    flag_error(); 
  }
}

#ifdef __cplusplus

template <typename T>
void check_array_equal( const T* A, size_t A_size,
                        const T* B, size_t B_size, 
                        const char* sA, const char* sB, 
                        int line, const char* file )
{
  size_t i = 0;
  for (;;) {
    if (i == A_size && i == B_size)
      return; // equal
    else if (i == A_size || i == B_size)
      break; // differene lengths
    else if (A[i] != B[i])
      break;
    ++i;
  }
  
  std::cout << "Equality Test Failed: " << sA << " == " << sB << std::endl;
  std::cout << "  at line " << line << " of '" << file << "'" << std::endl;
  std::cout << "  Vectors differ at position " << i << std::endl;
  
    // print at most 10 values, roughly centered on the unequal one
  size_t count = 10, num_front_values = std::min(count/2,i);
  size_t max_len = std::max(A_size,B_size);
  if (i + count - num_front_values > max_len) {
    if (count > max_len) {
      num_front_values = i;
      count = max_len;
    }
    else {
      num_front_values = count - (max_len - i);
    }
  }
  
  std::cout << "  Expected: ";
  if (!A_size) {
    std::cout << "(empty)" << std::endl;
  }
  else {
    size_t j = i - num_front_values;
    size_t end = std::min(j + count, A_size);
    if (j) 
      std::cout << "... ";
    for (; j < end; ++j) {
      if (j == i)
        std::cout << '>' << A[j] << "< ";
      else
        std::cout << A[j] << " ";
    }
    if (end != A_size)
      std::cout << "...";
    std::cout << std::endl;
  }
  
  std::cout << "  Actual:   ";
  if (!B_size) {
    std::cout << "(empty)" << std::endl;
  }
  else {
    size_t j = i - num_front_values;
    size_t end = std::min(j + count, B_size);
    if (j) 
      std::cout << "... ";
    for (; j < end; ++j) {
      if (j == i)
        std::cout << '>' << B[j] << "< ";
      else
        std::cout << B[j] << " ";
    }
    if (end != B_size)
      std::cout << ", ...";
    std::cout << std::endl;
  }
  
  flag_error(); 
}
  
 
template <typename T>
void check_equal( const std::vector<T>& A, const std::vector<T>& B, 
                  const char* sA, const char* sB, 
                  int line, const char* file )
{
   if(A.empty() || B.empty()) {
    if(A.size() != B.size()) {
      std::cout << "Equality Test Failed: " << sA << " == " << sB << std::endl;
      std::cout << "  at line " << line << " of '" << file << "'" << std::endl;
      std::cout << "  Both are not empty " << std::endl;
    }
    return;
  }
  check_array_equal( &A[0], A.size(), &B[0], B.size(), sA, sB, line, file );
}

#ifdef MOAB_RANGE_HPP

void check_equal( const moab::Range& A, const moab::Range& B, const char* sA, const char* sB, int line, const char* file )
{
  if (A == B)
    return;
    
  std::cout << "moab::ErrorCode Test Failed: " << sA << " == " << sB << std::endl;
  std::cout << "  at line " << line << " of '" << file << "'" << std::endl;
  std::cout << "   Expected: " << A << std::endl;
  std::cout << "   Actual  : " << B << std::endl;
  std::cout << std::endl;
  flag_error();
}

#endif  /* ifdef MOAB_RANGE_HPP */
    
#endif /* ifdef __cplusplus */




