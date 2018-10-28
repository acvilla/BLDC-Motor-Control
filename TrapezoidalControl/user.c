/*
 * user.c
 *
 *  Created on: Oct 23, 2018
 *      Author: Alex
 */

#include "user.h"

void initControl (CONTROL_Obj *ControlPtr) {
    ControlPtr->piController.k_i = K_I;
    ControlPtr->piController.k_p = K_P;
    ControlPtr->dutyCycle = DUTY_CYCLE_DEFAULT;
    ControlPtr->motor.npp = NUM_POLE_PAIRS;
    ControlPtr->motor.stallCurrent_A = STALL_CURRENT_A;
    ControlPtr->speedCalc.rpmRef = RPM_REF_DEFAULT;
    ControlPtr->speedCalc.speedUpdateReady = FALSE;
    ControlPtr->speedCalc.timerPeriod = TIMER_PERIOD;
    ControlPtr->speedCalc.rpmMax = RPM_MAX;
    ControlPtr->currentLegA_A = 0;
    ControlPtr->currentLegB_A = 0;
    ControlPtr->hall_states = C;
}

void updateHall_A(int val, CONTROL_Obj *ControlPtr){
    if (val){
        ControlPtr->hall_states |= (HALL_A_MASK); // Hall A is high
    } else {
        ControlPtr->hall_states &= ~(HALL_A_MASK); // Hall A is low
    }
}

void updateHall_B(int val, CONTROL_Obj *ControlPtr){
    if (val){
        ControlPtr->hall_states |= (HALL_B_MASK);
    } else {
        ControlPtr->hall_states &= ~(HALL_B_MASK);
    }
}

void updateHall_C(int val, CONTROL_Obj *ControlPtr){
    if (val){
        ControlPtr->hall_states |= (HALL_C_MASK);
    } else {
        ControlPtr->hall_states &= ~(HALL_C_MASK);
    }
}



