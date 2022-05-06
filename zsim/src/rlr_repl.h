#ifndef RLR_REPL_H_
#define RLR_REPL_H_

#include "repl_policies.h"

/*
notes:
memreq struct definition in memory_hierarchy.h, contains prefetch info
*/

//RLR
class RLRReplPolicy : public ReplPolicy {
    protected:
        //cache line variables
        uint32_t* ageCounter;
        bool* hitReg;
        bool* typeReg;
        

        //set variables
        
        
        //cache variables
        uint32_t RD;
        uint32_t accum;
        uint32_t missCounter;
        bool hit;
        uint32_t numLines;
        uint32_t candidates;



    public:
        // add member methods here, refer to repl_policies.h
        explicit RLRReplPolicy(uint32_t _numLines, uint32_t _candidates) : RD(0), accum(0), missCounter(0), hit(true), numLines(_numLines), candidates(_candidates){
            ageCounter = gm_calloc<uint32_t>(numLines);
            hitReg = gm_calloc<bool>(numLines);
            typeReg = gm_calloc<bool>(numLines);
        }
        
        ~RLRReplPolicy() {
            gm_free(ageCounter);
            gm_free(hitReg);
            gm_free(typeReg);
        }
        
        void update(uint32_t id, const MemReq* req) {
            //type register
            bool isPrefetch = req->flags & MemReq::PREFETCH;
            if (isPrefetch) typeReg[id]=0;
            else typeReg[id]=1;
            
            //hit register
            hitReg[id] = hit;
            
            ageCounter[id] = 0;
            hit = true;
        }
        
        void replaced(uint32_t id) {
            hit = false;
            
            //RD
            accum += ageCounter[id];
            missCounter++;
            if(missCounter==8){
                RD = accum >> 3;
                accum = 0;
                missCounter = 0;
            }
            //age counter
            uint32_t setIdx = id/candidates;
            uint32_t begin = setIdx*candidates;
            
            for(uint32_t i=begin;i<begin+candidates;i++)
                //if(ageCounter[i]<32)
                ageCounter[i]++;

            ageCounter[id] = 0;
        }
        
        template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
            uint32_t bestCand = -1;
            uint32_t Pa,Ph,Pt = 0;
            uint32_t Pline;
            uint32_t minP = UINT32_MAX;
            uint32_t minAge = UINT32_MAX;
            
            for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
                if(ageCounter[*ci] > RD)Pa = 0;
                else Pa = 8;

                if(hitReg[*ci]) Ph = 1;
                else Ph = 0;
                
                if(typeReg[*ci]) Pt = 1;
                else Pt = 0;
                
                Pline = Pa+Pt+Ph;
                
                //Recency approximation
                
                //info("Cand %d, rd %d, age %d, hit %d, type %d", *ci, RD[*ci/candidates], ageCounter[*ci], hitReg[*ci], typeReg[*ci]);
                if (Pline == minP){
                     if(ageCounter[*ci] < minAge){
                         bestCand = (*ci);
                        minAge = ageCounter[*ci];
                     }
                }
                else if(Pline < minP) {
                    minP = Pline;
                    bestCand = (*ci);
                    minAge = ageCounter[*ci];
                }
            }
            //info("Cand %d with priority %d are evicted.", bestCand, minP);
            return bestCand;
        }
        
        DECL_RANK_BINDINGS;
};
#endif // RLR_REPL_H_
