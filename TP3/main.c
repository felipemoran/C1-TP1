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
// QUESTION Ffrequency
// ======================================

void question_F() {
    U16 id;
    U16 value;

    id = Register_Card(PCI_9112, CARD_NUMBER);

    AI_9112_Config(id, TRIG_INT_PACER);
    AO_9112_Config(id, 0, AD_B_5_V);

    while(1) {
        AI_ReadChannel(id, 0, AD_B_5_V, &value);
        // printf("Value: %d\n", (value>>4));
        AO_WriteChannel(id, 0, value>>4);
    }

    Release_Card(id);
}


// ======================================
// QUESTION G
// ======================================

void question_G() {
    U16 id;
    U16 value = 0;
    I16 increment = 1;

    id = Register_Card(PCI_9112, CARD_NUMBER);

    AI_9112_Config(id, TRIG_INT_PACER);
    AO_9112_Config(id, 0, AD_B_5_V);

    while(1) {
        for(value=0; value<=4000; value+=100) {
            AO_WriteChannel(id, 0, value);
        }

        for(value=4000; value>0; value-=100) {
            AO_WriteChannel(id, 0, value);
        }
    }

    Release_Card(id);
}

// ======================================
// QUESTION H
// ======================================
F64 sample_rate = 10000;
U32 read_counts = 10000;


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

void question_H_a() {
    I16 id;
    U16 value;
    U16 buffer[read_counts];
    F64 frequency, average;
    U16 counter = 0;
    U16 min, max, period;

    id = Register_Card(PCI_9112, CARD_NUMBER);

    if(id<0) {
        // invalid id
        return;
    }

    AI_9112_Config(id, TRIG_INT_PACER);
    AO_9112_Config(id, 0, AD_B_5_V);

    while(1) {
        AI_ContReadChannel(id, 0, AD_B_5_V, buffer, read_counts, sample_rate, SYNCH_OP);
        calculate_params(buffer, &frequency, &min, &max, &average, &period);

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


void question_H_b() {
    I16 id, i;
    U16 value;
    U16 buffer[read_counts];
    F64 frequency, average;
    U16 counter = 0, AccessCnt;
    U16 min, max, period;
    BOOLEAN sampling_done = TRUE;
    time_t last_print, now;

    id = Register_Card(PCI_9112, CARD_NUMBER);

    if(id<0) {
        // invalid id
        return;
    }

    AI_9112_Config(id, TRIG_INT_PACER);
    AO_9112_Config(id, 0, AD_B_5_V);

    AI_AsyncDblBufferMode(id, FALSE);

    while(1) {
//        for (i=0; i<read_counts; i++) {
//            buffer[i] = 0;
//        }

        if (sampling_done) {
            sampling_done = FALSE;
            AI_ContReadChannel(id, 0, AD_B_5_V, buffer, read_counts, sample_rate, ASYNCH_OP);
        }

        AI_AsyncCheck(id, &sampling_done, &AccessCnt);
        time(&now);

        if(now > last_print && sampling_done) {
            last_print = now;
            AI_AsyncClear(id, &AccessCnt);

            calculate_params(buffer, &frequency, &min, &max, &average, &period);

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

int main()
{
    question_H_a();

    return 0;
}
