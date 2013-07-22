#include "fomacg_rule_applier.h"
/** 
 * Note: For Windows, there is a dirent.h file on the web that duplicates the
 * interface of the Unix header implements them using wraps Windows API calls.
 */
#include <dirent.h>
#include <cstdio>

#include "pcre.h"

RuleApplier::RuleApplier(const std::string& language, const std::string& directory) {
  load_files(language, directory);
}

bool RuleApplier::load_files(const std::string& language,
                             const std::string& directory) {
  std::string delimiters_file = language + "_DELIMITERS.fst";
  pcre* section_regex;
  const char* error;
  int erroffset;
  section_regex = pcre_compile((language + "_\\d+[.]fst").c_str(), 0, &error, &erroffset, 0); 
  if (section_regex == NULL) {
    // TODO: error handling
    return false;
  }
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(directory.c_str())) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir(dir)) != NULL) {
      printf ("%s\n", ent->d_name);
    }
    closedir(dir);
  } else {
    /* could not open directory */
    // TODO: error handling
    perror ("");
    return false;
  }
}

int main(int argc, char* argv[]) {
  RuleApplier("hu");
}
