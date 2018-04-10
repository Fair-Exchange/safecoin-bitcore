/******************************************************************************
 * Copyright © 2014-2017 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

#define SAFECOIN_INTEREST ((uint64_t)(0.05 * COIN))   // 5%
int64_t MAX_MONEY = 200000000 * 100000000LL;

uint64_t safecoin_earned_interest(int32_t height,int64_t paidinterest)
{
    static uint64_t *interests; static int32_t maxheight;
    uint64_t total; int32_t ind,incr = 100000;
    if ( height >= maxheight )
    {
        if ( interests == 0 )
        {
            maxheight = height + incr;
            interests = (uint64_t *)calloc(maxheight,sizeof(*interests) * 2);
        }
        else
        {
            interests = (uint64_t *)realloc(interests,(maxheight + incr) * sizeof(*interests) * 2);
            memset(&interests[maxheight << 1],0,incr * sizeof(*interests) * 2);
            maxheight += incr;
        }
    }
    ind = (height << 1);
    if ( paidinterest < 0 ) // request
        return(interests[ind]);
    else
    {
        if ( interests[ind + 1] != paidinterest )
        {
            interests[ind + 1] = paidinterest;
            if ( height == 0 )
                interests[ind] = interests[ind + 1];
            else interests[ind] = interests[ind - 2] + interests[ind + 1];
            total = interests[ind];
            for (++height; height<maxheight; height++)
            {
                ind = (height << 1);
                interests[ind] = total;
                interests[ind + 1] = 0;
            }
        }
    }
    return(0);
}

uint64_t safecoin_moneysupply(int32_t height)
{
    if ( height <= 1 || ASSETCHAINS_SYMBOL[0] == 0 )
        return(0);
    else return(COIN * 100000000 + (height-1) * 3 + safecoin_earned_interest(height,-1));
}

uint64_t safecoin_interest(int32_t txheight,uint64_t nValue,uint32_t nLockTime,uint32_t tiptime)
{
  int32_t minutes,exception; uint64_t numerator,denominator,interest_ratio,interest = 0;


    if(txheight<=77200){
      interest_ratio=1;   // 5% interest
    }
    else if(txheight>77200){
      interest_ratio=0;   // 0% interest before j1777 exploit
    }



    if ( ASSETCHAINS_SYMBOL[0] != 0 )
        return(0);
    if ( safecoin_moneysupply(txheight) < MAX_MONEY && nLockTime >= LOCKTIME_THRESHOLD && tiptime != 0 && nLockTime < tiptime && nValue >= 10*COIN )
    {
        if ( (minutes= (tiptime - nLockTime) / 60) >= 60 )
        {
            denominator = (((uint64_t)365 * 24 * 60) / minutes);
            if ( denominator == 0 )
                denominator = 1; // max SAFECOIN_INTEREST per transfer, do it at least annually!
            if ( nValue > 25000LL*COIN )
            {
                exception = 0;
                if ( txheight <= 155949 )
                {
                    if ( (txheight == 116607 && nValue == 2502721100000LL) ||
                        (txheight == 126891 && nValue == 2879650000000LL) ||
                        (txheight == 129510 && nValue == 3000000000000LL) ||
                        (txheight == 141549 && nValue == 3500000000000LL) ||
                        (txheight == 154473 && nValue == 3983399350000LL) ||
                        (txheight == 154736 && nValue == 3983406748175LL) ||
                        (txheight == 155013 && nValue == 3983414006565LL) ||
                        (txheight == 155492 && nValue == 3983427592291LL) ||
                        (txheight == 155613 && nValue == 9997409999999797LL) ||
                        (txheight == 157927 && nValue == 9997410667451072LL) ||
                        (txheight == 155613 && nValue == 2590000000000LL) ||
                        (txheight == 155949 && nValue == 4000000000000LL) )
                        exception = 1;
                    if ( exception == 0 || nValue == 4000000000000LL )
                        printf(">>>>>>>>>>>> exception.%d txheight.%d %.8f locktime %u vs tiptime %u <<<<<<<<<\n",exception,txheight,(double)nValue/COIN,nLockTime,tiptime);
                }
                //if ( nValue == 4000000000000LL )
                //    printf(">>>>>>>>>>>> exception.%d txheight.%d %.8f locktime %u vs tiptime %u <<<<<<<<<\n",exception,txheight,(double)nValue/COIN,nLockTime,tiptime);
                if ( exception == 0 )
                {
                    numerator = interest_ratio*(nValue / 20); // assumes 5%!
                    interest = (numerator / denominator);
                }
                else
                {
                    numerator = (nValue * SAFECOIN_INTEREST * interest_ratio);
                    interest = (numerator / denominator) / COIN;
                }
            }
            else
            {
                numerator = (nValue * SAFECOIN_INTEREST * interest_ratio);
                interest = (numerator / denominator) / COIN;
            }
            //fprintf(stderr,"safecoin_interest %lld %.8f nLockTime.%u tiptime.%u minutes.%d interest %lld %.8f (%llu / %llu)\n",(long long)nValue,(double)nValue/COIN,nLockTime,tiptime,minutes,(long long)interest,(double)interest/COIN,(long long)numerator,(long long)denominator);
        }
    }
    return(interest*interest_ratio);
}

