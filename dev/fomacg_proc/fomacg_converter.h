#ifndef FOMACG_CONVERTER_H
#define FOMACG_CONVERTER_H

/**
 * Converts a cohort between the Apertium stream format and fomacg's format via
 * the transducer created by apertium_to_fomacg.foma.
 *
 * @author Dávid Márk Nemeskey
 */

#include <stdbool.h>  // to avoid errors in fomalib.h
#include <string>
#include "fomalib.h"

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
  char* apertium_to_fomacg(const char* str);

  /**
   * Converts a cohort from fomacg's format back to the Apertium stream format by
   * inversely applying the transducer created by apertium_to_fomacg.foma.
   *
   * @return the converted string. Not persistent, make a copy if needed.
   */
  char* fomacg_to_apertium(const char* str);

  ~Converter();

private:
  /**
   * Loads the transducer in @p fst_file.
   * @return @c true, if the FST could be loaded; @c false otherwise.
   */
  bool load_fst(const std::string& fst_file);

  /** The converter FST. */
  struct fsm*          fst;
  struct apply_handle* ah;
};

#endif
