/**
 * Converts the output of hunmorph to the Apertium stream format. Uses the
 * FST file created by kr_to_apertium.foma.
 *
 * Usage: cat <hunmorph output> | hunmorph_to_apertium <fst file> > <result file>
 *
 * This script does NOT expect the default ocamorph output format. Instead, run
 * ocamorph with the following parameters:
 *
 *   ./ocamorph --bin morphdb.hu/morphdb_hu.bin --tag_preamble ""
 *              --tag_sep $'\t' --blocking --guess Fallback
 *
 * (Obviously, the points here are the values of the tag_preamble and tag_sep
 * parameters.)
 *
 * @author Dávid Márk Nemeskey
 */
#include <stdbool.h>  // to avoid errors in fomalib.h
#include <cstdlib>    // EXIT_FAILURE
#include <cstring>
#include <iostream>
#include <sstream>
#include <set>
#include <string>
#include "fomalib.h"

#ifndef HUN_2_AP_BUFF
#define HUN_2_AP_BUFF 4096
#endif

struct fsm* special;
struct fsm* general;
struct apply_handle* sh;
struct apply_handle* gh;

/** Cleans up the foma handles. */
void cleanup(bool handles_too=true) {
    if (handles_too) {
        apply_clear(sh);
        apply_clear(gh);
    }
    fsm_destroy(special);
    fsm_destroy(general);
}

int main(int argc, char* argv[]) {
    char* fst_file = argv[1];  // TODO: check

    fsm_read_binary_handle fsrh;

    if ((fsrh = fsm_read_binary_file_multiple_init(fst_file)) == NULL) {
        perror("FST file not found.");
        exit(EXIT_FAILURE);
    }

    special = fsm_read_binary_file_multiple(fsrh);  // TODO: error handling
    general = fsm_read_binary_file_multiple(fsrh);  // TODO: error handling

    sh = apply_init(special);
    gh = apply_init(general);
    // TODO: error handling

    char line[HUN_2_AP_BUFF];
    std::stringstream output;
    std::set<std::string> uniq;   // To uniq the readings -- hunmorph doesn't do that
    while (!std::cin.eof()) {
        std::cin.getline(line, HUN_2_AP_BUFF);
        if (std::cin.fail() && !std::cin.eof()) {
            perror("Input line too long; recompile with a longer HUN_2_AP_BUFF.");
            cleanup();
            exit(EXIT_FAILURE);
        }

        /* Apparently gcount() counts the \n at the end. */
        // TODO: Windows > 2?
        if (std::cin.gcount() > 1) {
            char* tab;
            tab = strtok(line, "\t");
            if (tab == NULL) {
                std::cout << "+?" << std::endl;
            } else {
                output << "^" << tab;

                bool good = true;
                while ((tab = strtok(NULL, "\t")) != NULL) {
                    char* analysis = apply_down(sh, tab);
                    if (analysis == NULL) {
                        analysis = apply_down(gh, tab);
                    }
                    if (analysis != NULL) {
                        /* 'Uniq' the readings. */
                        if (uniq.find(analysis) == uniq.end()) {
                            output << "/" << analysis;
                            uniq.insert(analysis);
                        }
                    } else {
                        std::cerr << "No analysis for " << tab << std::endl;
                        good = false;
                        break;
                    }
                }

                if (good) {
                    output << "$";
                    std::cout << output.str() << std::endl;
                } else {
                    std::cout << "+?" << std::endl;
                }

                output.str("");
                uniq.clear();
            }
        /* Empty lines. */
        } else if (!std::cin.eof()) {
            std::cout << std::endl;
        }
    }

    cleanup();
}
