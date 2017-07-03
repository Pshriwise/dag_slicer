

%module Dag_Slicer

#include dag_slicer.hpp
#include slicer.hpp
%include "typemaps.i"
%include "std_vector.i"
%include "std_string.i"

%template(VecDouble) std::vector<double>;
%template(VecVecdouble) std::vector< std::vector<double> >;
%template(VecInt) std::vector<int>;
%template(VecVecint) std::vector< std::vector<int> >;
%template(VecStr) std::vector<std::string>;

%inline %{

 class Dag_Slicer{
  
public:
  Dag_Slicer(std::string file_to_slice,
	     int ax, // slice axis
	     double coord, // coordinate of the slice
	     bool by_grp = false, // slice by group
	     bool ca = false);
  ~Dag_Slicer();
  
  //creation method
  int create_slice();
  
  //modify methods
  void rename_group(int group_global_id, std::string new_name);
  void write_file(std::string new_filename);

  // output data
  std::vector<std::string> group_names;
  std::vector<double> dum_pnts;
  std::vector< std::vector<double> > slice_x_pnts;
  std::vector< std::vector<double> > slice_y_pnts;
  std::vector<int> group_ids;
  std::vector< std::vector<int> > path_coding;

  // slice parameters  
  std::string filename;
  int axis;
  double coord;
  bool by_group;
  bool verbose;  
  bool debug;
  bool roam;

  // other
  std::string roam_warning;

};

%}
