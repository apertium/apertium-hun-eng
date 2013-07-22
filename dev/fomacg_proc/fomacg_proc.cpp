/**
 * Applies the transducers created by fomacg to the text.
 *
 * @author Dávid Márk Nemeskey
 */
#include <cstdlib>
#include <cstdio>
#include <string>
#include <iostream>
#include "fomacg_converter.h"
#include "fomacg_stream_reader.h"

void test_converter(Converter* conv) {
  std::string fomacg = conv->apertium_to_fomacg(
      L"^Volt/van<vbser><past>/volt<n><sg><nom>$^ebed/eb<n><sg><px2ss><nom>$^?/?<sent>$");
  if (fomacg != Converter::FAILED) {
    printf("fomacg: %s\n", fomacg.c_str());
    std::wstring apertium = conv->fomacg_to_apertium(fomacg);
    std::wcout << L"apertium: " << apertium << std::endl;
  } else {
    printf("NULL!\n");
  }
}

void test_reader(StreamReader* reader) {
  while (true) {
    std::wstring str = reader->read_cohort();
    if (str == L"") break;
    std::wcout << str << std::endl;
  }
}

int main(int argc, char* argv[]) {
  Converter* conv = Converter::get("apertium_to_fomacg.fst");
  if (conv == NULL) {
    perror("FST file could not be loaded.");
    exit(EXIT_FAILURE);
  }
  test_converter(conv);
  StreamReader* reader = new StreamReader(stdin);
  test_reader(reader);
  delete reader;
  delete conv;
}
