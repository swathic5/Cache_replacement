#ifndef REPL_STATE_H
#define REPL_STATE_H

//Parameters for cache replacement
#define SAMPLED_SET_DISTANCE 16
#define NO_OF_SAMPLED_SETS 256 
#define REUSE_THRESHOLD 128
#define UPDATE_THRESHOLD 74
#define BYPASS_THRESHOLD 3
#define BYPASS_NO 16
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This file is distributed as part of the Cache Replacement Championship     //
// workshop held in conjunction with ISCA'2010.                               //
//                                                                            //
//                                                                            //
// Everyone is granted permission to copy, modify, and/or re-distribute       //
// this software.                                                             //
//                                                                            //
// Please contact Aamer Jaleel <ajaleel@gmail.com> should you have any        //
// questions                                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cassert>
#include "utils.h"
#include "crc_cache_defs.h"
#include <iostream>
#include <bitset>
using namespace std;

// Replacement Policies Supported
typedef enum 
{
    CRC_REPL_LRU        = 0,
    CRC_REPL_RANDOM     = 1,
    CRC_REPL_CONTESTANT = 2
} ReplacemntPolicy;

// Replacement State Per Cache Line
typedef struct
{
    UINT32  LRUstackposition;

    // CONTESTANTS: Add extra state per cache line here
    // Reuse bit for every block
    bool prediction;	
} LINE_REPLACEMENT_STATE;

typedef struct {
	unsigned long long int partial_tag;		//Partial tag for tag match
	UINT32 hash1;					//Hash values
	UINT32 hash2;
	UINT32 hash3;
	UINT32 hash4;
	UINT32 hash5;
	UINT32 hash6;
	UINT32 LRU;					//Sampler LRU bit
	bool valid;					//Valid bit
} sampler; 

typedef struct {					//Weight tables
	signed int w;
} weight_table;

// The implementation for the cache replacement policy
class CACHE_REPLACEMENT_STATE
{
public:
    LINE_REPLACEMENT_STATE   **repl;
    sampler	 	     **my_sampler;
    weight_table 	     *w1, *w2, *w3, *w4, *w5, *w6;
  private:

    UINT32 numsets;
    UINT32 assoc;
    UINT32 replPolicy;

    COUNTER mytimer;  // tracks # of references to the cache

    // CONTESTANTS:  Add extra state for cache here
    UINT32 current_PC;
    UINT32 previous_PC1;
    UINT32 previous_PC2;
    UINT32 previous_PC3;
    UINT32 previous_PC4;
    UINT32 previous_PC5;
  public:
    ostream & PrintStats(ostream &out);

    // The constructor CAN NOT be changed
    CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol );

    INT32 GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType );

    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID );

    void   SetReplacementPolicy( UINT32 _pol ) { replPolicy = _pol; } 
    void   IncrementTimer() { mytimer++; } 

    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
                                   UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit);

    ~CACHE_REPLACEMENT_STATE(void);

  private:
    
    void   InitReplacementState();
    INT32  Get_Random_Victim( UINT32 setIndex );

    INT32  Get_LRU_Victim( UINT32 setIndex );
    INT32  Get_My_Victim( UINT32 setIndex, Addr_t PC, Addr_t paddr, UINT32 accessType);
    void   UpdateLRU( UINT32 setIndex, INT32 updateWayID );
    void   UpdateMyPolicy( UINT32 setIndex, INT32 updateWayID, Addr_t current_tag, Addr_t PC, bool cacheHit, UINT32 accessType);
};

#endif
