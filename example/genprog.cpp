/* genprog.cpp
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

#include <septima/chordgraph.h>
#include <iostream>
#include <assert.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc < 3 || argc > 4) {
    std::cerr << "Error: wrong number of input arguments" << std::endl;
    return 1;
  }
  /* read source chord and destination chord from input arguments */
  Chord c1(argv[1]), c2(argv[2]);
  if (!c1.is_valid() || !c2.is_valid()) {
    std::cerr << "Error: invalid chord specification" << std::endl;
    return 1;
  }
  /* get number of paths to generate */
  int N = argc == 3 ? 10 : atoi(argv[3]);
  if (N <= 0) {
    std::cerr << "Error: invalid number of paths" << std::endl;
    return 1;
  }
  /* make list of all 51 seventh chords */
  std::vector<Chord> chords = Chord::all_seventh_chords();
  /* create chord graph with chromatic prepared transitions (no augmented sixths) */
  std::cerr << "Creating chord graph..." << std::endl;
  ChordGraph cg(chords, 7, Domain::usual(), PREPARE_GENERIC, false, false, 0, true);
  /* set weights for arcs */
  int n = cg.number_of_vertices();
  for (int i = 1; i <= n; ++i) {
    for (int j = 1; j <= n; ++j) {
      if (i == j || cg.arc(i, j) == NULL)
        continue;
      const std::set<Transition> &tr = cg.transitions(i, j);
      assert(!tr.empty());
      double min_ls = DBL_MAX;
      for (std::set<Transition>::const_iterator it = tr.begin(); it != tr.end(); ++it) {
        double ls = (1.0 + it->directional_vl_shift()) / it->vl_shift();
        if (min_ls > ls)
          min_ls = ls;
      }
      cg.set_weight(i, j, min_ls);
    }
  }
  /* find N shortest paths from c1 to c2 */
  std::cerr << "Finding " << N << " shortest paths from "
            << c1 << " to " << c2 << "..." << std::endl;
  std::vector<ivector> paths;
  int src = cg.find_vertex_by_chord(c1), dest = cg.find_vertex_by_chord(c2);
  cg.yen(src, dest, N, 0, 0, paths);
  /* output paths to stdout */
  std::cerr << "Found " << paths.size() << " paths" << std::endl;
  int k = 0;
  for (std::vector<ivector>::const_iterator it = paths.begin(); it != paths.end(); ++it) {
    std::cout << "Path #" << ++k << ": ";
    for (ivector::const_iterator jt = it->begin(); jt != it->end(); ++jt) {
      if (jt != it->begin())
        std::cout << " -> ";
      std::cout << cg.vertex2chord(*jt);
    }
    std::cout << " (cost: " << cg.path_weight(*it) << ")" << std::endl;
  }
  return 0;
}
