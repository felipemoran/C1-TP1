#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include "Dask.h"

// ======================================
// DEFINES
// ======================================

#define CARD_NUMBER 0


// ======================================
// LOCAL VARIABLES
// ======================================


// ======================================
// QUESTION F
// --------------------------------------
// This function sets up the card and
// then copies the input signal to the
// output as fast as possible, one sample
// at a time
// ======================================

void question_F() {
    U16 id; // card id
    U16 value; // read/written value

    // register card and get it's id
    id = Register_Card(PCI_9112, CARD_NUMBER);

    // set input and output port configs
    AI_9112_Config(id, TRIG_INT_PACER);
    AO_9112_Config(id, 0, AD_B_5_V);

    // loop forever
    while(1) {
        // first read the input channel one single time
        AI_ReadChannel(id, 0, AD_B_5_V, &value);
        // then write the written value to the output with the propper offset
        AO_WriteChannel(id, 0, value>>4);
    }

    // gracefully exit (but this code should never execute because of the infinite loop before)
    Release_Card(id);
}


// ======================================
// QUESTION G
// --------------------------------------
// This function generates a triangular
// wave at the output channel. Speed can
// be modified by changing the step value
// inside each for loop or by adding
// delays before/after outputing a value
// ======================================

void question_G() {
    U16 id;
    U16 value = 0;
    I16 increment = 1;

    // register card and get it's id
    id = Register_Card(PCI_9112, CARD_NUMBER);

    // set input and output port configs
    AI_9112_Config(id, TRIG_INT_PACER);
    AO_9112_Config(id, 0, AD_B_5_V);

    // loop forever
    while(1) {
        // output ascending triangular wave ramp
        for(value=0; value<=4000; value+=100) {
            AO_WriteChannel(id, 0, value);
        }

        // output descending triangular wave ramp
        for(value=4000; value>0; value-=100) {
            AO_WriteChannel(id, 0, value);
        }
    }

    // gracefully exit (but this code should never execute because of the infinite loop before)
    Release_Card(id);
}

// ======================================
// QUESTION H
// --------------------------------------
// These functions are to be used for 
// measuring caracteristics of the input
// triangular wave such as min/max values
// , amplitude, mean value, frequency and
// periode in number of samples
// ======================================
F64 sample_rate = 10000;
U32 read_counts = 10000;


// --------------------------------------
// function: calculate_params
//
// brief: calculates wave characteristics from array of sample
// 
// params:
//      buffer: input array of samples
//      frequency: pointer to output frequency
//      min: pointer to output min of samples
//      max: pointer to output max of samples
//      average: pointer to output average
//      period: pointer to output periode
// --------------------------------------
void calculate_params(U16* buffer, F64* frequency, U16* min, U16* max, F64* average, U16* period) {
    I16 i;
    U16 zero_crossings = 0;
    F64 point_1, point_2;

    *average = 0;
    *min=buffer[0];
    *max=buffer[0];

    for (i=0; i<read_counts; i++) {
        *average+=buffer[i];

        if(buffer[i] > max) {
            *max = buffer[i];
        }
        if(buffer[i] < min) {
            *min = buffer[i];
        }
    }
    *average = *average/read_counts;

    for (i=1; i<read_counts; i++) {
        point_1 = buffer[i-1] - *average;
        point_2 = buffer[i] - *average;

        if(point_1*point_2 < 0) {
            zero_crossings++;
        }
    }

    *frequency = zero_crossings/2/sample_rate*read_counts;
    *period = sample_rate/(*frequency);
}

// --------------------------------------
// function: question_H_a
//
// brief: function that sets up the card, measures 
// read_counts samples at rate sample_rate samples 
// per second and outputs it's characteristics.
// Samples are acquired with a blocking function
// --------------------------------------
void question_H_a() {
    I16 id; // card id
    U16 buffer[read_counts]; // buffer containing all samples
    F64 frequency, average;
    U16 min, max, period;

    // register card and get it's id
    id = Register_Card(PCI_9112, CARD_NUMBER);

    if(id<0) {
        // invalid id
        return;
    }

    // set input and output port configs
    AI_9112_Config(id, TRIG_INT_PACER);
    AO_9112_Config(id, 0, AD_B_5_V);

    // loop forever
    while(1) {
        // read all samples at one, much faster than one by one. Block until complete
        AI_ContReadChannel(id, 0, AD_B_5_V, buffer, read_counts, sample_rate, SYNCH_OP);
        // Calculate all wave parameters
        calculate_params(buffer, &frequency, &min, &max, &average, &period);

        // print data after each acquisition
        printf("\n - Acquisition synchrone\n");
        printf("freq: %f\n", frequency);
        printf("sample_rate: %f\n", sample_rate);
        printf("read_counts: %d\n", (U16)read_counts);
        printf("min: %d\n", min);
        printf("max: %d\n", max);
        printf("average: %f\n", average);
        printf("period: %d\n", period);

    }
}

// --------------------------------------
// function: question_H_b
//
// brief: same as function question_H_a but with non blocking 
// function used for data acquisition
// --------------------------------------
void question_H_b() {
    I16 id, i;
    U16 buffer[read_counts]; // buffer containing all samples
    F64 frequency, average;
    U16 AccessCnt;
    U16 min, max, period;
    BOOLEAN sampling_done = TRUE;
    time_t last_print, now;

    // register card and get it's id
    id = Register_Card(PCI_9112, CARD_NUMBER);

    if(id<0) {
        // invalid id, stop everything
        return;
    }

    // set input and output port configs
    AI_9112_Config(id, TRIG_INT_PACER);
    AO_9112_Config(id, 0, AD_B_5_V);

    // set double buffer mode off 
    AI_AsyncDblBufferMode(id, FALSE);

    // loop forever
    while(1) {
        // if sampling process is done, start a new one
        if (sampling_done) {
            // mark sampling as ongoing
            sampling_done = FALSE;
            // trigger non blocking sampling
            AI_ContReadChannel(id, 0, AD_B_5_V, buffer, read_counts, sample_rate, ASYNCH_OP);
        }

        // check if sampling is done
        AI_AsyncCheck(id, &sampling_done, &AccessCnt);
        // get current timestamp (seconds)
        time(&now);

        // if sampling is done and last print was 1 second ago, print again
        if(now > last_print && sampling_done) {
            last_print = now;
            // stop sampling routine
            AI_AsyncClear(id, &AccessCnt);

            // calculate all parameters
            calculate_params(buffer, &frequency, &min, &max, &average, &period);

            // and print'em
            printf("\n - Acquisition asynchrone\n");
            printf("freq: %f\n", frequency);
            printf("sample_rate: %f\n", sample_rate);
            printf("read_counts: %d\n", (U16)read_counts);
            printf("min: %d\n", min);
            printf("max: %d\n", max);
            printf("average: %f\n", average);
            printf("period: %d\n", period);
            printf("AccessCnt: %d\n", AccessCnt);
            printf("Time: %d\n", now);
        }

    }
}


// ======================================
// MAIN
// ======================================

// simply calls the corresponding question function
int main()
{
    question_H_a();

    return 0;
}
