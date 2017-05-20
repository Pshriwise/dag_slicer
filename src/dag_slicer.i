

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
  Dag_Slicer(std::string file_to_slice, int ax, double coord, bool by_grp = false, bool ca = false);
  ~Dag_Slicer();
  //parameters
  int axis;
  std::string filename;
  std::string roam_warning;
  double coord;
  bool by_group;
  bool verbose;
  //creation method
  int create_slice();
  //modify methods
  void rename_group(int group_global_id, std::string new_name);
  void write_file(std::string new_filename);
  //data
  std::vector<std::string> group_names;
  std::vector<double> dum_pnts;
  std::vector< std::vector<double> > slice_x_pnts;
  std::vector< std::vector<double> > slice_y_pnts;
  std::vector<int> group_ids;
  std::vector< std::vector<int> > path_coding;
  bool debug;
  bool roam;
};

%}
