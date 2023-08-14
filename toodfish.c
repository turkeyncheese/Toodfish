#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <windows.h>
#include "nnue_eval.h"

#define version " Version (2.0 + SF NNUE)"
#define U64 unsigned long long

#define empty_board "8/8/8/8/8/8/8/8 b - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "
#define repetitions "2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 40 "

enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

enum {P, N, B, R, Q, K, p, n, b, r, q, k};

enum {white, black, both};

enum {rook, bishop};

enum {wk = 1, wq = 2, bk = 4, bq = 8};

const char *square_to_coordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

char ascii_pieces[13] = "PNBRQKpnbrqk";

int char_pieces[] = {
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k
};

char promoted_pieces[] = {
    [Q] = 'q',
    [R] = 'r',
    [B] = 'b',
    [N] = 'n',
    [q] = 'q',
    [r] = 'r',
    [b] = 'b',
    [n] = 'n'
};

U64 bitboards[12];
U64 occupancies[3];

int side;
int enpassant = no_sq; 
int castle;

U64 hash_key;
U64 repetition_table[1000];

int repetition_index;
int ply;
int fifty;

int quit = 0;
int movestogo = 30;
int movetime = -1;
int time = -1;
int inc = 0;
int starttime = 0;
int stoptime = 0;
int timeset = 0;
int stopped = 0;

int get_time_ms() {
    return GetTickCount();
}

int input_waiting() {
    fd_set readfds;
    struct timeval tv;
    FD_ZERO (&readfds);
    FD_SET (fileno(stdin), &readfds);
    tv.tv_sec=0; tv.tv_usec=0;
    select(16, &readfds, 0, 0, &tv);

    return (FD_ISSET(fileno(stdin), &readfds));
}


void read_input() {
    int bytes;
    
    char input[256] = "", *endc;
    
    if (input_waiting()) {
        stopped = 1;
        
        do { 
            bytes=read(fileno(stdin), input, 256);
        } while (bytes < 0);
        
        endc = strchr(input,'\n');
        
        if (endc) *endc=0;
        
        if (strlen(input) > 0) {
            if (!strncmp(input, "quit", 4))
                quit = 1;
            
            else if (!strncmp(input, "stop", 4))
                quit = 1;
        }   
    }
}

static void communicate() {	
    if (timeset == 1 && get_time_ms() > stoptime) {
		stopped = 1;
	}
	
	read_input();
}

unsigned int random_state = 1804289383;

unsigned int get_random_U32_number() {
    unsigned int number = random_state;
    
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;
    
    random_state = number;
    
    return number;
}


U64 get_random_U64_number() {
    U64 n1, n2, n3, n4;
    
    n1 = (U64)(get_random_U32_number()) & 0xFFFF;
    n2 = (U64)(get_random_U32_number()) & 0xFFFF;
    n3 = (U64)(get_random_U32_number()) & 0xFFFF;
    n4 = (U64)(get_random_U32_number()) & 0xFFFF;
    
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

U64 generate_magic_number() {
    return get_random_U64_number() & get_random_U64_number() & get_random_U64_number();
}

#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

static inline int count_bits(U64 bitboard) {
    int count = 0;
    
    while (bitboard) {
        count++;
        bitboard &= bitboard - 1;
    }
    
    return count;
}

static inline int get_ls1b_index(U64 bitboard) {
    if (bitboard) {
        return count_bits((bitboard & -bitboard) - 1);
    }
    
    else
        return -1;
}

U64 piece_keys[12][64];
U64 enpassant_keys[64];
U64 castle_keys[16];
U64 side_key;

void init_random_keys() {    
    random_state = 1804289383;

    for (int piece = P; piece <= k; piece++)
        for (int square = 0; square < 64; square++)
            piece_keys[piece][square] = get_random_U64_number();
    
    for (int square = 0; square < 64; square++)    
        enpassant_keys[square] = get_random_U64_number();
    
    for (int index = 0; index < 16; index++)
        castle_keys[index] = get_random_U64_number();
    
    side_key = get_random_U64_number();
}

U64 generate_hash_key() {
    U64 final_key = 0ULL;
    U64 bitboard;
    
    for (int piece = P; piece <= k; piece++) {
        bitboard = bitboards[piece];
        
        while (bitboard) {
            int square = get_ls1b_index(bitboard);
            
            final_key ^= piece_keys[piece][square];
            
            pop_bit(bitboard, square);
        }
    }
    
    if (enpassant != no_sq)    
        final_key ^= enpassant_keys[enpassant];
    
    final_key ^= castle_keys[castle];

    if (side == black) final_key ^= side_key;
        
    return final_key;
}

void print_bitboard(U64 bitboard) {
    printf("\n");
    
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            
            if (!file)
                printf("  %d ", 8 - rank);
            
            printf(" %d", get_bit(bitboard, square) ? 1 : 0);
        }
        
        printf("\n");
    }
    
    printf("\n     a b c d e f g h\n\n");
    printf("     Bitboard: %llud\n\n", bitboard);
}

void print_board() {
    printf("\n");
    
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            
            if (!file)
                printf("  %d ", 8 - rank);
            
            int piece = -1;
            
            for (int bb_piece = P; bb_piece <= k; bb_piece++) {
                if (get_bit(bitboards[bb_piece], square))
                    piece = bb_piece;
            }
            
            printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
        }
        
        printf("\n");
    }
    
    printf("\n     a b c d e f g h\n\n");
    printf("     Side:     %s\n", !side ? "white" : "black");
    printf("     Enpassant:   %s\n", (enpassant != no_sq) ? square_to_coordinates[enpassant] : "no");
    printf("     Castling:  %c%c%c%c\n\n", (castle & wk) ? 'K' : '-',
                                           (castle & wq) ? 'Q' : '-',
                                           (castle & bk) ? 'k' : '-',
                                           (castle & bq) ? 'q' : '-');
    printf("     Hash key:  %llx\n", hash_key);
    printf("     Fifty move: %d\n\n", fifty);
}

void reset_board() {
    memset(bitboards, 0ULL, sizeof(bitboards));
    memset(occupancies, 0ULL, sizeof(occupancies));
    
    side = 0;
    enpassant = no_sq;
    castle = 0;
    
    repetition_index = 0;
    fifty = 0;
    
    memset(repetition_table, 0ULL, sizeof(repetition_table));
}

void parse_fen(char *fen) {
    reset_board();
    
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
                int piece = char_pieces[*fen];
                set_bit(bitboards[piece], square);
                
                fen++;
            }
            
            if (*fen >= '0' && *fen <= '9') {
                int offset = *fen - '0';
                int piece = -1;
                
                for (int bb_piece = P; bb_piece <= k; bb_piece++) {
                    if (get_bit(bitboards[bb_piece], square))
                        piece = bb_piece;
                }
                
                if (piece == -1)    
                    file--;
                
                file += offset;
                fen++;
            }
            
            if (*fen == '/')    
                fen++;
        }
    }
    
    fen++;
    
    (*fen == 'w') ? (side = white) : (side = black);
    
    fen += 2;
    
    while (*fen != ' ') {
        switch (*fen) {
            case 'K': castle |= wk; break;
            case 'Q': castle |= wq; break;
            case 'k': castle |= bk; break;
            case 'q': castle |= bq; break;
            case '-': break;
        }

        fen++;
    }
    
    fen++;
        
    if (*fen != '-') {
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');
        
        enpassant = rank * 8 + file;
    }
    
    else
        enpassant = no_sq;
    
    for (int piece = P; piece <= K; piece++)    
        occupancies[white] |= bitboards[piece];

    for (int piece = p; piece <= k; piece++)        
        occupancies[black] |= bitboards[piece];
    
    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];
    
    hash_key = generate_hash_key();
}

const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file = 9187201950435737471ULL;
const U64 not_hg_file = 4557430888798830399ULL;
const U64 not_ab_file = 18229723555195321596ULL;

const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6
};


const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12
};


U64 rook_magic_numbers[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL
};


U64 bishop_magic_numbers[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL
};


U64 pawn_attacks[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];

U64 bishop_masks[64];
U64 rook_masks[64];
U64 bishop_attacks[64][512];
U64 rook_attacks[64][4096];

U64 mask_pawn_attacks(int side, int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    
    set_bit(bitboard, square);

    if (!side) {
        if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
        if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
    }
    
    else {
        if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
        if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);    
    }
    
    return attacks;
}

U64 mask_knight_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    
    set_bit(bitboard, square);
    
    if ((bitboard >> 17) & not_h_file) attacks |= (bitboard >> 17);
    if ((bitboard >> 15) & not_a_file) attacks |= (bitboard >> 15);
    if ((bitboard >> 10) & not_hg_file) attacks |= (bitboard >> 10);
    if ((bitboard >> 6) & not_ab_file) attacks |= (bitboard >> 6);
    
    if ((bitboard << 17) & not_a_file) attacks |= (bitboard << 17);
    if ((bitboard << 15) & not_h_file) attacks |= (bitboard << 15);
    if ((bitboard << 10) & not_ab_file) attacks |= (bitboard << 10);
    if ((bitboard << 6) & not_hg_file) attacks |= (bitboard << 6);

    return attacks;
}

U64 mask_king_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    
    set_bit(bitboard, square);
    
    if (bitboard >> 8) attacks |= (bitboard >> 8);
    if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
    if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
    if ((bitboard >> 1) & not_h_file) attacks |= (bitboard >> 1);
    
    if (bitboard << 8) attacks |= (bitboard << 8);
    if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
    if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
    if ((bitboard << 1) & not_a_file) attacks |= (bitboard << 1);
    
    return attacks;
}

U64 mask_bishop_attacks(int square) {
    U64 attacks = 0ULL;
    
    int r, f;
    int tr = square / 8;
    int tf = square % 8;
    
    for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));
    
    return attacks;
}

U64 mask_rook_attacks(int square) {
    U64 attacks = 0ULL;
    
    int r, f;
    int tr = square / 8;
    int tf = square % 8;
    
    for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
    for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
    for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
    for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));
    
    return attacks;
}

U64 bishop_attacks_on_the_fly(int square, U64 block) {
    U64 attacks = 0ULL;
    
    int r, f;
    int tr = square / 8;
    int tf = square % 8;
    
    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    
    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    
    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    
    return attacks;
}

U64 rook_attacks_on_the_fly(int square, U64 block) {
    U64 attacks = 0ULL;
    
    int r, f;
    int tr = square / 8;
    int tf = square % 8;
    
    for (r = tr + 1; r <= 7; r++) {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block) break;
    }
    
    for (r = tr - 1; r >= 0; r--) {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block) break;
    }
    
    for (f = tf + 1; f <= 7; f++) {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block) break;
    }
    
    for (f = tf - 1; f >= 0; f--) {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block) break;
    }

    return attacks;
}

void init_non_sliding_attacks() {
    for (int square = 0; square < 64; square++) {
        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
        pawn_attacks[black][square] = mask_pawn_attacks(black, square);
        
        knight_attacks[square] = mask_knight_attacks(square);
        
        king_attacks[square] = mask_king_attacks(square);
    }
}

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
    U64 occupancy = 0ULL;
    
    for (int count = 0; count < bits_in_mask; count++) {
        int square = get_ls1b_index(attack_mask);
        
        pop_bit(attack_mask, square);

        if (index & (1 << count))            
            occupancy |= (1ULL << square);
    }
    
    return occupancy;
}

U64 find_magic_number(int square, int relevant_bits, int bishop) {
    U64 occupancies[4096];
    U64 attacks[4096];
    U64 used_attacks[4096];
    U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);
    
    int occupancy_indices = 1 << relevant_bits;
    
    for (int index = 0; index < occupancy_indices; index++) {
        occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);
        attacks[index] = bishop ? bishop_attacks_on_the_fly(square, occupancies[index]) :
                                    rook_attacks_on_the_fly(square, occupancies[index]);
    }

    for (int random_count = 0; random_count < 100000000; random_count++) {
        U64 magic_number = generate_magic_number();
        
        if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;
        
        memset(used_attacks, 0ULL, sizeof(used_attacks));
        
        int index, fail;
        
        for (index = 0, fail = 0; !fail && index < occupancy_indices; index++) {
            int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));
            
            if (used_attacks[magic_index] == 0ULL)
                used_attacks[magic_index] = attacks[index];
            
            else if (used_attacks[magic_index] != attacks[index])    
                fail = 1;
        }
        
        if (!fail)    
            return magic_number;
    }

    printf("  Magic number fails!\n");
    return 0ULL;
}

void init_magic_numbers() {
    for (int square = 0; square < 64; square++)
        rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook);

    for (int square = 0; square < 64; square++)
        bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);
}

void init_sliders_attacks(int bishop) {
    for (int square = 0; square < 64; square++) {
        bishop_masks[square] = mask_bishop_attacks(square);
        rook_masks[square] = mask_rook_attacks(square);
        
        U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];
        
        int relevant_bits_count = count_bits(attack_mask);
        int occupancy_indices = (1 << relevant_bits_count);
        
        for (int index = 0; index < occupancy_indices; index++) {
            if (bishop) {
                U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                
                int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);
                
                bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
            }
            
            else {
                U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                
                int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);
                
                rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);
            
            }
        }
    }
}

static inline U64 get_bishop_attacks(int square, U64 occupancy) {
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64 - bishop_relevant_bits[square];
    
    return bishop_attacks[square][occupancy];
}

static inline U64 get_rook_attacks(int square, U64 occupancy) {
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_numbers[square];
    occupancy >>= 64 - rook_relevant_bits[square];
    
    return rook_attacks[square][occupancy];
}

static inline U64 get_queen_attacks(int square, U64 occupancy) {
    U64 queen_attacks = 0ULL;
    U64 bishop_occupancy = occupancy;
    U64 rook_occupancy = occupancy;
    
    bishop_occupancy &= bishop_masks[square];
    bishop_occupancy *= bishop_magic_numbers[square];
    bishop_occupancy >>= 64 - bishop_relevant_bits[square];
    
    queen_attacks = bishop_attacks[square][bishop_occupancy];
    
    rook_occupancy &= rook_masks[square];
    rook_occupancy *= rook_magic_numbers[square];
    rook_occupancy >>= 64 - rook_relevant_bits[square];

    queen_attacks |= rook_attacks[square][rook_occupancy];
    
    return queen_attacks;
}

static inline int is_square_attacked(int square, int side) {
    if ((side == white) && (pawn_attacks[black][square] & bitboards[P])) return 1;
    if ((side == black) && (pawn_attacks[white][square] & bitboards[p])) return 1;
    if (knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n])) return 1;

    if (get_bishop_attacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b])) return 1;
    if (get_rook_attacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r])) return 1;    
    if (get_queen_attacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q])) return 1;

    if (king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k])) return 1;
    
    return 0;
}


void print_attacked_squares(int side) {
    printf("\n");
    
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            
            if (!file)
                printf("  %d ", 8 - rank);
            
            printf(" %d", is_square_attacked(square, side) ? 1 : 0);
        }
        
        printf("\n");
    }

    printf("\n     a b c d e f g h\n\n");
}

#define encode_move(source, target, piece, promoted, capture, double, enpassant, castling) \
    (source) |          \
    (target << 6) |     \
    (piece << 12) |     \
    (promoted << 16) |  \
    (capture << 20) |   \
    (double << 21) |    \
    (enpassant << 22) | \
    (castling << 23)    \
    
#define get_move_source(move) (move & 0x3f)
#define get_move_target(move) ((move & 0xfc0) >> 6)
#define get_move_piece(move) ((move & 0xf000) >> 12)
#define get_move_promoted(move) ((move & 0xf0000) >> 16)
#define get_move_capture(move) (move & 0x100000)
#define get_move_double(move) (move & 0x200000)
#define get_move_enpassant(move) (move & 0x400000)
#define get_move_castling(move) (move & 0x800000)

typedef struct {    
    int moves[256];
    int count;
} moves;

static inline void add_move(moves *move_list, int move) {
    move_list->moves[move_list->count] = move;
    move_list->count++;
}

void print_move(int move) {
    if (get_move_promoted(move))
        printf("%s%s%c", square_to_coordinates[get_move_source(move)],
                           square_to_coordinates[get_move_target(move)],
                           promoted_pieces[get_move_promoted(move)]);
    else
        printf("%s%s", square_to_coordinates[get_move_source(move)],
                           square_to_coordinates[get_move_target(move)]);
}

void print_move_list(moves *move_list) {
    if (!move_list->count) {
        printf("\n     No move in the move list!\n");
        return;
    }
    
    printf("\n     move    piece     capture   double    enpass    castling\n\n");
    
    for (int move_count = 0; move_count < move_list->count; move_count++) {
        int move = move_list->moves[move_count];

        printf("      %s%s%c   %c         %d         %d         %d         %d\n", square_to_coordinates[get_move_source(move)],
                                                                                square_to_coordinates[get_move_target(move)],
                                                                                get_move_promoted(move) ? promoted_pieces[get_move_promoted(move)] : ' ',
                                                                                ascii_pieces[get_move_piece(move)],
                                                                                get_move_capture(move) ? 1 : 0,
                                                                                get_move_double(move) ? 1 : 0,
                                                                                get_move_enpassant(move) ? 1 : 0,
                                                                                get_move_castling(move) ? 1 : 0);
    }
    
    printf("\n\n     Total number of moves: %d\n\n", move_list->count);
}

#define copy_board()                                                      \
    U64 bitboards_copy[12], occupancies_copy[3];                          \
    int side_copy, enpassant_copy, castle_copy, fifty_copy;               \
    memcpy(bitboards_copy, bitboards, 96);                                \
    memcpy(occupancies_copy, occupancies, 24);                            \
    side_copy = side, enpassant_copy = enpassant, castle_copy = castle;   \
    fifty_copy = fifty;                                                   \
    U64 hash_key_copy = hash_key;                                         \


#define take_back()                                                       \
    memcpy(bitboards, bitboards_copy, 96);                                \
    memcpy(occupancies, occupancies_copy, 24);                            \
    side = side_copy, enpassant = enpassant_copy, castle = castle_copy;   \
    fifty = fifty_copy;                                                   \
    hash_key = hash_key_copy;                                             \


enum {all_moves, only_captures};

const int castling_rights[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};

static inline int make_move(int move, int move_flag) {
    if (move_flag == all_moves) {
        copy_board();
        
        int source_square = get_move_source(move);
        int target_square = get_move_target(move);
        int piece = get_move_piece(move);
        int promoted_piece = get_move_promoted(move);
        int capture = get_move_capture(move);
        int double_push = get_move_double(move);
        int enpass = get_move_enpassant(move);
        int castling = get_move_castling(move);
        
        pop_bit(bitboards[piece], source_square);
        set_bit(bitboards[piece], target_square);
        
        hash_key ^= piece_keys[piece][source_square]; 
        hash_key ^= piece_keys[piece][target_square]; 
        
        fifty++;
        
        if (piece == P || piece == p)    
            fifty = 0;
        
        if (capture) {
            fifty = 0;
            
            int start_piece, end_piece;
            
            if (side == white) {
                start_piece = p;
                end_piece = k;
            }
            
            else {
                start_piece = P;
                end_piece = K;
            }
            
            for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
                if (get_bit(bitboards[bb_piece], target_square)) {
                    pop_bit(bitboards[bb_piece], target_square);
                    
                    hash_key ^= piece_keys[bb_piece][target_square];
                    break;
                }
            }
        }
        
        if (promoted_piece) {
            if (side == white) {
                pop_bit(bitboards[P], target_square);
                hash_key ^= piece_keys[P][target_square];
            }
            
            else {
                pop_bit(bitboards[p], target_square);
                hash_key ^= piece_keys[p][target_square];
            }
            
            set_bit(bitboards[promoted_piece], target_square);
            
            hash_key ^= piece_keys[promoted_piece][target_square];
        }
        
        if (enpass) {
            (side == white) ? pop_bit(bitboards[p], target_square + 8) :
                              pop_bit(bitboards[P], target_square - 8);
                              
            if (side == white) {
                pop_bit(bitboards[p], target_square + 8);
                hash_key ^= piece_keys[p][target_square + 8];
            }
            
            else {
                pop_bit(bitboards[P], target_square - 8);
                hash_key ^= piece_keys[P][target_square - 8];
            }
        }
        
        if (enpassant != no_sq) hash_key ^= enpassant_keys[enpassant];
        
        enpassant = no_sq;
        
        if (double_push) {
            if (side == white) {
                enpassant = target_square + 8;
                hash_key ^= enpassant_keys[target_square + 8];
            }
            
            else {
                enpassant = target_square - 8;
                hash_key ^= enpassant_keys[target_square - 8];
            }
        }
        
        if (castling) {
            switch (target_square) {
                case (g1):
                    pop_bit(bitboards[R], h1);
                    set_bit(bitboards[R], f1);
                    
                    hash_key ^= piece_keys[R][h1];  
                    hash_key ^= piece_keys[R][f1];  
                    break;
                
                case (c1):
                    pop_bit(bitboards[R], a1);
                    set_bit(bitboards[R], d1);
                    
                    hash_key ^= piece_keys[R][a1];  
                    hash_key ^= piece_keys[R][d1];  
                    break;
                
                case (g8):
                    pop_bit(bitboards[r], h8);
                    set_bit(bitboards[r], f8);
                    
                    hash_key ^= piece_keys[r][h8];  
                    hash_key ^= piece_keys[r][f8];  
                    break;
                
                case (c8):
                    pop_bit(bitboards[r], a8);
                    set_bit(bitboards[r], d8);
                    
                    hash_key ^= piece_keys[r][a8];  
                    hash_key ^= piece_keys[r][d8];  
                    break;
            }
        }
        
        hash_key ^= castle_keys[castle];

        castle &= castling_rights[source_square];
        castle &= castling_rights[target_square];

        hash_key ^= castle_keys[castle];
        
        memset(occupancies, 0ULL, 24);

        for (int bb_piece = P; bb_piece <= K; bb_piece++) 
            occupancies[white] |= bitboards[bb_piece];

        for (int bb_piece = p; bb_piece <= k; bb_piece++)            
            occupancies[black] |= bitboards[bb_piece];

        occupancies[both] |= occupancies[white];
        occupancies[both] |= occupancies[black];        
        
        side ^= 1;
        
        hash_key ^= side_key;
        
        if (is_square_attacked((side == white) ? get_ls1b_index(bitboards[k]) : get_ls1b_index(bitboards[K]), side)) {
            take_back();
            return 0;
        }
        
        else    
            return 1;
    }
    
    else {
        if (get_move_capture(move))
            make_move(move, all_moves);
        
        else    
            return 0;
    }
}

static inline void generate_moves(moves *move_list) {
    move_list->count = 0;

    int source_square, target_square;
    
    U64 bitboard, attacks;    
    
    for (int piece = P; piece <= k; piece++) {
        bitboard = bitboards[piece];
        
        if (side == white) {
            if (piece == P) {
                while (bitboard) {
                    source_square = get_ls1b_index(bitboard);
                    target_square = source_square - 8;
                    
                    if (!(target_square < a8) && !get_bit(occupancies[both], target_square)) {
                        if (source_square >= a7 && source_square <= h7) {                            
                            add_move(move_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));
                        }
                        
                        else {
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            
                            if ((source_square >= a2 && source_square <= h2) && !get_bit(occupancies[both], target_square - 8))
                                add_move(move_list, encode_move(source_square, target_square - 8, piece, 0, 0, 1, 0, 0));
                        }
                    }
                    
                    attacks = pawn_attacks[side][source_square] & occupancies[black];
                    
                    while (attacks) {
                        target_square = get_ls1b_index(attacks);
                        
                        if (source_square >= a7 && source_square <= h7) {
                            add_move(move_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0));
                        }
                        
                        else
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        
                        pop_bit(attacks, target_square);
                    }
                    
                    if (enpassant != no_sq) {
                        U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);
                        
                        if (enpassant_attacks) {
                            int target_enpassant = get_ls1b_index(enpassant_attacks);
                            add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }
                    
                    pop_bit(bitboard, source_square);
                }
            }
            
            if (piece == K) {
                if (castle & wk) {
                    if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1)) {
                        if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
                            add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
                    }
                }
                
                if (castle & wq) {
                    if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1)) {
                        if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black))
                            add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }
        
        else {
            if (piece == p) {
                while (bitboard) {
                    source_square = get_ls1b_index(bitboard);
                    target_square = source_square + 8;
                    
                    if (!(target_square > h1) && !get_bit(occupancies[both], target_square)) {
                        if (source_square >= a2 && source_square <= h2) {
                            add_move(move_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));
                        }
                        
                        else {
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            
                            if ((source_square >= a7 && source_square <= h7) && !get_bit(occupancies[both], target_square + 8))
                                add_move(move_list, encode_move(source_square, target_square + 8, piece, 0, 0, 1, 0, 0));
                        }
                    }
                    
                    attacks = pawn_attacks[side][source_square] & occupancies[white];
                    
                    while (attacks) {
                        target_square = get_ls1b_index(attacks);
                        
                        if (source_square >= a2 && source_square <= h2) {
                            add_move(move_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0));
                        }
                        
                        else
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        
                        pop_bit(attacks, target_square);
                    }
                    
                    if (enpassant != no_sq) {
                        U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);
                        
                        if (enpassant_attacks) {
                            int target_enpassant = get_ls1b_index(enpassant_attacks);
                            add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }
                    
                    pop_bit(bitboard, source_square);
                }
            }
            
            if (piece == k) {
                if (castle & bk) {
                    if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8)) {
                        if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
                            add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
                    }
                }
                
                if (castle & bq) {
                    if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8)) {
                        if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
                            add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }
        
        if ((side == white) ? piece == N : piece == n){
            while (bitboard){
                source_square = get_ls1b_index(bitboard);
                attacks = knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                while (attacks) {
                    target_square = get_ls1b_index(attacks);    
                    
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else                        
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    
                    pop_bit(attacks, target_square);
                }
                
                pop_bit(bitboard, source_square);
            }
        }
        
        if ((side == white) ? piece == B : piece == b) {
            while (bitboard) {
                source_square = get_ls1b_index(bitboard);
                attacks = get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                while (attacks) {
                    target_square = get_ls1b_index(attacks);    
                    
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    
                    else 
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    
                    pop_bit(attacks, target_square);
                }
                
                pop_bit(bitboard, source_square);
            }
        }
        
        if ((side == white) ? piece == R : piece == r) {
            while (bitboard) {
                source_square = get_ls1b_index(bitboard);
                attacks = get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                while (attacks) {
                    target_square = get_ls1b_index(attacks);    
                    
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    
                    else    
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    
                    pop_bit(attacks, target_square);
                }
                
                pop_bit(bitboard, source_square);
            }
        }

        if ((side == white) ? piece == Q : piece == q) {
            while (bitboard) {
                source_square = get_ls1b_index(bitboard);
                attacks = get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                while (attacks){
                    target_square = get_ls1b_index(attacks);    
                    
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    
                    else
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    pop_bit(attacks, target_square);
                }

                pop_bit(bitboard, source_square);
            }
        }

        if ((side == white) ? piece == K : piece == k) {
            while (bitboard) {
                source_square = get_ls1b_index(bitboard);
                attacks = king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                while (attacks) {
                    target_square = get_ls1b_index(attacks);    
                    
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    
                    else    
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    
                    pop_bit(attacks, target_square);
                }
                
                pop_bit(bitboard, source_square);
            }
        }
    }
}

U64 nodes;

static inline void perft_driver(int depth) {
    if (depth == 0) {
        nodes++;
        return;
    }
    
    moves move_list[1];

    generate_moves(move_list);    
        
    for (int move_count = 0; move_count < move_list->count; move_count++) {   
        copy_board();
        
        if (!make_move(move_list->moves[move_count], all_moves))    
            continue;
        
        perft_driver(depth - 1);
        
        take_back();        
    }
}

void perft_test(int depth) {
    printf("\n     Performance test\n\n");
    
    moves move_list[1];
    
    generate_moves(move_list);
    
    long start = get_time_ms();
    
    for (int move_count = 0; move_count < move_list->count; move_count++) {   
        copy_board();
        
        if (!make_move(move_list->moves[move_count], all_moves))    
            continue;
        
        long cummulative_nodes = nodes;
        
        perft_driver(depth - 1);
        
        long old_nodes = nodes - cummulative_nodes;
        
        take_back();
        
        printf("     move: %s%s%c  nodes: %ld\n", square_to_coordinates[get_move_source(move_list->moves[move_count])],
                                                  square_to_coordinates[get_move_target(move_list->moves[move_count])],
                                                  get_move_promoted(move_list->moves[move_count]) ? promoted_pieces[get_move_promoted(move_list->moves[move_count])] : ' ',
                                                  old_nodes);
    }
    
    printf("\n    Depth: %d\n", depth);
    printf("    Nodes: %lld\n", nodes);
    printf("     Time: %ld\n\n", get_time_ms() - start);
}

int nnue_pieces[12] = { 6, 5, 4, 3, 2, 1, 12, 11, 10, 9, 8, 7 };

int nnue_squares[64] = {
    a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

static inline int evaluate() {    
    U64 bitboard;
    
    int piece, square;
    int pieces[33];
    int squares[33];
    int index = 2;
    
    for (int bb_piece = P; bb_piece <= k; bb_piece++) {
        bitboard = bitboards[bb_piece];
        
        while (bitboard) {
            piece = bb_piece;
            square = get_ls1b_index(bitboard);
            
            if (piece == K) {
                pieces[0] = nnue_pieces[piece];
                squares[0] = nnue_squares[square];
            }
            
            else if (piece == k) {
                pieces[1] = nnue_pieces[piece];
                squares[1] = nnue_squares[square];
            }
            
            else {
                pieces[index] = nnue_pieces[piece];
                squares[index] = nnue_squares[square];
                index++;    
            }

            pop_bit(bitboard, square);
        }
    }
    
    pieces[index] = 0;
    squares[index] = 0;
    
    return (evaluate_nnue(side, pieces, squares) * (100 - fifty) / 100);
}

#define infinity 50000
#define mate_value 49000
#define mate_score 48000

static int mvv_lva[12][12] = {
 	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};

#define max_ply 64

int killer_moves[2][max_ply];
int history_moves[12][64];

int pv_length[max_ply];
int pv_table[max_ply][max_ply];
int follow_pv, score_pv;

int hash_entries = 0;

#define no_hash_entry 100000
#define hash_flag_exact 0
#define hash_flag_alpha 1
#define hash_flag_beta 2

typedef struct {
    U64 hash_key;   
    int depth;      
    int flag;       
    int score;      
    int best_move;
} tt;               

tt *hash_table = NULL;

void clear_hash_table() {
    tt *hash_entry;
    
    for (hash_entry = hash_table; hash_entry < hash_table + hash_entries; hash_entry++) {
        hash_entry->hash_key = 0;
        hash_entry->depth = 0;
        hash_entry->flag = 0;
        hash_entry->score = 0;
    }
}

void init_hash_table(int mb) {
    int hash_size = 0x100000 * mb;
    
    hash_entries =  hash_size / sizeof(tt);

    if (hash_table != NULL) {
        printf("    Clearing hash memory...\n");
        free(hash_table);
    }
     
    hash_table = (tt *) malloc(hash_entries * sizeof(tt));

    if (hash_table == NULL) {
        printf("    Couldn't allocate memory for hash table, tryinr %dMB...", mb / 2);
        init_hash_table(mb / 2);
    }
    
    else {
        clear_hash_table();
        printf("Hash table is initialied with %d entries\n", hash_entries);
    }
}

static inline int read_hash_entry(int alpha, int beta, int* best_move, int depth) {    
    tt *hash_entry = &hash_table[hash_key % hash_entries];
    
    if (hash_entry->hash_key == hash_key) {
        if (hash_entry->depth >= depth) {
            int score = hash_entry->score;
            
            if (score < -mate_score) score += ply;
            if (score > mate_score) score -= ply;
        
            if (hash_entry->flag == hash_flag_exact)        
                return score;
            
            if ((hash_entry->flag == hash_flag_alpha) && (score <= alpha))
                return alpha;
            
            if ((hash_entry->flag == hash_flag_beta) && (score >= beta))
                return beta;
        }
        
        *best_move = hash_entry->best_move;
    }

    return no_hash_entry;
}

static inline void write_hash_entry(int score, int best_move, int depth, int hash_flag) {
    tt *hash_entry = &hash_table[hash_key % hash_entries];

    if (score < -mate_score) score -= ply;
    if (score > mate_score) score += ply;

    hash_entry->hash_key = hash_key;
    hash_entry->score = score;
    hash_entry->flag = hash_flag;
    hash_entry->depth = depth;
    hash_entry->best_move = best_move;
}

static inline void enable_pv_scoring(moves *move_list) {
    follow_pv = 0;
    
    for (int count = 0; count < move_list->count; count++) {
        if (pv_table[0][ply] == move_list->moves[count]) {
            score_pv = 1;
            follow_pv = 1;
        }
    }
}

static inline int score_move(int move) {
    if (score_pv) {
        if (pv_table[0][ply] == move) {
            score_pv = 0;
            return 20000;
        }
    }

    if (get_move_capture(move)) {
        int piece = get_move_piece(move);
        int target_piece = P;
        int start_piece, end_piece;
        
        if (side == white) {
            start_piece = p;
            end_piece = k;
        }

        else {
            start_piece = P;
            end_piece = K;
        }
        
        for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
            if (get_bit(bitboards[bb_piece], get_move_target(move))) {
                target_piece = bb_piece;
                break;
            }
        }
        
        return mvv_lva[piece][target_piece] + 10000;
    }
    
    else {
        if (killer_moves[0][ply] == move)
            return 9000;
        
        else if (killer_moves[1][ply] == move)
            return 8000;

        else
            return history_moves[get_move_piece(move)][get_move_target(move)];
    }
    
    return 0;
}

static inline int sort_moves(moves *move_list, int best_move) {
    int move_scores[move_list->count];
    
    for (int count = 0; count < move_list->count; count++) {
        if (best_move == move_list->moves[count])
            move_scores[count] = 30000;

        else
            move_scores[count] = score_move(move_list->moves[count]);
    }
    
    for (int current_move = 0; current_move < move_list->count; current_move++) {
        for (int next_move = current_move + 1; next_move < move_list->count; next_move++) {
            if (move_scores[current_move] < move_scores[next_move]) {
                int temp_score = move_scores[current_move];
                move_scores[current_move] = move_scores[next_move];
                move_scores[next_move] = temp_score;
                
                int temp_move = move_list->moves[current_move];
                move_list->moves[current_move] = move_list->moves[next_move];
                move_list->moves[next_move] = temp_move;
            }
        }
    }
}

void print_move_scores(moves *move_list) {
    printf("     Move scores:\n\n");
        
    for (int count = 0; count < move_list->count; count++) {
        printf("     move: ");
        print_move(move_list->moves[count]);
        printf(" score: %d\n", score_move(move_list->moves[count]));
    }
}

static inline int is_repetition() {
    for (int index = 0; index < repetition_index; index++)
        if (repetition_table[index] == hash_key)
            return 1;
    
    return 0;
}

static inline int quiescence(int alpha, int beta) {
    if((nodes & 2047 ) == 0)
		communicate();
    
    nodes++;

    if (ply > max_ply - 1)        
        return evaluate();

    int evaluation = evaluate();    
    
    if (evaluation >= beta) {
        return beta;
    }
    
    if (evaluation > alpha) {
        alpha = evaluation;
    }
    
    moves move_list[1];
    
    generate_moves(move_list);
    
    sort_moves(move_list, 0);
    
    for (int count = 0; count < move_list->count; count++) {
        copy_board();
        
        ply++;

        repetition_index++;
        repetition_table[repetition_index] = hash_key;

        if (make_move(move_list->moves[count], only_captures) == 0) {
            ply--;
            repetition_index--;
            
            continue;
        }

        int score = -quiescence(-beta, -alpha);
            
        ply--;    
        repetition_index--;

        take_back();
        
        if (stopped == 1) return 0;
        
        if (score > alpha) {
            alpha = score;
            
            if (score >= beta) {
                return beta;
            }
        }
    }
    
    return alpha;
}

const int full_depth_moves = 4;
const int reduction_limit = 3;

static inline int negamax(int alpha, int beta, int depth) {
    pv_length[ply] = ply;
    
    int score;
    int best_move = 0;
    int hash_flag = hash_flag_alpha;
    
    if (ply && is_repetition() || fifty >= 100)    
        return 0;
    
    int pv_node = beta - alpha > 1;
    
    if (ply && (score = read_hash_entry(alpha, beta, &best_move, depth)) != no_hash_entry && pv_node == 0)
        return score;
        
    if((nodes & 2047 ) == 0)
		communicate();

    if (depth == 0)        
        return quiescence(alpha, beta);
    
    if (ply > max_ply - 1)    
        return evaluate();
    
    nodes++;
    
    int in_check = is_square_attacked((side == white) ? get_ls1b_index(bitboards[K]) : 
                                                        get_ls1b_index(bitboards[k]),
                                                        side ^ 1);
    
    if (in_check) depth++;
    
    int legal_moves = 0;
	int static_eval = evaluate();
    
	if (depth < 3 && !pv_node && !in_check &&  abs(beta - 1) > -infinity + 100) {   
		int eval_margin = 120 * depth;
		
		if (static_eval - eval_margin >= beta)    
			return static_eval - eval_margin;
	}
    
    if (depth >= 3 && in_check == 0 && ply) {
        copy_board();
        
        ply++;

        repetition_index++;
        repetition_table[repetition_index] = hash_key;        
        
        if (enpassant != no_sq) hash_key ^= enpassant_keys[enpassant];
        
        enpassant = no_sq;
        side ^= 1;
        hash_key ^= side_key;
                
        score = -negamax(-beta, -beta + 1, depth - 1 - 2);

        ply--;        
        repetition_index--;
            
        take_back();

        if (stopped == 1) return 0;

        if (score >= beta)            
            return beta;
    }
    
    if (!pv_node && !in_check && depth <= 3) {
        score = static_eval + 125;

        int new_score;
                
        if (score < beta) {
            if (depth == 1) {
                new_score = quiescence(alpha, beta);
                
                return (new_score > score) ? new_score : score;
            }
            
            score += 175;
            
            if (score < beta && depth <= 2) {
                new_score = quiescence(alpha, beta);
                
                if (new_score < beta)    
                    return (new_score > score) ? new_score : score;
            }
        }
	}
    
    moves move_list[1];
    
    generate_moves(move_list);
    
    if (follow_pv)    
        enable_pv_scoring(move_list);
    
    sort_moves(move_list, best_move);
    
    int moves_searched = 0;
    
    for (int count = 0; count < move_list->count; count++) {
        copy_board();
        
        ply++;
        
        repetition_index++;
        repetition_table[repetition_index] = hash_key;
        
        if (make_move(move_list->moves[count], all_moves) == 0) {
            ply--;
            repetition_index--;
            
            continue;
        }
        
        legal_moves++;
        
        if (moves_searched == 0)    
            score = -negamax(-beta, -alpha, depth - 1);
        
        else {
            if (
                moves_searched >= full_depth_moves &&
                depth >= reduction_limit &&
                in_check == 0 && 
                get_move_capture(move_list->moves[count]) == 0 &&
                get_move_promoted(move_list->moves[count]) == 0
              ) 
                score = -negamax(-alpha - 1, -alpha, depth - 2);
            
            else score = alpha + 1;
            
            if(score > alpha) {
                score = -negamax(-alpha - 1, -alpha, depth-1);

                if((score > alpha) && (score < beta))
                    score = -negamax(-beta, -alpha, depth-1);
            }
        }
        
        ply--;
        repetition_index--;

        take_back();
            
        if (stopped == 1)
            return 0;
            
        moves_searched++;
        
        if (score > alpha) {
            hash_flag = hash_flag_exact;
            best_move = move_list->moves[count];
        
            if (get_move_capture(move_list->moves[count]) == 0)        
                history_moves[get_move_piece(move_list->moves[count])][get_move_target(move_list->moves[count])] += depth;
            
            alpha = score;
            
            pv_table[ply][ply] = move_list->moves[count];
            
            for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)    
                pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
            
            pv_length[ply] = pv_length[ply + 1];
            
            if (score >= beta) {
                write_hash_entry(beta, best_move, depth, hash_flag_beta);
            
                if (get_move_capture(move_list->moves[count]) == 0) {
                    killer_moves[1][ply] = killer_moves[0][ply];
                    killer_moves[0][ply] = move_list->moves[count];
                }
                
                return beta;
            }            
        }
    }
    
    if (legal_moves == 0) {
        if (in_check)
            return -mate_value + ply;
        
        else    
            return 0;
    }
    
    write_hash_entry(alpha, best_move, depth, hash_flag);

    return alpha;
}

void search_position(int depth) {
    int start = get_time_ms();
    int score = 0;
    
    nodes = 0;
    stopped = 0;
    follow_pv = 0;
    score_pv = 0;
    
    memset(killer_moves, 0, sizeof(killer_moves));
    memset(history_moves, 0, sizeof(history_moves));
    memset(pv_table, 0, sizeof(pv_table));
    memset(pv_length, 0, sizeof(pv_length));
    
    int alpha = -infinity;
    int beta = infinity;
 
    for (int current_depth = 1; current_depth <= depth; current_depth++) {
        if (stopped == 1)
			break;
		
        follow_pv = 1;

        score = negamax(alpha, beta, current_depth);
 
        if ((score <= alpha) || (score >= beta)) {
            alpha = -infinity;    
            beta = infinity;      
            continue;
        }
               
        alpha = score - 50;
        beta = score + 50;
        
        if (pv_length[0]) {
            if (score > -mate_value && score < -mate_score)
                printf("info score mate %d depth %d nodes %lld time %d pv ", -(score + mate_value) / 2 - 1, current_depth, nodes, get_time_ms() - start);
            
            else if (score > mate_score && score < mate_value)
                printf("info score mate %d depth %d nodes %lld time %d pv ", (mate_value - score) / 2 + 1, current_depth, nodes, get_time_ms() - start);   
            
            else
                printf("info score cp %d depth %d nodes %lld time %d pv ", score, current_depth, nodes, get_time_ms() - start);
            
            for (int count = 0; count < pv_length[0]; count++) {
                print_move(pv_table[0][count]);
                printf(" ");
            }
            
            printf("\n");
        }
    }

    printf("bestmove ");
    
    if (pv_table[0][0])
        print_move(pv_table[0][0]);
    
    else        
        printf("(none)");
    
    printf("\n");
}

int parse_move(char *move_string) {
    moves move_list[1];
    
    generate_moves(move_list);
    
    int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;
    
    for (int move_count = 0; move_count < move_list->count; move_count++) {
        int move = move_list->moves[move_count];
        
        if (source_square == get_move_source(move) && target_square == get_move_target(move)) {
            int promoted_piece = get_move_promoted(move);
            
            if (promoted_piece) {
                if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
                    return move;
                
                else if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
                    return move;
                
                else if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
                    return move;
                
                else if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
                    return move;
                
                continue;
            }
            
            return move;
        }
    }
    
    return 0;
}

void parse_position(char *command) {
    command += 9;
    
    char *current_char = command;
    
    if (strncmp(command, "startpos", 8) == 0)
        parse_fen(start_position);
    
    else {
        current_char = strstr(command, "fen");
        
        if (current_char == NULL)
            parse_fen(start_position);
            
        else {
            current_char += 4;
            
            parse_fen(current_char);
        }
    }
    
    current_char = strstr(command, "moves");
    
    if (current_char != NULL) {
        current_char += 6;
        
        while(*current_char) {
            int move = parse_move(current_char);
            
            if (move == 0)
                break;
            
            
            repetition_index++;
            repetition_table[repetition_index] = hash_key;
            
            make_move(move, all_moves);
            
            while (*current_char && *current_char != ' ') current_char++;
            
            current_char++;
        }        
    }
}

void reset_time_control() { 
    quit = 0;
    movestogo = 30;
    movetime = -1;
    time = -1;
    inc = 0;
    starttime = 0;
    stoptime = 0;
    timeset = 0;
    stopped = 0;
}

void parse_go(char *command) {
    reset_time_control();
    
    int depth = -1;

    char *argument = NULL;

    if ((argument = strstr(command,"infinite"))) {}
    
    if ((argument = strstr(command,"binc")) && side == black)
        inc = atoi(argument + 5);
    
    if ((argument = strstr(command,"winc")) && side == white)
        inc = atoi(argument + 5);
    
    if ((argument = strstr(command,"wtime")) && side == white)
        time = atoi(argument + 6);
    
    if ((argument = strstr(command,"btime")) && side == black)
        time = atoi(argument + 6);
    
    if ((argument = strstr(command,"movestogo")))
        movestogo = atoi(argument + 10);
    
    if ((argument = strstr(command,"movetime")))
        movetime = atoi(argument + 9);
    
    if ((argument = strstr(command,"depth")))
        depth = atoi(argument + 6);
    
    if (movetime != -1) {
        time = movetime;
        movestogo = 1;
    }
    
    starttime = get_time_ms();
    depth = (depth == -1) ? 64 : depth;
    
    if (time != -1) {
        timeset = 1;
        time /= movestogo;
        time -= 50;
        
        if (time < 0) {
            time = 0;
            inc -= 50;

            if (inc < 0) inc = 1;
        }
        
        stoptime = starttime + time + inc;        
    }

    printf("time: %d  inc: %d  start: %u  stop: %u  depth: %d  timeset:%d\n",
            time, inc, starttime, stoptime, depth, timeset);
    
    search_position(depth);
}

void uci_loop() {
    #define INPUT_BUFFER 10000
    
    int max_hash = 128;
    int mb = 64;
    
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    
    char input[INPUT_BUFFER];
    
    while (1) {
        memset(input, 0, sizeof(input));
        
        fflush(stdout);
        
        if (!fgets(input, INPUT_BUFFER, stdin))
            continue;
        
        if (input[0] == '\n')
            continue;
        
        if (strncmp(input, "isready", 7) == 0) {
            printf("readyok\n");
            continue;
        }
        
        else if (strncmp(input, "position", 8) == 0) { 
            parse_position(input);            
            clear_hash_table();
        }
        
        else if (strncmp(input, "ucinewgame", 10) == 0) {
            parse_position("position startpos");
            clear_hash_table();
        }

        else if (strncmp(input, "go", 2) == 0)
            parse_go(input);
        
        else if (strncmp(input, "quit", 4) == 0)
            break;
        
        else if (strncmp(input, "uci", 3) == 0) {
            printf("id name Toodfish %s\n", version);
            printf("id author Logan James\n");
            printf("option name Hash type spin default 64 min 4 max %d\n", max_hash);
            printf("uciok\n");
        }
        
        else if (!strncmp(input, "setoption name Hash value ", 26)) {			
            sscanf(input,"%*s %*s %*s %*s %d", &mb);
            
            if(mb < 4) mb = 4;
            if(mb > max_hash) mb = max_hash;
            
            printf("    Set hash table size to %dMB\n", mb);
            init_hash_table(mb);
        }
    }
}

void init_all() {
    init_non_sliding_attacks();
    
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);
    
    init_random_keys();
    init_hash_table(64);
    init_nnue("nn-eba324f53044.nnue");
}

int main() {
    init_all();
    uci_loop();
    
    free(hash_table);

    return 0;
}