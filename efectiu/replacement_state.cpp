#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include <map>
#include <iostream>

using namespace std;

#include "replacement_state.h"

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

/*
** This file implements the cache replacement state. Users can enhance the code
** below to develop their cache replacement ideas.
**
*/


////////////////////////////////////////////////////////////////////////////////
// The replacement state constructor:                                         //
// Inputs: number of sets, associativity, and replacement policy to use       //
// Outputs: None                                                              //
//                                                                            //
// DO NOT CHANGE THE CONSTRUCTOR PROTOTYPE                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
CACHE_REPLACEMENT_STATE::CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol )
{

    numsets    = _sets;
    assoc      = _assoc;
    replPolicy = _pol;

    mytimer    = 0;

    InitReplacementState();
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// The function prints the statistics for the cache                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
ostream & CACHE_REPLACEMENT_STATE::PrintStats(ostream &out)
{

    out<<"=========================================================="<<endl;
    out<<"=========== Replacement Policy Statistics ================"<<endl;
    out<<"=========================================================="<<endl;

    // CONTESTANTS:  Insert your statistics printing here
    
    return out;

}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function initializes the replacement policy hardware by creating      //
// storage for the replacement state on a per-line/per-cache basis.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void CACHE_REPLACEMENT_STATE::InitReplacementState()
{
    // Create the state for sets, then create the state for the ways

    repl  = new LINE_REPLACEMENT_STATE* [ numsets ];

    // ensure that we were able to create replacement state

    assert(repl);

    // Create the state for the sets
    for(UINT32 setIndex=0; setIndex<numsets; setIndex++) 
    {
        repl[ setIndex ]  = new LINE_REPLACEMENT_STATE[ assoc ];

        for(UINT32 way=0; way<assoc; way++) 
        {
            // initialize stack position (for true LRU)
            repl[ setIndex ][ way ].LRUstackposition = way;
	    repl[ setIndex ][ way ].prediction = false;
        }
    }

    //if (replPolicy == CRC_REPL_CONTESTANT) {

    // Contestants:  ADD INITIALIZATION FOR YOUR HARDWARE HERE
  
     //Sampler initializations 
     my_sampler = new sampler* [NO_OF_SAMPLED_SETS];
     assert(my_sampler);
     for(UINT32 setIndex=0; setIndex<NO_OF_SAMPLED_SETS; setIndex++){
	my_sampler[setIndex] = new sampler[assoc];
	for(UINT32 way=0; way<assoc; way++){
		my_sampler[setIndex][way].partial_tag=0;
		my_sampler[setIndex][way].hash1=0;
		my_sampler[setIndex][way].hash2=0;
		my_sampler[setIndex][way].hash3=0;
		my_sampler[setIndex][way].hash4=0;
		my_sampler[setIndex][way].hash5=0;
		my_sampler[setIndex][way].hash6=0;
		my_sampler[setIndex][way].LRU=way;
		my_sampler[setIndex][way].valid=false;	
	}
     }
     //End of sampler initializations
     
    //Weight table initializations 
     w1 = new weight_table [1024];
     w2 = new weight_table [1024];
     w3 = new weight_table [1024];
     w4 = new weight_table [1024];
     w5 = new weight_table [1024];
     w6 = new weight_table [1024];
	
     for (int i=0; i<1024; i++){
	w1[i].w = 0;
	w2[i].w = 0;
	w3[i].w = 0;
	w4[i].w = 0;
        w5[i].w = 0;
        w6[i].w = 0;
     }
     //End of weight table initializations	

}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache on every cache miss. The input        //
// argument is the set index. The return value is the physical way            //
// index for the line being replaced.                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType ) {
    // If no invalid lines, then replace based on replacement policy
    if( replPolicy == CRC_REPL_LRU ) 
    {
        return Get_LRU_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        return Get_Random_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE
	return Get_My_Victim (setIndex, PC, paddr, accessType);
    }

    // We should never here here

    assert(0);
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache after every cache hit/miss            //
// The arguments are: the set index, the physical way of the cache,           //
// the pointer to the physical line (should contestants need access           //
// to information of the line filled or hit upon), the thread id              //
// of the request, the PC of the request, the accesstype, and finall          //
// whether the line was a cachehit or not (cacheHit=true implies hit)         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateReplacementState( 
    UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
    UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit)
{
	//fprintf (stderr, "ain't I a stinker? %lld\n", get_cycle_count ());
	//fflush (stderr);
    // What replacement policy?
    if( replPolicy == CRC_REPL_LRU ) 
    {
        UpdateLRU( setIndex, updateWayID );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        // Random replacement requires no replacement state update
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
        // Feel free to use any of the input parameters to make
        // updates to your replacement policy
        UpdateMyPolicy(setIndex, updateWayID, currLine->tag, PC, cacheHit, accessType);
    }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//////// HELPER FUNCTIONS FOR REPLACEMENT UPDATE AND VICTIM SELECTION //////////
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds the LRU victim in the cache set by returning the       //
// cache block at the bottom of the LRU stack. Top of LRU stack is '0'        //
// while bottom of LRU stack is 'assoc-1'                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_LRU_Victim( UINT32 setIndex )
{
	// Get pointer to replacement state of current set

	LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];
	INT32   lruWay   = 0;

	// Search for victim whose stack position is assoc-1

	for(UINT32 way=0; way<assoc; way++) {
		if (replSet[way].LRUstackposition == (assoc-1)) {
			lruWay = way;
			break;
		}
	}

	// return lru way

	return lruWay;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds a random victim in the cache set                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_Random_Victim( UINT32 setIndex )
{
    INT32 way = (rand() % assoc);
    
    return way;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function implements the LRU update routine for the traditional        //
// LRU replacement policy. The arguments to the function are the physical     //
// way and set index.                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void CACHE_REPLACEMENT_STATE::UpdateLRU( UINT32 setIndex, INT32 updateWayID )
{
	UINT32 currLRUstackposition = repl[ setIndex ][ updateWayID ].LRUstackposition;
	for(UINT32 way=0; way<assoc; way++) {
		if( repl[setIndex][way].LRUstackposition < currLRUstackposition ) {
			repl[setIndex][way].LRUstackposition++;
		}
	}
	repl[ setIndex ][ updateWayID ].LRUstackposition = 0;
}

INT32 CACHE_REPLACEMENT_STATE::Get_My_Victim( UINT32 setIndex, Addr_t PC, Addr_t paddr, UINT32 accessType) {
       
	UINT32 current_hash1, current_hash2, current_hash3, current_hash4, current_hash5, current_hash6;
	//Compute new hash values for incoming block 
	unsigned long long int current_tag = paddr >> 18;
        current_hash1 = ((PC >> 2) ^ (PC)) & 1023;
        current_hash2 = ((current_PC >> 1) ^ (PC)) & 1023;
        current_hash3 = ((previous_PC1 >> 2) ^ (PC)) & 1023;
        current_hash4 = ((previous_PC2 >> 3) ^ (PC)) & 1023;
        current_hash5 = ((current_tag >> 4) ^ (PC)) & 1023;
        current_hash6 = ((current_tag >> 7)) & 1023;
	//End of hash value computation 
	
	//This variable way will be returned
	UINT32 way ;
	int found = 0;
	
	//Check if the block should not be bypassed		
	if(((w1[current_hash1].w + w2[current_hash2].w + w3[current_hash3].w + w4[current_hash4].w + w5[current_hash5].w + w6[current_hash6].w) < BYPASS_THRESHOLD) | (((setIndex+1)%SAMPLED_SET_DISTANCE == 0))){
			//If not bypass, look for a dead block
			if ((accessType != ACCESS_PREFETCH) & (accessType != ACCESS_WRITEBACK)){
			for( way=0; way<assoc; way++){
				if (repl[setIndex][way].prediction == false){
					found = 1;
					break;
				}
			}
			}
			
			//If no dead block, fall back on LRU
			if (!found){	
				LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];
                         	for(way=0; way<assoc; way++) {
                                	if (replSet[way].LRUstackposition == (assoc-1)) {
                                        break;
                                	}       
                         	}
			}
		}
		//End of searching for a way when no bypass
		
		//When bypass, return -1
		//Update PC registers only when it is not WRITEBACK and PREFETCH
		else{
			if ((accessType != ACCESS_WRITEBACK) | (accessType != ACCESS_PREFETCH)){ 
			previous_PC3 = previous_PC2;
		        previous_PC2 = previous_PC1;
	        	previous_PC1 = current_PC;
        		current_PC = PC;
 			}

        		way=-1;
        	}
		//End of Bypass
   	return way;
}

void CACHE_REPLACEMENT_STATE::UpdateMyPolicy( UINT32 setIndex, INT32 updateWayID, Addr_t current_tag, Addr_t PC, bool cacheHit, UINT32 accessType) {

	UINT32 current_hash1, current_hash2, current_hash3, current_hash4, current_hash5, current_hash6;
	//Compute new hashes for updating block
	
	current_hash1 = ((PC >> 2) ^ (PC)) & 1023;
        current_hash2 = ((current_PC >> 1) ^ (PC)) & 1023;
        current_hash3 = ((previous_PC1 >> 2) ^ (PC)) & 1023;
        current_hash4 = ((previous_PC2 >> 3) ^ (PC)) & 1023;
        current_hash5 = ((current_tag >> 4) ^ (PC)) & 1023;
        current_hash6 = ((current_tag >> 7)) & 1023;

	//Update sampler and LRU only when not writeback and Prefetch. No LRU update on these yields better results as the prefetch data will b evicted before actual data in cache
	if ((accessType != ACCESS_WRITEBACK ) & (accessType != ACCESS_PREFETCH)){
	previous_PC3 = previous_PC2;
	previous_PC2 = previous_PC1;
	previous_PC1 = current_PC;
	current_PC = PC;

	//Compute the sampler set index
	UINT32 INDEX=((setIndex+1)/SAMPLED_SET_DISTANCE)-1;

	//Check if it is a sampler set - If yes, update sampler
	if (((setIndex+1)%SAMPLED_SET_DISTANCE == 0)){

		
		UINT32 sampler_way=0;
		int tag_in_sampler = 0;
		
		//Look for a tag in sampler
		for (sampler_way = 0; sampler_way < assoc; sampler_way++){
			if (my_sampler[INDEX][sampler_way].partial_tag == current_tag){
				tag_in_sampler = 1;
				//std::cout << "tag found in sampler" << std::endl;
				break;
			}
		}
		
		//A tag match implies a tag hit. Decrement counters
		if (tag_in_sampler == 1){
                                if((w1[my_sampler[INDEX][sampler_way].hash1].w  - 1) > -32)  w1[my_sampler[INDEX][sampler_way].hash1].w = w1[my_sampler[INDEX][sampler_way].hash1].w - 1;
                                if((w2[my_sampler[INDEX][sampler_way].hash2].w  - 1) > -32)  w2[my_sampler[INDEX][sampler_way].hash2].w = w2[my_sampler[INDEX][sampler_way].hash2].w - 1;
                                if((w3[my_sampler[INDEX][sampler_way].hash3].w  - 1) > -32)  w3[my_sampler[INDEX][sampler_way].hash3].w = w3[my_sampler[INDEX][sampler_way].hash3].w - 1;
                                if((w4[my_sampler[INDEX][sampler_way].hash4].w  - 1) > -32)  w4[my_sampler[INDEX][sampler_way].hash4].w = w4[my_sampler[INDEX][sampler_way].hash4].w - 1;
                                if((w5[my_sampler[INDEX][sampler_way].hash5].w  - 1) > -32)  w5[my_sampler[INDEX][sampler_way].hash5].w = w5[my_sampler[INDEX][sampler_way].hash5].w - 1;
                                if((w6[my_sampler[INDEX][sampler_way].hash6].w  - 1) > -32)  w6[my_sampler[INDEX][sampler_way].hash6].w = w6[my_sampler[INDEX][sampler_way].hash6].w - 1;
			
			//Place the new hash values in the sampler
			my_sampler[INDEX][sampler_way].hash1 = current_hash1;
			my_sampler[INDEX][sampler_way].hash2 = current_hash2;
			my_sampler[INDEX][sampler_way].hash3 = current_hash3;
			my_sampler[INDEX][sampler_way].hash4 = current_hash4;
                        my_sampler[INDEX][sampler_way].hash5 = current_hash5;
                        my_sampler[INDEX][sampler_way].hash6 = current_hash6;
			

			//Update the LRU of sampler
			UINT32 currLRU = my_sampler[INDEX][sampler_way].LRU;
		
			for(UINT32 way=0; way<assoc; way++) {
				if( my_sampler[INDEX][way].LRU < currLRU) {
					my_sampler[INDEX][way].LRU++;
				}
			}
			my_sampler[INDEX][sampler_way].LRU = 0;
			}
			//End of sampler LRU update
			
			//No tag implies a tag miss - Increment counter
		else{	
			UINT32 sampler_lru_temp, sampler_lru, sampler_victim_found;
			sampler_victim_found = 0;
			sampler_lru = 0;

			//Look for victim -First look if there is an invalid block
			for(sampler_lru_temp=0; sampler_lru_temp<assoc; sampler_lru_temp++) {
                                if (my_sampler[INDEX][sampler_lru_temp].valid == false) {
                                sampler_lru = sampler_lru_temp;
				sampler_victim_found = 1;
                                break;
                                }
			}
			//If no invalid block - see if there is any block whose yout is more than threshold
			if (sampler_victim_found == 0){
				for(sampler_lru_temp=0; sampler_lru_temp<assoc; sampler_lru_temp++) {
				if((w1[my_sampler[INDEX][sampler_lru_temp].hash1].w + w2[my_sampler[INDEX][sampler_lru_temp].hash2].w + w3[my_sampler[INDEX][sampler_lru_temp].hash3].w + w4[my_sampler[INDEX][sampler_lru_temp].hash4].w + w5[my_sampler[INDEX][sampler_lru_temp].hash5].w + w6[my_sampler[INDEX][sampler_lru_temp].hash6].w) > UPDATE_THRESHOLD){

					sampler_victim_found = 1;
					sampler_lru = sampler_lru_temp;
					break;
				}
				}
			}
			//If none, just choose the LRU block to evict
			if (sampler_victim_found == 0){
			for(sampler_lru_temp=0; sampler_lru_temp<assoc; sampler_lru_temp++) {
                        	if (my_sampler[INDEX][sampler_lru_temp].LRU == (assoc-1)) {
				sampler_lru = sampler_lru_temp;
				break;
				}
			}
			}
			//End of searching block to evict

			//If sum of weights of hashes in victim is less than threshold, then increment
				if (((w1[my_sampler[INDEX][sampler_lru].hash1].w + w2[my_sampler[INDEX][sampler_lru].hash2].w + w3[my_sampler[INDEX][sampler_lru].hash3].w + w4[my_sampler[INDEX][sampler_lru].hash4].w + w5[my_sampler[INDEX][sampler_lru].hash5].w + w6[my_sampler[INDEX][sampler_lru].hash6].w) < UPDATE_THRESHOLD)& (my_sampler[INDEX][sampler_lru].valid==true)){
				if((w1[my_sampler[INDEX][sampler_lru].hash1].w + 2) < 31)  w1[my_sampler[INDEX][sampler_lru].hash1].w = w1[my_sampler[INDEX][sampler_lru].hash1].w + 2;
				if((w2[my_sampler[INDEX][sampler_lru].hash2].w + 2) < 31)  w2[my_sampler[INDEX][sampler_lru].hash2].w = w2[my_sampler[INDEX][sampler_lru].hash2].w + 2;
                                if((w3[my_sampler[INDEX][sampler_lru].hash3].w + 2) < 31)  w3[my_sampler[INDEX][sampler_lru].hash3].w = w3[my_sampler[INDEX][sampler_lru].hash3].w + 2;
                                if((w4[my_sampler[INDEX][sampler_lru].hash4].w + 2) < 31)  w4[my_sampler[INDEX][sampler_lru].hash4].w = w4[my_sampler[INDEX][sampler_lru].hash4].w + 2;
                                if((w5[my_sampler[INDEX][sampler_lru].hash5].w + 2) < 31)  w5[my_sampler[INDEX][sampler_lru].hash5].w = w5[my_sampler[INDEX][sampler_lru].hash5].w + 2;
                          	if((w6[my_sampler[INDEX][sampler_lru].hash6].w + 2) < 31)  w6[my_sampler[INDEX][sampler_lru].hash6].w = w6[my_sampler[INDEX][sampler_lru].hash6].w + 2;
			}
				//Place the new hash values, tag and valid bit in the sampler
                                my_sampler[INDEX][sampler_lru].hash1 = current_hash1;
                        	my_sampler[INDEX][sampler_lru].hash2 = current_hash2;
                        	my_sampler[INDEX][sampler_lru].hash3 = current_hash3;
                        	my_sampler[INDEX][sampler_lru].hash4 = current_hash4;
                        	my_sampler[INDEX][sampler_lru].hash5 = current_hash5;
                        	my_sampler[INDEX][sampler_lru].hash6 = current_hash6;

                        	my_sampler[INDEX][sampler_lru].partial_tag = current_tag; //Update tag
                        	my_sampler[INDEX][sampler_lru].valid = true;

			//Update LRU for new block
			UINT32 currLRU = my_sampler[INDEX][sampler_lru].LRU;
			
			for(UINT32 way=0; way<assoc; way++) {
				if( my_sampler[INDEX][way].LRU < currLRU) {
			                    my_sampler[INDEX][way].LRU++;
				}
			}
			my_sampler[INDEX][sampler_lru].LRU = 0;
			//End of LRU update
			
		}//End of miss in sampler

                }//End of sampler update

		//For any block update the prediction bit and LRU		
		if((w1[current_hash1].w + w2[current_hash2].w + w3[current_hash3].w + w4[current_hash4].w + w5[current_hash5].w + w6[current_hash6].w) < REUSE_THRESHOLD){
		repl[setIndex][updateWayID].prediction = true;
		}
		else{ 
		repl[setIndex][updateWayID].prediction = false;
		}

		UINT32 currLRUstackposition = repl[ setIndex ][ updateWayID ].LRUstackposition;
	        for(UINT32 way=0; way<assoc; way++) {
        	        if( repl[setIndex][way].LRUstackposition < currLRUstackposition ) {
                	        repl[setIndex][way].LRUstackposition++;
                	}
        	}
        	repl[ setIndex ][ updateWayID ].LRUstackposition = 0;	
		}
		//End of prediction bit and LRU update	
}

CACHE_REPLACEMENT_STATE::~CACHE_REPLACEMENT_STATE (void) {
}
