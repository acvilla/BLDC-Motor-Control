/*
 * user.c
 *
 *  Created on: Oct 23, 2018
 *      Author: Alex
 */

#include "user.h"


void initControl (CONTROL_Obj *ControlPtr) {
    ControlPtr->state = RUN;
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
    ControlPtr->currentPhA_A = 0;
    ControlPtr->currentPhB_A = 0;
    ControlPtr->battery.capacity_mAH = BAT_CAPACITY_MAH;
    ControlPtr->battery.percBat = DEFAULT_BAT_LIFE;
    ControlPtr->hallErr = FALSE;
    ControlPtr->hallErrCnt = 0;
    ControlPtr->expectedHallStates = NULL;
    hall_state hallStateTable[6] = {C,AC,A,AB,B,BC};
    memcpy(ControlPtr->motor.hallStateTable, hallStateTable, sizeof(ControlPtr->motor.hallStateTable));
    ring_buffer_init(&(ControlPtr->ringBuf));
}

void updateHall_A(int val, CONTROL_Obj *ControlPtr){
    if (val){
        ControlPtr->currentHallStates |= (HALL_A_MASK); // Hall A is high
    } else {
        ControlPtr->currentHallStates &= ~(HALL_A_MASK); // Hall A is low
    }
    ControlPtr->currentHallStates &= 0b111;

}

void updateHall_B(int val, CONTROL_Obj *ControlPtr){
    if (val){
        ControlPtr->currentHallStates |= (HALL_B_MASK);
    } else {
        ControlPtr->currentHallStates &= ~(HALL_B_MASK);
    }
    ControlPtr->currentHallStates &= 0b111;

}

void updateHall_C(int val, CONTROL_Obj *ControlPtr){
    if (val){
        ControlPtr->currentHallStates |= (HALL_C_MASK);
    } else {
        ControlPtr->currentHallStates &= ~(HALL_C_MASK);
    }
    ControlPtr->currentHallStates &= 0b111;

}

/*
 * Updates the expectedHallStates based on the currentHallStates.
 * If the Hall states are either all High or all low, then the state will not be found in the table
 * and expectedHallStates will arbitrarily be set to hallStateTable[0] and an error will be asserted.
 */
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
    ControlPtr->expectedHallStates = (ControlPtr->motor.hallStateTable + (hall_state)(j+1)%6); // points to expected place in commutation table
    return;
}

void checkHallErr(CONTROL_Obj *ControlPtr){
    if(ControlPtr->currentHallStates != *(ControlPtr->expectedHallStates)){
        ControlPtr->hallErr = TRUE;
        ControlPtr->hallErrCnt = ControlPtr->hallErrCnt + 1;
    } else {
        ControlPtr->hallErr = FALSE;
    }
    updateExpectedHallStates(ControlPtr); // Update expected hall state whether or not there's an error
}

/*
 * HOW TO USE HALL ERROR FUNC'S:
 * -----------------------------
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
    ControlPtr->currentHallStates &= 0b111;
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

void commutateMotor(CONTROL_Obj *ControlPtr){
    switch (ControlPtr->currentHallStates){
                   case C:
                       // Phases: Aoff, B-, C+
                       updatePWMState(&EPwm1Regs, LOW, LOW); // Phase A
                       updatePWMState(&EPwm2Regs, LOW, HIGH); // Phase B
                       updatePWMState(&EPwm3Regs, PWM, PWM); // Phase C
                       break;
                   case AC:
                       // Phases: A+, B-, Coff
                       updatePWMState(&EPwm1Regs, PWM, PWM); // Phase A
                       updatePWMState(&EPwm2Regs, LOW, HIGH); // Phase B
                       updatePWMState(&EPwm3Regs, LOW, LOW); // Phase C
                       break;
                   case A:
                       // Phases: A+, Boff, C-
                       updatePWMState(&EPwm1Regs, PWM, PWM); // Phase A
                       updatePWMState(&EPwm2Regs, LOW, LOW); // Phase B
                       updatePWMState(&EPwm3Regs, LOW, HIGH); // Phase C
                       break;
                   case AB:
                       // Phases: Aoff, B+, C-
                       updatePWMState(&EPwm1Regs, LOW, LOW); // Phase A
                       updatePWMState(&EPwm2Regs, PWM, PWM); // Phase B
                       updatePWMState(&EPwm3Regs, LOW, HIGH); // Phase C
                       break;
                   case B:
                       // Phases: A-, B+, Coff
                       updatePWMState(&EPwm1Regs, LOW, HIGH); // Phase A
                       updatePWMState(&EPwm2Regs, PWM, PWM); // Phase B
                       updatePWMState(&EPwm3Regs, LOW, LOW); // Phase C
                       break;
                   case BC:
                       // Phases: A-, Boff, C+
                       updatePWMState(&EPwm1Regs, LOW, HIGH); // Phase A
                       updatePWMState(&EPwm2Regs, LOW, LOW); // Phase B
                       updatePWMState(&EPwm3Regs, PWM, PWM); // Phase C
                       break;
                   default:
                       break;
               }
}

void setDutyCycle(uint8_t dutyCycle){
    double duty = (double) dutyCycle/255;
    uint16_t CMP = duty * TBPRD_VALUE;
    CMP_GLOBAL = CMP;
    uint16_t CMPB = CMP + 20;
    EPwm1Regs.CMPA.half.CMPA = CMP; // adjust duty for output EPWM1A
    EPwm1Regs.CMPB = CMPB; // adjust duty for output EPWM3B

    EPwm2Regs.CMPA.half.CMPA = CMP; // adjust duty for output EPWM2A
    EPwm2Regs.CMPB = CMPB; // adjust duty for output EPWM3B

    EPwm3Regs.CMPA.half.CMPA = CMP; // adjust duty for output EPWM3A
    EPwm3Regs.CMPB = CMPB; // adjust duty for output EPWM3B
}
