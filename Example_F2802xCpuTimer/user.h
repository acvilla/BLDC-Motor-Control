/*
 * user.h
 *
 *  Created on: Oct 23, 2018
 *      Author: Alex
 */

#ifndef USER_H_
#define USER_H_

#define NUM_POLE_PAIRS 4;
#define STALL_CURRENT_A 20.0;
#define RPM_MAX 1400;
#define K_I 0;
#define K_P 0;
#define RPM_REF_DEFAULT 0;
#define DUTY_CYCLE_DEFAULT 50;
#define DUTY_CYCLE_MAX 99;
#define DUTY_CYCLE_MIN 50;
#define TIMER_PERIOD 60000000; // 1 Second with 60MHz Clock

typedef enum _boolean_{
    FALSE = 0,
    TRUE = 1
}boolean;

typedef struct _MOTOR_Params_
{
    int npp; // Number of Pole Pairs of Motor
    double stallCurrent_A; // Stall Current in Amps
} MOTOR_Params;

typedef struct _PI_Params_
{
    double k_i;
    double k_p;
} PI_Params;

typedef struct _SPEED_CALC_Obj_ {
    unsigned long timerVal;
    unsigned long timerPeriod;
    boolean speedUpdateReady;
    double rpmRef;
    double rpm;
    double rpmMax; // Motor max RPM
}SPEED_CALC_Obj;

typedef struct _CONTROL_Obj_
{
    PI_Params piController;
    MOTOR_Params motor;
    SPEED_CALC_Obj speedCalc;
    volatile int dutyCycle;
    int dutyCycleMin;
    int dutyCycleMax;
    double currentLegA_A;
    double currentLegB_A;
} CONTROL_Obj;



void initControl(CONTROL_Obj *ControlPtr);

#endif /* USER_H_ */
