/**
 * Applies the transducers created by fomacg to the text.
 *
 * @author Dávid Márk Nemeskey
 */
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include "fomacg_converter.h"
#include "fomacg_stream_reader.h"

void test_converter(Converter* conv) {
  char* fomacg = conv->apertium_to_fomacg(
      "^Volt/van<vbser><past>/volt<n><sg><nom>$^ebed/eb<n><sg><px2ss><nom>$^?/?<sent>$");
  if (fomacg != NULL) {
    printf("fomacg: %s\n", fomacg);
    char* buff = new char[strlen(fomacg) + 1];
    sprintf(buff, "%s", fomacg);
    char* apertium = conv->fomacg_to_apertium(fomacg);
    printf("apertium: %s\n", apertium);
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
  StreamReader* reader = new StreamReader(stdin);
  test_reader(reader);
  delete reader;
  delete conv;
}
