/* septima.cpp
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

#include "lib/chordgraph.h"
#include "lib/transitionnetwork.h"
#include <glpk.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>

static void show_usage(std::string name) {
    std::cerr << "Usage: " << name << " <task> [<option(s)>] CHORDS or FILE\n"
              << "Tasks:\n"
              << " -h, --help               Show this help message\n"
              << " -t, --transitions        Generate transitions between two seventh chords\n"
              << " -tc,--transition-classes Generate all structural classes of transitions between seventh chords\n"
              << " -cg,--chord-graph        Create chord graph from chords\n"
              << " -v, --voicing            Output an optimal voicing for the given chord sequence\n"
              << " -av,--all-voicings       Output all optimal voicings for the given chord sequence\n"
              << "Options:\n"
              << " -c, --class              Specify upper bound for voice-leading infinity norm\n"
              << " -dg,--degree             Specify degree of elementary transitions\n"
              << " -aa,--allow-augmented    Allow augmented realizations\n"
              << " -d, --domain             Specify domain on the line of fifths\n"
              << " -z, --tonal-center       Specify tonal center on the line of fifths\n"
              << " -lf,--label-format       Specify format for chord graph labels\n"
              << " -p, --preparation        Specify preparation scheme for elementary transitions\n"
              << " -w, --weights            Specify weight parameters for voicing algorithm\n"
              << " -vc,--vertex-centrality  Show centrality measure with each vertex of the chord graph\n"
              << " -ly,--lilypond           Output transitions and voicings in Lilypond code\n"
              << " -cs,--chord-symbols      Print chord symbols above realizations in Lilypond output\n"
              << " -q, --quiet              Suppress messages"
              << std::endl;
}

static void output_transitions(const std::vector<Transition> &trans, PreparationScheme prep_scheme, int lily) {
    if (lily) {
        std::cout << "\\include \"lilypond-book-preamble.ly\"\n"
                  << "\\paper {\n\toddFooterMarkup = ##f\n\t#(include-special-characters)\n}\n"
                  << "\\score {\n"
                  << "\t\\new Staff {\n\t\t\\override Score.TimeSignature.stencil = ##f\n"
                  << "\t\t\\override Score.BarNumber.stencil = ##f\n"
                  << "\t\t\\time 2/1\n\t\\accidentalStyle modern\n";
        for (std::vector<Transition>::const_iterator it = trans.begin(); it != trans.end(); ++it) {
            std::cout << "\t\t" << it->to_lily(70, prep_scheme == PREPARE_GENERIC, lily == 2) << " |\n";
        }
        std::cout << "\t}\n\t\\layout { indent = 0\\cm }\n}\n";
    } else std::cout << trans;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        show_usage(argv[0]);
        return 1;
    }
    int task = 0, deg = 0, cls = 7, z = 0, lily = 0;
    double w1 = 1.0, w2 = 1.75, w3 = 0.25;
    bool aug = false, verbose = true, cs = false;
    PreparationScheme prep_scheme = NO_PREPARATION;
    std::string label_format = "symbol", vc_format = "none";
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
            } else if (arg == "-t" || arg == "--transitions") {
                task = 4;
            } else if (arg == "-tc" || arg == "--transition-classes") {
                task = 5;
            } else {
                std::cerr << "Error: invalid task specification" << std::endl;
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
                            std::cerr << "Error: invalid class-index specification, expected a positive integer"
                                      << std::endl;
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
                        std::cerr << "Error: invalid degree specification, expected a positive integer"
                                  << std::endl;
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
                        std::cerr << "Error: invalid domain specification, expected a comma-separated list of integers"
                                  << std::endl;
                        return 1;
                    }
                } else {
                    std::cerr << "Error: --domain option requires one argument" << std::endl;
                    return 1;
                }
            } else if (arg == "-z" || arg == "--tonal-center") {
                if (i + 1 < argc) {
                    std::string val = argv[++i];
                    if (val != "0") {
                        z = atoi(val.c_str());
                        if (z == 0) {
                            std::cerr << "Error: invalid tonal center specification, expected an integer"
                                      << std::endl;
                            return 1;
                        }
                    }
                } else {
                    std::cerr << "Error: --tonal-center requires one argument" << std::endl;
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
                        std::cerr << "Error: invalid preparation scheme specification, expected either 'none', 'generic', or 'acoustic'"
                                  << std::endl;
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
                        std::cerr << "Error: invalid label format specification, expected either 'symbol', 'number', or 'latex'"
                                  << std::endl;
                        return 1;
                    }
                } else {
                    std::cerr << "Error: --label-format requires one argument" << std::endl;
                    return 1;
                }
            } else if (arg == "-vc" || arg == "--vertex-centrality") {
                if (i + 1 < argc) {
                    vc_format = argv[++i];
                    if (vc_format != "none" && vc_format != "label" && vc_format != "color") {
                        std::cerr << "Error: invalid vertex centrality specifier, expected either 'none', 'label', or 'color'"
                                  << std::endl;
                        return 1;
                    }
                } else {
                    std::cerr << "Error: --vertex-centrality requires one argument" << std::endl;
                    return 1;
                }
            } else if (arg == "-w" || arg == "--weights") {
                if (i + 3 < argc) {
                    if (argv[i+1][0] == '-' || argv[i+2][0] == '-' || argv[i+3][0] == '-') {
                        std::cerr << "Error: invalid weight specification, expected floating-point values"
                                  << std::endl;
                        return 1;
                    }
                    w1 = atof(argv[++i]);
                    w2 = atof(argv[++i]);
                    w3 = atof(argv[++i]);
                    if (w1 < 0 || w2 < 0 || w3 < 0) {
                        std::cerr << "Error: invalid weight specification, expected nonnegative floating-point values"
                                  << std::endl;
                        return 1;
                    }
                } else {
                    std::cerr << "Error: --weights requires three arguments" << std::endl;
                    return 1;
                }
            } else if (arg == "-ly" || arg == "--lilypond") {
                lily = 1;
            } else if (arg == "-cs" || arg == "--chord-symbols") {
                cs = true;
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
                                std::cerr << "Error: invalid chord specification" << std::endl;
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
    if (lily && cs)
        lily = 2;
    if (verbose && task <= 3)
        std::cerr << "Using GLPK " << glp_version() << std::endl;
    if (task == 1) { // create chord graph
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
        int vc = vc_format == "none" ? 0 : (vc_format == "label" ? 1 : 2);
        ChordGraph cg(chords, cls, domain, prep_scheme, aug, label_format != "number", vc, false, label_format == "latex");
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
    } else if (task == 2) { // find optimal voicing
        if (verbose)
            std::cerr << "Finding optimal voicing for the sequence " << chords << std::endl;
        std::vector<Chord> all_chords = Chord::all_seventh_chords();
        ChordGraph cg(all_chords, cls, domain, prep_scheme, aug, false, 0, false, false);
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
    } else if (task == 3) { // find all optimal voicings
        if (verbose)
            std::cerr << "Finding all optimal voicings for the sequence " << chords << std::endl;
        std::vector<Chord> all_chords = Chord::all_seventh_chords();
        ChordGraph cg(all_chords, cls, domain, prep_scheme, aug, false, 0, false, false);
        std::set<voicing> vs;
        int i = 0;
        if (cg.best_voicings(chords, w1, w2, w3, vs)) {
            if (verbose)
                std::cerr << "Found " << vs.size() << " voicing(s)" << std::endl;
            for (std::set<voicing>::const_iterator it = vs.begin(); it != vs.end(); ++it) {
                std::cout << std::endl << "Voicing #" << ++i << ":" << std::endl;
                std::cout << *it;
            }
        } else std::cerr << "Error: the given progression does not match chord graph specifications" << std::endl;
    } else if (task == 4) { // generate elementary transitions between two seventh chords
        if (chords.size() == 2 && chords.front() != chords.back()) {
            const Chord &c1 = chords.front(), &c2 = chords.back();
            std::vector<Transition> trans = Transition::elementary_classes(c1, c2, cls, prep_scheme, z, aug);
            if (trans.empty()) {
                if (verbose)
                    std::cerr << "No transitions found between " << c1 << " and " << c2 << std::endl;
            } else {
                if (verbose)
                    std::cerr << "Found " << trans.size() << " transitions between " << c1 << " and " << c2 << std::endl;
                output_transitions(trans, prep_scheme, lily);
            }
        } else std::cerr << "Error: task --transitions requires exactly two mutually different chords, found "
                         << chords.size() << std::endl;
    } else if (task == 5) { // generate classes of elementary transitions
        if (chords.size() > 1) {
            std::vector<Transition> trans = Transition::elementary_types(chords, cls, prep_scheme, z, aug);
            if (trans.empty()) {
                if (verbose)
                    std::cerr << "No transitions found for chords " << chords << std::endl;
            } else {
                if (verbose)
                    std::cerr << "Found " << trans.size() << " transition types for "
                              << chords.size() << " chords " << chords << std::endl;
                output_transitions(trans, prep_scheme, lily);
            }
        } else std::cerr << "Error: at least two chords must be specified" << std::endl;
    } else assert(false);
    return 0;
}
