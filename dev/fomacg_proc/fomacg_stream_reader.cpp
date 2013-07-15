#include "fomacg_stream_reader.h"

#include <sstream>
#include <iostream>

StreamReader::StreamReader(FILE* ins) : ins(ins) {}

std::wstring StreamReader::read_cohort() {
  wchar_t wc;
  std::wstringstream word_ss;
  bool in_cohort = false;
  bool escape = false;

  while ((wc = fgetwc(ins)) != WEOF) {
    if (!in_cohort) {             // between cohorts: skip everything
      if (wc == L'^') {
        in_cohort = true;
      }
    } else {                      // in a cohort
      if (escape) {
        word_ss << wc;
        escape = false;
      } else if (wc == L'\\') {
        escape = true;
      } else if (wc == L'$') {
        return word_ss.str();
      } else {
        if (!iswspace(wc)) {  // Might not be needed, or not everywhere
          word_ss << wc;
        }
      }
    }
  }  // while
  /* WEOF -- what to do, what to do? */
  return L"";
}

