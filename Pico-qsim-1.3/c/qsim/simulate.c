#include "simulate.h"

// for the RNG
// From https://forums.raspberrypi.com/viewtopic.php?t=302960
#include "pico/stdlib.h"
#include "hardware/regs/rosc.h"
#include "hardware/regs/addressmap.h"

uint32_t rnd(void){
    int k, random=0;
    volatile uint32_t *rnd_reg=(uint32_t *)(ROSC_BASE + ROSC_RANDOMBIT_OFFSET);
    
    for(k=0;k<32;k++){
        random = random << 1;
        random=random + (0x00000001 & (*rnd_reg));
    }
    return random;
}

uint32_t rnd_percent(void){
    // gen a 32-bit rand, then return mod 100
    uint32_t r = rnd();
    uint8_t p = r % 100;
    return p;
}

uint8_t biased_bit(uint32_t val){
    if (val > 99 || val == 0){
        // wrong...
        return 0;
    }
    int dist[100] = {0};
    // add val-many 1's... 
    for(int i = 0; i < val; i++){
        dist[i] = 1;
    }
    // then pick one at random...
    return dist[rnd_percent()];
}


int simulate(double complex stateVec[16], uint8_t results[8]){
    // returns four sample results from a given stateVec
    // First we check if anything is '1' and just return that:
    for(int i = 0; i<16; i++){
        if (stateVec[i] == 1){
            // set the results to just that value
            for(int j = 0; j < 8; j++){
                results[j] = i;
            }
            return 0;
        }
    }
    double nmsq_sv[16];
    int norms_pc[16];
    for (int i = 0; i < 16; i++){
        // create measurement norm-sq vector
        nmsq_sv[i] = pow(cabs(stateVec[i]),2);
    }
    for (int i = 0; i<16; i++){
        norms_pc[i] = floor(nmsq_sv[i] * 100); // take the floor of val times 100 to get percentage
    }
    /*
    // NB - the probability that, say, the second qubit is '1' is the same as the
    // probability that the system finds itself in a state with a '1' in the second qubit...
    // as such, we just add the abs^2 values for the various states together to get the probability
    // that a given qubit is a 1, and calculate that with a random distribution array.
    //
    // So given that... here's what we do if there isn't a 1:
    // 1. create a vector of the measurement outcomes (norm-squared values)
    // 2. for each state..., add up the values that the given qubit is a 1, as integer percentage, without mantissa
    // 3. for each qubit, create an array of 100 0's, and replace X many with 1 
    // (this is to simulate the bias that qubit has fro outcome '1')
    // 4. generate a random number 0-99, and add 'that bit' to the outcome.
    // values for qubits
    uint32_t q0,q1,q2,q3;
    // start from i=1 as 0 is always '0000' outcome
    // so just add any value where the given qubit could be '1'
    for (int i = 1; i<16; i++){
        q3 += ((i & 0x1) == 1) ? norms_pc[i] : 0;
        q2 += ((i & 0x2) >> 1 == 1) ? norms_pc[i] : 0;
        q1 += ((i & 0x4) >> 2 == 1) ? norms_pc[i] : 0;
        q0 += ((i & 0x8) >> 3 == 1) ? norms_pc[i] : 0;
    }
    */
    // generate a distribution from the percent vector:
    int dist[100] = {0};
    int p = 0;
    for(int i = 0; i< 16; i++){
        int v = norms_pc[i];
        for(int j = 0; j < v; j++){
            // put v-many i's into dist
            dist[p] = i;
            // track p for dist
            p++;
        }
    }
    for(int i = 0; i < 8; i++){
        // pick a random state from the distribution
        results[i] = dist[rnd_percent()];
        //results[i] = (biased_bit(q3) + biased_bit(q2) << 1 + biased_bit(q1) << 2 + biased_bit(q0) << 3);
    }
    return 0;
}