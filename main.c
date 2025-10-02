#include "bitboard.h"
#include "et.h"
#include "perft.h"
#include "search.h"
#include "tt.h"
#include "uci.h"
#include "wrapper.h"

//#define VALIDATE

#define TEST_POSITION_DRAW_50 "k7/5R2/1K6/8/8/8/8/8 b - - 99 1"
#define TEST_POSITION_KPENDGAME "4k3/8/3K4/3P4/8/8/8/8 w - - 1 2"
#define TEST_POSITION_M5 "8/3K4/5k2/8/8/3Q4/8/8 w - - 7 9"
#define TEST_POSITION_M2 "r2qkbnr/ppp2ppp/2np4/4N3/2B1P3/2N5/PPPP1PPP/R1BbK2R w KQkq - 0 6"
#define TEST_POSITION_M1 "3r1r2/pQ2pp1N/3pk1p1/2pNb2n/P7/1P4P1/7P/R1B1KR2 w Q - 6 22"
#define TEST_SEE "1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - -"
#define TEST_SEE_2 "1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -"

int main(int argc, char *argv[])
{
    initNNUE("nn-62ef826d1a6d.nnue");
    initKeys();
    initAttacks();
    initTT(&GLOBAL_TT);
    initET(&GLOBAL_ET);

#ifdef VALIDATE
    moveGeneratorValidator();
#else
    uciLoop();
#endif
    return 0;
}
