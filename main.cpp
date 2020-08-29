#include <glpk.h>
#include "chordgraph.h"
#include "transitionnetwork.h"
#include <map>
#include <assert.h>

using namespace std;

const char *chopin_op6_no1[] = {
    "3:hdim7", "8:d7", "1:d7", "1:hdim7", "6:d7", "0:dim7",
    "11:hdim7", "4:d7", "1:dim7", "9:hdim7", "2:d7", "8:hdim7", "1:d7"
};

const char *chopin_op68_no4[] = {
    "7:d7", "7:hdim7", "0:d7", "0:m7", "5:d7", "5:hdim7", "10:d7", "10:m7",
    "1:dim7", "3:d7", "9:hdim7", "0:dim7", "2:d7", "8:hdim7", "2:dim7", "1:d7"
};

const char *chopin_op68_no4_beginning[] = {
    "7:d7", "7:hdim7", "6:d7", "0:hdim7", "5:d7", "5:hdim7", "2:dim7", "4:d7", "1:dim7"
};

const char *chopin_op7_no2[] = {
    "8:d7", "5:hdim7", "10:d7", "7:m7", "1:dim7", "9:m7", "0:dim7", "5:m7", "2:dim7", "4:hdim7", "9:d7"
};

const char *chopin_op30_no4[] = {
    "6:d7", "5:d7", "4:d7", "3:d7", "2:d7", "1:d7", "0:d7", "11:d7"
};

const char *chopin_op28[] = {
    "5:d7", "5:hdim7", "2:dim7", "4:d7", "4:m7", "1:dim7", "6:hdim7", "11:d7"
};

const char *chopin_op67_no2[] = {
    "7:d7", "0:d7", "5:d7", "10:d7", "3:d7", "8:d7", "1:d7", "6:d7", "0:d7", "5:d7"
};

const char *bach[] = {
    "5:d7", "2:d7", "7:d7", "4:d7", "9:d7", "5:d7"
};

const char *proba[] = {
    "7:m7", "1:d7", "1:hdim7", "2:maj7", "11:hdim7"
};

const char *wagner1[] = {
    "5:hdim7", "4:d7", "8:hdim7", "7:d7", "2:hdim7", "11:d7"
};

const char *wagner2[] = {
    "9:d7", "7:hdim7", "0:d7", "10:hdim7", "3:d7", "1:hdim7", "1:d7"
};

const char *rachmaninov[] = {
    "2:m7", "10:maj7", "4:hdim7", "0:d7", "9:m7", "2:m7", "7:m7"
};

const char *mozart[] {
    "1:dim7", "0:dim7", "2:dim7", "1:dim7", "0:dim7"
};

int main(int argc, char *argv[]) {
    cout << "GLPK version: " << glp_version() << endl;
    /*
    Chord c(0, MAJOR), d(2, MINOR);
    std::vector<Transition> tr = Transition::generate_proper(c, d);
    cout << " -- Transitions (" << tr.size() << "):" << endl;
    for (std::vector<Transition>::const_iterator it = tr.begin(); it != tr.end(); ++it) {
        cout << it->to_string(true) << endl;
    }
    */

    std::vector<Chord> chords(0);
    for (int i = 0; i < 3; ++i) chords.push_back(Chord(i, DIMINISHED));
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 12; ++i) {
            chords.push_back(Chord(i,j));
        }
    }
    std::map<int,int> var_map;
    std::vector<Transition> cls;
    int vlmax = 7, max_diam = 0;
    std::set<std::pair<int,std::set<int> > > pmn1, pmn2, pmn3;
    std::set<ipair> all_pmn;
    for (std::vector<Chord>::const_iterator it = chords.begin(); it != chords.end(); ++it) {
        for (std::vector<Chord>::const_iterator jt = chords.begin(); jt != chords.end(); ++jt) {
            if (it == jt)
                continue;
            std::set<int> type_set;
            type_set.insert(it->type());
            type_set.insert(jt->type());
            std::pair<int,std::set<int> > chord_pair = std::make_pair(Tone::modb(jt->root() - it->root(), 12), type_set);
            std::vector<Transition> cl = Transition::elementary_classes(*it, *jt, vlmax, NONE, 0, true);
            std::set<std::vector<Transition> > ecl = Transition::enharmonic_classes(cl);
            cl.clear();
            std::vector<Transition> tv;
            for (std::set<std::vector<Transition> >::const_iterator et = ecl.begin(); et != ecl.end(); ++et) {
                tv = *et;
                Transition::simplify_enharmonic_class(tv);
                cl.insert(cl.end(), tv.begin(), tv.end());
            }
            if (!cl.empty()) {
                bool mandatory_tristan = true;
                for (std::vector<Transition>::const_iterator kt = cl.begin(); mandatory_tristan && kt != cl.end(); ++kt) {
                    if (kt->augmented_count(true) == 0)
                        mandatory_tristan = false;
                }
                /*
                if (mandatory_tristan)
                    std::cout << "MANDATORY TRISTAN: " << it->to_string() << " -> " << jt->to_string() << std::endl;
                    */
            }
            std::set<ipair> mn = it->Pmn_relations(*jt);
            bool is_pmn = !mn.empty();
            if (is_pmn) {
                all_pmn.insert(mn.begin(), mn.end());
                if (!cl.empty())
                    pmn1.insert(chord_pair);
                pmn2.insert(chord_pair);
            }
            for (std::vector<Transition>::const_iterator kt = cl.begin(); kt != cl.end(); ++kt) {
                bool found = false;
                std::vector<Transition>::const_iterator st;
                for (st = cls.begin(); st != cls.end(); ++st) {
                    if (kt->is_structurally_equal(*st)
                            || kt->is_structurally_equal(st->retrograde())
                            ) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    //std::cout << "EQUIVALENT: " << it->to_string() << ", " << jt->to_string() << std::endl;
                    if (kt->is_closer_than(*st, 0)) {
                        cls.erase(st);
                        cls.push_back(*kt);
                    }
                } else {
                    cls.push_back(*kt);
                    if (!is_pmn) {
                        pmn3.insert(chord_pair);
                        std::cout << "NEW: " << kt->to_lily() << std::endl;
                    }
                }
            }
            //if (cl.size() != 2) continue;
            var_map[cl.size()]++;
            /*
            std::cout << "(" << it->to_string() << ", " << jt->to_string() << "): ";
            std::cout << "TRANSITIONS: " << std::endl;
            for (std::set<Transition>::const_iterator it = cl.begin(); it != cl.end(); ++it) {
                std::cout << it->to_string() << std::endl;
            }
            */
        }
    }
    std::sort(cls.begin(), cls.end());
    /*
    for (int i = 0; i + 1 < int(cls.size()); ++i) {
        if (!(cls[i] < cls[i+1]))
            std::cout << "BAD ";
    }
    */
    std::cout << std::endl;
    for (std::set<ipair>::const_iterator it = all_pmn.begin(); it != all_pmn.end(); ++it) {
        std::cout << "(" << it->first << "," << it->second << "), ";
    }
    std::cout << std::endl;
    std::cout << "Pmn: " << pmn1.size() << ", " << pmn2.size() - pmn1.size() << ", " << pmn3.size() << std::endl;
    std::cout << "var_map: " << var_map[0] << ", " << var_map[1] << ", " << var_map[2] << ", " << var_map[3] << ", " << var_map[4] << std::endl;
    std::cout << "total cls: " << cls.size() << std::endl;
    std::map<int,int> chord_map;
    int augm = 0;
    for (std::vector<Transition>::const_iterator it = cls.begin(); it != cls.end(); ++it) {
        /*
        std::cout << it->first().chord().to_string() << ", " << it->second().chord().to_string()
                  //<< ": " << it->to_string() << " --- " << it->lof_spread() << ", " << it->pitch_displacement()
                  //<< ", " << (it->voice_leading_inf_norm() < 7 ? "diatonic" : "chromatic")
                  << std::endl;
                  */
        chord_map[it->first().chord().type()]++;
        chord_map[it->second().chord().type()]++;
        augm += it->augmented_count();
    }
    std::cout << "AUG: " << augm << std::endl;
    for (std::map<int,int>::const_iterator it = chord_map.begin(); it != chord_map.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "LILY : ----------------- " << std::endl;
    const char *colors[] = { "black", "red", "darkgreen", "blue", "yellow", "cyan", "magenta" };
    std::map<int,int> vl_types;
    for (std::vector<Transition>::iterator it = cls.begin(); it != cls.end(); ++it) {
        int color = 0, v = it->degree();
        if (!it->acts_identically_on_pc_intersection()) std::cout << "INTERESTING: " << *it << std::endl;
        if (!it->is_structurally_equal(it->structural_inversion().retrograde()))
            continue;
        //if (v != vlmax) continue;
        //if (v <= 5 || v == 9) continue;
        //if (v < 7 && it->augmented_count() == 0) continue;
        /*
        if (it->vl_shift() > abs(it->directional_vl_shift()))
            std::cout << "Protupomak: " << v << ", " << it->augmented_count() << " -- " << it->to_string() << std::endl;
            */
        /*
        if (mn.empty() && v != 9)
            std::cout << it->to_lily(70, false, true) << " |" << std::endl;
            */
        int pr = 0;
        if (it->is_prepared_generic()) {
            pr = 1;
            /*
            if (it->retrograde().is_prepared_generic())
                pr = 2;
                */
        } else if (it->retrograde().is_prepared_generic()) {
            *it = it->retrograde();
            pr = 1;
        }
        if (v == 2)
            color = 1;
        else if (v == 5)
            color = 2;
        else if (v == 7)
            color = 3;
        else if (v == 9)
            color = 4;
        else if (v == 12)
            color = 5;
        else if (v == 14)
            color = 6;
        ++vl_types[color];
        //std::cout << "\\override NoteHead.color = #" << colors[color] << " ";
        //std::cout << it->to_lily(70, pr, true) << " |" << std::endl;
        if (it->diameter() > max_diam)
            max_diam = it->diameter();
    }
    std::cout << "END LILY --------------- " << std::endl << "DIAM: " << max_diam << std::endl;
    std::cout << "VL TYPES: ";
    for (std::map<int,int>::const_iterator it = vl_types.begin(); it != vl_types.end(); ++it) {
        std::cout << it->first << ":" << it->second << ", ";
    }
    std::cout << std::endl;
    std::set<ivector> patterns, pat;
    int total = 0, tonal = 0;
    for (std::vector<Chord>::const_iterator it = chords.begin(); it != chords.end(); ++it) {
        pat = Realization::lof_patterns(*it, total, tonal, Domain::usual());
        patterns.insert(pat.begin(), pat.end());
    }
    std::cout << "Pattern count: " << patterns.size() << std::endl;
    std::cout << "Percentage: " << tonal << ", " << total << ", " << (tonal * 100.0) / total << std::endl;
    /*
    for (std::set<ivector>::const_iterator it = patterns.begin(); it != patterns.end(); ++it) {
        ChordGraph::print_list(*it);
    }
    for (std::vector<Chord>::const_iterator it = chords.begin(); it != chords.end(); ++it) {
        std::vector<Realization> rv = Realization::generate(*it);
        cout << it->to_string() << ":  ";
        for (std::vector<Realization>::const_iterator jt = rv.begin(); jt != rv.end(); ++jt) {
            cout << jt->to_string() << ", ";
        }
        cout << std::endl;
    }
    */
    Domain usual_domain = Domain::usual();
    Domain test_domain;
    test_domain.insert_range(-7, -6);
    test_domain.insert_range(-4, -1);
    test_domain.insert(2);
    /*
    chords.clear();
    for (int i = 0; i < 12; ++i) chords.push_back(Chord(i, DOMINANT));
    */
    ChordGraph cg(chords, 7, usual_domain, NONE, true, true);

    //cg.export_dot("/home/luka/Documents/cg_d7_5_aug.dot");

    std::cout << "NUM_VERTICES: " << cg.number_of_vertices() << ", NUM_ARCS: " << cg.number_of_arcs() << std::endl;

    pathmap path_map;
    cg.all_shortest_paths(path_map);

    for (int i = 1; i <= int(chords.size()); ++i) {
        std::cout << "Chord: " << cg.vertex2chord(i) << " -- "
                  << "Betweenness: " << cg.betweenness_centrality(i, path_map) << ", "
                  << "Comm. betweenness: " << cg.communicability_betweenness_centrality(i) << ", "
                  //<< "Closeness: " << cg.closeness_centrality(i) << ", "
                  //<< "Katz: " << cg.katz_centrality(i, true) << ", "
                  //<< "Degree: " << cg.out_degree(i)
                  << std::endl;
    }

    pitchSpelling ps;
    int z0;
    const char* bach_wtk[] = {"11:m7", "4:m7", "9:m7", "2:d7", "4:d7"};
    const char* chopin_op28_no4[] = { "0:maj7", "11:d7", "0:dim7", "11:hdim7", "5:hdim7", "4:d7", "4:m7", "1:dim7", "9:m7", "6:hdim7", "0:dim7", "2:d7", "2:m7", "11:hdim7", "2:dim7" };
    const char* coltrane[] = { "11:maj7", "2:d7", "7:maj7", "10:d7", "3:maj7", "9:m7", "2:d7", "7:maj7", "10:d7", "3:maj7", "6:d7", "11:maj7", "5:m7", "10:d7", "3:maj7", "9:m7", "2:d7", "7:maj7", "1:m7", "6:d7", "11:maj7", "5:m7", "10:d7", "3:maj7", "1:d7", "6:d7" };
    if (cg.pitch_spelling(chopin_op68_no4, 16, z0, 1, 1.75, 0.25, ps)) {
        std::cout << "GRAVITY CENTER: " << z0 << std::endl;
        std::cout << "PATH:" << std::endl << ps << std::endl;
    } else std::cout << "Error: the given progression is not a walk" << std::endl;
    std::set<pitchSpelling> best_ps;
    if (cg.best_pitch_spellings(chopin_op67_no2, 10, 1, 1.75, 0.25, best_ps)) {
        std::cout << "---> NUMBER OF PS: " << best_ps.size() << std::endl;
        for (std::set<pitchSpelling>::const_iterator it = best_ps.begin(); it != best_ps.end(); ++it) {
            std::cout << "PITCH SPELLING:" << std::endl << *it << std::endl;
        }
    }
    /*
    std::map<int,int> spm, plen;
    std::vector<ivector> spaths;
    int total_spaths = 0;
    for (int i = 1; i <= int(chords.size()); ++i) {
        for (int j = 1; j <= int(chords.size()); ++j) {
            if (i == j) continue;
            spaths.clear();
            cg.shortest_paths(i, j, spaths);
            if (spaths.size()==22)
                cout << cg.vertex_data(i)->chord.to_string() << ", "
                     << cg.vertex_data(j)->chord.to_string() << endl;
            plen[spaths.front().size()]++;
            total_spaths+=spaths.size();
            spm[spaths.size()]++;
        }
    }
    cout << "Total short paths: " << total_spaths << endl;
    for (std::map<int,int>::const_iterator it = spm.begin(); it != spm.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    for (std::map<int,int>::const_iterator it = plen.begin(); it != plen.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    ivector path;
    for (int i = 0; i < 8; ++i) {
        path.push_back(cg.vertex_index(chopin_op28[i]));
    }
    int np;
    std::vector<double> wgh(4);
    wgh[0] = 5.0; wgh[1] = 2.5; wgh[2] = 0.5; wgh[3] = 1.0;
    std::vector<Realization> R0v = Realization::generate(Chord(5, DOMINANT), -15, 15);
    const Realization &R0 = R0v[1];
    cout << "R0: " << R0.to_string() << endl
         << "Building the transition network..." << endl;
    glp_graph *tn = cg.transition_network(path, R0, wgh, 1, np);
    if (tn == NULL) {
        cout << "Failed to find the specified walk!" << endl;
        return 1;
    }
    cout << "TN size: " << tn->nv << ", arcs: " << tn->na << ", paths: " << np << endl;
    ivector sources, sinks;
    Network::tn_get_sources_and_sinks(tn, sources, sinks);
    cout << "TN has " << sources.size() << " sources and " << sinks.size() << " sinks" << endl;
    double min_weight, cand_weight;
    ivector best_path, cand_path;
    for (ivector::const_iterator it = sources.begin(); it != sources.end(); ++it) {
        for (ivector::const_iterator jt = sinks.begin(); jt != sinks.end(); ++jt) {
            cand_path = Network::dijkstra(tn, *it, *jt);
            cand_weight = Network::tn_path_size(tn, cand_path);
            if (best_path.empty() || cand_weight < min_weight) {
                min_weight = cand_weight;
                best_path = cand_path;
            }
        }
    }
    cout << "Best path has weight " << min_weight/(wgh[0]+wgh[1]+wgh[2]+wgh[3]) << endl;
    Network::tn_print_path(tn, R0, best_path);
    glp_delete_graph(tn);
    //glp_graph *atg=cg.all_transitions_graph();
    //glp_delete_graph(atg);
    int src = cg.vertex_index("7:maj7"), dest = cg.vertex_index("5:hdim7");
    cout << "Source index: " << src << ", destination index: " << dest << endl;
    srand(time(NULL));
    cg.enable_all_arcs();
    cg.enable_all_vertices();
    cg.yen(src, dest, 0, 5, 5, paths);
    cout << "Found " << paths.size() << " paths" << endl;
    cg.find_fixed_length_paths(dest, src, 8, 200, paths);
    for (vector<ivector>::const_iterator it = paths.begin(); it != paths.end(); ++it) {
        Progression prog = cg.inefficient_realization(*it);
        cout << "VLS: " << prog.vls() << endl;
        if (vls <= (pvls = prog.vls())) {
            if (pvls > vls) {
                best_progs.clear();
                vls = pvls;
            }
            best_progs.push_back(prog);
        }
    }
    for (vector<Progression>::const_iterator it = best_progs.begin(); it != best_progs.end(); ++it) {
        cout << it->to_lily(2, 2) << endl;
    }
    cout << endl << "Printed " << best_progs.size() << " paths" << endl;
    */
    return 0;
}
