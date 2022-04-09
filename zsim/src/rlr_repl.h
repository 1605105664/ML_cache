#ifndef RLR_REPL_H_
#define RLR_REPL_H_

#include "repl_policies.h"

// Static RLR
class RLRReplPolicy : public ReplPolicy {
    protected:
        // add class member variables here
        uint64_t* ageCounter;
        uint64_t* hitRegisters;
        uint64_t* typeRegister;

        uint64_t RD;
        uint32_t numLines;
        uint32_t hitVal;



    public:
        // add member methods here, refer to repl_policies.h
        explicit RLRReplPolicy(uint32_t _numLines) : RD(0), numLines(_numLines),hitVal(0) {
            ageCounter = gm_calloc<uint64_t>(numLines);
            hitRegisters = gm_calloc<uint64_t>(numLines);
            typeRegister = gm_calloc<uint64_t>(numLines);
        }
        ~RLRReplPolicy() {
            gm_free(ageCounter);
            gm_free(hitRegisters);
            gm_free(typeRegister);
        }
    void update(uint32_t id, const MemReq* req) {
        hitRegisters[id] = hitVal;
        ageCounter[id]++;
        hitVal = 1;
    }
    void replaced(uint32_t id) {
            hitVal = 0;
            ageCounter[id] = 0;
            typeRegister[id] = 0;
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

                if(hitRegisters[i]>0)Ph = 1;
                else Ph = 0;
                Pline = Pa+Pt+Ph;
                if(Pline < minP) {
                    minP = Pline;
                    bestCand = (*ci);
                }
                i++;
            }
            
            return bestCand;
        }
        DECL_RANK_BINDINGS;
};
#endif // RLR_REPL_H_
