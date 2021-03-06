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

#include "src/chordgraph.h"
#include "src/transitionnetwork.h"
#include <glpk.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>

static void show_usage(std::string name) {
    std::cerr << "Usage: " << name << " <task> [<option(s)>] CHORDS or FILE\n"
              << "Tasks:\n"
              << " -h, --help               Show this help message\n"
              << " -t, --transitions        Generate transitions from the first seventh chord to the rest\n"
              << " -tc,--transition-classes Generate all structural classes of transitions between seventh chords\n"
              << " -ts,--transition-stats   Output voice-leading statistics for the given chords\n"
              << " -cg,--chord-graph        Create chord graph from chords\n"
              << " -v, --voicing            Output optimal voicing for the given chord sequence\n"
              << " -av,--all-voicings       Output all optimal voicings for the given chord sequence\n"
              << " -mn,--Pmn-relations      Output all (m,n) such that the given two chords are Pmn-related\n"
              << "Options:\n"
              << " -c, --class              Specify upper bound for voice-leading infinity norm\n"
              << " -dg,--degree             Specify degree of elementary transitions\n"
              << " -aa,--allow-augmented    Allow augmented realizations\n"
              << " -fa,--force-augmented    Spell first realization in a transition as augmented sixth\n"
              << " -nr,--no-respell         Do not respell augmented sixths\n"
              << " -ns,--no-simplification  Do not discard enharmonic equivalents with larger voice-leading L1 norm\n"
              << " -d, --domain             Specify domain on the line of fifths\n"
              << " -z, --tonal-center       Specify tonal center on the line of fifths\n"
              << " -lf,--label-format       Specify format for chord graph labels\n"
              << " -p, --preparation        Specify preparation scheme for elementary transitions\n"
              << " -w, --weights            Specify weight parameters for voicing algorithm\n"
              << " -wv,--worst-voicing      Output worst instead of best voicing\n"
              << " -vc,--vertex-centrality  Show centrality measure with each vertex of the chord graph\n"
              << " -ly,--lilypond           Output transitions and voicings in Lilypond code\n"
              << " -cs,--chord-symbols      Print chord symbols above realizations in Lilypond output\n"
              << " -q, --quiet              Suppress messages"
              << std::endl;
}

static void output_transitions(const std::vector<Transition> &trans, PreparationScheme prep_scheme, int lily, bool full_chord_names) {
    std::string tab = "\t\t";
    if (lily) {
        std::cout << "\\include \"lilypond-book-preamble.ly\"\n"
                  << "\\paper {\n\toddFooterMarkup = ##f\n\t#(include-special-characters)\n}\n"
                  << "\\score {\n";
        if (full_chord_names) {
            tab += "\t";
            std::cout << "\t<<\n\t\t\\new ChordNames \\chordmode{\n";
            for (std::vector<Transition>::const_iterator it = trans.begin(); it !=trans.end(); ++it) {
                std::cout << tab << "s1 " << it->second().chord().to_lily(1) << "\n";
            }
            std::cout << "\t\t}\n\t";
        }
        std::cout << "\t\\new Staff {\n"
                  << tab << "\\override Score.TimeSignature.stencil = ##f\n"
                  << tab << "\\override Score.BarNumber.stencil = ##f\n"
                  << tab << "\\time 2/1\n\t\t\\accidentalStyle modern\n";
        for (std::vector<Transition>::const_iterator it = trans.begin(); it != trans.end(); ++it) {
            std::cout << tab << it->to_lily(70, prep_scheme == PREPARE_GENERIC, lily == 2 && !full_chord_names)
                      << " |\n";
        }
        if (full_chord_names)
            std::cout << "\t";
        std::cout << "\t}\n";
        if (full_chord_names)
            std::cout << "\t>>\n";
        std::cout << "\t\\layout { indent = 0\\cm }\n}\n";
    } else std::cout << trans;
}

static void isolate_degree(std::vector<Transition> &trans, int deg) {
    if (deg > 0) {
        for (int i = trans.size(); i-->0;) {
            if (trans[i].degree() != deg)
                trans.erase(trans.begin() + i);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        show_usage(argv[0]);
        return 1;
    }
    int task = 0, deg = 0, cls = 7, z = 0, lily = 0;
    double w1 = 1.0, w2 = 1.75, w3 = 1.4;
    bool aug = false, faug = false, respell = true, verbose = true, cs = false, best = true, simp = true;
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
            } else if (arg == "-t"  || arg == "--transitions") {
                task = 4;
            } else if (arg == "-tc" || arg == "--transition-classes") {
                task = 5;
            } else if (arg == "-mn" || arg == "--Pmn-relations") {
                task = 6;
            } else if (arg == "-ts" || arg == "--transition-stats") {
                task = 7;
            } else {
                std::cerr << "Error: invalid task specification" << std::endl;
                return 1;
            }
        } else { // parse options
            if (arg == "-c" || arg == "--class") {
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
            } else if (arg == "-dg" || arg == "--degree") {
                if (i + 1 < argc) {
                    deg = atoi(argv[++i]);
                    if (deg <= 0) {
                        std::cerr << "Error: invalid degree specification, expected a positive integer"
                                  << std::endl;
                        return 1;
                    }
                } else {
                    std::cerr << "Error: --class option requires one argument" << std::endl;
                    return 1;
                }
            } else if (arg == "-aa" || arg == "--allow-augmented") {
                aug = true;
            } else if (arg == "-fa" || arg == "--force-augmented") {
                faug = true;
            } else if (arg == "-nr" || arg == "--no-respell") {
                respell = false;
            } else if (arg == "-ns" || arg == "--no-simplification") {
                simp = false;
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
                    else if (val == "classical")
                        prep_scheme = PREPARE_ACOUSTIC_NO_DOMINANT;
                    else {
                        std::cerr << "Error: invalid preparation scheme specification, expected either 'none', 'generic', 'acoustic', or 'classical'"
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
            } else if (arg == "-wv" || arg == "--worst-voicing") {
                best = false;
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
            if (line.front() == '#')
                continue;
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
    if (task == 1 || task == 4 || task == 5) { // delete chord duplicates
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
            std::cerr << "Warning: removed " << ndup << " chord duplicate(s)" << std::endl;
    }
    std::clock_t clock_start = clock();
    if (task == 1) { // create chord graph
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
            std::cerr << "Finding " << (best ? "optimal" : "worst") << " voicing for the sequence "
                      << chords << std::endl;
        std::vector<Chord> all_chords = Chord::all_seventh_chords();
        ChordGraph cg(all_chords, cls, domain, prep_scheme, aug, false, 0, false, false);
        voicing v;
        int z0;
        if (cg.find_voicing(chords, z0, w1, w2, w3, v, best)) {
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
        if (cg.find_voicings(chords, w1, w2, w3, vs)) {
            if (verbose)
                std::cerr << "Found " << vs.size() << " voicing(s)" << std::endl;
            for (std::set<voicing>::const_iterator it = vs.begin(); it != vs.end(); ++it) {
                std::cout << std::endl << "Voicing #" << ++i << ":" << std::endl;
                std::cout << *it;
            }
        } else std::cerr << "Error: the given progression does not match chord graph specifications" << std::endl;
    } else if (task == 4) { // generate elementary transitions from the given seventh chord to one of other chords
        if (chords.size() >= 2) {
            bool duo = chords.size() == 2;
            const Chord &c = chords.front();
            std::vector<Chord> rest(chords.begin()+1, chords.end());
            std::vector<Transition> trans;
            for (std::vector<Chord>::const_iterator it = chords.begin() + 1; it != chords.end(); ++it) {
                std::vector<Transition> tr = Transition::elementary_classes(c, *it, cls, prep_scheme, z, aug || faug);
                if (faug) {
                    for (int i = tr.size(); i-->0;) {
                        if (!tr[i].first().is_augmented_sixth())
                            tr.erase(tr.begin()+i);
                    }
                    if (!aug) {
                        for (int i = tr.size(); i-->0;) {
                            if (tr[i].second().is_augmented_sixth())
                                tr.erase(tr.begin()+i);
                        }
                    }
                }
                trans.insert(trans.end(), tr.begin(), tr.end());
            }
            Transition::simplify_enharmonic_classes(trans, respell, simp);
            isolate_degree(trans, deg);
            std::sort(trans.begin(), trans.end());
            if (trans.empty()) {
                if (verbose) {
                    if (duo)
                        std::cerr << "No transitions found between " << c << " and " << chords.back() << std::endl;
                    else
                        std::cerr << "No transitions found from " << c << " to one of " << rest << std::endl;
                }
            } else {
                if (verbose) {
                    std::cerr << "Found " << trans.size() << " transitions between " << c
                              << " and " << (duo ? "" : "one of ");
                    if (duo)
                        std::cerr << chords.back() << std::endl;
                    else
                        std::cerr << rest << std::endl;
                }
                output_transitions(trans, prep_scheme, lily, true);
            }
        } else std::cerr << "Error: task --transitions requires at least two distinct chords" << std::endl;
    } else if (task == 5) { // generate structural classes of elementary transitions
        if (chords.size() > 1) {
            std::vector<Transition> trans = Transition::elementary_types(chords, cls, prep_scheme, z, aug, respell, simp);
            isolate_degree(trans, deg);
            std::map<int,int> vl_types;
            for (std::vector<Transition>::const_iterator it = trans.begin(); it != trans.end(); ++it) {
                ++vl_types[prep_scheme == NO_PREPARATION ? abs(it->generic_vl_type()) : it->generic_vl_type()];
            }
            if (trans.empty()) {
                if (verbose)
                    std::cerr << "No transitions found for chords " << chords << std::endl;
            } else {
                if (verbose)
                    std::cerr << "Found " << trans.size() << " transition types for "
                              << chords.size() << " chords " << chords << std::endl;
                output_transitions(trans, prep_scheme, lily, false);
                if (verbose) {
                    std::cerr << "\nStatistics:" << std::endl;
                    for (std::map<int,int>::const_iterator it = vl_types.begin(); it != vl_types.end(); ++it) {
                        std::cerr << "Exactly " << abs(it->first) << " voice(s) move stepwise "
                                  << (prep_scheme != NO_PREPARATION ? (it->first < 0 ? "downwards " : "upwards ") : "")
                                  << "in " << it->second << " transitions" << std::endl;
                    }
                }
            }
        } else std::cerr << "Error: at least two chords must be specified" << std::endl;
    } else if (task == 6) {
        if (chords.size() == 2 && chords.front() != chords.back()) {
            const Chord &c1 = chords.front(), &c2 = chords.back();
            std::set<ipair> pmn = c1.Pmn_relations(c2);
            if (pmn.empty()) {
                if (verbose)
                    std::cerr << "Chords " << c1 << " and " << c2 << " are not Pmn-related" << std::endl;
            } else {
                if (verbose)
                    std::cerr << "Found " << pmn.size() << " Pmn-relations" << std::endl;
                for (std::set<ipair>::const_iterator it = pmn.begin(); it != pmn.end(); ++it) {
                    std::cout << *it << std::endl;
                }
            }
        } else {
            std::cerr << "Error: task -mn requires exactly two distinct chords" << std::endl;
            return (1);
        }
    } else if (task == 7) {
        if (verbose)
            std::cerr << "Computing transitions..." << std::endl;
        std::vector<Transition> trans, lst;
        for (std::vector<Chord>::const_iterator it = chords.begin(); it != chords.end(); ++it) {
            for (std::vector<Chord>::const_iterator jt = chords.begin(); jt != chords.end(); ++jt) {
                if (it == jt)
                    continue;
                lst = Transition::elementary_classes(*it, *jt, cls, prep_scheme, z, aug);
                Transition::simplify_enharmonic_classes(lst, respell, simp);
                trans.insert(trans.end(), lst.begin(), lst.end());
            }
        }
        int erased_count = 0;
        for (int i = 0; i < int(trans.size()); ++i) {
            for (int j = trans.size(); j-->i+1;) {
                if (trans[i].is_congruent(trans[j]) ||
                        (prep_scheme == NO_PREPARATION && trans[i].is_congruent(trans[j].retrograde()))) {
                    trans.erase(trans.begin() + j);
                    ++erased_count;
                }
            }
        }
        if (verbose && erased_count > 0)
            std::cerr << "Removed " << erased_count << " duplicates" << std::endl;
        int total = trans.size(), eff = 0, D, vls, vls_avg = 0, ct = 0, contrary = 0;
        double pde = 0.0;
        std::map<int,int> vl_map;
        std::map<ipair,int> vlp_map;
        ipair mn;
        for (std::vector<Transition>::const_iterator it = trans.begin(); it != trans.end(); ++it) {
            D = it->first().chord().vl_efficiency_metric(it->second().chord());
            vls = it->vl_shift();
            mn = it->mn_type();
            ++vl_map[vls];
            ++vlp_map[mn];
            vls_avg += vls;
            if (vls <= D)
                ++eff;
            else pde += (vls - D) / (double)D;
            if (it->acts_identically_on_pc_intersection())
                ++ct;
            if (it->directional_vl_shift() < it->vl_shift())
                ++contrary;
        }
        double eff_pc = (eff * 100.0) / (double)total;
        double exc = pde / (double)total;
        double vlsa = vls_avg / (double)total;
        std::cout << "Total transitions: " << total << "\n"
                  << "Efficient transitions: " << eff << " (" << eff_pc << "%)\n"
                  << "Average voice-leading shift: " << vlsa << " semitones\n"
                  << "Average relative excess: " << exc * 100.0 << "%\n"
                  << "Common tones are fixed in " << ct << " transitions\n"
                  << "Contrary motion occurs in " << contrary << " transitions\n"
                  << "Distribution by voice-leading shift:\n";
        for (std::map<int,int>::const_iterator it = vl_map.begin(); it != vl_map.end(); ++it) {
            std::cout << it->first << ": " << it->second << "\n";
        }
        std::cout << "Distribution over mn-pair types:\n";
        for (std::map<ipair,int>::const_iterator it = vlp_map.begin(); it != vlp_map.end(); ++it) {
            std::cout << it->first << ": " << it->second << "\n";
        }
        if (verbose)
            std::cerr << "Done." << std::endl;
    } else assert(false);
    std::clock_t clock_end = clock();
    double elapsed_secs = double(clock_end - clock_start) / CLOCKS_PER_SEC;
    if (verbose)
        std::cerr << "Time elapsed: " << elapsed_secs << " seconds" << std::endl;
    return 0;
}
