#include "fomacg_common.h"

//#include <cstring>
#include <stdexcept>
//#include <iostream>

FstPair::FstPair() : fst(NULL), ah(NULL) {}
FstPair::FstPair(struct fsm* fst, struct apply_handle* ah) : fst(fst), ah(ah) {}

void FstPair::cleanup() {
  if (ah != NULL) {
    apply_clear(ah);
    fsm_destroy(fst);
  }
}

FstPair load_fst(const std::string& fst_file)
    throw (std::invalid_argument, std::runtime_error) {
  struct fsm* fst = fsm_read_binary_file(const_cast<char*>(fst_file.c_str()));
  if (fst == NULL) {
    throw std::invalid_argument("Could not load FST from " + fst_file);
  }
  /* Should be sorted already by fomacg. */
  if (!fst->arcs_sorted_in) {
//    std::cerr << "Sorting " << fst->name << std::endl;
    fsm_sort_arcs(fst, 1);
  }
  struct apply_handle* ah = apply_init(fst);
  if (ah == NULL) {
    fsm_destroy(fst);
    throw std::runtime_error("Failed to initialize apply handle for FST " + fst_file);
  }
  return FstPair(fst, ah);
}

FstVector load_fsts(const std::string& fst_file) throw (std::invalid_argument) {
  FstVector ret;
  fsm_read_binary_handle fsrh;
  char* file_name = const_cast<char*>(fst_file.c_str());  // See foma bug #48
  if ((fsrh = fsm_read_binary_file_multiple_init(file_name)) == NULL) {
    throw std::invalid_argument("Could not open FST file " + fst_file);
  }

  struct fsm* net;
  while ((net = fsm_read_binary_file_multiple(fsrh)) != NULL) {
    /* Should be sorted already by fomacg. */
    if (!net->arcs_sorted_in) {
//      std::cerr << "Sorting " << net->name << std::endl;
      fsm_sort_arcs(net, 1);
    }
    struct apply_handle* ah = apply_init(net);
    if (ah == NULL) {
      fsm_destroy(net);
    } else {
      ret.push_back(FstPair(net, ah));
    }
//    if (strcmp(net->name, "Cond_1_198") == 0) {
//      apply_print_sigma(net);
//    }
  }
  return ret;
}

