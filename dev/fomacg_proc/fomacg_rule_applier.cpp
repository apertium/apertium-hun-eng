#include "fomacg_rule_applier.h"
/** 
 * Note: For Windows, there is a dirent.h file on the web that duplicates the
 * interface of the Unix header implements them using wraps Windows API calls.
 */
#include <dirent.h>
#include <cstdio>
#include <cstring>

#include <sstream>

/*
 * TODO: change this to C++11 regex as soon as it is done (e.g. this project:
 * http://www.google-melange.com/gsoc/project/google/gsoc2013/tim_shen/56001 is
 * completed).
 */
#include "pcre.h"

#include "fomacg_converter.h"

const std::string RuleApplier::begin_cohort =
    std::string("$0$ \">>>\" #BOC# | #0# \">>>\" <>>>> | #EOC# ");

RuleApplier::RuleApplier(Converter& converter, const std::string& language, const std::string& directory)
    : converter(converter), language(language), directory(directory) {}

RuleApplier RuleApplier::get(Converter& converter,
                             const std::string& language,
                             const std::string& directory)
    throw (std::invalid_argument, std::runtime_error, std::length_error) {
  RuleApplier ra(converter, language, directory);
  ra.load_files();
  return ra;
}

bool RuleApplier::is_delimiter(const std::string& cohort) const {
  // TODO: get rid of the const_cast once the foma interface becomes sane
  return apply_down(delimiters.ah, const_cast<char*>(cohort.c_str())) != NULL;
}

size_t RuleApplier::apply_rules2(std::string& result,
                                 const std::string& sentence) const {
  size_t applied = 0;
  /* Add the >>> and <<< tags. >>> comes in a separate cohort, while <<< is
   * appended to the tag list of the last cohort. It comes before the "| #EOC# "
   * at the end of the sentence (length = 8).
   */
  result = begin_cohort + sentence.substr(0, sentence.length() - 8) + "<<<<> " +
           sentence.substr(sentence.length() - 8);
//  fprintf(stderr, "Input: \n[%s]\n", sentence.c_str());
  struct fsm* sentence_fsa = converter.fomacg_to_fsa(result);
  if (sentence_fsa == NULL) {
    fprintf(stderr, "Could not convert.\n");
    result = sentence;
    return 0;
  }
//  fsm_write_binary_file(sentence_fsa, "sentence.fst");

  while (true) {
Continue:
    for (size_t section = 0; section < sections.size(); section++) {
      for (size_t rule = 0; rule < sections[section].size(); rule++) {
//        fprintf(stderr, "Trying rule %s...\n", sections[section][rule].fst->name);
        struct fsm* result_fsa = fsm_epsilon_remove(fsm_lower(fsm_compose(
                                     fsm_copy(sentence_fsa),
                                     fsm_copy(sections[section][rule].fst))));
        if (sentence_fsa->statecount != result_fsa->statecount) {
//          fprintf(stderr, "Applied rule %s, result:\n[%s]\n",
//              sections[section][rule].fst->name, converter.fsa_to_fomacg(fsm_copy(result_fsa)).c_str());
//          fsm_write_binary_file(result_fsa, "result.fst");
          fsm_destroy(sentence_fsa);
          sentence_fsa = result_fsa;
          applied++;
          goto Continue;
        } else {
          fsm_destroy(result_fsa);
//          fprintf(stderr, "Couldn't do anything, result:\n%s\n",
//              converter.fsa_to_fomacg(fsm_copy(result_fsa)).c_str());
        }
      }  // for rule
    }  // for section
    break;
  }
  result = converter.fsa_to_fomacg(sentence_fsa);
  /* Return the resulting string without the >>> cohort and <<< tags. */
  result = result.erase(result.length() - 14, 6).substr(begin_cohort.length());
//  fprintf(stderr, "Output: \n[%s]\n", result.c_str());
  return applied;
}

size_t RuleApplier::apply_rules(std::string& result,
                                const std::string& sentence) const {
  size_t applied = 0;
  /* Add the >>> and <<< tags. >>> comes in a separate cohort, while <<< is
   * appended to the tag list of the last cohort. It comes before the "| #EOC# "
   * at the end of the sentence (length = 8).
   */
  result = begin_cohort + sentence.substr(0, sentence.length() - 8) + "<<<<> " +
           sentence.substr(sentence.length() - 8);
//  fprintf(stderr, "Input: \n%s\n", sentence.c_str());
  size_t old_length = result.length();

  while (true) {
Continue:
    for (size_t section = 0; section < sections.size(); section++) {
      for (size_t rule = 0; rule < sections[section].size(); rule++) {
//        fprintf(stderr, "Trying rule %s...\n", sections[section][rule].fst->name);
        char* fomacg_result = apply_down(sections[section][rule].ah,
                                         const_cast<char*>(result.c_str()));
        if (fomacg_result != NULL) {
          size_t new_length = strlen(fomacg_result);
          if (old_length != new_length) {
//          fprintf(stderr, "Applied rule %s, result:\n%s\n",
//              sections[section][rule].fst->name, fomacg_result);
            result = fomacg_result;
            old_length = new_length;
            applied++;
            goto Continue;
          } else {
//            fprintf(stderr, "Same: >>>%s<<<\n", sentence.c_str());
          }
        } else {
//          fprintf(stderr, "Couldn't do anything for >>>%s<<<\n", sentence.c_str());
        }
      }  // for rule
    }  // for section
    break;
  }
  /* Return the resulting string without the >>> cohort and <<< tags. */
  result = result.erase(result.length() - 14, 6).substr(begin_cohort.length());
//  fprintf(stderr, "Output: %s\n", result.c_str());
  return applied;
}

size_t RuleApplier::apply_rules3(std::string& result,
                                 const std::string& sentence) const {
  size_t applied = 0;
  /* Add the >>> and <<< tags. >>> comes in a separate cohort, while <<< is
   * appended to the tag list of the last cohort. It comes before the "| #EOC# "
   * at the end of the sentence (length = 8).
   */
  result = begin_cohort + sentence.substr(0, sentence.length() - 8) + "<<<<> " +
           sentence.substr(sentence.length() - 8);
//  fprintf(stderr, "Input: \n%s\n", sentence.c_str());

  while (true) {
Continue:
    for (size_t section = 0; section < sections.size(); section++) {
      for (size_t rule = 0; rule < sections[section].size(); rule +=2) {
//        fprintf(stderr, "Trying rule %s...\n", sections[section][rule].fst->name);
        char* fomacg_result = apply_down(sections[section][rule + 1].ah,
                                         const_cast<char*>(result.c_str()));
        if (fomacg_result != NULL) {
          fomacg_result = apply_down(sections[section][rule].ah,
                                     const_cast<char*>(result.c_str()));
//          fprintf(stderr, "Applied rule %s, result:\n%s\n",
//              sections[section][rule].fst->name, fomacg_result);
          result = fomacg_result;
          applied++;
          goto Continue;
        } else {
//          fprintf(stderr, "Couldn't do anything for >>>%s<<<\n", sentence.c_str());
        }
      }  // for rule
    }  // for section
    break;
  }
  /* Return the resulting string without the >>> cohort and <<< tags. */
  result = result.erase(result.length() - 14, 6).substr(begin_cohort.length());
//  fprintf(stderr, "Output: %s\n", result.c_str());
  return applied;
}

// TODO: logging?
void RuleApplier::load_files() {
  std::string delimiters_file = language + "_DELIMITERS.fst";
  /* PCRE stuff. */
  pcre* section_regex;
  const char* error;
  int erroffset;
  int ovector[6];

  /* The regex that extracts the section number from the file name. */
  section_regex = pcre_compile("^_(\\d+)[.]fst$", 0, &error, &erroffset, NULL); 
  if (section_regex == NULL) {
    throw std::runtime_error(error);
  }

  DIR *dir;
  struct dirent *ent;
  size_t num_sections = 0;
  if ((dir = opendir(directory.c_str())) != NULL) {
    /* Find the rule files in the directory. */
    while ((ent = readdir(dir)) != NULL) {
      if (!strcmp(ent->d_name, delimiters_file.c_str())) {
        try {
          delimiters = load_fst(directory + "/" + ent->d_name);
        } catch (std::exception& e) {
          pcre_free(section_regex);
          throw;
        }
      /* First a string check on the file name ... */
      } else if(!strncmp(ent->d_name, language.c_str(), language.size())) {
        char* section_part = ent->d_name + language.size();
        /* ... to avoid problems with regex-special characters. */
        int ret = pcre_exec(section_regex, NULL,
                            section_part, strlen(section_part), 0, 0,
                            ovector, 6);
        if (ret >= 0) {
          size_t section;
          std::istringstream(std::string(section_part).substr(
                ovector[2], ovector[3] - ovector[2])) >> section;
          if (section > sections.size()) sections.resize(section);
          try {
            sections[section - 1] = load_fsts(directory + "/" + ent->d_name);
          } catch (std::exception& e) {
            pcre_free(section_regex);
            throw;
          }
          num_sections++;
        }
      }
    }
    closedir(dir);
    pcre_free(section_regex);

    /* One last check: are the section numbers contiguous? */
    if (num_sections != sections.size()) {
      throw std::length_error("Section numbers are not contiguous. ");
    }
  } else {
    /* could not open directory */
    pcre_free(section_regex);
    throw std::invalid_argument("Directory " + directory +
                                " could not be opened.");
  }
}

RuleApplier::~RuleApplier() {
  delimiters.cleanup();
  for (size_t i = 0; i < sections.size(); i++) {
    for (size_t j = 0; j < sections[i].size(); j++) {
      sections[i][j].cleanup();
    }
  }
}

//int main(int argc, char* argv[]) {
//  try {
//    RuleApplier::get(argv[1], ".");
//  } catch (std::exception& e) {
//    printf("Exception: %s\n", e.what());
//  }
//}
