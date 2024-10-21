#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "math.h"

#define ASPC_SCANRATE_50MV  50
#define ASPC_SCANRATE_100MV 100 
#define ASPC_SCANRATE_300MV 300
#define ASPC_SCANRATE_500MV 500

#define ASPC_5V_REF 5.0
#define ASPC_3_3V_REF 3.3

#define SAMPLERATE 20   //#samples generated per second
#define DAC_RESOLUTION 12 //12-bit
#define TIA_RESISTOR 1000 //1k Resistor is placed in the TIA potentiostat
#define ON_PRODUCTION 1

typedef struct
{
    /* data all voltages are in mV*/ 
    uint16_t V_scanRate;
    uint16_t V_ref;
    int16_t V_start;
    int16_t V_final;
    uint8_t DAC_RES;
    uint16_t rate;
    uint8_t mode;
    uint16_t duration;
    
    //private generated from get_dac_sequence
    uint16_t _dac_size;
    uint16_t *dac_sequence;
    int16_t *raw_data

}ASPC;

typedef enum {
	CYCLIC_VOLTAMMETRY = 1,
	FORWARD_VOLTAMMETRY,
    REVERSE_VOLTAMMETRY,
    CYCLIC_VOLTAMMETRY_SPEC,
	CHRONOAMPEROMETRY,
}ASPC_Mode_t;

// typedef struct 
// {
//     uint16_t DAC_upper_limit;
//     uint16_t DAC_lower_limit;
//     uint16_t DAC_starting_value;
//     float DAC_step_value;
//     float V_ref;

// }ASPC_DAC;


// ASPC_DAC *ASPC_init(ASPC* _ASPC);
// void start(ASPC_DAC *_ASPC_DAC);
// float dac_get_voltage(ASPC_DAC *_ASPC,uint16_t DACValue);
void ASPC_init(ASPC *_ASPC);
void ASPC_deinit(ASPC *_ASPC);
float get_voltage(ASPC *_ASPC,uint16_t DACValue);
float get_DAC_step_value(ASPC *_ASPC);
uint16_t get_DAC_initial_voltage(ASPC *_ASPC);
void set_ASPC_DAC_resolution(ASPC *_ASPC,uint8_t _DAC_RES);
void set_ASPC_reference_voltage(ASPC *_ASPC,float Vref);
void set_ASPC_start_voltage(ASPC *_ASPC,float VStart);
void set_ASPC_scan_rate(ASPC *_ASPC,uint8_t VScan);
void set_ASPC_mode(ASPC *_ASPC, uint8_t mode);
uint16_t get_DAC_final_voltage(ASPC *_ASPC);
uint16_t get_dac_desired_voltage(uint16_t vRef, int16_t vTarget);
uint16_t * get_dac_sequence(ASPC *_ASPC);
float get_current_value(ASPC *_ASPC, uint16_t Rval, int16_t adcValue);
void get_raw_data(ASPC* _ASPC);