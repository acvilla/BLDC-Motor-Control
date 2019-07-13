//#############################################################################
//
//  File:   Example_F2802xCpuTimer.c
//
//  Title:  F2802x CPU Timer Example
//
//! \addtogroup example_list
//!  <h1>CPU Timer</h1>
//!
//!   This example configures CPU Timer0, 1, & 2 and increments
//!   a counter each time the timer asserts an interrupt.
//!
//!   Watch Variables:
//!   - timer0IntCount
//!   - timer1IntCount
//!   - timer2IntCount
//
//#############################################################################
// $TI Release: F2802x Support Library v3.02.00.00 $
// $Release Date: Sun Mar 25 13:23:09 CDT 2018 $
// $Copyright:
// Copyright (C) 2009-2018 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//#############################################################################

//
// Included Files
//
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "common/include/clk.h"
#include "common/include/flash.h"
#include "common/include/gpio.h"
#include "common/include/pie.h"
#include "common/include/pll.h"
#include "common/include/timer.h"
#include "common/include/wdog.h"
#include "user.h"
#include "uart.h"
#include "printf.h"


//
// Function Prototypes
//
//__interrupt void cpu_timer0_isr(void);
__interrupt void cpu_timer1_isr(void);
__interrupt void cpu_timer2_isr(void);
__interrupt void adc_isr(void);
__interrupt void hall_a_isr(void);
__interrupt void hall_b_isr(void);
__interrupt void hall_c_isr(void);
void updatePWMState(volatile struct EPWM_REGS *pwmReg, pwm_state CSFA, pwm_state CSFB);
void setDutyCycle(uint8_t dutyCycle);
void initPWM(void);
void initGPIO(void);
void initADC(void);
void itoa(char *buf, int data);

//
// Globals
//
//unsigned long Xint1Count;
//unsigned long Xint2Count;
//unsigned long Xint3Count;
//unsigned long adcIntCount = 0;

CLK_Handle myClk;
ADC_Handle myAdc;
FLASH_Handle myFlash;
GPIO_Handle myGpio;
PIE_Handle myPie;
TIMER_Handle myTimer0, myTimer1;
PWM_Handle myPwm1, myPwm2, myPwm3;
SCI_Handle mySci;
CONTROL_Obj Control;
CONTROL_Obj *ControlPtr = &Control;
uint8_t dutyCycle = 0;
uint16_t CMP_GLOBAL;
//
// Main
//
void main(void)
{
    CPU_Handle myCpu;
    PLL_Handle myPll;
    WDOG_Handle myWDog;

    //
    // Initialize all the handles needed for this application
    //
    myAdc = ADC_init((void *)ADC_BASE_ADDR, sizeof(ADC_Obj));
    myClk = CLK_init((void *)CLK_BASE_ADDR, sizeof(CLK_Obj));
    myCpu = CPU_init((void *)NULL, sizeof(CPU_Obj));
    myFlash = FLASH_init((void *)FLASH_BASE_ADDR, sizeof(FLASH_Obj));
    myGpio = GPIO_init((void *)GPIO_BASE_ADDR, sizeof(GPIO_Obj));
    myPie = PIE_init((void *)PIE_BASE_ADDR, sizeof(PIE_Obj));
    myPll = PLL_init((void *)PLL_BASE_ADDR, sizeof(PLL_Obj));
    myTimer0 = TIMER_init((void *)TIMER0_BASE_ADDR, sizeof(TIMER_Obj));
    myTimer1 = TIMER_init((void *)TIMER1_BASE_ADDR, sizeof(TIMER_Obj));
    myWDog = WDOG_init((void *)WDOG_BASE_ADDR, sizeof(WDOG_Obj));
    myPwm1 = PWM_init((void *)PWM_ePWM1_BASE_ADDR, sizeof(PWM_Obj));
    myPwm2 = PWM_init((void *)PWM_ePWM2_BASE_ADDR, sizeof(PWM_Obj));
    myPwm3 = PWM_init((void *)PWM_ePWM3_BASE_ADDR, sizeof(PWM_Obj));
    mySci = SCI_init((void *)SCIA_BASE_ADDR, sizeof(SCI_Obj));

    /*
     * Initialize the Main Control Object.
     */
    initControl(ControlPtr);



    //
    // Step 1. Initialize System Control:
    // PLL, WatchDog, enable Peripheral Clocks
    // This example function is found in the f2802x_SysCtrl.c file.
    //
    //InitSysCtrl();

    //
    // Perform basic system initialization
    //
    WDOG_disable(myWDog);
    CLK_enableAdcClock(myClk);
    (*Device_cal)();

    //
    // Select the internal oscillator 1 as the clock source
    //
    CLK_setOscSrc(myClk, CLK_OscSrc_Internal);

    //
    // Setup the PLL for x12 /2 which will yield 60Mhz = 10Mhz * 12 / 2
    //
    PLL_setup(myPll, PLL_Multiplier_12, PLL_DivideSelect_ClkIn_by_2);

    //
    // Disable the PIE and all interrupts
    //
    PIE_disable(myPie);
    PIE_disableAllInts(myPie);
    CPU_disableGlobalInts(myCpu);
    CPU_clearIntFlags(myCpu);

    //
    // Step 2. Initialize GPIO:
    // This example function is found in the f2802x_Gpio.c file and
    // illustrates how to set the GPIO to it's default state.
    //
    //InitGpio();  // Skipped for this example

    //
    // Step 3. Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    //
    //DINT;

    //
    // Initialize the PIE control registers to their default state.
    // The default state is all PIE interrupts disabled and flags
    // are cleared.
    // This function is found in the f2802x_PieCtrl.c file.
    //
    //InitPieCtrl();

    //
    // Disable CPU interrupts and clear all CPU interrupt flags:
    //
    //IER = 0x0000;
    //IFR = 0x0000;

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    // This will populate the entire table, even if the interrupt
    // is not used in this example.  This is useful for debug purposes.
    // The shell ISR routines are found in f2802x_DefaultIsr.c.
    // This function is found in f2802x_PieVect.c.
    //
    //InitPieVectTable();
    PIE_setDebugIntVectorTable(myPie);
    PIE_enable(myPie);

    //
    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    //
    EALLOW;            // This is needed to write to EALLOW protected registers

    PIE_registerSystemIntHandler(myPie, PIE_SystemInterrupts_TINT1, 
                                 (intVec_t)&cpu_timer1_isr);
    PIE_registerPieIntHandler(myPie, PIE_GroupNumber_10, PIE_SubGroupNumber_1,
                                  (intVec_t)&adc_isr);
    PIE_registerPieIntHandler(myPie, PIE_GroupNumber_1, PIE_SubGroupNumber_4,
                                  (intVec_t)&hall_a_isr);
    PIE_registerPieIntHandler(myPie, PIE_GroupNumber_1, PIE_SubGroupNumber_5,
                                  (intVec_t)&hall_b_isr);
    PIE_registerPieIntHandler(myPie, PIE_GroupNumber_12, PIE_SubGroupNumber_1,
                                      (intVec_t)&hall_c_isr);

    PIE_enableInt(myPie, PIE_GroupNumber_1, PIE_InterruptSource_XINT_1);
    PIE_enableInt(myPie, PIE_GroupNumber_1, PIE_InterruptSource_XINT_2);
    PIE_enableInt(myPie, PIE_GroupNumber_12, PIE_InterruptSource_XINT_3);


    EDIS;    // This is needed to disable write to EALLOW protected registers

#ifdef _FLASH
    //
    // Copy time critical code and Flash setup code to RAM
    // This includes the following ISR functions: EPwm1_timer_isr(),
    // EPwm2_timer_isr() and FLASH_setup();
    // The  RamfuncsLoadStart, RamfuncsLoadSize, and RamfuncsRunStart
    // symbols are created by the linker. Refer to the F228027.cmd file.
    //
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);

    //
    // Call Flash Initialization to setup flash waitstates
    // This function must reside in RAM
    //
    FLASH_setup(myFlash);
#endif
    //
    // Step 4. Initialize the Device Peripheral. This function can be
    // found in f2802x_CpuTimers.c
    //
    //InitCpuTimers();   // For this example, only initialize the Cpu Timers
    TIMER_stop(myTimer0);
    TIMER_stop(myTimer1);

    initGPIO();
       //
       // Configure XINT1
       //
     PIE_setExtIntPolarity(myPie, CPU_ExtIntNumber_1, PIE_ExtIntPolarity_RisingAndFallingEdge);
     PIE_setExtIntPolarity(myPie, CPU_ExtIntNumber_2, PIE_ExtIntPolarity_RisingAndFallingEdge);
     PIE_setExtIntPolarity(myPie, CPU_ExtIntNumber_3, PIE_ExtIntPolarity_RisingAndFallingEdge);

       //
       // Enable XINT1
       //
     PIE_enableExtInt(myPie, CPU_ExtIntNumber_1);
     PIE_enableExtInt(myPie, CPU_ExtIntNumber_2);
     PIE_enableExtInt(myPie, CPU_ExtIntNumber_3);



#if (CPU_FRQ_60MHZ)
    //
    // Configure CPU-Timer 0, 1, and 2 to interrupt every second:
    // 60MHz CPU Freq, 1 second Period (in uSeconds)
    //
    //ConfigCpuTimer(&CpuTimer0, 60, 1000000);
    TIMER_setPeriod(myTimer0, 60 * 1000000);
    //ConfigCpuTimer(&CpuTimer1, 60, 1000000);
    TIMER_setPeriod(myTimer1, 60 * 1000000);
    //ConfigCpuTimer(&CpuTimer2, 60, 1000000);

#endif
#if (CPU_FRQ_40MHZ)
    //
    // Configure CPU-Timer 0, 1, and 2 to interrupt every second:
    // 40MHz CPU Freq, 1 second Period (in uSeconds)
    //
    //ConfigCpuTimer(&CpuTimer0, 40, 1000000);
    TIMER_setPeriod(myTimer0, 40 * 1000000);
    //ConfigCpuTimer(&CpuTimer1, 40, 1000000);
    TIMER_setPeriod(myTimer1, 40 * 1000000);
    //ConfigCpuTimer(&CpuTimer2, 40, 1000000);
    TIMER_setPeriod(myTimer2, 40 * 1000000);
#endif


    TIMER_setPreScaler(myTimer0, 0); // No prescaler
    TIMER_reload(myTimer0);
    TIMER_setEmulationMode(myTimer0, 
                           TIMER_EmulationMode_StopAfterNextDecrement);


    TIMER_setPreScaler(myTimer1, 0);
    TIMER_reload(myTimer1);
    TIMER_setEmulationMode(myTimer1, 
                           TIMER_EmulationMode_StopAfterNextDecrement);
    //TIMER_enableInt(myTimer1);
    //
    // To ensure precise timing, use write-only instructions to write to the 
    // entire register. Therefore, if any of the configuration bits are changed
    // in ConfigCpuTimer and InitCpuTimers (in f2802x_CpuTimers.h), the
    // below settings must also be updated.
    //
    
    //
    // Use write-only instruction to set TSS bit = 0
    //
    //CpuTimer0Regs.TCR.all = 0x4001;
    //TIMER_start(myTimer0);
    
    //
    // Use write-only instruction to set TSS bit = 0
    //
    //CpuTimer1Regs.TCR.all = 0x4001;
    TIMER_start(myTimer1);
    
    //
    // Use write-only instruction to set TSS bit = 0
    //
    //CpuTimer2Regs.TCR.all = 0x4001;
    //TIMER_start(myTimer2);

    //
    // Step 5. User specific code, enable interrupts:
    //

    //
    // Enable CPU int1 which is connected to CPU-Timer 0, CPU int13
    // which is connected to CPU-Timer 1, and CPU int 14, which is connected
    // to CPU-Timer 2
    //
    CPU_enableInt(myCpu, CPU_IntNumber_13);
    CPU_enableInt(myCpu, CPU_IntNumber_10);
    CPU_enableInt(myCpu, CPU_IntNumber_1);
    CPU_enableInt(myCpu, CPU_IntNumber_12);

    //CPU_enableInt(myCpu, CPU_IntNumber_14);

    //
    // Enable TINT0 in the PIE: Group 1 interrupt 7
    //
    PIE_enableTimer0Int(myPie);

    //
    // Enable global Interrupts and higher priority real-time debug events
    //
    CPU_enableGlobalInts(myCpu);       // Enable Global interrupt INTM


    CPU_enableDebugInt(myCpu);         // Enable Global realtime interrupt DBGM

    //
    // Step 6. IDLE loop. Just sit and loop forever (optional)
    //
    CLK_disableTbClockSync(myClk);
    initPWM();
    CLK_enableTbClockSync(myClk);
    initADC();
    scia_init();                // Initialize SCI
    scia_fifo_init();           // Initialize the SCI FIFO
    char *msg = "\r\n\n\nHello World!\0";
    char buf[50];
    scia_msg(msg);
    initHallStates(myGpio, ControlPtr, GPIO_Number_12, GPIO_Number_6, GPIO_Number_7);
    for(;;)
      {
        if (ControlPtr->speedCalc.speedUpdateReady == TRUE){
            double periodSecs, freqHz;
            unsigned long periodCycles;
            periodCycles = (ControlPtr->speedCalc.timerPeriod - ControlPtr->speedCalc.timerVal); // Electrical rotation period in seconds
            periodSecs = (double) periodCycles/(ControlPtr->speedCalc.timerPeriod);
            freqHz = 1/periodSecs;
            ControlPtr->speedCalc.rpm = freqHz * 60.0/ControlPtr->motor.npp; // Update the speed value
            ControlPtr->speedCalc.speedUpdateReady = FALSE;
            if (ControlPtr->speedCalc.rpm > MIN_CLOSED_LOOP_RPM){
               //dutyCycle = updatePI(ControlPtr);
               //setDutyCycle(dutyCycle);
            }
        }


        //itoa(buf, (int) ControlPtr->speedCalc.rpm);
        snprintf(buf, sizeof(buf), "{RPM: %f, Battery: %d}\n\r", ControlPtr->speedCalc.rpm, 100);
        scia_msg(buf);



        switch (ControlPtr->currentHallStates){
        case C:
            // Phases: Aoff, B+, C-
            updatePWMState(&EPwm1Regs, LOW, LOW); // Phase A
            updatePWMState(&EPwm2Regs, PWM, PWM); // Phase B
            updatePWMState(&EPwm3Regs, LOW, HIGH); // Phase C
            break;
        case AC:
            // Phases: A-, B+, Coff
            updatePWMState(&EPwm1Regs, LOW, HIGH); // Phase A
            updatePWMState(&EPwm2Regs, PWM, PWM); // Phase B
            updatePWMState(&EPwm3Regs, LOW, LOW); // Phase C
            break;
        case A:
            // Phases: A-, Boff, C+
            updatePWMState(&EPwm1Regs, LOW, HIGH); // Phase A
            updatePWMState(&EPwm2Regs, LOW, LOW); // Phase B
            updatePWMState(&EPwm3Regs, PWM, PWM); // Phase C
            break;
        case AB:
            // Phases: Aoff, B-, C+
            updatePWMState(&EPwm1Regs, LOW, LOW); // Phase A
            updatePWMState(&EPwm2Regs, LOW, HIGH); // Phase B
            updatePWMState(&EPwm3Regs, PWM, PWM); // Phase C
            break;
        case B:
            // Phases: A+, B-, Coff
            updatePWMState(&EPwm1Regs, PWM, PWM); // Phase A
            updatePWMState(&EPwm2Regs, LOW, HIGH); // Phase B
            updatePWMState(&EPwm3Regs, LOW, LOW); // Phase C
            break;
        case BC:
            // Phases: A+, Boff, C-
            updatePWMState(&EPwm1Regs, PWM, PWM); // Phase A
            updatePWMState(&EPwm2Regs, LOW, LOW); // Phase B
            updatePWMState(&EPwm3Regs, LOW, HIGH); // Phase C
            break;
        default:
            break;
        }

      }

}

void initGPIO(void){
        // Set GPIO28 at RX and GPIO29 as TX for SCIA
        GPIO_setPullUp(myGpio, GPIO_Number_28, GPIO_PullUp_Enable);
        GPIO_setPullUp(myGpio, GPIO_Number_29, GPIO_PullUp_Disable);
        GPIO_setQualification(myGpio, GPIO_Number_28, GPIO_Qual_ASync);
        GPIO_setMode(myGpio, GPIO_Number_28, GPIO_28_Mode_SCIRXDA);
        GPIO_setMode(myGpio, GPIO_Number_29, GPIO_29_Mode_SCITXDA);

         // Set Up GPIO12 (Hall Sensor A) as input.
        GPIO_setMode(myGpio, GPIO_Number_12, GPIO_12_Mode_GeneralPurpose);
        GPIO_setDirection(myGpio, GPIO_Number_12, GPIO_Direction_Input);
        GPIO_setQualification(myGpio, GPIO_Number_12, GPIO_Qual_Sample_6);
        GPIO_setQualificationPeriod(myGpio, GPIO_Number_12, 0xFF); // 60MHz/(2*30) = 1uS

        // Set Up GPIO6 (Hall Sensor B) as input
        GPIO_setMode(myGpio, GPIO_Number_6, GPIO_6_Mode_GeneralPurpose);
        GPIO_setDirection(myGpio, GPIO_Number_6, GPIO_Direction_Input);
        GPIO_setQualification(myGpio, GPIO_Number_6, GPIO_Qual_Sample_6);
        GPIO_setQualificationPeriod(myGpio, GPIO_Number_6, 0xFF); // 60MHz/(2*30) = 1uS

        // Set Up GPIO7 (Hall Sensor C) as input.
        GPIO_setMode(myGpio, GPIO_Number_7, GPIO_7_Mode_GeneralPurpose);
        GPIO_setDirection(myGpio, GPIO_Number_7, GPIO_Direction_Input);
        GPIO_setQualification(myGpio, GPIO_Number_7, GPIO_Qual_Sample_6);
        GPIO_setQualificationPeriod(myGpio, GPIO_Number_7, 0xFF); // 60MHz/(2*30) = 1uS

        GPIO_setPullUp(myGpio, GPIO_Number_12, GPIO_PullUp_Enable);
        GPIO_setPullUp(myGpio, GPIO_Number_6, GPIO_PullUp_Enable);
        GPIO_setPullUp(myGpio, GPIO_Number_7, GPIO_PullUp_Enable);
        //
        // GPIO12 is XINT1 (Hall A), GPIO6 is XINT2 (Hall B), GPIO7 is XINT3 (Hall C)
        //
        GPIO_setExtInt(myGpio, GPIO_Number_12, CPU_ExtIntNumber_1);
        GPIO_setExtInt(myGpio, GPIO_Number_6, CPU_ExtIntNumber_2);
        GPIO_setExtInt(myGpio, GPIO_Number_7, CPU_ExtIntNumber_3);

        //
        // Initialize GPIO for PWM
        //
        GPIO_setPullUp(myGpio, GPIO_Number_0, GPIO_PullUp_Disable);
        GPIO_setPullUp(myGpio, GPIO_Number_1, GPIO_PullUp_Disable);
        GPIO_setMode(myGpio, GPIO_Number_0, GPIO_0_Mode_EPWM1A);
        GPIO_setMode(myGpio, GPIO_Number_1, GPIO_1_Mode_EPWM1B);

        GPIO_setPullUp(myGpio, GPIO_Number_2, GPIO_PullUp_Disable);
        GPIO_setPullUp(myGpio, GPIO_Number_3, GPIO_PullUp_Disable);
        GPIO_setMode(myGpio, GPIO_Number_2, GPIO_2_Mode_EPWM2A);
        GPIO_setMode(myGpio, GPIO_Number_3, GPIO_3_Mode_EPWM2B);

        GPIO_setPullUp(myGpio, GPIO_Number_4, GPIO_PullUp_Disable);
        GPIO_setPullUp(myGpio, GPIO_Number_5, GPIO_PullUp_Disable);
        GPIO_setMode(myGpio, GPIO_Number_4, GPIO_4_Mode_EPWM3A);
        GPIO_setMode(myGpio, GPIO_Number_5, GPIO_5_Mode_EPWM3B);
}

void initADC(void){
    //
       // Initialize the ADC
       //
       ADC_enableBandGap(myAdc);
       ADC_enableRefBuffers(myAdc);
       ADC_powerUp(myAdc);
       ADC_enable(myAdc);
       ADC_setVoltRefSrc(myAdc, ADC_VoltageRefSrc_Int);

       ADC_setIntPulseGenMode(myAdc, ADC_IntPulseGenMode_Prior);
       ADC_enableInt(myAdc, ADC_IntNumber_1);
       ADC_setIntMode(myAdc, ADC_IntNumber_1, ADC_IntMode_EOC);
       ADC_setIntSrc(myAdc, ADC_IntNumber_1, ADC_IntSrc_EOC0);
       ADC_setSocChanNumber (myAdc, ADC_SocNumber_0, ADC_SocChanNumber_A6);
       ADC_setSocTrigSrc(myAdc, ADC_SocNumber_0, ADC_SocTrigSrc_CpuTimer_1);
       ADC_setSocSampleWindow(myAdc, ADC_SocNumber_0,
                                 ADC_SocSampleWindow_37_cycles);
       PIE_enableAdcInt(myPie, ADC_IntNumber_1);
}

void initPWM(void)
{
    CLK_enablePwmClock(myClk, PWM_Number_1);
    CLK_enablePwmClock(myClk, PWM_Number_2);
    CLK_enablePwmClock(myClk, PWM_Number_3);
    EPwm1Regs.TBPRD = TBPRD_VALUE; // Period = 2*TBPRD TBCLK counts
    EPwm1Regs.TBPHS.half.TBPHS = 0; // Set Phase register to zero
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Symmetrical mode
    EPwm1Regs.DBCTL.bit.OUT_MODE = DB_DISABLE; // Disable Dead-band module
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE; // Master module
    EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // Sync down-stream module
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR; // set actions for EPWM1A
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;
    EPwm1Regs.AQCTLB.bit.CBU = AQ_SET; // set actions for EPWM1A
    EPwm1Regs.AQCTLB.bit.CBD = AQ_CLEAR;

   //EPwm1Regs.AQCSFRC.bit.CSFA = 0x0; // PWM still on
   //EPwm1Regs.AQCSFRC.bit.CSFA = 0x1; // Use these commands to force PWM outputs low or high
   //EPwm1Regs.AQCSFRC.bit.CSFB = 0x2;

    // EPWM Module 2 config
    EPwm2Regs.TBPRD = TBPRD_VALUE;
    EPwm2Regs.TBPHS.half.TBPHS = 0; // Set Phase register to zero
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Symmetrical mode
    EPwm2Regs.DBCTL.bit.OUT_MODE = DB_DISABLE; // Disable Dead-band module
    EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE; // Slave module
    EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN; // sync flow-through
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR; // set actions for EPWM2A
    EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;
    EPwm2Regs.AQCTLB.bit.CBU = AQ_SET; // set actions for EPWM1A
    EPwm2Regs.AQCTLB.bit.CBD = AQ_CLEAR;

    // EPWM Module 3 config
    EPwm3Regs.TBPRD = TBPRD_VALUE;
    EPwm3Regs.TBPHS.half.TBPHS = 0; // Set Phase register to zero
    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Symmetrical mode
    EPwm3Regs.DBCTL.bit.OUT_MODE = DB_DISABLE; // Disable Dead-band module
    EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE; // Slave module
    EPwm3Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN; // sync flow-through
    EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm3Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR; // set actions for EPWM3A
    EPwm3Regs.AQCTLA.bit.CAD = AQ_SET;
    EPwm3Regs.AQCTLB.bit.CBU = AQ_SET; // set actions for EPWM1A
    EPwm3Regs.AQCTLB.bit.CBD = AQ_CLEAR;


    EPwm1Regs.CMPA.half.CMPA = 750; // adjust duty for output EPWM1A
    EPwm1Regs.CMPB = 750; // adjust duty for output EPWM3B

    EPwm2Regs.CMPA.half.CMPA = 750; // adjust duty for output EPWM2A
    EPwm2Regs.CMPB = 750; // adjust duty for output EPWM3B

    EPwm3Regs.CMPA.half.CMPA = 750; // adjust duty for output EPWM3A
    EPwm3Regs.CMPB = 750; // adjust duty for output EPWM3B


}


//
// cpu_timer1_isr - 
//
__interrupt void
cpu_timer1_isr(void)
{
    /*
     * Main ISR: Sample Throttle to calculate speed ref.
     */

    // ADC SOC should occur when this interrupt fires


}

//
// adc_isr -
//
__interrupt void
adc_isr(void)
{
    unsigned int adc_counts = ADC_readResult(myAdc, ADC_ResultNumber_0);
    double result = (double)(adc_counts*ControlPtr->speedCalc.rpmMax)/((1 << NUM_ADC_BITS) - 1);
    ControlPtr->speedCalc.rpmRef = result; // Update the command value used by the PI controller
    ADC_clearIntFlag(myAdc, ADC_IntNumber_1);
    PIE_clearInt(myPie, PIE_GroupNumber_10);
    return;
}
//
// hall_a_isr-
//
__interrupt void
hall_a_isr(void)
{
    uint32_t gpioVal = GPIO_getData(myGpio, GPIO_Number_12);
    updateHall_A(gpioVal, ControlPtr);
    checkHallErr(ControlPtr);
    //
    // Acknowledge this interrupt to get more from group 1
    //
    PIE_clearInt(myPie, PIE_GroupNumber_1);
}
//
// hall_b_isr-
//
__interrupt void
hall_b_isr(void)
{
    uint32_t gpioVal = GPIO_getData(myGpio, GPIO_Number_6);
    updateHall_B(gpioVal, ControlPtr);
    checkHallErr(ControlPtr);
    GPIO_toggle(myGpio, GPIO_Number_29);
      if (gpioVal == 1){      // Used to get time between rising edge to calculate speed.
          //GPIO_setHigh(myGpio, GPIO_Number_29);
      } else {
          if (myTimer0->TCR & TIMER_TCR_TSS_BITS){ // If timer is stopped, start timer and begin count
                     //GPIO_setLow(myGpio, GPIO_Number_28);
                     TIMER_reload(myTimer0);
                     TIMER_start(myTimer0);
          }else {
              // GPIO_setHigh(myGpio, GPIO_Number_28);
              TIMER_stop(myTimer0);

              ControlPtr->speedCalc.timerVal = TIMER_getCount(myTimer0);
              ControlPtr->speedCalc.speedUpdateReady = TRUE;
           }
           //GPIO_setLow(myGpio, GPIO_Number_29);
      }


    //
    // Acknowledge this interrupt to get more from group 1
    //
    PIE_clearInt(myPie, PIE_GroupNumber_1);
}
//
// hall_c_isr -
//
__interrupt void
hall_c_isr(void)
{
    uint32_t gpioVal = GPIO_getData(myGpio, GPIO_Number_7);
    updateHall_C(gpioVal, ControlPtr);
    checkHallErr(ControlPtr);
    //
    // Acknowledge this interrupt to get more from group 12
    //
    PIE_clearInt(myPie, PIE_GroupNumber_12);
}

void itoa(char *buf, int data)
{
    int n = data;
    int i = 0;

    bool isNeg = n<0;

            unsigned int n1 = isNeg ? -n : n;

            while(n1!=0)
            {
                buf[i++] = n1%10+'0';
                n1=n1/10;
            }

            if(isNeg)
                buf[i++] = '-';

            buf[i] = '\n'; // insert newline
            buf[i+1] = '\0';
            int t;
            for(t = 0; t < i/2; t++)
            {
                buf[t] ^= buf[i-t-1];
                buf[i-t-1] ^= buf[t];
                buf[t] ^= buf[i-t-1];
            }

            if(n == 0)
            {
                buf[0] = '0';
                buf[1] = '\n';
                buf[2] = '\0';
            }
}
//
// End of File
//

