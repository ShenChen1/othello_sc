/*
    Egaroucid Project

    @date 2021-2023
    @author Takuto Yamana
    @license GPL-3.0 license
*/
#define N_EMBED_BOOK 52850
struct Embed_book{
    uint64_t player;
    uint64_t opponent;
    int_fast8_t value;
};
Embed_book embed_book[N_EMBED_BOOK] = {