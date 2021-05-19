#include "judge/include/judge_inc.h"
#include "math.h"

#define PLAYERS_NUM 8
#define RATING_INT 1500
#define RATING_MAX 4000
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

typedef struct tagContestant {
    int id;
    char name[32];
    int rating;
    int rank;
    double seed;
    int needRating;
    int newRating;
    int delta;
}Contestant;

Contestant players[PLAYERS_NUM] = {0};
int rankings[PLAYERS_NUM] = {0};

double judge_elo_get_win_probability(double ra, double rb) {
    return 1.0 / (1 + pow(10, (rb - ra) / 400.0));
}

void printf_player(Contestant *player)
{
    printf("%s, rating(%d), rank(%d).\n", player->name, player->rating, player->rank);
}

void printf_players(Contestant *player, int num)
{
    int i;
    printf("----------------------------------------\n");
    for (i = 0; i < num; i++) {
        printf("%s, rating(%d), rank(%d), newRating(%d), seed(%f), delta(%d), needRating(%d).\n",
            player[i].name, player[i].rating, player[i].rank, player[i].newRating,
            player[i].seed, player[i].delta,  player[i].needRating);
    }
}

double judge_elo_get_seed(Contestant *players, int num, int rating) {

    double result = 1;
    int i = 0;

    for (i= 0; i < num; i++) {
        result += judge_elo_get_win_probability(players[i].rating, rating);
    }

    return result;
}

int judge_elo_get_rating_by_rank(Contestant *players, int num, double rank) {
    int left = 1;
    int right = 8000;

    while (right - left > 1) {
        int mid = (left + right) / 2;

        if (judge_elo_get_seed(players, num, mid) < rank) {
            right = mid;
        } else {
            left = mid;
        }
    }

    return left;
}

static int judge_elo_sort_compare_rating(const void *p1, const void *p2) {
    return ((*(Contestant*)p2).rating > (*(Contestant*)p1).rating) ? 1 : -1;
}

static void judge_elo_sort_by_rating_desc(Contestant *players, int num) {
    qsort(players, num, sizeof(players[0]), judge_elo_sort_compare_rating);
}

static int judge_elo_sort_compare_rank(const void *p1, const void *p2) {
    return ((*(Contestant*)p1).rank > (*(Contestant*)p2).rank) ? 1 : -1;
}

static void judge_elo_sort_by_rank_desc(Contestant *players, int num) {
    qsort(players, num, sizeof(players[0]), judge_elo_sort_compare_rank);
}

void judge_elo_validate_deltas(Contestant *players, int num) {

    judge_elo_sort_by_rank_desc(players, num);

    int i, j;
    for (i = 0; i < num; i++) {
        for (j = i + 1; j < num; j++) {
            if (players[i].rating > players[j].rating) {
                if (players[i].rating + players[i].delta < players[j].rating + players[j].delta) {
                    printf("First rating invariant failed: %d vs. %d\n", players[i].id, players[j].id);
                }
            }
            if (players[i].rating < players[j].rating) {
                if (players[i].delta < players[j].delta) {
                    printf("Second rating invariant failed: %d vs. %d\n", players[i].id, players[j].id);
                }
            }
        }
    }
}

void judge_elo_rating_process(Contestant *players, int num) {
    int i = 0;
    int j = 0;

    /* calc expect seed */
    for (i = 0; i < num; i++) {
        players[i].seed = 1;
        for (j = 0; j < num; j++) {
            if (i != j) {
                players[i].seed += judge_elo_get_win_probability(players[j].rating, players[i].rating);
            }
        }
    }
    
    for (i = 0; i < num; i++) {
        double midRank = sqrt(players[i].rank * players[i].seed);
        players[i].needRating = judge_elo_get_rating_by_rank(players, num, midRank);
        players[i].delta = (players[i].needRating - players[i].rating) / 2;
    }

    judge_elo_sort_by_rating_desc(players, num);

    /* Total sum should not be more than zero. */
    {
        int sum = 0;
        for (i = 0; i < num; i++) {
            sum += players[i].delta;
        }

        int inc = -sum / num - 1;
        for (i = 0; i < num; i++) {
            players[i].delta += inc;
        }
    }
 
    /* Sum of top-4*sqrt should be adjusted to zero. */
    {
        int sum = 0;
        int zeroSumCount = min((int) (4 * round(sqrt(num))), num);
        for (i = 0; i < zeroSumCount; i++) {
            sum += players[i].delta;
        }
        int inc = min(max(-sum / zeroSumCount, -10), 0);
        for (i = 0; i < num; i++) {
            players[i].delta += inc;
        }
    }

    /* calc new rating, more than 0 */
    for (i = 0; i < num; i++) {
        players[i].newRating = max(players[i].rating + players[i].delta, 1);
    }
    
    judge_elo_validate_deltas(players, num);
}

void judge_elo_rating_caculate(int contestId) {
    int num = 0;
    char **users = SQL_getContestActiveUser(contestId, &num);
    if (users == NULL) {
        return;
    }

    Contestant *players = NULL;
    players = (Contestant *)malloc(num * sizeof(Contestant));
    if (players == NULL) {
        free(users);
        return;
    }
    memset(players, 0, num * sizeof(Contestant));

    for (int i = 0; i < num; i++) {
        strcpy(players[i].name, users[i]);
        players[i].rank = i + 1;

        /* get rating before end_time of contest */
        players[i].rating = SQL_getRatingBeforeContest(players[i].name, contestId);
        if (players[i].rating == 0) {
            players[i].rating = 1500;
        }
        //players[i].rating = SQL_getRatingFromUser(players[i].name);
    }

    printf_players(players, num);
    
    judge_elo_rating_process(players, num);

    printf_players(players, num);
   
    /* update rating to db */
    for (int i = 0; i < num; i++) { 
        SQL_updateRating(players[i].name, contestId, players[i].rank, players[i].newRating, players[i].delta);        
    }  
    
    for (int i = 0; i < num; i++) {
        free(users[i]);
    }
    free(users);
    free(players);
}

int judge_elo_get_ranking_rand() {

    do {
        int rank = rand()%PLAYERS_NUM;
        if (rankings[rank] == 1) {
            Sleep(1000);
            continue;
        }

        rankings[rank] = 1;
        return rank + 1;

    } while(1);
}

void judge_eol_test_init()
{
    int i, j;

    srand(1);

    memset(rankings, 0, sizeof(rankings));

    for (i = 0; i < PLAYERS_NUM; i++) {
        players[i].id = i + 1;
        sprintf(players[i].name, "name-%d", i + 1);
        players[i].rating = RATING_INT /* + rand()%1000*/;
        players[i].rank = judge_elo_get_ranking_rand();
        Sleep(1000);

        printf_player(&players[i]);
    }
}

void judge_eol_test()
{
    judge_eol_test_init();

    judge_elo_rating_process(players, PLAYERS_NUM);

    printf_players(players, PLAYERS_NUM);

}