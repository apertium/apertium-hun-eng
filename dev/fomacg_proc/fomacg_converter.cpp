#include "fomacg_converter.h"

#include <cstdlib>

#include "fomacg_common.h"

std::string Converter::FAILED  = "";
std::wstring Converter::WFAILED = L"";
size_t Converter::UTF_8_LENGTH = 6;

Converter* Converter::get(const std::string& fst_file) {
  try {
    FstVector fsts = load_fsts(fst_file);
    if (fsts.size() != 2) {
      throw 3;  // TODO
    }
    return new Converter(fsts[0], fsts[1]);
  } catch (...) {
    // TODO: error reporting
    return NULL;
  }
}

Converter::Converter(FstPair a2f, FstPair f2a)
    : a2f(a2f), f2a(f2a), utf_8_input(NULL), woutput(NULL), buffer_length(0) {
  ensure_buffers(1024);
}

void Converter::ensure_buffers(size_t len) {
  if (len > buffer_length) {
    delete[] utf_8_input;
    delete[] woutput;
    utf_8_input = new char[len * UTF_8_LENGTH + 1];
    woutput     = new wchar_t[len + 1];
  }
}

std::string Converter::apertium_to_fomacg(const std::wstring& str) {
  /* Longest UTF-8 character: 6 bytes (4 bytes since 2003, but what about
   * character pairs? */
  size_t wlen = str.length() * UTF_8_LENGTH;
  ensure_buffers(wlen);
  size_t clen = wcstombs(utf_8_input, str.c_str(), wlen);
  utf_8_input[clen] = 0;

  char* fomacg = apply_down(a2f.ah, utf_8_input);
  //printf("fomacg: >%s<\n", fomacg);
  if (fomacg != NULL) {
    return std::string(fomacg);
  } else {
    return FAILED;
  }
}

std::wstring Converter::fomacg_to_apertium(const std::string& str) {
  char* apertium = apply_up(f2a.ah, const_cast<char*>(str.c_str()));
  if (apertium != NULL) {
    size_t len = strlen(apertium);
    ensure_buffers(len);
    size_t clen = mbstowcs(woutput, apertium, len);
//    woutput[clen] = 0;
    return std::wstring(woutput, clen);
  } else {
    return WFAILED;
  }
}

Converter::~Converter() {
  a2f.cleanup();
  f2a.cleanup();
  delete utf_8_input;
  delete woutput;
}

