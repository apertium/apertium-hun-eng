/**
 * Loads the rules created by fomacg and applies them to sentences.
 *
 * The rules for a CG are stored in a separate directory, one rule a file; the
 * file names are indexed with the section and the line the rules were in the
 * original .rle file. There is also a DELIMITERS file, which matches any cohort
 * that represents a delimiter.
 *
 * @author Dávid Márk Nemeskey
 */

#ifndef FOMACG_STREAM_READER_H
#define FOMACG_STREAM_READER_H

#include <string>

class StreamReader {
public:
  /**
   * Constructor.
   * @param ins[in] the input stream.
   */
  StreamReader(FILE* ins);

  /** Reads a cohort from @c ins. */
  std::wstring read_cohort();

private:
  /** The input stream. */
  FILE* ins;
};

#endif

