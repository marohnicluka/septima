/* main.cpp
 *
 * Copyright (c) 2020  Luka Marohnić
 *
 * This file is part of Septima.
 *
 * Septima is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Septima is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Septima.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "chordgraph.h"
#include "transitionnetwork.h"
#include <glpk.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>

static void show_usage(std::string name) {
    std::cerr << "Usage: " << name << " <task> [<option(s)>] CHORDS or FILE\n"
              << "Tasks:\n"
              << "  -h,--help\t\tShow this help message\n"
              << "  -cg,--chord-graph\tCreate chord graph from chords\n"
              << "  -v,--voicing\t\tOutput an optimal voicing for the given chord sequence\n"
              << "  -av,--all-voicings\tOutput all optimal voicings for the given chord sequence\n"
              << "Options:\n"
              << "  -c,--class\t\tSpecify class index for elementary transitions\n"
              << "  -dg,--degree\t\tSpecify degree of elementary transitions\n"
              << "  -aa,--allow-augmented\tAllow augmented realizations\n"
              << "  -d,--domain\t\tSpecify domain on the line of fifths\n"
              << "  -lf,--label-format\tSpecify format for chord graph labels\n"
              << "  -p,--preparation\tSpecify preparation scheme for elementary transitions\n"
              << "  -w,--weights\t\tSpecify weight parameters for voicing algorithm\n"
              << "  -q,--quiet\t\tDisable messages"
              << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        show_usage(argv[0]);
        return 1;
    }
    int task = 0, deg = 0, cls = 7;
    double w1 = 1.0, w2 = 1.75, w3 = 0.25;
    bool aug = false, verbose = true;
    PreparationScheme prep_scheme = NO_PREPARATION;
    std::string label_format = "symbol";
    std::string input_filename = "";
    Domain domain = Domain::usual();
    std::vector<Chord> chords;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (i == 1) { // parse task
            if (arg == "-h" || arg == "--help") {
                show_usage(argv[0]);
                return 0;
            } else if (arg == "-cg" || arg == "--chord-graph") {
                task = 1;
            } else if (arg == "-v" || arg == "--voicing") {
                task = 2;
            } else if (arg == "-av" || arg == "--all-voicings") {
                task = 3;
            } else {
                std::cerr << "Error: bad task specification" << std::endl;
                return 1;
            }
        } else { // parse options
            if (arg == "-c" || arg == "--class") {
                if (deg > 0) {
                    std::cerr << "Warning: --degree option is already specified" << std::endl;
                } else {
                    if (i + 1 < argc) {
                        cls = atoi(argv[++i]);
                        if (cls <= 0) {
                            std::cerr << "Error: bad class-index specification" << std::endl;
                            return 1;
                        }
                    } else {
                        std::cerr << "Error: --class option requires one argument" << std::endl;
                        return 1;
                    }
                }
            } else if (arg == "-dg" || arg == "--degree") {
                if (i + 1 < argc) {
                    deg = atoi(argv[++i]);
                    if (deg <= 0) {
                        std::cerr << "Error: bad degree specification" << std::endl;
                        return 1;
                    }
                    cls = 0;
                } else {
                    std::cerr << "Error: --class option requires one argument" << std::endl;
                    return 1;
                }
            } else if (arg == "-aa" || arg == "--allow-augmented") {
                aug = true;
            } else if (arg == "-d" || arg == "--domain") {
                if (i + 1 < argc) {
                    domain = Domain::parse(argv[++i]);
                    if (domain.empty()) {
                        std::cerr << "Error: bad domain specification" << std::endl;
                        return 1;
                    }
                } else {
                    std::cerr << "Error: --domain option requires one argument" << std::endl;
                    return 1;
                }
            } else if (arg == "-p" || arg == "--preparation") {
                if (i + 1 < argc) {
                    std::string val = argv[++i];
                    if (val == "none")
                        prep_scheme = NO_PREPARATION;
                    else if (val == "generic")
                        prep_scheme = PREPARE_GENERIC;
                    else if (val == "acoustic")
                        prep_scheme = PREPARE_ACOUSTIC;
                    else {
                        std::cerr << "Error: bad preparation scheme specification" << std::endl;
                        return 1;
                    }
                } else {
                    std::cerr << "Error: --preparation requires one argument" << std::endl;
                    return 1;
                }
            } else if (arg == "-lf" || arg == "--label-format") {
                if (i + 1 < argc) {
                    label_format = argv[++i];
                    if (label_format != "number" && label_format != "symbol" && label_format != "latex") {
                        std::cerr << "Error: bad label format specification" << std::endl;
                        return 1;
                    }
                } else {
                    std::cerr << "Error: --label-format requires one argument" << std::endl;
                    return 1;
                }
            } else if (arg == "-w" || arg == "--weights") {
                if (i + 3 < argc) {
                    if (argv[i+1][0] == '-' || argv[i+2][0] == '-' || argv[i+3][0] == '-') {
                        std::cerr << "Error: bad weight specification" << std::endl;
                        return 1;
                    }
                    w1 = atof(argv[++i]);
                    w2 = atof(argv[++i]);
                    w3 = atof(argv[++i]);
                } else {
                    std::cerr << "Error: --weights requires three arguments" << std::endl;
                    return 1;
                }
            } else if (arg == "-q" || arg == "--quiet") {
                verbose = false;
            } else { // parse chords or file
                for (; i < argc; ++i) {
                    Chord c(argv[i]);
                    if (!c.is_valid()) {
                        int j = 0;
                        for (; j < 5 && strcmp(argv[i], Chord::symbols[j]); ++j);
                        if (j == 5) {
                            if (chords.size() > 0) {
                                std::cerr << "Error: bad chord specification" << std::endl;
                                return 1;
                            }
                            input_filename = argv[i];
                            break;
                        } else {
                            std::vector<Chord> tmp;
                            switch (j) {
                            case 0:
                                tmp = Chord::dominant_seventh_chords();
                                break;
                            case 1:
                                tmp = Chord::half_diminished_seventh_chords();
                                break;
                            case 2:
                                tmp = Chord::minor_seventh_chords();
                                break;
                            case 3:
                                tmp = Chord::major_seventh_chords();
                                break;
                            case 4:
                                tmp = Chord::diminished_seventh_chords();
                                break;
                            default:
                                assert(false);
                            }
                            chords.insert(chords.end(), tmp.begin(), tmp.end());
                        }
                    } else chords.push_back(c);
                }
                break;
            }
        }
    }
    if (!input_filename.empty()) { // read chords from file
        assert(chords.empty());
        std::ifstream ifs;
        ifs.open(input_filename);
        if (!ifs.is_open()) {
            std::cerr << "Error: failed to open file '" << input_filename << "'" << std::endl;
            return 1;
        }
        std::string line;
        std::stringstream ss;
        while (std::getline(ifs, line)) {
            if (ss.str().size() > 0)
                ss << " ";
            ss << line;
        }
        std::string contents = ss.str();
        char *ct = new char[contents.size()+1];
        strcpy(ct, contents.c_str());
        const char *delim = ",; \t";
        char *symb = strtok(ct, delim);
        while (symb != NULL) {
            if (strlen(symb) > 0) {
                Chord c(symb);
                if (!c.is_valid()) {
                    std::cerr << "Error reading input file: '" << symb << "' is not a chord" << std::endl;
                    return 1;
                }
                chords.push_back(c);
            }
            symb = strtok(NULL, delim);
        }
        delete[] ct;
    }
    if (chords.empty()) {
        std::cerr << "Error: no chords found" << std::endl;
        return 1;
    }
    if (verbose)
        std::cerr << "Using GLPK " << glp_version() << std::endl;
    if (task == 1) {
        int ndup = 0;
        for (int i = 0; i < int(chords.size()); ++i) {
            for (int j = chords.size(); j-->i+1;) {
                if (chords[i] == chords[j]) {
                    chords.erase(chords.begin()+j);
                    ++ndup;
                }
            }
        }
        if (ndup > 0 && verbose)
            std::cerr << "Warning: removed " << ndup << " chord duplicates" << std::endl;
        if (verbose)
            std::cerr << "Creating chord graph for " << chords.size() << " chords..." << std::endl;
        ChordGraph cg(chords, cls, domain, prep_scheme, aug, label_format != "number", false, label_format == "latex");
        bool is_undirected = prep_scheme == NO_PREPARATION;
        int ne = cg.number_of_arcs();
        if (is_undirected) {
            assert(ne % 2 == 0);
            ne /= 2;
        }
        if (verbose)
            std::cerr << "Created a " << (is_undirected ? "" : "di") << "graph with "
                      << cg.number_of_vertices() << " vertices and "
                      << ne << (is_undirected ? " edges" : " arcs") << std::endl;
        cg.export_dot("-", is_undirected);
    } else if (task == 2) {
        if (verbose)
            std::cerr << "Finding optimal voicing for the sequence:" << std::endl
                      << chords << std::endl;
        std::vector<Chord> all_chords = Chord::all_seventh_chords();
        ChordGraph cg(all_chords, cls, domain, prep_scheme, aug, false, false, false);
        voicing v;
        int z0;
        if (cg.best_voicing(chords, z0, w1, w2, w3, v)) {
                std::cout << v;
                if (verbose) {
                    std::cerr << "Recommended key signature: "
                              << abs(z0) << (z0 == 0 ? " sharps/flats" : (z0 > 0 ? " sharps" : " flats"))
                              << std::endl;
                }
        } else std::cerr << "Error: the given progression does not match chord graph specifications" << std::endl;
    } else if (task == 3) {
        if (verbose)
            std::cerr << "Finding all optimal voicings for the sequence:" << std::endl
                      << chords << std::endl;
        std::vector<Chord> all_chords = Chord::all_seventh_chords();
        ChordGraph cg(all_chords, cls, domain, prep_scheme, aug, false, false, false);
        std::set<voicing> vs;
        int i = 0;
        if (cg.best_voicings(chords, w1, w2, w3, vs)) {
            std::cout << vs.size() << " voicing(s) found" << std::endl;
            for (std::set<voicing>::const_iterator it = vs.begin(); it != vs.end(); ++it) {
                std::cout << std::endl << "Voicing #" << ++i << ":" << std::endl;
                std::cout << *it;
            }
        } else std::cerr << "Error: the given progression does not match chord graph specifications" << std::endl;
    } else assert(false);
    return 0;
}
