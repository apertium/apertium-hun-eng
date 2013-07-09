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
#include <vector>

#include <lttoolbox/alphabet.h>
#include <lttoolbox/transducer.h>
#include <lttoolbox/compression.h>

#define UNDECIDED 0
#define WORD      1
#define PUNCT     2
#define BOTH      3

using namespace std;

/** Bitmask; 1 = WORD, 2 = PUNCT, 3 = BOTH. */
typedef unsigned int TransducerType;

namespace {
/** Splits a string into fields. */
vector<wstring>& split(const wstring& s, wchar_t delim, vector<wstring> &out) {
    wistringstream ss(s);
    wstring item;
    while (getline(ss, item, delim)) {
      out.push_back(item);
    }
    return out;
}

/** Converts a string to a number. Slow, but at this point I don't care. */
int convert(const wstring& s) {
  int ret;
  wistringstream ss(s);
  ss >> ret;
  return ret;
}
};

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
  /** Used in Node. */
  struct Transduction {
    int            to;
    wstring        upper;
    wstring        lower;
    int            tag;
    TransducerType type;

    Transduction(int to, wstring upper, wstring lower, int tag,
                 TransducerType type=UNDECIDED) :
      to(to), upper(upper), lower(lower), tag(tag), type(type) {}
  };

  /** A node in the transducer graph. */
  struct Node {
    int                  id;
    vector<Transduction> transductions;

    Node(int id) : id(id) {}
  };

public:
  /**
   * Reads the AT&T format file @p file_name. The transducer and the alphabet
   * are both cleared before reading the new file.
   */
  void read_att_file(char const* const file_name) {
    clear();

    wifstream infile(file_name);  // TODO: error checking
    vector<wstring> tokens;
    wstring line;
    bool first_line = true;       // First line -- see below

    while (getline(infile, line)) {
      tokens.clear();
      int from, to;
      wstring upper, lower;

      /* Empty line. */
      if (line.length() == 0) {
        continue;
      }
      split(line, L'\t', tokens);
      from = convert(tokens[0]);

      Node* source = get_node(from);
      /* First line: the initial state is of both types. */
      if (first_line) {
        starting_state = from;
        first_line = false;
      }

      /* Final state. */
      if (tokens.size() <= 2) {
        finals.insert(from);
      } else {
        to = convert(tokens[1]);
        upper = tokens[2];
        lower = tokens[3];
        convert_hfst(upper);
        convert_hfst(lower);
        int tag = alphabet(symbol_code(upper), symbol_code(lower));
        /* We don't read the weights, even if they are defined. */
        source->transductions.push_back(Transduction(to, upper, lower, tag));

        get_node(to);
      }
    }

    /* Classify the nodes of the graph. */
    map<int, TransducerType> classified;
    classify(starting_state, classified, false, BOTH);

    infile.close();
  }

  /** Extracts the sub-transducer made of states of type @p type. */
  Transducer extract_transducer(TransducerType type) {
    Transducer transducer;
    /* Correlation between the graph's state ids and those in the transducer. */
    map<int, int> corr;
    set<int> visited;

    corr[starting_state] = transducer.getInitial();
    _extract_transducer(type, starting_state, transducer, corr, visited);

    /* The final states. */
    for (set<int>::const_iterator f = finals.begin(); f != finals.end(); ++f) {
      if (corr.find(*f) != corr.end()) {
        transducer.setFinal(corr[*f]);
      }
    }

    return transducer;
  }

  /**
   * Recursively fills @p transducer (and @p corr) -- helper method called by
   * extract_transducer().
   */
  void _extract_transducer(TransducerType type, int from,
                           Transducer& transducer, map<int, int>& corr,
                           set<int>& visited) {
    if (visited.find(from) != visited.end()) {
      return;
    } else {
      visited.insert(from);
    }

    Node* source = get_node(from);

    /* Is the source state new? */
    bool new_from = corr.find(from) == corr.end();
    int from_t, to_t;

    for (vector<Transduction>::const_iterator it = source->transductions.begin();
         it != source->transductions.end(); ++it) {
      if ((it->type & type) != type) {
        continue;  // Not the right type
      }

      /* Is the target state new? */
      bool new_to = corr.find(it->to) == corr.end();

      if (new_from) {
        corr[from] = transducer.size() + (new_to ? 1 : 0);
      }
      from_t = corr[from];

      /* Now with the target state: */
      if (!new_to) {
        /* We already know it, possibly by a different name: link them! */
        to_t = corr[it->to];
        transducer.linkStates(from_t, to_t, it->tag);
      } else {
        /* We haven't seen it yet: add a new state! */
        to_t = transducer.insertNewSingleTransduction(it->tag, from_t);
        corr[it->to] = to_t;
      }
      _extract_transducer(type, it->to, transducer, corr, visited);
    }  // for
  }

  /**
   * Classifies the edges of the transducer graphs recursively. It works like
   * this:
   * - the type of the starting state is BOTH (already set)
   * - in case of an epsilon move, the type of the target state is the same as
   *   that of the source
   * - the first non-epsilon transition determines the type of the whole path
   * - it is also the time from which we begin filling the @p visited set.
   *
   * @param from the id of the source state.
   * @param visited the ids of states visited by this path.
   * @param path are we in a path?
   */
  void classify(int from, map<int, TransducerType>& visited, bool path,
                TransducerType type) {
    Node* source = get_node(from);
    if (visited.find(from) != visited.end()) {
      if (path && ( (visited[from] & type) == type) ) {
        return;
      }
    }

    if (path) {
      visited[from] |= type;
    }
    for (vector<Transduction>::iterator it = source->transductions.begin();
         it != source->transductions.end(); ++it) {
      bool next_path = path;
      int  next_type = type;
      bool first_transition = !path && it->upper != L"";
      if (first_transition) {
        /* First transition: we now know the type of the path! */
        bool upper_word  = (it->upper.length() == 1 &&
                            letters.find(it->upper[0]) != letters.end());
        bool upper_punct = (it->upper.length() == 1 && iswpunct(it->upper[0]));
        next_type = UNDECIDED;
        if (upper_word)  next_type |= WORD;
        if (upper_punct) next_type |= PUNCT;
        next_path = true;
      } else {
        /* Otherwise (not yet, already): target's type is the same as ours. */
        next_type = type;
      }
      it->type |= next_type;
      classify(it->to, visited, next_path, next_type);
    }  // for
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
    Transducer word_fst = extract_transducer(WORD);
    word_fst.write(output);
    Compression::wstring_write(L"final@inconditional", output);
    Transducer punct_fst = extract_transducer(PUNCT);
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

private:
  /** Clears the data associated with the current transducer. */
  void clear() {
    for (map<int, Node*>::const_iterator it = graph.begin(); it != graph.end();
        ++it) {
      delete it->second;
    }
    graph.clear();
    alphabet = Alphabet();
  }

  /**
   * Returns the Node that represents the state @id. If it does not exist,
   * creates it and inserts it into @c graph.
   */
  Node* get_node(int id) {
    Node* state;
    if (graph.find(id) != graph.end()) {
      state = graph[id];
    } else {
      state = new Node(id);
      graph[id] = state;
    }
    return state;
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
  int symbol_code(const wstring& symbol) {
    if (symbol.length() > 1) {
      alphabet.includeSymbol(symbol);
      return alphabet(symbol);
    } else if (symbol == L"") {
      return 0;
    } else if (iswpunct(symbol[0]) || iswspace(symbol[0])) {
      return symbol[0];
    } else {
      letters.insert(symbol[0]);
      return symbol[0];
    }
  }

private:
  /** Stores the transducer graph. */
  map<int, Node*> graph;
  /** The final state(s). */
  set<int> finals;
  /**
   * Id of the starting state. We assume it is the source state of the first
   * transduction in the file.
   */
  int starting_state;

  Alphabet alphabet;
  /** All non-multicharacter symbols. */
  set<wchar_t> letters;
};

int main(int argc, char **argv) {
  locale::global(locale(""));
  AttReader().convert_file(argv[1], argv[2]);
}

