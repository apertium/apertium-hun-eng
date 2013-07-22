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

RuleApplier::RuleApplier(const std::string& language, const std::string& directory)
    : language(language), directory(directory) {}

RuleApplier RuleApplier::get(const std::string& language,
                             const std::string& directory)
    throw (std::invalid_argument, std::runtime_error, std::length_error) {
  RuleApplier ra(language, directory);
  ra.load_files();
  return ra;
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
  section_regex = pcre_compile(".*_(\\d+)[.]fst", 0, &error, &erroffset, NULL); 
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
        /* ... to avoid problems with regex-special characters. */
        int ret = pcre_exec(section_regex, NULL,
                            ent->d_name, strlen(ent->d_name), 0, 0,
                            ovector, 6);
        if (ret >= 0) {
          size_t section;
          std::istringstream(std::string(ent->d_name).substr(
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
