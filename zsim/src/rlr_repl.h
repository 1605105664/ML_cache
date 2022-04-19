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
		uint32_t accum;
		uint32_t missCounter;
		
		uint32_t ways;
		uint32_t numLines;



    public:
        // add member methods here, refer to repl_policies.h
        explicit RLRReplPolicy(uint32_t _numLines) : hit(true), RD(0), accum(0), missCounter(0), numLines(_numLines){
			//info("nlines %d", numLines);
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
			bool isPrefetch = req->flags & MemReq::PREFETCH;
			//info("isPrefetch %d", isPrefetch);
			
			if (isPrefetch) typeReg[id]=0;
			else typeReg[id]=1;
			
			hitReg[id] = hit;
			hit = true;
		}
		
		void replaced(uint32_t id) {
			hit = false;
			accum += ageCounter[id];
			//ageCounter[id] = 0;
		}
		
		template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
			missCounter++;
			if(missCounter==8) {
				for(auto ci = cands.begin(); ci != cands.end(); ci.inc())
					if(ageCounter[*ci]<4)
						ageCounter[*ci]++;
			
				RD = accum >> 1;
				accum = 0;
				missCounter = 0;
			}
			
			
			uint32_t bestCand = -1;
			uint32_t Pa,Ph,Pt = 0;
			uint32_t Pline;
			uint32_t minP = (1>>31)-1;
			
			for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
				if(ageCounter[*ci] > RD)Pa = 0;
				else Pa = 8;

				if(hitReg[*ci]>0) Ph = 1;
				else Ph = 0;
				
				if(typeReg[*ci]>0) Pt = 1;
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
