#include "fomacg_converter.h"

Converter* Converter::get(const std::string& fst_file) {
  Converter* ret = new Converter();
  if (!ret->load_fst(fst_file)) {
    delete ret;
    ret = NULL;
  }
  return ret;
}

bool Converter::load_fst(const std::string& fst_file) {
  fst = fsm_read_binary_file(const_cast<char*>(fst_file.c_str()));
  if (fst == NULL) return false;
  ah = apply_init(fst);
  if (ah == NULL) {
    fsm_destroy(fst);
    return false;
  }
  return true;
}

char* Converter::apertium_to_fomacg(const char* str) {
  return apply_down(ah, const_cast<char*>(str));
}
char* Converter::fomacg_to_apertium(const char* str) {
  return apply_up(ah, const_cast<char*>(str));
}

Converter::~Converter() {
  if (ah != NULL) {
    apply_clear(ah);
    fsm_destroy(fst);
  }
}

