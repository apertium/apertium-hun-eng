#ifndef FOMACG_CONVERTER_H
#define FOMACG_CONVERTER_H

/**
 * Converts a cohort between the Apertium stream format and fomacg's format via
 * the transducer created by apertium_to_fomacg.foma. This includes a conversion
 * from wchar_t* to char* and back as well.
 *
 * @author Dávid Márk Nemeskey
 */

#include <string>

#include "fomacg_common.h"

class Converter {
public:
  /** Returns a new Converter instance from the specified FST file. */
  static Converter* get(const std::string& fst_file);
  /**
   * Converts a cohort from the Apertium stream format to fomacg's format via the
   * transducer created by apertium_to_fomacg.foma.
   *
   * @return the converted string. Not persistent, make a copy if needed.
   */
  std::string apertium_to_fomacg(const std::wstring& str);


  /**
   * Converts a cohort from fomacg's format back to the Apertium stream format by
   * inversely applying the transducer created by apertium_to_fomacg.foma.
   *
   * @return the converted string. Not persistent, make a copy if needed.
   */
  std::wstring fomacg_to_apertium(const std::string& str);

  ~Converter();

  /** Returned by apertium_to_fomacg, if the conversion fails. */
  static std::string FAILED;
  /** Returned by fomacg_to_apertium, if the conversion fails. */
  static std::wstring WFAILED;

private:
  Converter(FstPair fst);

  /**
   * Ensures that @c utf_8_input and @c woutput are at least characters long.
   */
  void ensure_buffers(size_t len);

  /** The converter FST. */
  FstPair fst;

  /** Buffer used by fomacg_to_apertium. */
  char* utf_8_input;
  /** Buffer used by apertium_to_fomacg. */
  wchar_t* woutput;
  /** Length of the two buffers. */
  size_t buffer_length;

  /**
   * The maximum number of bytes in a utf-8 character (4 bytes since 2003, but
   * what about character pairs? So it's 6).
   */
  static size_t UTF_8_LENGTH;
};

#endif
