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

// paxdeposit equivalent in reverse makes opreturn and SAFE does the same in reverse

int32_t pax_fiatstatus(uint64_t *available,uint64_t *deposited,uint64_t *issued,uint64_t *withdrawn,uint64_t *approved,uint64_t *redeemed,char *base)
{
    int32_t baseid; struct safecoin_state *sp; int64_t netliability,maxallowed;
    *available = *deposited = *issued = *withdrawn = *approved = *redeemed = 0;
    if ( (baseid= safecoin_baseid(base)) >= 0 )
    {
        if ( (sp= safecoin_stateptrget(base)) != 0 )
        {
            *deposited = sp->deposited;
            *issued = sp->issued;
            *withdrawn = sp->withdrawn;
            *approved = sp->approved;
            *redeemed = sp->redeemed;
            netliability = (sp->deposited - sp->withdrawn) - sp->shorted;
            maxallowed = safecoin_maxallowed(baseid);
            if ( netliability < maxallowed )
                *available = (maxallowed - netliability);
            //printf("%p %s %.8f %.8f %.8f %.8f %.8f\n",sp,base,dstr(*deposited),dstr(*issued),dstr(*withdrawn),dstr(*approved),dstr(*redeemed));
            return(0);
        } else printf("pax_fiatstatus cant get basesp.%s\n",base);
    } else printf("pax_fiatstatus illegal base.%s\n",base);
    return(-1);
}

void pax_keyset(uint8_t *buf,uint256 txid,uint16_t vout,uint8_t type)
{
    memcpy(buf,&txid,32);
    memcpy(&buf[32],&vout,2);
    buf[34] = type;
}

struct pax_transaction *safecoin_paxfind(uint256 txid,uint16_t vout,uint8_t type)
{
    struct pax_transaction *pax; uint8_t buf[35];
    pthread_mutex_lock(&safecoin_mutex);
    pax_keyset(buf,txid,vout,type);
    HASH_FIND(hh,PAX,buf,sizeof(buf),pax);
    pthread_mutex_unlock(&safecoin_mutex);
    return(pax);
}

struct pax_transaction *safecoin_paxfinds(uint256 txid,uint16_t vout)
{
    struct pax_transaction *pax; int32_t i; uint8_t types[] = { 'I', 'D', 'X', 'A', 'W' };
    for (i=0; i<sizeof(types)/sizeof(*types); i++)
        if ( (pax= safecoin_paxfind(txid,vout,types[i])) != 0 )
            return(pax);
    return(0);
}

struct pax_transaction *safecoin_paxmark(int32_t height,uint256 txid,uint16_t vout,uint8_t type,int32_t mark)
{
    struct pax_transaction *pax; uint8_t buf[35];
    pthread_mutex_lock(&safecoin_mutex);
    pax_keyset(buf,txid,vout,type);
    HASH_FIND(hh,PAX,buf,sizeof(buf),pax);
    if ( pax == 0 )
    {
        pax = (struct pax_transaction *)calloc(1,sizeof(*pax));
        pax->txid = txid;
        pax->vout = vout;
        pax->type = type;
        memcpy(pax->buf,buf,sizeof(pax->buf));
        HASH_ADD_KEYPTR(hh,PAX,pax->buf,sizeof(pax->buf),pax);
        //printf("ht.%d create pax.%p mark.%d\n",height,pax,mark);
    }
    if ( pax != 0 )
        pax->marked = mark;
    pthread_mutex_unlock(&safecoin_mutex);
    return(pax);
}

void safecoin_paxdelete(struct pax_transaction *pax)
{
    return; // breaks when out of order
    pthread_mutex_lock(&safecoin_mutex);
    HASH_DELETE(hh,PAX,pax);
    pthread_mutex_unlock(&safecoin_mutex);
}

void safecoin_gateway_deposit(char *coinaddr,uint64_t value,char *symbol,uint64_t fiatoshis,uint8_t *rmd160,uint256 txid,uint16_t vout,uint8_t type,int32_t height,int32_t otherheight,char *source,int32_t approved) // assetchain context
{
    struct pax_transaction *pax; uint8_t buf[35]; int32_t addflag = 0; struct safecoin_state *sp; char str[16],dest[16],*s;
    if ( SAFECOIN_PAX == 0 )
        return;
    sp = safecoin_stateptr(str,dest);
    pthread_mutex_lock(&safecoin_mutex);
    pax_keyset(buf,txid,vout,type);
    HASH_FIND(hh,PAX,buf,sizeof(buf),pax);
    if ( pax == 0 )
    {
        pax = (struct pax_transaction *)calloc(1,sizeof(*pax));
        pax->txid = txid;
        pax->vout = vout;
        pax->type = type;
        memcpy(pax->buf,buf,sizeof(pax->buf));
        HASH_ADD_KEYPTR(hh,PAX,pax->buf,sizeof(pax->buf),pax);
        addflag = 1;
        if ( 0 && ASSETCHAINS_SYMBOL[0] == 0 )
        {
            int32_t i; for (i=0; i<32; i++)
                printf("%02x",((uint8_t *)&txid)[i]);
            printf(" v.%d [%s] kht.%d ht.%d create pax.%p symbol.%s source.%s\n",vout,ASSETCHAINS_SYMBOL,height,otherheight,pax,symbol,source);
        }
    }
    pthread_mutex_unlock(&safecoin_mutex);
    if ( coinaddr != 0 )
    {
        strcpy(pax->coinaddr,coinaddr);
        if ( value != 0 )
            pax->safecoinshis = value;
        if ( symbol != 0 )
            strcpy(pax->symbol,symbol);
        if ( source != 0 )
            strcpy(pax->source,source);
        if ( fiatoshis != 0 )
            pax->fiatoshis = fiatoshis;
        if ( rmd160 != 0 )
            memcpy(pax->rmd160,rmd160,20);
        if ( height != 0 )
            pax->height = height;
        if ( otherheight != 0 )
            pax->otherheight = otherheight;
    }
    else
    {
        pax->marked = height;
        //printf("pax.%p MARK DEPOSIT ht.%d other.%d\n",pax,height,otherheight);
    }
}

int32_t safecoin_rwapproval(int32_t rwflag,uint8_t *opretbuf,struct pax_transaction *pax)
{
    int32_t i,len = 0;
    if ( rwflag == 1 )
    {
        for (i=0; i<32; i++)
            opretbuf[len++] = ((uint8_t *)&pax->txid)[i];
        opretbuf[len++] = pax->vout & 0xff;
        opretbuf[len++] = (pax->vout >> 8) & 0xff;
    }
    else
    {
        for (i=0; i<32; i++)
            ((uint8_t *)&pax->txid)[i] = opretbuf[len++];
        //for (i=0; i<32; i++)
        //    printf("%02x",((uint8_t *)&pax->txid)[31-i]);
        pax->vout = opretbuf[len++];
        pax->vout += ((uint32_t)opretbuf[len++] << 8);
        //printf(" txid v.%d\n",pax->vout);
    }
    len += iguana_rwnum(rwflag,&opretbuf[len],sizeof(pax->safecoinshis),&pax->safecoinshis);
    len += iguana_rwnum(rwflag,&opretbuf[len],sizeof(pax->fiatoshis),&pax->fiatoshis);
    len += iguana_rwnum(rwflag,&opretbuf[len],sizeof(pax->height),&pax->height);
    len += iguana_rwnum(rwflag,&opretbuf[len],sizeof(pax->otherheight),&pax->otherheight);
    if ( rwflag != 0 )
    {
        memcpy(&opretbuf[len],pax->rmd160,20), len += 20;
        for (i=0; i<4; i++)
            opretbuf[len++] = pax->source[i];
    }
    else
    {
        memcpy(pax->rmd160,&opretbuf[len],20), len += 20;
        for (i=0; i<4; i++)
            pax->source[i] = opretbuf[len++];
    }
    return(len);
}

int32_t safecoin_issued_opreturn(char *base,uint256 *txids,uint16_t *vouts,int64_t *values,int64_t *srcvalues,int32_t *safeheights,int32_t *otherheights,int8_t *baseids,uint8_t *rmd160s,uint8_t *opretbuf,int32_t opretlen,int32_t issafecoin)
{
    struct pax_transaction p,*pax; int32_t i,n=0,j,len=0,incr,height,otherheight; uint8_t type,rmd160[20]; uint64_t fiatoshis; char symbol[16];
    if ( SAFECOIN_PAX == 0 )
        return(0);
    incr = 34 + (issafecoin * (2*sizeof(fiatoshis) + 2*sizeof(height) + 20 + 4));
    //41e77b91cb68dc2aa02fa88550eae6b6d44db676a7e935337b6d1392d9718f03cb0200305c90660400000000fbcbeb1f000000bde801006201000058e7945ad08ddba1eac9c9b6c8e1e97e8016a2d152
    
    // 41e94d736ec69d88c08b5d238abeeca609c02357a8317e0d56c328bcb1c259be5d0200485bc80200000000404b4c000000000059470200b80b000061f22ba7d19fe29ac3baebd839af8b7127d1f9075553440046bb4cc7a3b5cd39dffe7206507a3482a00780e617f68b273cce9817ed69298d02001069ca1b0000000080f0fa02000000005b470200b90b000061f22ba7d19fe29ac3baebd839af8b7127d1f90755
    
    //for (i=0; i<opretlen; i++)
    //    printf("%02x",opretbuf[i]);
    //printf(" opretlen.%d (%s)\n",opretlen,base);
    //printf(" opretlen.%d vs %d incr.%d (%d)\n",opretlen,(int32_t)(2*sizeof(fiatoshis) + 2*sizeof(height) + 20 + 2),incr,opretlen/incr);
    //if ( ASSETCHAINS_SYMBOL[0] == 0 || strncmp(ASSETCHAINS_SYMBOL,base,strlen(base)) == 0 )
    {
        type = opretbuf[0];
        opretbuf++, opretlen--;
        for (n=0; n<opretlen/incr; n++)
        {
            if ( issafecoin != 0 )
            {
                memset(&p,0,sizeof(p));
                len += safecoin_rwapproval(0,&opretbuf[len],&p);
                if ( values != 0 && srcvalues != 0 && safeheights != 0 && otherheights != 0 && baseids != 0 && rmd160s != 0 )
                {
                    txids[n] = p.txid;
                    vouts[n] = p.vout;
                    values[n] = (strcmp("SAFE",base) == 0) ? p.safecoinshis : p.fiatoshis;
                    srcvalues[n] = (strcmp("SAFE",base) == 0) ? p.fiatoshis : p.safecoinshis;
                    safeheights[n] = p.height;
                    otherheights[n] = p.otherheight;
                    memcpy(&rmd160s[n * 20],p.rmd160,20);
                    baseids[n] = safecoin_baseid(p.source);
                    if ( 0 )
                    {
                        char coinaddr[64];
                        bitcoin_address(coinaddr,60,&rmd160s[n * 20],20);
                        printf(">>>>>>> %s: (%s) fiat %.8f safeheight.%d other.%d -> %s %.8f\n",type=='A'?"approvedA":"issuedX",baseids[n]>=0?CURRENCIES[baseids[n]]:"???",dstr(p.fiatoshis),safeheights[n],otherheights[n],coinaddr,dstr(values[n]));
                    }
                }
            }
            else
            {
                for (i=0; i<4; i++)
                    base[i] = opretbuf[opretlen-4+i];
                for (j=0; j<32; j++)
                {
                    ((uint8_t *)&txids[n])[j] = opretbuf[len++];
                    //printf("%02x",((uint8_t *)&txids[n])[j]);
                }
                vouts[n] = opretbuf[len++];
                vouts[n] = (opretbuf[len++] << 8) | vouts[n];
                baseids[n] = safecoin_baseid(base);
                if ( (pax= safecoin_paxfinds(txids[n],vouts[n])) != 0 )
                {
                    values[n] = (strcmp("SAFE",base) == 0) ? pax->safecoinshis : pax->fiatoshis;
                    srcvalues[n] = (strcmp("SAFE",base) == 0) ? pax->fiatoshis : pax->safecoinshis;
                    safeheights[n] = pax->height;
                    otherheights[n] = pax->otherheight;
                    memcpy(&rmd160s[n * 20],pax->rmd160,20);
                }
            }
            //printf(" safecoin_issued_opreturn issuedtxid v%d i.%d opretlen.%d\n",vouts[n],n,opretlen);
        }
    }
    return(n);
}

int32_t safecoin_paxcmp(char *symbol,int32_t safeheight,uint64_t value,uint64_t checkvalue,uint64_t seed)
{
    int32_t ratio;
    if ( seed == 0 && checkvalue != 0 )
    {
        ratio = ((value << 6) / checkvalue);
        if ( ratio >= 63 && ratio <= 65 )
            return(0);
        else
        {
            if ( safeheight >= 86150 )
                printf("ht.%d ignore mismatched %s value %lld vs checkvalue %lld -> ratio.%d\n",safeheight,symbol,(long long)value,(long long)checkvalue,ratio);
            return(-1);
        }
    }
    else if ( checkvalue != 0 )
    {
        ratio = ((value << 10) / checkvalue);
        if ( ratio >= 1023 && ratio <= 1025 )
            return(0);
    }
    return(value != checkvalue);
}

uint64_t safecoin_paxtotal()
{
    struct pax_transaction *pax,*pax2,*tmp,*tmp2; char symbol[16],dest[16],*str; int32_t i,ht; int64_t checktoshis; uint64_t seed,total = 0; struct safecoin_state *basesp;
    if ( SAFECOIN_PAX == 0 )
        return(0);
    if ( safecoin_isrealtime(&ht) == 0 )
        return(0);
    else
    {
        HASH_ITER(hh,PAX,pax,tmp)
        {
            if ( pax->marked != 0 )
                continue;
            if ( pax->type == 'A' || pax->type == 'D' || pax->type == 'X' )
                str = pax->symbol;
            else str = pax->source;
            basesp = safecoin_stateptrget(str);
            if ( basesp != 0 && pax->didstats == 0 )
            {
                if ( pax->type == 'I' && (pax2= safecoin_paxfind(pax->txid,pax->vout,'D')) != 0 )
                {
                    if ( pax2->fiatoshis != 0 )
                    {
                        pax->safecoinshis = pax2->safecoinshis;
                        pax->fiatoshis = pax2->fiatoshis;
                        basesp->issued += pax->fiatoshis;
                        pax->didstats = 1;
                        if ( strcmp(str,ASSETCHAINS_SYMBOL) == 0 )
                            printf("########### %p issued %s += %.8f safeheight.%d %.8f other.%d\n",basesp,str,dstr(pax->fiatoshis),pax->height,dstr(pax->safecoinshis),pax->otherheight);
                        pax2->marked = pax->height;
                        pax->marked = pax->height;
                    }
                }
                else if ( pax->type == 'W' )
                {
                    //bitcoin_address(coinaddr,addrtype,rmd160,20);
                    if ( (checktoshis= safecoin_paxprice(&seed,pax->height,pax->source,(char *)"SAFE",(uint64_t)pax->fiatoshis)) != 0 )
                    {
                        if ( safecoin_paxcmp(pax->source,pax->height,pax->safecoinshis,checktoshis,seed) != 0 )
                        {
                            pax->marked = pax->height;
                            //printf("WITHDRAW.%s mark <- %d %.8f != %.8f\n",pax->source,pax->height,dstr(checktoshis),dstr(pax->safecoinshis));
                        }
                        else if ( pax->validated == 0 )
                        {
                            pax->validated = pax->safecoinshis = checktoshis;
                            //int32_t j; for (j=0; j<32; j++)
                            //    printf("%02x",((uint8_t *)&pax->txid)[j]);
                            //if ( strcmp(str,ASSETCHAINS_SYMBOL) == 0 )
                            //    printf(" v%d %p got WITHDRAW.%s safe.%d ht.%d %.8f -> %.8f/%.8f\n",pax->vout,pax,pax->source,pax->height,pax->otherheight,dstr(pax->fiatoshis),dstr(pax->safecoinshis),dstr(checktoshis));
                        }
                    }
                }
            }
        }
    }
    safecoin_stateptr(symbol,dest);
    HASH_ITER(hh,PAX,pax,tmp)
    {
        pax->ready = 0;
        if ( 0 && pax->type == 'A' )
            printf("%p pax.%s <- %s marked.%d %.8f -> %.8f validated.%d approved.%d\n",pax,pax->symbol,pax->source,pax->marked,dstr(pax->safecoinshis),dstr(pax->fiatoshis),pax->validated != 0,pax->approved != 0);
        if ( pax->marked != 0 )
            continue;
        if ( strcmp(symbol,pax->symbol) == 0 || pax->type == 'A' )
        {
            if ( pax->marked == 0 )
            {
                if ( safecoin_is_issuer() != 0 )
                {
                    if ( pax->validated != 0 && pax->type == 'D' )
                    {
                        total += pax->fiatoshis;
                        pax->ready = 1;
                    }
                }
                else if ( pax->approved != 0 && pax->type == 'A' )
                {
                    if ( pax->validated != 0 )
                    {
                        total += pax->safecoinshis;
                        pax->ready = 1;
                    }
                    else
                    {
                        seed = 0;
                        checktoshis = safecoin_paxprice(&seed,pax->height,pax->source,(char *)"SAFE",(uint64_t)pax->fiatoshis);
                        //printf("paxtotal PAX_fiatdest ht.%d price %s %.8f -> SAFE %.8f vs %.8f\n",pax->height,pax->symbol,(double)pax->fiatoshis/COIN,(double)pax->safecoinshis/COIN,(double)checktoshis/COIN);
                        //printf(" v%d %.8f k.%d ht.%d\n",pax->vout,dstr(pax->safecoinshis),pax->height,pax->otherheight);
                        if ( seed != 0 && checktoshis != 0 )
                        {
                            if ( checktoshis == pax->safecoinshis )
                            {
                                total += pax->safecoinshis;
                                pax->validated = pax->safecoinshis;
                                pax->ready = 1;
                            } else pax->marked = pax->height;
                        }
                    }
                }
                if ( 0 && pax->ready != 0 )
                    printf("%p (%c) pax.%s marked.%d %.8f -> %.8f validated.%d approved.%d\n",pax,pax->type,pax->symbol,pax->marked,dstr(pax->safecoinshis),dstr(pax->fiatoshis),pax->validated != 0,pax->approved != 0);
            }
        }
    }
    //printf("paxtotal %.8f\n",dstr(total));
    return(total);
}

static int _paxorder(const void *a,const void *b)
{
#define pax_a (*(struct pax_transaction **)a)
#define pax_b (*(struct pax_transaction **)b)
    uint64_t aval,bval;
    aval = pax_a->fiatoshis + pax_a->safecoinshis + pax_a->height;
    bval = pax_b->fiatoshis + pax_b->safecoinshis + pax_b->height;
	if ( bval > aval )
		return(-1);
	else if ( bval < aval )
		return(1);
	return(0);
#undef pax_a
#undef pax_b
}

int32_t safecoin_pending_withdraws(char *opretstr) // todo: enforce deterministic order
{
    struct pax_transaction *pax,*pax2,*tmp,*paxes[64]; uint8_t opretbuf[16384]; int32_t i,n,ht,len=0; uint64_t total = 0;
    if ( SAFECOIN_PAX == 0 )
        return(0);
    if ( safecoin_isrealtime(&ht) == 0 || ASSETCHAINS_SYMBOL[0] != 0 )
        return(0);
    n = 0;
    HASH_ITER(hh,PAX,pax,tmp)
    {
        if ( pax->type == 'W' )
        {
            if ( (pax2= safecoin_paxfind(pax->txid,pax->vout,'A')) != 0 )
            {
                if ( pax2->approved != 0 )
                    pax->approved = pax2->approved;
            }
            else if ( (pax2= safecoin_paxfind(pax->txid,pax->vout,'X')) != 0 )
                pax->approved = pax->height;
            //printf("pending_withdraw: pax %s marked.%u approved.%u validated.%llu\n",pax->symbol,pax->marked,pax->approved,(long long)pax->validated);
            if ( pax->marked == 0 && pax->approved == 0 && pax->validated != 0 ) //strcmp((char *)"SAFE",pax->symbol) == 0 &&
            {
                if ( n < sizeof(paxes)/sizeof(*paxes) )
                {
                    paxes[n++] = pax;
                    //int32_t j; for (j=0; j<32; j++)
                    //    printf("%02x",((uint8_t *)&pax->txid)[j]);
                    //printf(" %s.(safeht.%d ht.%d marked.%u approved.%d validated %.8f) %.8f\n",pax->source,pax->height,pax->otherheight,pax->marked,pax->approved,dstr(pax->validated),dstr(pax->safecoinshis));
                }
            }
        }
    }
    opretstr[0] = 0;
    if ( n > 0 )
    {
        opretbuf[len++] = 'A';
        qsort(paxes,n,sizeof(*paxes),_paxorder);
        for (i=0; i<n; i++)
        {
            if ( len < (sizeof(opretbuf)>>3)*7 )
                len += safecoin_rwapproval(1,&opretbuf[len],paxes[i]);
        }
        if ( len > 0 )
            init_hexbytes_noT(opretstr,opretbuf,len);
    }
    //fprintf(stderr,"safecoin_pending_withdraws len.%d PAXTOTAL %.8f\n",len,dstr(safecoin_paxtotal()));
    return(len);
}

int32_t safecoin_gateway_deposits(CMutableTransaction *txNew,char *base,int32_t tosafecoin)
{
    struct pax_transaction *pax,*tmp; char symbol[16],dest[16]; uint8_t *script,opcode,opret[16384],data[16384]; int32_t i,baseid,ht,len=0,opretlen=0,numvouts=1; struct safecoin_state *sp; uint64_t available,deposited,issued,withdrawn,approved,redeemed,mask;
    if ( SAFECOIN_PAX == 0 )
        return(0);
    struct safecoin_state *safesp = safecoin_stateptrget((char *)"SAFE");
    sp = safecoin_stateptr(symbol,dest);
    strcpy(symbol,base);
    if ( ASSETCHAINS_SYMBOL[0] != 0 && safecoin_baseid(ASSETCHAINS_SYMBOL) < 0 )
        return(0);
    PENDING_SAFECOIN_TX = 0;
    if ( tosafecoin == 0 )
    {
        opcode = 'I';
        if ( safecoin_isrealtime(&ht) == 0 )
            return(0);
    }
    else
    {
        opcode = 'X';
        if ( safecoin_paxtotal() == 0 )
            return(0);
    }
    HASH_ITER(hh,PAX,pax,tmp)
    {
        if ( pax->type != 'D' && pax->type != 'A' )
            continue;
        {
#ifdef SAFECOIN_ASSETCHAINS_WAITNOTARIZE
            if ( safesp != 0 && (safesp->NOTARIZED_HEIGHT >= pax->height || safesp->CURRENT_HEIGHT > pax->height+30) ) // assumes same chain as notarize
                pax->validated = pax->safecoinshis; //safesp->NOTARIZED_HEIGHT;
            else pax->validated = pax->ready = 0;
#endif
        }
        if ( ASSETCHAINS_SYMBOL[0] != 0 && (pax_fiatstatus(&available,&deposited,&issued,&withdrawn,&approved,&redeemed,symbol) != 0 || available < pax->fiatoshis) )
        {
            if ( strcmp(ASSETCHAINS_SYMBOL,symbol) == 0 )
                printf("miner.[%s]: skip %s %.8f when avail %.8f\n",ASSETCHAINS_SYMBOL,symbol,dstr(pax->fiatoshis),dstr(available));
            continue;
        }
        /*printf("pax.%s marked.%d %.8f -> %.8f ready.%d validated.%d\n",pax->symbol,pax->marked,dstr(pax->safecoinshis),dstr(pax->fiatoshis),pax->ready!=0,pax->validated!=0);
        if ( pax->marked != 0 || (pax->type != 'D' && pax->type != 'A') || pax->ready == 0 )
        {
            printf("reject 2\n");
            continue;
        }*/
        if ( ASSETCHAINS_SYMBOL[0] != 0 && (strcmp(pax->symbol,symbol) != 0 || pax->validated == 0) )
        {
            //printf("pax->symbol.%s != %s or null pax->validated %.8f\n",pax->symbol,symbol,dstr(pax->validated));
            continue;
        }
        if ( pax->ready == 0 )
            continue;
        if ( pax->type == 'A' && ASSETCHAINS_SYMBOL[0] == 0 )
        {
            if ( safesp != 0 )
            {
                if ( (baseid= safecoin_baseid(pax->symbol)) < 0 || ((1LL << baseid) & sp->RTmask) == 0 )
                {
                    printf("not RT for (%s) %llx baseid.%d %llx\n",pax->symbol,(long long)sp->RTmask,baseid,(long long)(1LL<<baseid));
                    continue;
                }
            } else continue;
        }

        //printf("redeem.%d? (%c) %p pax.%s marked.%d %.8f -> %.8f ready.%d validated.%d approved.%d\n",tosafecoin,pax->type,pax,pax->symbol,pax->marked,dstr(pax->safecoinshis),dstr(pax->fiatoshis),pax->ready!=0,pax->validated!=0,pax->approved!=0);
        if ( 0 && ASSETCHAINS_SYMBOL[0] != 0 )
            printf("pax.%s marked.%d %.8f -> %.8f\n",ASSETCHAINS_SYMBOL,pax->marked,dstr(pax->safecoinshis),dstr(pax->fiatoshis));
        txNew->vout.resize(numvouts+1);
        txNew->vout[numvouts].nValue = (opcode == 'I') ? pax->fiatoshis : pax->safecoinshis;
        txNew->vout[numvouts].scriptPubKey.resize(25);
        script = (uint8_t *)&txNew->vout[numvouts].scriptPubKey[0];
        *script++ = 0x76;
        *script++ = 0xa9;
        *script++ = 20;
        memcpy(script,pax->rmd160,20), script += 20;
        *script++ = 0x88;
        *script++ = 0xac;
        if ( tosafecoin == 0 )
        {
            for (i=0; i<32; i++)
                data[len++] = ((uint8_t *)&pax->txid)[i];
            data[len++] = pax->vout & 0xff;
            data[len++] = (pax->vout >> 8) & 0xff;
            PENDING_SAFECOIN_TX += pax->fiatoshis;
        }
        else
        {
            len += safecoin_rwapproval(1,&data[len],pax);
            PENDING_SAFECOIN_TX += pax->safecoinshis;
            printf(" len.%d vout.%u DEPOSIT %.8f <- pax.%s pending %.8f | ",len,pax->vout,(double)txNew->vout[numvouts].nValue/COIN,symbol,dstr(PENDING_SAFECOIN_TX));
        }
        if ( numvouts++ >= 64 )
            break;
    }
    if ( numvouts > 1 )
    {
        if ( tosafecoin != 0 )
            strcpy(symbol,(char *)"SAFE");
        for (i=0; symbol[i]!=0; i++)
            data[len++] = symbol[i];
        data[len++] = 0;
        for (i=0; i<len; i++)
            printf("%02x",data[i]);
        printf(" <- data[%d]\n",len);
        opretlen = safecoin_opreturnscript(opret,opcode,data,len);
        txNew->vout.resize(numvouts+1);
        txNew->vout[numvouts].nValue = 0;
        txNew->vout[numvouts].scriptPubKey.resize(opretlen);
        script = (uint8_t *)&txNew->vout[numvouts].scriptPubKey[0];
        memcpy(script,opret,opretlen);
        for (i=0; i<8; i++)
            printf("%02x",opret[i]);
        printf(" <- opret, MINER deposits.%d (%s) vouts.%d %.8f opretlen.%d\n",tosafecoin,ASSETCHAINS_SYMBOL,numvouts,dstr(PENDING_SAFECOIN_TX),opretlen);
        return(1);
    }
    return(0);
}

int32_t safecoin_check_deposit(int32_t height,const CBlock& block) // verify above block is valid pax pricing
{
    int32_t i,j,n,ht,num,opretlen,offset=1,errs=0,matched=0,safeheights[64],otherheights[64]; uint256 hash,txids[64]; char symbol[16],base[16]; uint16_t vouts[64]; int8_t baseids[64]; uint8_t *script,opcode,rmd160s[64*20]; uint64_t available,deposited,issued,withdrawn,approved,redeemed; int64_t values[64],srcvalues[64]; struct pax_transaction *pax;
    if ( SAFECOIN_PAX == 0 )
        return(0);
    memset(baseids,0xff,sizeof(baseids));
    memset(values,0,sizeof(values));
    memset(srcvalues,0,sizeof(srcvalues));
    memset(rmd160s,0,sizeof(rmd160s));
    memset(safeheights,0,sizeof(safeheights));
    memset(otherheights,0,sizeof(otherheights));
    n = block.vtx[0].vout.size();
    script = (uint8_t *)block.vtx[0].vout[n-1].scriptPubKey.data();
    if ( n <= 2 || script[0] != 0x6a )
        return(0);
    offset += safecoin_scriptitemlen(&opretlen,&script[offset]);
    if ( ASSETCHAINS_SYMBOL[0] == 0 )
    {
        //for (i=0; i<opretlen; i++)
        //    printf("%02x",script[i]);
        //printf(" height.%d checkdeposit n.%d [%02x] [%c] %d vs %d\n",height,n,script[0],script[offset],script[offset],'X');
        opcode = 'X';
        strcpy(symbol,(char *)"SAFE");
    }
    else
    {
        strcpy(symbol,ASSETCHAINS_SYMBOL);
        opcode = 'I';
        if ( safecoin_baseid(symbol) < 0 )
        {
            if ( block.vtx[0].vout.size() != 1 )
            {
                printf("%s has more than one coinbase?\n",symbol);
                return(-1);
            }
            return(0);
        }
    }
    if ( script[offset] == opcode && opretlen < block.vtx[0].vout[n-1].scriptPubKey.size() )
    {
        if ( (num= safecoin_issued_opreturn(base,txids,vouts,values,srcvalues,safeheights,otherheights,baseids,rmd160s,&script[offset],opretlen,opcode == 'X')) > 0 )
        {
            for (i=1; i<n-1; i++)
            {
                if ( (pax= safecoin_paxfinds(txids[i-1],vouts[i-1])) != 0 ) // finds... make sure right one
                {
                    pax->type = opcode;
                    if ( opcode == 'I' && pax_fiatstatus(&available,&deposited,&issued,&withdrawn,&approved,&redeemed,symbol) != 0 || available < pax->fiatoshis )
                    {
                        printf("checkdeposit: skip %s %.8f when avail %.8f\n",pax->symbol,dstr(pax->fiatoshis),dstr(available));
                        continue;
                    }
                    if ( ((opcode == 'I' && (pax->fiatoshis == 0 || pax->fiatoshis == block.vtx[0].vout[i].nValue)) || (opcode == 'X' && (pax->safecoinshis == 0 || pax->safecoinshis == block.vtx[0].vout[i].nValue))) )
                    {
                        if ( pax->marked != 0 && height >= 80820 )
                        {
                            printf(">>>>>>>>>>> %c errs.%d i.%d match %.8f vs %.8f paxmarked.%d kht.%d ht.%d\n",opcode,errs,i,dstr(opcode == 'I' ? pax->fiatoshis : pax->safecoinshis),dstr(block.vtx[0].vout[i].nValue),pax->marked,pax->height,pax->otherheight);
                            if ( pax->safecoinshis != 0 || pax->fiatoshis != 0 )
                                errs++;
                            else matched++; // onetime init bypass
                        }
                        else
                        {
                            if ( opcode == 'X' )
                                printf("check deposit validates %s %.8f -> %.8f\n",CURRENCIES[baseids[i]],dstr(srcvalues[i]),dstr(values[i]));
                            matched++;
                        }
                    }
                    else
                    {
                        for (j=0; j<32; j++)
                            printf("%02x",((uint8_t *)&txids[i-1])[j]);
                        printf(" cant paxfind %c txid\n",opcode);
                        printf(">>>>>>>>>>> %c errs.%d i.%d match %.8f vs %.8f pax.%p\n",opcode,errs,i,dstr(opcode == 'I' ? pax->fiatoshis : pax->safecoinshis),dstr(block.vtx[0].vout[i].nValue),pax);
                    }
                }
                else
                {
                    hash = block.GetHash();
                    for (j=0; j<32; j++)
                        printf("%02x",((uint8_t *)&hash)[j]);
                    printf(" kht.%d ht.%d %.8f %.8f blockhash couldnt find vout.[%d]\n",safeheights[i-1],otherheights[i-1],dstr(values[i-1]),dstr(srcvalues[i]),i);
                }
            }
            if ( (height < chainActive.Tip()->nHeight || (height >= chainActive.Tip()->nHeight && safecoin_isrealtime(&ht) != 0)) && matched != num )
            {
                printf("WOULD REJECT %s: ht.%d (%c) matched.%d vs num.%d tip.%d isRT.%d\n",symbol,height,opcode,matched,num,(int32_t)chainActive.Tip()->nHeight,safecoin_isrealtime(&ht));
                // can easily happen depending on order of loading
                if ( height > 200000 )
                {
                    printf("REJECT: ht.%d (%c) matched.%d vs num.%d\n",height,opcode,matched,num);
                    return(-1);
                }
            }
        }
        //printf("opretlen.%d num.%d\n",opretlen,num);
    }
    return(0);
}

const char *safecoin_opreturn(int32_t height,uint64_t value,uint8_t *opretbuf,int32_t opretlen,uint256 txid,uint16_t vout,char *source)
{
    uint8_t rmd160[20],rmd160s[64*20],addrtype,shortflag,pubkey33[33]; int32_t didstats,i,j,n,kvheight,len,tosafecoin,safeheight,otherheights[64],safeheights[64]; int8_t baseids[64]; char base[4],coinaddr[64],destaddr[64]; uint256 txids[64]; uint16_t vouts[64]; uint64_t convtoshis,seed; int64_t fee,fiatoshis,safecoinshis,checktoshis,values[64],srcvalues[64]; struct pax_transaction *pax,*pax2; struct safecoin_state *basesp; double diff; 
    const char *typestr = "unknown";
    if ( ASSETCHAINS_SYMBOL[0] != 0 && safecoin_baseid(ASSETCHAINS_SYMBOL) < 0 && opretbuf[0] != 'K' )
    {
        //printf("safecoin_opreturn skip %s\n",ASSETCHAINS_SYMBOL);
        return("assetchain");
    }
    //else if ( SAFECOIN_PAX == 0 )
    //    return("nopax");
    memset(baseids,0xff,sizeof(baseids));
    memset(values,0,sizeof(values));
    memset(srcvalues,0,sizeof(srcvalues));
    memset(rmd160s,0,sizeof(rmd160s));
    memset(safeheights,0,sizeof(safeheights));
    memset(otherheights,0,sizeof(otherheights));
    tosafecoin = (safecoin_is_issuer() == 0);
    if ( opretbuf[0] == 'K' && opretlen != 40 )
    {
        safecoin_kvupdate(opretbuf,opretlen,value);
    }
    else if ( opretbuf[0] == 'D' )
    {
        tosafecoin = 0;
        if ( opretlen == 38 ) // any SAFE tx
        {
            iguana_rwnum(0,&opretbuf[34],sizeof(safeheight),&safeheight);
            memset(base,0,sizeof(base));
            PAX_pubkey(0,&opretbuf[1],&addrtype,rmd160,base,&shortflag,&fiatoshis);
            bitcoin_address(coinaddr,addrtype,rmd160,20);
            checktoshis = PAX_fiatdest(&seed,tosafecoin,destaddr,pubkey33,coinaddr,safeheight,base,fiatoshis);
            typestr = "deposit";
            if ( safeheight <= height )
            {
                didstats = 0;
                if ( 0 && strcmp(base,ASSETCHAINS_SYMBOL) == 0 )
                {
                    printf("(%s) (%s) safeheight.%d vs height.%d check %.8f vs %.8f tosafecoin.%d %d seed.%llx\n",ASSETCHAINS_SYMBOL,base,safeheight,height,dstr(checktoshis),dstr(value),safecoin_is_issuer(),strncmp(ASSETCHAINS_SYMBOL,base,strlen(base)) == 0,(long long)seed);
                    for (i=0; i<32; i++)
                        printf("%02x",((uint8_t *)&txid)[i]);
                    printf(" <- txid.v%u ",vout);
                    for (i=0; i<33; i++)
                        printf("%02x",pubkey33[i]);
                    printf(" checkpubkey check %.8f v %.8f dest.(%s) safeheight.%d height.%d\n",dstr(checktoshis),dstr(value),destaddr,safeheight,height);
                }
                if ( safecoin_paxcmp(base,safeheight,value,checktoshis,seed) == 0 )
                {
                    if ( (pax= safecoin_paxfind(txid,vout,'D')) == 0 )
                    {
                        if ( (basesp= safecoin_stateptrget(base)) != 0 )
                        {
                            basesp->deposited += fiatoshis;
                            didstats = 1;
                            if ( 0 && strcmp(base,ASSETCHAINS_SYMBOL) == 0 )
                                printf("########### %p deposited %s += %.8f safeheight.%d %.8f\n",basesp,base,dstr(fiatoshis),safeheight,dstr(value));
                        } else printf("cant get stateptr.(%s)\n",base);
                        safecoin_gateway_deposit(coinaddr,value,base,fiatoshis,rmd160,txid,vout,'D',safeheight,height,(char *)"SAFE",0);
                    }
                    if ( (pax= safecoin_paxfind(txid,vout,'D')) != 0 )
                    {
                        pax->height = safeheight;
                        pax->validated = value;
                        pax->safecoinshis = value;
                        pax->fiatoshis = fiatoshis;
                        if ( didstats == 0 && pax->didstats == 0 )
                        {
                            if ( (basesp= safecoin_stateptrget(base)) != 0 )
                            {
                                basesp->deposited += fiatoshis;
                                didstats = 1;
                                if ( 0 && strcmp(base,ASSETCHAINS_SYMBOL) == 0 )
                                    printf("########### %p deposited %s += %.8f/%.8f safeheight.%d/%d %.8f/%.8f\n",basesp,base,dstr(fiatoshis),dstr(pax->fiatoshis),safeheight,pax->height,dstr(value),dstr(pax->safecoinshis));
                            }
                        }
                        if ( didstats != 0 )
                            pax->didstats = 1;
                        if ( (pax2= safecoin_paxfind(txid,vout,'I')) != 0 )
                        {
                            pax2->fiatoshis = pax->fiatoshis;
                            pax2->safecoinshis = pax->safecoinshis;
                            pax->marked = pax2->marked = pax->height;
                            pax2->height = pax->height = height;
                            if ( pax2->didstats == 0 )
                            {
                                if ( (basesp= safecoin_stateptrget(base)) != 0 )
                                {
                                    basesp->issued += pax2->fiatoshis;
                                    pax2->didstats = 1;
                                    if ( 0 && strcmp(base,ASSETCHAINS_SYMBOL) == 0 )
                                        printf("########### %p issueda %s += %.8f safeheight.%d %.8f other.%d\n",basesp,base,dstr(pax2->fiatoshis),pax2->height,dstr(pax2->safecoinshis),pax2->otherheight);
                                }
                            }
                        }
                    }
                }
                else if ( seed != 0 && safeheight > 91800 && strcmp(base,ASSETCHAINS_SYMBOL) == 0 )
                    printf("pax %s deposit %.8f rejected safeheight.%d %.8f SAFE check %.8f seed.%llu\n",base,dstr(fiatoshis),safeheight,dstr(value),dstr(checktoshis),(long long)seed);
            }
        }
    }
    else if ( opretbuf[0] == 'I' )
    {
        tosafecoin = 0;
        if ( strncmp((char *)"SAFE",(char *)&opretbuf[opretlen-4],3) != 0 )
        {
            if ( (n= safecoin_issued_opreturn(base,txids,vouts,values,srcvalues,safeheights,otherheights,baseids,rmd160s,opretbuf,opretlen,0)) > 0 )
            {
                for (i=0; i<n; i++)
                {
                    if ( baseids[i] < 0 )
                    {
                        printf("%d of %d illegal baseid.%d\n",i,n,baseids[i]);
                        continue;
                    }
                    bitcoin_address(coinaddr,60,&rmd160s[i*20],20);
                    safecoin_gateway_deposit(coinaddr,0,0,0,0,txids[i],vouts[i],'I',height,0,CURRENCIES[baseids[i]],0);
                    safecoin_paxmark(height,txids[i],vouts[i],'I',height);
                    if ( (pax= safecoin_paxfind(txids[i],vouts[i],'I')) != 0 )
                    {
                        pax->type = opretbuf[0];
                        strcpy(pax->source,(char *)&opretbuf[opretlen-4]);
                        if ( (pax2= safecoin_paxfind(txids[i],vouts[i],'D')) != 0 && pax2->fiatoshis != 0 && pax2->safecoinshis != 0 )
                        {
                            // realtime path?
                            pax->fiatoshis = pax2->fiatoshis;
                            pax->safecoinshis = pax2->safecoinshis;
                            pax->marked = pax2->marked = pax2->height;
                            if ( pax->didstats == 0 )
                            {
                                if ( (basesp= safecoin_stateptrget(CURRENCIES[baseids[i]])) != 0 )
                                {
                                    basesp->issued += pax->fiatoshis;
                                    pax->didstats = 1;
                                    pax->height = pax2->height;
                                    pax->otherheight = height;
                                    if ( 0 && strcmp(CURRENCIES[baseids[i]],ASSETCHAINS_SYMBOL) == 0 )
                                        printf("########### %p issuedb %s += %.8f safeheight.%d %.8f other.%d\n",basesp,CURRENCIES[baseids[i]],dstr(pax->fiatoshis),pax->height,dstr(pax->safecoinshis),pax->otherheight);
                                }
                            }
                        }
                    }
                    if ( (pax= safecoin_paxmark(height,txids[i],vouts[i],'I',height)) != 0 )
                        safecoin_paxdelete(pax);
                    if ( (pax= safecoin_paxmark(height,txids[i],vouts[i],'D',height)) != 0 )
                        safecoin_paxdelete(pax);
                }
            } //else printf("opreturn none issued?\n");
        }
    }
    else if ( opretbuf[0] == 'W' )//&& opretlen >= 38 )
    {
        if ( safecoin_baseid((char *)&opretbuf[opretlen-4]) >= 0 && strcmp("SAFE",(char *)&opretbuf[opretlen-4]) != 0 )
        {
            for (i=0; i<opretlen; i++)
                printf("%02x",opretbuf[i]);
            printf(" [%s] reject obsolete withdraw request.%s\n",ASSETCHAINS_SYMBOL,(char *)&opretbuf[opretlen-4]);
            return(typestr);
        }
        tosafecoin = 1;
        iguana_rwnum(0,&opretbuf[34],sizeof(safeheight),&safeheight);
        memset(base,0,sizeof(base));
        PAX_pubkey(0,&opretbuf[1],&addrtype,rmd160,base,&shortflag,&safecoinshis);
        bitcoin_address(coinaddr,addrtype,rmd160,20);
        checktoshis = PAX_fiatdest(&seed,tosafecoin,destaddr,pubkey33,coinaddr,safeheight,base,value);
        typestr = "withdraw";
        //printf(" [%s] WITHDRAW %s.height.%d vs height.%d check %.8f/%.8f vs %.8f tosafecoin.%d %d seed.%llx -> (%s) len.%d\n",ASSETCHAINS_SYMBOL,base,safeheight,height,dstr(checktoshis),dstr(safecoinshis),dstr(value),safecoin_is_issuer(),strncmp(ASSETCHAINS_SYMBOL,base,strlen(base)) == 0,(long long)seed,coinaddr,opretlen);
        didstats = 0;
        //if ( safecoin_paxcmp(base,safeheight,safecoinshis,checktoshis,seed) == 0 )
        {
            if ( value != 0 && ((pax= safecoin_paxfind(txid,vout,'W')) == 0 || pax->didstats == 0) )
            {
                if ( (basesp= safecoin_stateptrget(base)) != 0 )
                {
                    basesp->withdrawn += value;
                    didstats = 1;
                    if ( strcmp(base,ASSETCHAINS_SYMBOL) == 0 )
                        printf("########### %p withdrawn %s += %.8f check %.8f\n",basesp,base,dstr(value),dstr(checktoshis));
                }
                if ( 0 && strcmp(base,"RUB") == 0 && (pax == 0 || pax->approved == 0) )
                    printf("notarize %s %.8f -> %.8f safe.%d other.%d\n",ASSETCHAINS_SYMBOL,dstr(value),dstr(safecoinshis),safeheight,height);
            }
            safecoin_gateway_deposit(coinaddr,0,(char *)"SAFE",value,rmd160,txid,vout,'W',safeheight,height,source,0);
            if ( (pax= safecoin_paxfind(txid,vout,'W')) != 0 )
            {
                pax->type = opretbuf[0];
                strcpy(pax->source,base);
                strcpy(pax->symbol,"SAFE");
                pax->height = safeheight;
                pax->otherheight = height;
                pax->safecoinshis = safecoinshis;
            }
        } // else printf("withdraw %s paxcmp ht.%d %d error value %.8f -> %.8f vs %.8f\n",base,safeheight,height,dstr(value),dstr(safecoinshis),dstr(checktoshis));
        // need to allocate pax
    }
    else if ( tosafecoin != 0 && opretbuf[0] == 'A' )
    {
        tosafecoin = 1;
        if ( 0 && ASSETCHAINS_SYMBOL[0] != 0 )
        {
            for (i=0; i<opretlen; i++)
                printf("%02x",opretbuf[i]);
            printf(" opret[%c] else path tosafecoin.%d ht.%d before %.8f opretlen.%d\n",opretbuf[0],tosafecoin,height,dstr(safecoin_paxtotal()),opretlen);
        }
        if ( (n= safecoin_issued_opreturn(base,txids,vouts,values,srcvalues,safeheights,otherheights,baseids,rmd160s,opretbuf,opretlen,1)) > 0 )
        {
            for (i=0; i<n; i++)
            {
                //for (j=0; j<32; j++)
                //    printf("%02x",((uint8_t *)&txids[i])[j]);
                //printf(" v%d %.8f %.8f k.%d ht.%d base.%d\n",vouts[i],dstr(values[i]),dstr(srcvalues[i]),safeheights[i],otherheights[i],baseids[i]);
                if ( baseids[i] < 0 )
                {
                    for (i=0; i<opretlen; i++)
                        printf("%02x",opretbuf[i]);
                    printf(" opret[%c] else path tosafecoin.%d ht.%d before %.8f opretlen.%d\n",opretbuf[0],tosafecoin,height,dstr(safecoin_paxtotal()),opretlen);
                    //printf("baseids[%d] %d\n",i,baseids[i]);
                    if ( (pax= safecoin_paxfind(txids[i],vouts[i],'W')) != 0 || (pax= safecoin_paxfind(txids[i],vouts[i],'X')) != 0 )
                    {
                        baseids[i] = safecoin_baseid(pax->symbol);
                        printf("override neg1 with (%s)\n",pax->symbol);
                    }
                    if ( baseids[i] < 0 )
                        continue;
                }
                didstats = 0;
                seed = 0;
                checktoshis = safecoin_paxprice(&seed,safeheights[i],CURRENCIES[baseids[i]],(char *)"SAFE",(uint64_t)values[i]);
                //printf("PAX_fiatdest ht.%d price %s %.8f -> SAFE %.8f vs %.8f\n",safeheights[i],CURRENCIES[baseids[i]],(double)values[i]/COIN,(double)srcvalues[i]/COIN,(double)checktoshis/COIN);
                if ( srcvalues[i] == checktoshis )
                {
                    if ( (pax= safecoin_paxfind(txids[i],vouts[i],'A')) == 0 )
                    {
                        bitcoin_address(coinaddr,60,&rmd160s[i*20],20);
                        safecoin_gateway_deposit(coinaddr,srcvalues[i],CURRENCIES[baseids[i]],values[i],&rmd160s[i*20],txids[i],vouts[i],'A',safeheights[i],otherheights[i],CURRENCIES[baseids[i]],safeheights[i]);
                        if ( (pax= safecoin_paxfind(txids[i],vouts[i],'A')) == 0 )
                            printf("unexpected null pax for approve\n");
                        else pax->validated = checktoshis;
                        if ( (pax2= safecoin_paxfind(txids[i],vouts[i],'W')) != 0 )
                            pax2->approved = safeheights[i];
                        safecoin_paxmark(height,txids[i],vouts[i],'W',height);
                        //safecoin_paxmark(height,txids[i],vouts[i],'A',height);
                        if ( values[i] != 0 && (basesp= safecoin_stateptrget(CURRENCIES[baseids[i]])) != 0 )
                        {
                            basesp->approved += values[i];
                            didstats = 1;
                            printf("pax.%p ########### %p approved %s += %.8f -> %.8f/%.8f kht.%d %d\n",pax,basesp,CURRENCIES[baseids[i]],dstr(values[i]),dstr(srcvalues[i]),dstr(checktoshis),safeheights[i],otherheights[i]);
                        }
                        //printf(" i.%d (%s) <- %.8f ADDFLAG APPROVED\n",i,coinaddr,dstr(values[i]));
                    }
                    else if ( pax->didstats == 0 && srcvalues[i] != 0 )
                    {
                        if ( (basesp= safecoin_stateptrget(CURRENCIES[baseids[i]])) != 0 )
                        {
                            basesp->approved += values[i];
                            didstats = 1;
                            printf("pax.%p ########### %p approved %s += %.8f -> %.8f/%.8f kht.%d %d\n",pax,basesp,CURRENCIES[baseids[i]],dstr(values[i]),dstr(srcvalues[i]),dstr(checktoshis),safeheights[i],otherheights[i]);
                        }
                    } //else printf(" i.%d of n.%d pax.%p baseids[] %d\n",i,n,pax,baseids[i]);
                    if ( (pax= safecoin_paxfind(txids[i],vouts[i],'A')) != 0 )
                    {
                        pax->type = opretbuf[0];
                        pax->approved = safeheights[i];
                        pax->validated = checktoshis;
                        if ( didstats != 0 )
                            pax->didstats = 1;
                        //if ( strcmp(CURRENCIES[baseids[i]],ASSETCHAINS_SYMBOL) == 0 )
                        //printf(" i.%d approved.%d <<<<<<<<<<<<< APPROVED %p\n",i,safeheights[i],pax);
                    }
                }
            }
        } else printf("n.%d from opreturns\n",n);
        //printf("extra.[%d] after %.8f\n",n,dstr(safecoin_paxtotal()));
    }
    else if ( opretbuf[0] == 'X' )
    {
        tosafecoin = 1;
        if ( (n= safecoin_issued_opreturn(base,txids,vouts,values,srcvalues,safeheights,otherheights,baseids,rmd160s,opretbuf,opretlen,1)) > 0 )
        {
            for (i=0; i<n; i++)
            {
                if ( baseids[i] < 0 )
                    continue;
                bitcoin_address(coinaddr,60,&rmd160s[i*20],20);
                safecoin_gateway_deposit(coinaddr,0,0,0,0,txids[i],vouts[i],'X',height,0,(char *)"SAFE",0);
                safecoin_paxmark(height,txids[i],vouts[i],'W',height);
                safecoin_paxmark(height,txids[i],vouts[i],'A',height);
                safecoin_paxmark(height,txids[i],vouts[i],'X',height);
                if ( (pax= safecoin_paxfind(txids[i],vouts[i],'X')) != 0 )
                {
                    pax->type = opretbuf[0];
                    if ( height < 121842 ) // fields got switched around due to legacy issues and approves
                        value = srcvalues[i];
                    else value = values[i];
                    if ( baseids[i] >= 0 && value != 0 && (basesp= safecoin_stateptrget(CURRENCIES[baseids[i]])) != 0 )
                    {
                        basesp->redeemed += value;
                        pax->didstats = 1;
                        if ( strcmp(CURRENCIES[baseids[i]],ASSETCHAINS_SYMBOL) == 0 )
                            printf("ht.%d %.8f ########### %p redeemed %s += %.8f %.8f kht.%d ht.%d\n",height,dstr(value),basesp,CURRENCIES[baseids[i]],dstr(value),dstr(srcvalues[i]),safeheights[i],otherheights[i]);
                    }
                }
                if ( (pax= safecoin_paxmark(height,txids[i],vouts[i],'W',height)) != 0 )
                    safecoin_paxdelete(pax);
                if ( (pax= safecoin_paxmark(height,txids[i],vouts[i],'A',height)) != 0 )
                    safecoin_paxdelete(pax);
                if ( (pax= safecoin_paxmark(height,txids[i],vouts[i],'X',height)) != 0 )
                    safecoin_paxdelete(pax);
            }
        } //else printf("safecoin_issued_opreturn returned %d\n",n);
    }
    return(typestr);
}

void safecoin_passport_iteration()
{
    static long lastpos[34]; static char userpass[33][1024];
    FILE *fp; int32_t baseid,isrealtime,refid,blocks,longest; struct safecoin_state *sp,*refsp; char *retstr,fname[512],*base,symbol[16],dest[16]; uint32_t buf[3]; cJSON *infoobj,*result; uint64_t RTmask = 0;
    //printf("PASSPORT.(%s)\n",ASSETCHAINS_SYMBOL);
    while ( SAFECOIN_INITDONE == 0 )
    {
        fprintf(stderr,"[%s] PASSPORT iteration waiting for SAFECOIN_INITDONE\n",ASSETCHAINS_SYMBOL);
        sleep(3);
    }
    refsp = safecoin_stateptr(symbol,dest);
    if ( ASSETCHAINS_SYMBOL[0] == 0 )
        refid = 33;
    else
    {
        refid = safecoin_baseid(ASSETCHAINS_SYMBOL)+1; // illegal base -> baseid.-1 -> 0
        if ( refid == 0 )
        {
            SAFECOIN_PASSPORT_INITDONE = 1;
            return;
        }
    }
    if ( SAFECOIN_PAX == 0 )
    {
        SAFECOIN_PASSPORT_INITDONE = 1;
        return;
    }
    //printf("PASSPORT %s refid.%d\n",ASSETCHAINS_SYMBOL,refid);
    for (baseid=32; baseid>=0; baseid--)
    {
        sp = 0;
        isrealtime = 0;
        base = (char *)CURRENCIES[baseid];
        if ( baseid+1 != refid )
        {
            safecoin_statefname(fname,baseid<32?base:(char *)"",(char *)"safecoinstate");
            safecoin_nameset(symbol,dest,base);
            sp = safecoin_stateptrget(symbol);
            if ( (fp= fopen(fname,"rb")) != 0 && sp != 0 )
            {
                fseek(fp,0,SEEK_END);
                if ( ftell(fp) > lastpos[baseid] )
                {
                    if ( 0 && lastpos[baseid] == 0 && strcmp(symbol,"SAFE") == 0 )
                        printf("passport refid.%d %s fname.(%s) base.%s\n",refid,symbol,fname,base);
                    fseek(fp,lastpos[baseid],SEEK_SET);
                    while ( safecoin_parsestatefile(sp,fp,symbol,dest) >= 0 )
                        ;
                    lastpos[baseid] = ftell(fp);
                    if ( 0 && lastpos[baseid] == 0 && strcmp(symbol,"SAFE") == 0 )
                        printf("from.(%s) lastpos[%s] %ld\n",ASSETCHAINS_SYMBOL,CURRENCIES[baseid],lastpos[baseid]);
                } //else fprintf(stderr,"%s.%ld ",CURRENCIES[baseid],ftell(fp));
                fclose(fp);
            } else printf("error.(%s) %p\n",fname,sp);
            safecoin_statefname(fname,baseid<32?base:(char *)"",(char *)"realtime");
            if ( (fp= fopen(fname,"rb")) != 0 )
            {
                if ( fread(buf,1,sizeof(buf),fp) == sizeof(buf) )
                {
                    sp->CURRENT_HEIGHT = buf[0];
                    if ( buf[0] != 0 && buf[0] >= buf[1] && buf[2] > time(NULL)-300 )
                    {
                        isrealtime = 1;
                        RTmask |= (1LL << baseid);
                        memcpy(refsp->RTbufs[baseid+1],buf,sizeof(refsp->RTbufs[baseid+1]));
                    } else if ( (time(NULL)-buf[2]) > 1200 )
                        fprintf(stderr,"[%s]: %s not RT %u %u %d\n",ASSETCHAINS_SYMBOL,base,buf[0],buf[1],(int32_t)(time(NULL)-buf[2]));
                } //else fprintf(stderr,"%s size error RT\n",base);
                fclose(fp);
            } //else fprintf(stderr,"%s open error RT\n",base);
        }
        else
        {
            safecoin_statefname(fname,baseid<32?base:(char *)"",(char *)"realtime");
            if ( (fp= fopen(fname,"wb")) != 0 )
            {
                buf[0] = (uint32_t)chainActive.Tip()->nHeight;
                buf[1] = (uint32_t)safecoin_longestchain();
                if ( buf[0] != 0 && buf[0] == buf[1] )
                {
                    buf[2] = (uint32_t)time(NULL);
                    RTmask |= (1LL << baseid);
                    memcpy(refsp->RTbufs[baseid+1],buf,sizeof(refsp->RTbufs[baseid+1]));
                    if ( refid != 0 )
                        memcpy(refsp->RTbufs[0],buf,sizeof(refsp->RTbufs[0]));
                }
                if ( fwrite(buf,1,sizeof(buf),fp) != sizeof(buf) )
                    fprintf(stderr,"[%s] %s error writing realtime\n",ASSETCHAINS_SYMBOL,base);
                fclose(fp);
            } else fprintf(stderr,"%s create error RT\n",base);
        }
        if ( sp != 0 && isrealtime == 0 )
            refsp->RTbufs[0][2] = 0;
    }
    safecoin_paxtotal();
    refsp->RTmask = RTmask;
    SAFECOIN_PASSPORT_INITDONE = 1;
    //printf("done PASSPORT %s refid.%d\n",ASSETCHAINS_SYMBOL,refid);
}

