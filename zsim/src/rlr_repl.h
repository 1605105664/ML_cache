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
        uint64_t* ageCounter;
        uint64_t* hitReg;
        uint64_t* typeReg;

        bool hit;
        uint32_t RD;
        uint32_t numLines;
		uint32_t accum;
		uint32_t hitCounter;



    public:
        // add member methods here, refer to repl_policies.h
        explicit RLRReplPolicy(uint32_t _numLines) : hit(true), RD(0), numLines(_numLines), accum(0), hitCounter(0){
			//info("nlines %d",numLines);
            ageCounter = gm_calloc<uint64_t>(numLines);
            hitReg = gm_calloc<uint64_t>(numLines);
            typeReg = gm_calloc<uint64_t>(numLines);
        }
		
        ~RLRReplPolicy() {
            gm_free(ageCounter);
            gm_free(hitReg);
            gm_free(typeReg);
        }
		
		void update(uint32_t id, const MemReq* req) {
			hitReg[id] = hit;
			if(hit) {
				accum += ageCounter[id];
				for(uint32_t i =0; i < numLines; i++)
					if(ageCounter[i]<31) ageCounter[i]++;
				ageCounter[id]=0;
				hitCounter++;
			}
			if(hitCounter==32) {
				RD = accum >> 4;
				accum = 0;
				hitCounter = 0;
			}
			hit = true;
		}
		
		void replaced(uint32_t id) {
			hit = false;
			ageCounter[id] = 0;
			typeReg[id] = 0;
		}
		
		template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
			uint32_t bestCand = -1;
			uint32_t Pa,Ph,Pt = 0;
			uint64_t Pline;
			uint32_t minP = (1>>31)-1;
			uint32_t i =0;
			
			for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
				if(ageCounter[i] > RD)Pa = 0;
				else Pa = 8;

				if(hitReg[i]>0)Ph = 1;
				else Ph = 0;
				Pline = Pa+Pt+Ph;
				if(Pline < minP) {
					minP = Pline;
					bestCand = (*ci);
				}
				i++;
			}
			//info("Cand %d", bestCand);
			return bestCand;
		}
		
		DECL_RANK_BINDINGS;
};
#endif // RLR_REPL_H_
