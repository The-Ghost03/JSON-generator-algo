#ifndef SCENARIO_H
#define SCENARIO_H

#include "../graph/graph.h"

/** Structure opaque scénario. */
typedef struct Scenario Scenario;

/** Charge un scénario (stub). */
Scenario *load_scenario(const char *name);

/** Applique-le (stub). */
void apply_scenario(Scenario *sc, Graph *g);

/** Libère-le (stub). */
void free_scenario(Scenario *sc);

#endif // SCENARIO_H
