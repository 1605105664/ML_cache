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
		//per-cache-line variables
        uint32_t* ageCounter;
        bool* hitReg;
        bool* typeReg;

		//per-set variables
		uint32_t* RD;
		uint32_t* accum;
		uint32_t* missCounter;
		
		//constants
		bool hit;
		uint32_t numLines;
		uint32_t candidates;



    public:
        // add member methods here, refer to repl_policies.h
        explicit RLRReplPolicy(uint32_t _numLines, uint32_t _candidates) : hit(true), numLines(_numLines), candidates(_candidates){
			//info("nlines:%d ncand:%d", numLines, candidates);
            ageCounter = gm_calloc<uint32_t>(numLines);
            hitReg = gm_calloc<bool>(numLines);
            typeReg = gm_calloc<bool>(numLines);
			
			RD = gm_calloc<uint32_t>(numLines/candidates);
			accum = gm_calloc<uint32_t>(numLines/candidates);
			missCounter = gm_calloc<uint32_t>(numLines/candidates);
        }
		
        ~RLRReplPolicy() {
            gm_free(ageCounter);
            gm_free(hitReg);
            gm_free(typeReg);
        }
		
		void update(uint32_t id, const MemReq* req) {
			bool isPrefetch = req->flags & MemReq::PREFETCH;
			//info("isPrefetch %d", isPrefetch);
			
			if (isPrefetch) typeReg[id]=0;
			else typeReg[id]=1;
			
			hitReg[id] = hit;
			hit = true;
		}
		
		void replaced(uint32_t id) {
			hit = false;
			accum[id/candidates] += ageCounter[id];
			ageCounter[id] = 0;
		}
		
		template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
			assert(*cands.end()-*cands.begin()==candidates);
			uint32_t setIdx = *cands.begin()/candidates;
			//info("Handling cache miss at set %d", setIdx);
			//info("r:%d a:%d m:%d", RD[setIdx], accum[setIdx], missCounter[setIdx]);
			
			missCounter[setIdx]++;
			if(missCounter[setIdx]==8) {
				//info("r:%d a:%d m:%d", RD[setIdx], accum[setIdx], missCounter[setIdx]);
				for(auto ci = cands.begin(); ci != cands.end(); ci.inc())
					if(ageCounter[*ci]<4)
						ageCounter[*ci]++;
				RD[setIdx] = accum[setIdx] >> 1;
				accum[setIdx] = 0;
				missCounter[setIdx] = 0;
			}
			
			
			uint32_t bestCand = -1;
			uint32_t Pa,Ph,Pt = 0;
			uint32_t Pline;
			uint32_t minP = (1>>31)-1;
			
			for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
				if(ageCounter[*ci] > RD[setIdx])Pa = 0;
				else Pa = 8;

				if(hitReg[*ci]) Ph = 1;
				else Ph = 0;
				
				if(typeReg[*ci]) Pt = 1;
				else Pt = 0;
				
				Pline = Pa+Pt+Ph;
				//info("Cand %d have priority of %d", *ci, Pline);
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
