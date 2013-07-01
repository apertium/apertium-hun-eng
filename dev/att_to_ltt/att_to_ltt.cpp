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
#include <map>

#include <lttoolbox/alphabet.h>
#include <lttoolbox/transducer.h>

using namespace std;

/** Converts symbols like @0@ to epsilon, @_SPACE_@ to space, etc. */
void convert_hfst(wstring& symbol) {
  if (symbol == L"@0@") {
    symbol = L"ε";
  } else if (symbol == L"@_SPACE_@") {
    symbol = L" ";
  }
}

/**
 * Returns the code of the symbol in alphabet. Run after convert_hfst has run.
 * @return the code of the symbol, if @p symbol is multichar; its first and only
 *         character otherwise.
 */
int symbol_code(wstring& symbol, Alphabet& alphabet) {
  if (symbol.length() > 1) {
    alphabet.includeSymbol(symbol);
    return alphabet(symbol);
  } else {
    return symbol[0];
  }
}

int main(int argc, char **argv) {
  locale::global(locale(""));

  Alphabet alphabet;
  Transducer transducer;

  wifstream infile(argv[1]);  // TODO: error checking
  wistringstream iss;
  wstring line;
  map<int, int> corr;  // state id correspondence in the file and in the FST

  while (std::getline(infile, line)) {
    iss.clear();
    iss.str(line);
    int from, to;
    wstring upper, lower;
    if (!(iss >> from)) {
      continue;
    }
    
    /* Get the id of the state from in the transducer we are building. */
    map<int, int>::iterator it = corr.find(from);
    if (it == corr.end()) {
      /* First time we see this id: register it. */
      corr[from] = from;  // TODO *it=?
    } else {
      /* This id already has a mapping: use the id in the transducer. */
      from = corr[from];  // TODO = *it
    }

    /* Final state. */
    if (!(iss >> to >> upper >> lower)) {
      transducer.setFinal(from);
    } else {
      convert_hfst(upper);
      convert_hfst(lower);
      int tag = alphabet(symbol_code(upper, alphabet),
                         symbol_code(lower, alphabet));

      /* Now with the target state: */
      map<int, int>::iterator it_to = corr.find(to);
      if (it_to != corr.end()) {
        /* We already know it, possibly by a different name: link them! */
        to = corr[to];
        transducer.linkStates(from, to, tag);
      } else {
        /* We haven't seen it yet: add a new state! */
        int target = transducer.insertNewSingleTransduction(tag, from);
        corr[to] = target;
      }
    }

    /* We don't read the weights, even if they are defined. */
  }
  infile.close();
//  for (map<int, int>::const_iterator it = corr.begin(); it != corr.end(); ++it) {
//    wcerr << it->first << "\t" << it->second << endl;
//  }
//  wcerr << transducer.size() << " - " << transducer.numberOfTransitions() << endl;
  transducer.show(alphabet, stdout);
  // TODO: save the transducer
}
