#ifndef FOMACG_RULE_APPLIER_H
#define FOMACG_RULE_APPLIER_H

/**
 * Loads the rules created by fomacg and applies them to sentences.
 *
 * The rules for a CG are stored in separate files, grouped by sections. The
 * file names are indexed with the section the rules were in the original .rle
 * file. There is also a DELIMITERS file, which matches any cohort that
 * represents a delimiter. The files each have a common prefix: the name of the
 * original grammar file without the extension (.rle or .rlx).
 *
 * The rule applier does not deal with the Apertium stream format directly; the
 * input must be converted to the fomacg format via the Converter class before
 * feeding it to methods in this class.
 *
 * @author Dávid Márk Nemeskey
 */

#include <string>
#include <vector>
#include <stdexcept>

#include "fomacg_common.h"

class RuleApplier {
public:
  /** Frees the resources assigned to the rule FSTs. */
  ~RuleApplier();

  /**
   * Static factory method.
   * @param[in] language the language -- the prefix of the grammar FSTs (the
   *                     name of the grammar file without the extension).
   * @param[in] directory the directory where the FST files are.
   * @throws std::invalid_argument if the directory doesn't exist.
   * @throws std::length_error if the section numbers are not contiguous.
   * @throws std::runtime_error if a miscellaneous error occurs. Also see
   *                            load_fst().
   * @todo Handle unicode file names!
   * @todo Decide if it should return a pointer.
   */
  static RuleApplier get(const std::string& language,
                         const std::string& directory=".")
    throw (std::invalid_argument, std::runtime_error, std::length_error);

  /** Tells if the @p cohort is a delimiter. */
  bool is_delimiter(const std::string cohort) const;

private:
  /** Private constructor. */
  RuleApplier(const std::string& language, const std::string& directory=".");
  /** Loads the FST files. */
  void load_files();

  const std::string& language;
  const std::string& directory;

  /* ==== fomacg stuff ==== */

  /** The rules by sections. */
  std::vector<FstVector> sections;
  /** The delimiters rule. */
  FstPair delimiters;
};

#endif

