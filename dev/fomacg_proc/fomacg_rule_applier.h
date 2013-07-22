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
 * @author Dávid Márk Nemeskey
 */

#include <string>

class RuleApplier {
public:
  /**
   * Constructor.
   * @param[in] language the language -- the prefix of the grammar FSTs (the
   *                     name of the grammar file without the extension).
   * @param[in] directory the directory where the FST files are.
   */
  RuleApplier(const std::string& language, const std::string& directory=".");

  /** Reads a cohort from @c ins. */
  std::wstring read_cohort();

private:
  /** Loads the FST files. */
  bool load_files(const std::string& language, const std::string& directory);
};

#endif

