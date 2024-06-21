#include "../include/rand.h"
#include "../include/math.h"

/*
 * Codigo criado a partir do modelo de numeros pseudo aleatorios Itamaraca
 * e modificado para o contexto do SO
 * 
 * autor: Daniel Henrique Pereira
 * artigo: Itamaracá: A Novel Simple Way to Generate Pseudo-random Numbers
 * link: https://www.cambridge.org/engage/coe/article-details/61b410fadcbea24f839f0235
 */

static int s0;
static int s1;
static int s2;

int RAND_MAX = 4206911;

static int __frns(int x, int y) {
    return abs(RAND_MAX - (int)(abs(x - y) * 1.97));
}

inline void mrand(int max) {
    RAND_MAX = max;
}

void srand(int seed) {
    mrand(4206911);
    seed %= RAND_MAX;
    s0 = seed;
    s1 = (s0 * RAND_MAX - 1) % RAND_MAX;
    s2 = __frns(s0, s1);
}

int rand() {
    s0 = __frns(s2, s0);
    s1 = __frns(s0, s1);
    s2 = __frns(s0, s2);
    return s0;
}
