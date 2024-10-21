/*
ARASTAT POTENTIOSTAT CORE
Written by arachanx21
ARA APPLIED 2023


This software is licensed by ARA APPLIED through BSD-3 License. All text here must be included in any distribution
You may copy, modify,redistribute directly or in derivatives as long as it's in compliance with the license.
*/
#include "ASPC.h"

uint16_t length;

void ASPC_init(ASPC *_ASPC){
    if (!_ASPC->DAC_RES) _ASPC->DAC_RES=12;
    if (!_ASPC->V_ref) _ASPC->V_ref=3300;
    if (!_ASPC->V_start) _ASPC->V_start=-1000;
    if (!_ASPC->V_scanRate) _ASPC->V_scanRate=100;
    if (!_ASPC->rate) _ASPC->rate=SAMPLERATE;
    if (_ASPC->mode==5) _ASPC->duration=_ASPC->V_final;

    _ASPC->dac_sequence=get_dac_sequence(_ASPC);
}

void ASPC_deinit(ASPC *_ASPC){
	//free the dac sequence
	free(_ASPC->dac_sequence);

	//free the ASPC
	free(_ASPC);
    _ASPC->dac_sequence=NULL;
    _ASPC=NULL;
    
	return;
}

void set_ASPC_scan_rate(ASPC *_ASPC,uint8_t VScan){
    _ASPC->V_scanRate=VScan;
    free(_ASPC->dac_sequence);
    _ASPC->dac_sequence=get_dac_sequence(_ASPC);
    return;
}

void set_ASPC_start_voltage(ASPC *_ASPC,float VStart){
    _ASPC->V_start=VStart;
    free(_ASPC->dac_sequence);
    _ASPC->dac_sequence=get_dac_sequence(_ASPC);
    return;
}

void set_ASPC_reference_voltage(ASPC *_ASPC,float Vref){
    _ASPC->V_ref=Vref;
    free(_ASPC->dac_sequence);
    _ASPC->dac_sequence=get_dac_sequence(_ASPC);
    return;
}

void set_ASPC_DAC_resolution(ASPC *_ASPC,uint8_t _DAC_RES){
    _ASPC->DAC_RES=_DAC_RES;
    free(_ASPC->dac_sequence);
    _ASPC->dac_sequence=get_dac_sequence(_ASPC);
    return;
}

void set_ASPC_mode(ASPC *_ASPC, uint8_t mode){
    _ASPC->mode=mode;
    free(_ASPC->dac_sequence);
    _ASPC->dac_sequence=get_dac_sequence(_ASPC);
    return;
}


uint16_t get_DAC_initial_voltage(ASPC *_ASPC){
    uint16_t dacValue;
    float voltage=(_ASPC->V_ref/2)+_ASPC->V_start; 
    dacValue =(uint16_t) (voltage/_ASPC->V_ref*4095);
    return dacValue;
}

uint16_t get_DAC_final_voltage(ASPC *_ASPC){
    uint16_t dacValue;
    float voltage=(_ASPC->V_ref/2)-_ASPC->V_start; 
    dacValue =(uint16_t) (voltage/_ASPC->V_ref*4095);
    return dacValue;
}

uint16_t get_dac_desired_voltage(uint16_t vRef, int16_t vTarget){
    uint16_t dacValue;
    //convert the voltage relative to the reference
    uint16_t voltage = (vRef/2) + vTarget;
    dacValue = (uint16_t) vTarget*4095/vRef;
    return dacValue;
}

float get_DAC_step_value(ASPC *_ASPC){
    float stepValue;
    stepValue = (float) _ASPC->V_scanRate/(_ASPC->V_ref)*4095/_ASPC->rate;
    return stepValue;
}

float get_voltage(ASPC *_ASPC,uint16_t DACValue){
    float volt;
    volt = (float) DACValue/4095*_ASPC->V_ref;
    return volt;
}

uint16_t * get_dac_sequence(ASPC *_ASPC){
    /*generating array of dac values for potentiostat voltage sequences
    @params ASPC ASPC struct

    @return array pointer of dac sequences of values 
    */
    //setting up the multiplier
    int multiplier;
    if (_ASPC->V_start>=0) multiplier=1;
    else multiplier=-1;
    uint16_t *dac_seq=NULL;
    uint16_t size;
    if (_ASPC->mode ==0){
        size = (uint16_t) 2*2*multiplier*_ASPC->V_start*(_ASPC->rate)/(_ASPC->V_scanRate);
        //printf("array size..");
        uint16_t mid;
        // uint16_t size = (uint16_t) 2*multiplier*(get_DAC_initial_voltage(_ASPC)-get_DAC_final_voltage(_ASPC))/get_DAC_step_value(_ASPC);
        if (size%2!=0) {
            size++;
            mid=size/2;
            length+=2;
            _ASPC->_dac_size=size+1;
            dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
            }
        else{
            mid=size/2;
            _ASPC->_dac_size=size+1;
            dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
            length++;
        }
        //printf("%d\n",_ASPC->_dac_size);
        *dac_seq=get_DAC_initial_voltage(_ASPC);
        *(dac_seq+size)=*dac_seq;
        float step_val = get_DAC_step_value(_ASPC);
        float val=*dac_seq;
        //if Vstart <0 DAC value increases, then from 1 should be multiplied by -1 otherwise changed from -1
        multiplier*=-1;

        for (int i=1;i<mid+1;i++){
            val+=step_val*multiplier;
            *(dac_seq+i)=(uint16_t) val;
            *(dac_seq+size-i)=*(dac_seq+i);
        }
        *(dac_seq+mid)=get_DAC_final_voltage(_ASPC);
        // val=*(dac_seq+mid);
        //printf("Halfway! dac value: %hu\n",*(dac_seq+mid));    
    }
    
    else if (_ASPC->mode == 1) { //linear forward
        if (_ASPC->V_start<0)  size = (uint16_t) multiplier*(get_DAC_initial_voltage(_ASPC)-2048)/get_DAC_step_value(_ASPC); //from negative to zero
        if (_ASPC->V_start>0 || _ASPC->V_start==0) size = (uint16_t) (4095-get_DAC_initial_voltage(_ASPC))/get_DAC_step_value(_ASPC); //from zero or positive to max
        dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
        _ASPC->_dac_size=size;
        
        *dac_seq=get_DAC_initial_voltage(_ASPC);
        float step_val = get_DAC_step_value(_ASPC);
        float val = *dac_seq;

        for (int i=1;i<size;i++){
            val+=step_val;
            *(dac_seq+i)=(uint16_t) val;
        }

    }
    else if (_ASPC->mode == 2) { //linear reverse
        if (_ASPC->V_start<0)  size = (uint16_t) get_DAC_initial_voltage(_ASPC)/get_DAC_step_value(_ASPC); //from negative to zero
        else if (_ASPC->V_start>0) size = (uint16_t) get_DAC_initial_voltage(_ASPC)/get_DAC_step_value(_ASPC); //from negative to zero //from zero or positive to max
        dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
        _ASPC->_dac_size=size;
        
        *dac_seq=get_DAC_initial_voltage(_ASPC);
        float step_val = get_DAC_step_value(_ASPC);
        float val = *dac_seq;

        for (int i=1;i<size;i++){
            val-=step_val;
            *(dac_seq+i)=(uint16_t) val;
        }

    }
    
    else if (_ASPC->mode ==4){ //updated CV
        uint16_t range;
        if ((_ASPC->V_start==_ASPC->V_final) && (_ASPC->V_start!=0)) size = (uint16_t) 2*2*multiplier*_ASPC->V_start*(_ASPC->rate)/(_ASPC->V_scanRate);
        else if ((_ASPC->V_start)>(_ASPC->V_final)) size = (uint16_t) 2*((_ASPC->V_start)-(_ASPC->V_final))*(_ASPC->rate)/(_ASPC->V_scanRate);
        else size = (uint16_t) 2*(-_ASPC->V_start+_ASPC->V_final)*(_ASPC->rate)/(_ASPC->V_scanRate);
        printf("size:%hu\n",size);
        uint16_t mid;

        if (size%2!=0) {
            size++;
            mid=size/2;
            length+=2;
            _ASPC->_dac_size=size+1;//yet to debug
            dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
            }

        else{
            mid=size/2;
            _ASPC->_dac_size=size;
            dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
            length++;
        }

        *dac_seq=get_DAC_initial_voltage(_ASPC);
        *(dac_seq+size)=*dac_seq;
        float step_val = get_DAC_step_value(_ASPC);
        printf("Step value: %f",step_val);
        float val=*dac_seq;
        //if Vstart <0 DAC value increases, then from 1 should be multiplied by -1 otherwise changed from -1
        if (_ASPC->V_start < 0 || _ASPC->V_start>_ASPC->V_final ) multiplier*=-1;

        for (int i=1;i<mid+1;i++){
            val+=step_val*multiplier;
            *(dac_seq+i)=(uint16_t) val;
            *(dac_seq+size-i)=*(dac_seq+i);
        } 
    }
    else if (_ASPC->mode == 5) { //chronoamperometry
        _ASPC->duration=_ASPC->V_final;
        size = _ASPC->duration*_ASPC->rate/1000; 
        _ASPC->_dac_size=size;
        dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
        for (int i;i<size;i++){
            *(dac_seq+i)=_ASPC->V_start;
        }
    }
    // _ASPC->raw_data=(int16_t *) malloc((_ASPC->_dac_size)*sizeof(int16_t));
    return dac_seq;
}
/*
 get_current_value
 @params ASPC *_ASPC
 @params uint16_t Rval Resistor value in the Transimpedance Amplifier
 @params int16_t adcValue adc measured value


@return current in mA
 */

float get_current_value(ASPC *_ASPC, uint16_t Rval, int16_t adcValue){
	int value = adcValue - (_ASPC->V_ref/2);
	float current = -value/Rval;

	return current;
}

/*
get data*/
void get_raw_data(ASPC* _ASPC){
    for (int i=0;i<_ASPC->_dac_size;i++){
        printf("DAC Value: %d\t",*(_ASPC->dac_sequence+i));
        printf("I-Voltage: %d\n",*(_ASPC->raw_data+i));
    }
}