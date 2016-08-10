#ifndef SLICER_OPTS_H
#define SLICER_OPTS_H

struct program_option_struct{
  bool verbose;
  bool debug;
};

extern struct program_option_struct opts;

#define OPT_VERBOSE (opts.verbose || opts.debug)
#define OPT_DEBUG   (opts.debug)

#endif /* SLICER_OPTS_H */
