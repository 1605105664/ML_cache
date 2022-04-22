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
		uint32_t* RD;
		uint32_t* accum;
		uint32_t* hitCounter;
		
		//cache variables
		bool hit;
		uint32_t numLines;
		uint32_t candidates;



    public:
        // add member methods here, refer to repl_policies.h
        explicit RLRReplPolicy(uint32_t _numLines, uint32_t _candidates) : hit(true), numLines(_numLines), candidates(_candidates){
            ageCounter = gm_calloc<uint32_t>(numLines);
            hitReg = gm_calloc<bool>(numLines);
            typeReg = gm_calloc<bool>(numLines);
			
			RD = gm_calloc<uint32_t>(numLines/candidates);
			accum = gm_calloc<uint32_t>(numLines/candidates);
			hitCounter = gm_calloc<uint32_t>(numLines/candidates);
        }
		
        ~RLRReplPolicy() {
            gm_free(ageCounter);
            gm_free(hitReg);
            gm_free(typeReg);
			gm_free(RD);
			gm_free(accum);
			gm_free(hitCounter);
        }
		
		void update(uint32_t id, const MemReq* req) {
			//type register
			bool isPrefetch = req->flags & MemReq::PREFETCH;
			if (isPrefetch) typeReg[id]=0;
			else typeReg[id]=1;
			
			//hit register
			hitReg[id] = hit;
			
			//info("update: id %d",id);
			if(hit){
				//age counter
				uint32_t setIdx=id/candidates;
				uint32_t begin = setIdx*candidates;
				accum[setIdx] += ageCounter[id];
				for(uint32_t i=begin;i<begin+candidates;i++)
					if(ageCounter[i]<32)
						ageCounter[i]++;
				ageCounter[id] = 0;
				
				//RD
				hitCounter[setIdx]++;
				if(hitCounter[setIdx]==32){
					RD[setIdx] = accum[setIdx] >> 4;
					accum[setIdx] = 0;
					hitCounter[setIdx] = 0;
				}
			}
			hit = true;
		}
		
		void replaced(uint32_t id) {
			hit = false;
			ageCounter[id] = 0;
		}
		
		template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
			uint32_t bestCand = -1;
			uint32_t Pa,Ph,Pt = 0;
			uint32_t Pline;
			uint32_t minP = UINT32_MAX;
			
			for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
				if(ageCounter[*ci] > RD[*ci/candidates])Pa = 0;
				else Pa = 8;

				if(hitReg[*ci]) Ph = 1;
				else Ph = 0;
				
				if(typeReg[*ci]) Pt = 1;
				else Pt = 0;
				
				Pline = (Pa+Pt+Ph)*2;
				
				//Recency approximation
				if(ageCounter[*ci]==0) Pline--;
				
				//info("Cand %d, rd %d, age %d, hit %d, type %d", *ci, RD[*ci/candidates], ageCounter[*ci], hitReg[*ci], typeReg[*ci]);
				if(Pline < minP) {
					minP = Pline;
					bestCand = (*ci);
				}
			}
			//info("Cand %d with priority %d are evicted.", bestCand, minP);
			return bestCand;
		}
		
		DECL_RANK_BINDINGS;
};
#endif // RLR_REPL_H_
