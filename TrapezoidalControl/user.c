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
    ControlPtr->speedCalc.rpm = 0;
    ControlPtr->speedCalc.speedUpdateReady = FALSE;
    ControlPtr->speedCalc.timerPeriod = TIMER_PERIOD;
    ControlPtr->speedCalc.timerVal = 0;
    ControlPtr->speedCalc.rpmMax = RPM_MAX;
    ControlPtr->currentLegA_A = 0;
    ControlPtr->currentLegB_A = 0;
    ControlPtr->hallErr = FALSE;
    ControlPtr->expectedHallStates = NULL;
    hall_state hallStateTable[6] = {C,AC,A,AB,B,BC};
    memcpy(ControlPtr->motor.hallStateTable, hallStateTable, sizeof(ControlPtr->motor.hallStateTable));
}

void updateHall_A(int val, CONTROL_Obj *ControlPtr){
    if (val){
        ControlPtr->currentHallStates |= (HALL_A_MASK); // Hall A is high
    } else {
        ControlPtr->currentHallStates &= ~(HALL_A_MASK); // Hall A is low
    }
}

void updateHall_B(int val, CONTROL_Obj *ControlPtr){
    if (val){
        ControlPtr->currentHallStates |= (HALL_B_MASK);
    } else {
        ControlPtr->currentHallStates &= ~(HALL_B_MASK);
    }
}

void updateHall_C(int val, CONTROL_Obj *ControlPtr){
    if (val){
        ControlPtr->currentHallStates |= (HALL_C_MASK);
    } else {
        ControlPtr->currentHallStates &= ~(HALL_C_MASK);
    }
}

void updateExpectedHallStates(CONTROL_Obj *ControlPtr){
    int i,j = 0;
    for (i = 0; i < 6; i++){
        if(ControlPtr->currentHallStates == ControlPtr->motor.hallStateTable[i]){
            j = i;
        } else if (i >= 6){
            ControlPtr->hallErr = TRUE;
            return;
        }

    }
    ControlPtr->expectedHallStates = ControlPtr->motor.hallStateTable + (j+1)%6; // points to expected place in commutation table
    return;
}

void checkHallErr(CONTROL_Obj *ControlPtr){
    if(ControlPtr->currentHallStates != *(ControlPtr->expectedHallStates)){
        ControlPtr->hallErr = TRUE;
    } else {
        ControlPtr->hallErr = FALSE;
    }
    updateExpectedHallStates(ControlPtr); // Update expected hall state whether or not there's an error
}

/*
 * initHallStates
 * updateExpectedHallStates
 * for(;;){
 *    updateHall_X()
 *    checkHallErr()
 * }
 *
 */


void initHallStates(GPIO_Handle myGpio, CONTROL_Obj *ControlPtr, GPIO_Number_e HallA, GPIO_Number_e HallB, GPIO_Number_e HallC){
    int aVal = GPIO_getData(myGpio, HallA);
    int bVal = GPIO_getData(myGpio, HallB);
    int cVal = GPIO_getData(myGpio, HallC);
    updateHall_A(aVal, ControlPtr);
    updateHall_B(bVal, ControlPtr);
    updateHall_C(cVal, ControlPtr);
    updateExpectedHallStates(ControlPtr); // Update the next hall state that we expect, used to check for hall out of order errors
}

double updatePI(CONTROL_Obj *ControlPtr){
    double cmd = ControlPtr->speedCalc.rpmRef;
    double speed = ControlPtr->speedCalc.rpm;
    double error = cmd - speed;

    double pTerm = ControlPtr->piController.k_p * error;

    static double iState;
    iState += error;
    if(iState > 255){
        iState = 255;
    } else if (iState < -255){
        iState = -255;
    }

    double iTerm = ControlPtr->piController.k_i * iState;

    double pi = pTerm + iTerm;
    if (pi < 0){
         pi = 0;
    } else if(pi > 255){
         pi = 255;
    }

    return pi;

}

void updatePWMState(volatile struct EPWM_REGS *pwmReg, pwm_state CSFA, pwm_state CSFB){
    pwmReg->AQCSFRC.bit.CSFA = CSFA;
    pwmReg->AQCSFRC.bit.CSFB = CSFB;
}

void setDutyCycle(uint8_t dutyCycle){
    double duty = (double) dutyCycle/255;
    uint16_t CMP = duty * TBPRD_VALUE;
    CMP_GLOBAL = CMP;
    EPwm1Regs.CMPA.half.CMPA = CMP; // adjust duty for output EPWM1A
    EPwm1Regs.CMPB = CMP; // adjust duty for output EPWM3B

    EPwm2Regs.CMPA.half.CMPA = CMP; // adjust duty for output EPWM2A
    EPwm2Regs.CMPB = CMP; // adjust duty for output EPWM3B

    EPwm3Regs.CMPA.half.CMPA = CMP; // adjust duty for output EPWM3A
    EPwm3Regs.CMPB = CMP; // adjust duty for output EPWM3B
}
