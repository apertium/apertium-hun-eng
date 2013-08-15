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

class Converter;

class RuleApplier {
public:
  /** Frees the resources assigned to the rule FSTs. */
  ~RuleApplier();

  /**
   * Static factory method.
   * @param[in] fst_file the name of the file that contains the grammar FSTs.
   * @throws std::length_error if the section numbers are not contiguous.
   * @throws std::runtime_error if a miscellaneous error occurs. Also see
   *                            load_fst().
   * @todo Handle unicode file names!
   * @todo Decide if it should return a pointer.
   * @todo max_section
   */
  static RuleApplier get(Converter& converter, const std::string& fst_file)
    throw (std::invalid_argument, std::length_error);

  /** Tells if the @p cohort is a delimiter. */
  bool is_delimiter(const std::string& cohort) const;

  /**
   * Applies rules.
   * @param[out] result the transformed sentence that results from the rule
   *                    application. This is also the work area, so don't expect
   *                    the referenced variable to remain unchanged even if no
   *                    rules apply to the sentence.
   * @param[in] sentence the original sentence.
   * @return the number of rules applied.
   */
  size_t apply_rules(std::string& result, const std::string& sentence) const;
//  size_t apply_rules2(std::string& result, const std::string& sentence) const;
  size_t apply_rules3(std::string& result, const std::string& sentence) const;

private:
  /** Private constructor. */
  RuleApplier(Converter& converter, const std::string& fst_file);
  /** Loads the FST file. */
  void load_file();

  Converter& converter;
  std::string fst_file;

  /* ==== fomacg stuff ==== */

  /** The rules by sections. */
  std::vector<FstVector> sections;
  /** The delimiters rule. */
  FstPair delimiters;
  /** The >>> cohort. */
  static const std::string begin_cohort;
};

#endif

