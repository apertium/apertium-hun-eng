/**
 * Creates a transducer from an ATT file.
 *
 * @author Dávid Márk Nemeskey
 */
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <lttoolbox/alphabet.h>
#include <lttoolbox/transducer.h>

using namespace std;

int main(int argc, char **argv) {
  locale::global(locale(""));

  Alphabet alphabet;
  Transducer transducer;

//  int tag2 = alphabet(L'a', L'b');
//  transducer.setFinal(transducer.insertSingleTransduction(tag2, 0));
//  transducer.show(alphabet);

  wifstream infile(argv[1]);  // TODO: error checking
  wistringstream iss;
  std::wstring line;
  while (std::getline(infile, line)) {
    iss.str(line);
    int from, to;
    wchar_t upper, lower;
    if (!(iss >> from)) {
      continue;
    }
    /* Final state. */
    if (!(iss >> to >> upper >> lower)) {
      transducer.setFinal(from);
    } else {
      int tag = alphabet(upper, lower);
      transducer.insertSingleTransduction(tag, from);
    }
  }
  infile.close();
//  cout << transducer.size() << " - " << transducer.numberOfTransitions() << endl;
  transducer.show(alphabet, stdout);
}
