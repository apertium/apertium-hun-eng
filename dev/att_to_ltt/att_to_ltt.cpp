/**
 * Creates a transducer from an ATT file.
 *
 * @author Dávid Márk Nemeskey
 */
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <set>

#include <lttoolbox/alphabet.h>
#include <lttoolbox/transducer.h>
#include <lttoolbox/compression.h>

using namespace std;

/**
 * Converts transducers from AT&T text format to lt binary format.
 *
 * @note To ensure that the AT&T file is read correctly, set std::locale to an
 *       appropriate value. If your current locale is compatible with the
 *       encoding of the file, just set it by adding
 *       <tt>std::locale::global(locale(""));</tt> to your code.
 */
class AttReader {
public:
  /**
   * Reads the AT&T format file @p file_name. The transducer and the alphabet
   * are both cleared before reading the new file.
   */
  void read_att_file(char const* const file_name) {
    clear();

    wifstream infile(file_name);  // TODO: error checking
    wistringstream iss;
    wstring line;

    while (getline(infile, line)) {
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
        corr[from] = from;
      } else {
        /* This id already has a mapping: use the id in the transducer. */
        from = it->second;
      }

      /* Final state. */
      if (!(iss >> to >> upper >> lower)) {
        transducer.setFinal(from);
      } else {
        convert_hfst(upper);
        convert_hfst(lower);
        int tag = alphabet(symbol_code(upper), symbol_code(lower));

        /* Now with the target state: */
        map<int, int>::iterator it_to = corr.find(to);
        if (it_to != corr.end()) {
          /* We already know it, possibly by a different name: link them! */
          to = it_to->second;
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
  }

  /** Writes the transducer to @p file_name in lt binary format. */
  void write_lt_file(char const* const file_name) {
    FILE* output = fopen(file_name, "w");
    /* Non-multichar symbols. */
    Compression::wstring_write(wstring(letters.begin(), letters.end()), output);
    /* Multichar symbols. */
    alphabet.write(output);
    /* And now the FST. */
    Compression::multibyte_write(1, output);
    Compression::wstring_write(L"main@standard", output);
    transducer.write(output);
    fclose(output);
  }

  /**
   * Converts a transducer in an AT&T format text file to an lt binary.
   * @param[in] att_file the input file in AT&T format.
   * @param[in] lt_file the output file in lt binary format.
   */
  void convert_file(char const* const att_file, char const* const lt_file) {
    read_att_file(att_file);
    write_lt_file(lt_file);
  }

  /** Returns a copy of the transducer. */
  Transducer get_transducer() const {
    return transducer;
  }

  /** Returns a copy of the alphabet. */
  Alphabet get_alphabet() const {
    return alphabet;
  }

private:
  /** Clears the data associated with the current transducer. */
  void clear() {
    transducer.clear();
    alphabet = Alphabet();
    corr.clear();
    letters.clear();
  }

  /** 
   * Converts symbols like @0@ to epsilon, @_SPACE_@ to space, etc.
   * @todo Are there other special symbols? If so, add them, and maybe use a map
   *       for conversion?
   */
  void convert_hfst(wstring& symbol) {
    if (symbol == L"@0@") {
      symbol = L"";
    } else if (symbol == L"@_SPACE_@") {
      symbol = L" ";
    }
  }

  /**
   * Returns the code of the symbol in the alphabet. Run after convert_hfst has
   * run.
   *
   * Also adds all non-multicharacter symbols (letters) to the @p letters set.
   *
   * @return the code of the symbol, if @p symbol is multichar; its first (and
   *         only) character otherwise.
   */
  int symbol_code(wstring& symbol) {
    if (symbol.length() > 1) {
      alphabet.includeSymbol(symbol);
      return alphabet(symbol);
    } else if (symbol == L"") {
      return 0;
    } else if (symbol == L" " || symbol == L".") {
      return symbol[0];
    } else {
      letters.insert(symbol[0]);
      return symbol[0];
    }
  }

private:
  Alphabet alphabet;
  Transducer transducer;

  /** State id correspondance in the file and in the FST. */
  map<int, int> corr;
  /** All non-multicharacter symbols. */
  set<wchar_t> letters;
};

int main(int argc, char **argv) {
  locale::global(locale(""));
  AttReader().convert_file(argv[1], argv[2]);
}

