#include "search.h"

void search()
{
}

int negamax_ab(int alpha, int beta, int depth_left)
{
    if( 0 == depth_left )
    {
        return quiesce(alpha, beta);
    }

    // for(Move_t move : moves)
    // {
    //     ScoreVal_t local_score = -negamax_ab(-beta, -alpha, depth_left-1);
    //     if( local_score >= beta)
    //     {
    //         return beta; // hard beta-cutoff
    //     }
    //     if( local_score > alpha )
    //     {
    //         alpha = local_score;
    //     }
    //     return alpha;
    // }
    return 0;
}

int quiesce(int alpha, int beta)
{
    return 0;
}
