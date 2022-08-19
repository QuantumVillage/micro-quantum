#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <string.h>

#include "measure.h"
#include "gates.h"

// STATIC GATES 

gate X = {
    {0,1},
    {1,0}
};

// h is 1/sqrt(2)
const double h = 0.70710678118;
gate H = {
    {h, h},
    {h, -h}
};

gate Rx = {
    {1, 0},
    {0, 1}
}; // set to Id, gets reset by Rx() later

gate Id = { //named so it doesn't conflict with the I in complex.h
    {1, 0},
    {0, 1}
};

gate OP0 = {
    {1, 0},
    {0, 0}
};

gate OP1 = {
    {0, 0},
    {0, 1}
};

gate V = {
    {0.5 + 0.5 * I, 0.5 - 0.5 * I},
    {0.5 - 0.5 * I, 0.5 + 0.5 * I}
};

// local vars

// setup gate pointers for each step
gate* step[4] = {0};
// initialize the StateMatrix
double complex SM[16][16];
// initialize a blank stateVec - if something fails, this will give no measurements
double complex stateVec[16] = {0};

// maths and utility functions
void set_Rx(double complex rot)
{
    Rx[0][0] = ccos(rot); 
    Rx[0][1] = -I * csin(rot);
    Rx[1][0] = -I * csin(rot);
    Rx[1][1] = ccos(rot);
}

gate *char2gate(char c)
{
    switch(c)
    {
        case 'I':
            return (gate *)Id;
        case '-':
            return (gate *)Id;
        case 'X':
            return (gate *)X;
        case 'H':
            return (gate *)H;
        case 'R':
            //set_Rx((-3*M_PI)/16);
            set_Rx(-0.125*M_PI);
            return (gate *)Rx;
        case 'V':
            return (gate *)V;
    }
    return (gate *)Id;
}

int tensor_prod(int a, int b, int dimP, double complex A[a][a], double complex B[b][b], double complex P[dimP][dimP])
{
    for(int a_x = 0; a_x < a; a_x++)
    {
        for(int b_x = 0; b_x < b; b_x++)
        {
            for(int a_y = 0; a_y < a; a_y++)
            {
                for(int b_y = 0; b_y < b; b_y++)
                {
                    double complex t = A[a_x][a_y] * B[b_x][b_y];
                    P[(a_x * b) + b_x][(a_y * b) + b_y] = t;
                }
            }
        }
    }
    return 0;
}

void updateSV()
{
    double complex new_SV[16] = {0};
    for (int i = 0; i < 16; i++)
    {
        double complex res = 0;
        for (int j = 0; j < 16; j++)
        {
            res += (stateVec[j] * SM[i][j]);
        }
        new_SV[i] = res;
    }
    for (int k = 0; k < 16; k++)
    {
        stateVec[k] = new_SV[k];
    }
}

void add_ctrlSM(double complex c_SM0[16][16], double complex c_SM1[16][16], double complex out_SM[16][16])
{
    for (int i = 0; i <16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            out_SM[i][j] = c_SM0[i][j] + c_SM1[i][j];
        }
    }
}

void evalCtrlStep(gate *c_step[4], double complex c_SM[16][16])
{
    double complex aa[4][4];
    double complex ab[8][8];
    tensor_prod(2, 2, 4, *c_step[1], *c_step[0], aa);
    tensor_prod(2, 4, 8, *c_step[2], aa, ab);
    tensor_prod(2, 8, 16, *c_step[3], ab, c_SM);
}

void evaluate_ctrl(char slice[4][8], int index)
{
    // assume single control for now... -MC
    gate *step_0[4];
    gate *step_1[4];
    double complex ctrl_SM0[16][16] = {0}; // for when ctrl is |0>
    double complex ctrl_SM1[16][16] = {0}; // for when ctrl is |1>
    for (int i = 0; i < 4; i++) // create two control states (add together later)
    {
        if (i == index)
        {
            step_0[i] = (gate *)OP0;
            step_1[i] = (gate *)OP1;
            continue;
        }
        step_0[i] = (gate *)Id;
        step_1[i] = char2gate(slice[i][0]);
    }
    evalCtrlStep(step_0, ctrl_SM0);
    evalCtrlStep(step_1, ctrl_SM1);
    add_ctrlSM(ctrl_SM0, ctrl_SM1, SM);
    updateSV();
}

void evalStep()
{
    double complex aa[4][4];
    double complex ab[8][8];
    double complex ac[16][16];
    tensor_prod(2, 2, 4, *step[1], *step[0], aa);
    tensor_prod(2, 4, 8, *step[2], aa, ab);
    tensor_prod(2, 8, 16, *step[3], ab, SM);
    updateSV(); // update the state vector using the step's matrix in SM
}

void evaluate_swap(char slice[4][8], int i1)
{
    // Work out where the 'x' chars are
    int i2 = 0;
    for (int i = i1; i < 4; i++)
    {
        if (slice[i][0] == 'x')
        {
            i2 = i;
        }
    }
    
    // now gosub for three ctrl slices that
    // are equivalent to SWAP

    // first, make the slices
    char s1[4][8];
    char s2[4][8];
    for (int j = 0; j < 4; j++)
    {
        strcpy(s1[j], "-");
        strcpy(s2[j], "-");
    }

    strcpy(s1[i1], "c");
    strcpy(s1[i2], "X");
    strcpy(s2[i1], "X");
    strcpy(s2[i2], "c");

    //then process them
    evaluate_ctrl(s1, i1);
    evaluate_ctrl(s2, i2);
    evaluate_ctrl(s1, i1); // repeat 
    return;
}

void processSlice(char slice[4][8])
{
    for (int i = 0; i < 4; i++)
    {
        if (slice[i][0] == 'c') // process a ctrl gate
        {
            evaluate_ctrl(slice, i);
            return;
        }
        if (slice[i][0] == 'x') // process a SWAP gate
        {
            evaluate_swap(slice, i);
            return;
        }
    }
    for (int i = 0; i < 4; i++){
        char digit = slice[i][0];
        step[i] = char2gate(digit);
    }
    evalStep(); // update the SV
    return;
}

void evalCircuit(char circuit[14][4][8])
{
    for (int i = 0; i < 14; i++)
    {
        processSlice(circuit[i]); //sets 'step' to be the gates for a slice of the quantum tape
        // then evaluates and updates the stateVec
    }
}


int measure(char circuit[14][4][8], double complex stateVecIn[16])
{
    // reset the stateVec 
    for (int i = 0; i < 16; i++)
    {
        stateVec[i] = 0;
    }
    stateVec[0] = 1;

    for (int i = 0; i <16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            SM[i][j] = 0;
        }
    }

   // take the circuit as input
   // update the local stateVec 
   evalCircuit(circuit);

   // pass the output to the regular stateVec...
   // this isn't necessary... I don't need to do this...
   // but this is a livestream and I'm feeling so tired and lazy
   for (int j = 0; j < 16; j++)
   {
       stateVecIn[j] = stateVec[j];
   }
    return 0;
}
