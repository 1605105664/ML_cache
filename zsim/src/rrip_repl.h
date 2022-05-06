#ifndef RRIP_REPL_H_
#define RRIP_REPL_H_

#include "repl_policies.h"

// Static RRIP

class SRRIPReplPolicy : public ReplPolicy {
    protected:
        uint32_t* array;
        uint32_t numLines;
		uint32_t repVal;

    public:
        // add member methods here, refer to repl_policies.h
		explicit SRRIPReplPolicy(uint32_t _numLines) : numLines(_numLines), repVal(0) {
            array = gm_calloc<uint32_t>(numLines);
			for(uint32_t i=0; i!=numLines; i++)
				array[i] = 3;
        }
		
		~SRRIPReplPolicy() {
            gm_free(array);
        }
		
		void update(uint32_t id, const MemReq* req) {
            array[id] = repVal;
			repVal = 0;
        }
		
		void replaced(uint32_t id) {
            repVal = 2;
        }
		
		template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
			while(true){
				for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
					if (array[*ci] == 3)
						return *ci;
				}
				for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
					array[*ci] += 1;
				}
			}
            return -1;
        }
        
		DECL_RANK_BINDINGS;
};


//Place Holder
/*
template <bool sharersAware>
class SRRIPReplPolicy : public ReplPolicy {
    protected:
        uint64_t timestamp; // incremented on each access
        uint64_t* array;
        uint32_t numLines;

    public:
        explicit SRRIPReplPolicy(uint32_t _numLines) : timestamp(1), numLines(_numLines) {
            array = gm_calloc<uint64_t>(numLines);
        }

        ~SRRIPReplPolicy() {
            gm_free(array);
        }

        void update(uint32_t id, const MemReq* req) {
            array[id] = timestamp++;
        }

        void replaced(uint32_t id) {
            array[id] = 0;
        }

        template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
            uint32_t bestCand = -1;
            uint64_t bestScore = (uint64_t)-1L;
            for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
                uint32_t s = score(*ci);
                bestCand = (s < bestScore)? *ci : bestCand;
                bestScore = MIN(s, bestScore);
            }
            return bestCand;
        }

        DECL_RANK_BINDINGS;

    private:
        inline uint64_t score(uint32_t id) { //higher is least evictable
            //array[id] < timestamp always, so this prioritizes by:
            // (1) valid (if not valid, it's 0)
            // (2) sharers, and
            // (3) timestamp
            return (sharersAware? cc->numSharers(id) : 0)*timestamp + array[id]*cc->isValid(id);
        }
};
*/


#endif // RRIP_REPL_H_
