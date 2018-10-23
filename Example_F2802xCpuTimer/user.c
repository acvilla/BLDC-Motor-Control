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
    ControlPtr->dutyCycleMin = DUTY_CYCLE_MIN;
    ControlPtr->dutyCycleMax = DUTY_CYCLE_MAX;
    ControlPtr->dutyCycle = DUTY_CYCLE_DEFAULT;
    ControlPtr->motor.npp = NUM_POLE_PAIRS;
    ControlPtr->motor.stallCurrent_A = STALL_CURRENT_A;
    ControlPtr->speedCalc.rpmRef = RPM_REF_DEFAULT;
    ControlPtr->speedCalc.speedUpdateReady = FALSE;
    ControlPtr->speedCalc.timerPeriod = TIMER_PERIOD;
    ControlPtr->speedCalc.rpmMax = RPM_MAX;
    ControlPtr->currentLegA_A = 0;
    ControlPtr->currentLegB_A = 0;
}
