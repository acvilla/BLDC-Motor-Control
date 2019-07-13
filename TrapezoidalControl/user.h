/*
 * user.h
 *
 *  Created on: Oct 23, 2018
 *      Author: Alex
 */

#include "DSP28x_Project.h"
#include "common/include/gpio.h"
#include "common/include/clk.h"

#ifndef USER_H_
#define USER_H_

#define NUM_POLE_PAIRS 4
#define STALL_CURRENT_A 20.0
#define RPM_MAX 1400
#define K_I 0.0 // 0.5 is roughly tuned value for Ki
#define K_P 0.1 // 0.2 is roughly tuned value for Kp
#define RPM_REF_DEFAULT 800
#define MIN_CLOSED_LOOP_RPM 200

// PWM Definitions
#define DUTY_CYCLE_DEFAULT 50
#define DUTY_CYCLE_MAX 99
#define DUTY_CYCLE_MIN 50
#define PWM_FREQ_HZ 10000
#define TBPRD_VALUE 1500

// Timer Definitions
#define TIMER_PERIOD 60000000 // 1 Second with 60MHz Clock

#define ADC_REF_VOLTAGE 3.30
#define NUM_ADC_BITS 12 // 12-bit ADC

// Hall State Definitions.
#define HALL_A_MASK (0b100)
#define HALL_B_MASK (0b010)
#define HALL_C_MASK (0b001)

#define GPIO_MODE_GENERAL_PURPOSE 0

extern uint16_t CMP_GLOBAL;
extern CLK_Handle myClk;


typedef enum _boolean_{
    FALSE = 0,
    TRUE = 1
}boolean;

typedef enum _motor_phase_{
    PHASE_A,
    PHASE_B,
    PHASE_C
}motor_phase;

typedef enum _pwm_state_{
    LOW = 0x1,
    HIGH = 0x2,
    PWM = 0x0
}pwm_state;
/*
 * hall states based on which hall signals are currently high (the rest are low).
 * For example state C would mean C is high and A and B are low.
 */
typedef enum _hall_state_{
    C = HALL_C_MASK,
    AC = HALL_A_MASK | HALL_C_MASK,
    A = HALL_A_MASK,
    AB = HALL_A_MASK | HALL_B_MASK,
    B = HALL_B_MASK,
    BC = HALL_B_MASK | HALL_C_MASK
}hall_state;


typedef struct _MOTOR_Params_
{
    int npp; // Number of Pole Pairs of Motor
    double stallCurrent_A; // Stall Current in Amps
    hall_state hallStateTable[6];
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
    double currentLegA_A;
    double currentLegB_A;
    volatile hall_state currentHallStates; // observed Hall States from gpio pins
    volatile hall_state *expectedHallStates; // pointer to expected place in hallStateTable
    boolean hallErr;
} CONTROL_Obj;



void initControl(CONTROL_Obj *ControlPtr);
void updateHall_A(int val, CONTROL_Obj *ControlPtr);
void updateHall_B(int val, CONTROL_Obj *ControlPtr);
void updateHall_C(int val, CONTROL_Obj *ControlPtr);
void initHallStates(GPIO_Handle myGpio, CONTROL_Obj *ControlPtr, GPIO_Number_e HallA, GPIO_Number_e HallB, GPIO_Number_e HallC);
void updateExpectedHallStates(CONTROL_Obj *ControlPtr);
void checkHallErr(CONTROL_Obj *ControlPtr);
double updatePI(CONTROL_Obj *ControlPtr);
void updatePWMState(volatile struct EPWM_REGS *pwmReg, pwm_state CSFA, pwm_state CSFB);
void setDutyCycle(uint8_t dutyCycle);

#endif /* USER_H_ */
