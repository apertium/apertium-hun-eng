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
private:
  /** Which transducer(s) is the state in? */
  enum TransducerType {
    WORD,
    PUNCT,
    BOTH
  };

  /** The value in the @c *_corr maps: */
  struct Correlation {
    TransducerType type;
    int            state;

    Correlation() : type(BOTH), state(0) {}
    Correlation(TransducerType type, int state) : type(type), state(state) {}
  };

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
    bool first_line = true;  // First line -- see below
    set<int> finals;    // Store the _original_ id of the final states here

    while (getline(infile, line)) {
      iss.clear();
      iss.str(line);
      int from, to;
      wstring upper, lower;
      bool new_word_from = false;
      bool new_punct_from = false;

      if (!(iss >> from)) {
        continue;
      }

      /* First line: handle files where the first ID is not 0. */
      if (first_line) {
        word_corr[from] = Correlation(BOTH, word_fst.getInitial());
        punct_corr[from] = Correlation(BOTH, punct_fst.getInitial());
        first_line = false;
      }

      /* Is the source state new? */
      new_word_from = word_corr.find(from) == word_corr.end();
      new_punct_from = punct_corr.find(from) == punct_corr.end();

      /* Final state. */
      if (!(iss >> to >> upper >> lower)) {
        /* The old id. */
        finals.insert(from);
      } else {
        convert_hfst(upper);
        convert_hfst(lower);
        int tag = alphabet(symbol_code(upper), symbol_code(lower));

        /* If we are in exactly one of the transducers, we stay in it. */
        if (new_word_from ^ new_punct_from) {
          if (!new_word_from) {
            addTransduction(from, to, tag, WORD, word_fst, word_corr, new_word_from);
          } else {
            addTransduction(from, to, tag, PUNCT, punct_fst, punct_corr, new_punct_from);
          }
        } else {
          bool upper_word  = (upper.length() == 1 &&
                              letters.find(upper[0]) != letters.end());
          bool upper_punct = (upper.length() == 1 && iswpunct(upper[0]));
          if (upper_word) {
            addTransduction(from, to, tag, WORD, word_fst, word_corr, new_word_from);
          } else if (upper_punct) {
            addTransduction(from, to, tag, PUNCT, punct_fst, punct_corr, new_punct_from);
          } else {
            addTransduction(from, to, tag, BOTH, word_fst, word_corr, new_word_from);
            addTransduction(from, to, tag, BOTH, punct_fst, punct_corr, new_punct_from);
          }
        }
      }

      /* We don't read the weights, even if they are defined. */
    }

    /* Set the final state(s). */
    for (set<int>::const_iterator it = finals.begin(); it != finals.end();
        ++it) {
      map<int, Correlation>::const_iterator state = word_corr.find(*it);
      if (state != word_corr.end()) {
        word_fst.setFinal(state->second.state);
      } else {
        // TODO: report error
      }
      state = punct_corr.find(*it);
      if (state != punct_corr.end()) {
        punct_fst.setFinal(state->second.state);
      } else {
        // TODO: report error
      }
    }

    infile.close();
  }

  void addTransduction(int from, int to, int tag, TransducerType type,
                       Transducer& transducer,
                       map<int, Correlation>& corr, bool new_from) {
    /* Is the target state new? */
    bool new_to = corr.find(to) == corr.end();

    if (new_from) {
      corr[from] = Correlation(type, transducer.size() + (new_to ? 1 : 0));
    }
    from = corr[from].state;

    if (!new_to) {
      /* We already know it, possibly by a different name: link them! */
      to = corr[to].state;
      transducer.linkStates(from, to, tag);
    } else {
      /* We haven't seen it yet: add a new state! */
      int target = transducer.insertNewSingleTransduction(tag, from);
      corr[to] = Correlation(WORD, target);
    }
  }

  /** Writes the transducer to @p file_name in lt binary format. */
  void write_lt_file(char const* const file_name) {
    FILE* output = fopen(file_name, "w");
    /* Non-multichar symbols. */
    Compression::wstring_write(wstring(letters.begin(), letters.end()), output);
    /* Multichar symbols. */
    alphabet.write(output);
    /* And now the FST. */
    Compression::multibyte_write(2, output);
    Compression::wstring_write(L"main@standard", output);
    word_fst.write(output);
    Compression::wstring_write(L"final@inconditional", output);
    punct_fst.write(output);
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

  /** Returns a copy of the word transducer. */
  Transducer get_word_fst() const {
    return word_fst;
  }

  /** Returns a copy of the punctuation transducer. */
  Transducer get_punct_fst() const {
    return punct_fst;
  }

  /** Returns a copy of the alphabet. */
  Alphabet get_alphabet() const {
    return alphabet;
  }

private:
  /** Clears the data associated with the current transducer. */
  void clear() {
    word_fst.clear();
    punct_fst.clear();
    alphabet = Alphabet();
    word_corr.clear();
    punct_corr.clear();
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
//    } else if (symbol == L" " || symbol == L".") {
    } else if (iswpunct(symbol[0]) || iswspace(symbol[0])) {
      return symbol[0];
    } else {
      letters.insert(symbol[0]);
      return symbol[0];
    }
  }


private:
  Alphabet alphabet;
  Transducer word_fst;
  Transducer punct_fst;

  /** State id correspondance in the file and in the FST. */
  map<int, Correlation> word_corr;
  map<int, Correlation> punct_corr;
  /** All non-multicharacter symbols. */
  set<wchar_t> letters;
};

int main(int argc, char **argv) {
  locale::global(locale(""));
  AttReader().convert_file(argv[1], argv[2]);
}

