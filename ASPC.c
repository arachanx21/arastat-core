/*
ARASTAT POTENTIOSTAT CORE
Written by arachanx21
ARA APPLIED 2023


This software is licensed by ARA APPLIED through BSD-3 License. All text here must be included in any distribution
You may copy, modify,redistribute directly or in derivatives as long as it's in compliance with the license.
*/
#include "ASPC.h"

void ASPC_init(ASPC *_ASPC){
    _ASPC->dac_sequence=NULL;
    _ASPC->DAC_RES=12;
    _ASPC->V_ref=3300;
    _ASPC->V_start=-1000;
    _ASPC->V_initial=_ASPC->V_ref;
    _ASPC->V_scanRate=100;
    _ASPC->mode=3;
    _ASPC->rate=1;
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

//Setters
void set_ASPC_DAC_Resolution(ASPC *_ASPC, uint8_t DACResolution){
    _ASPC->DAC_RES=DACResolution;
    return;
}

void set_ASPC_scan_rate(ASPC *_ASPC,uint16_t VScan){
    _ASPC->V_scanRate=VScan;
    return;
}

void set_ASPC_start_voltage(ASPC *_ASPC,int16_t VStart){
    _ASPC->V_start=VStart;
    return;
}

void set_ASPC_final_voltage(ASPC *_ASPC,int16_t Vfinal){
    _ASPC->V_final=Vfinal;
    return;
}

void set_ASPC_reference_voltage(ASPC *_ASPC,uint16_t Vref){
/* set the reference voltage for the DAC value generation
    @param
    ASPC *: pointer to store the data in the structs
    vRef: Desired reference voltage, in mV
    
    @return None
*/
    _ASPC->V_ref=Vref;
    return;
}

void set_ASPC_DAC_resolution(ASPC *_ASPC,uint8_t _DAC_RES){
    _ASPC->DAC_RES=_DAC_RES;
    //update the DAC sequence following the resolution change
    free(_ASPC->dac_sequence);
    _ASPC->dac_sequence=get_dac_sequence(_ASPC);
    return;
}

void set_ASPC_mode(ASPC *_ASPC, uint8_t mode){
    _ASPC->mode=mode;
    //update the DAC sequence following the mode change
    return;
}

//getters
uint16_t get_DAC_initial_voltage(ASPC *_ASPC){
    uint16_t dacValue;
    uint16_t dacResValue = (uint16_t) pow(2,_ASPC->DAC_RES)-1;
    float voltage=(_ASPC->V_ref/2)+_ASPC->V_start; 
    dacValue =(uint16_t) (voltage/_ASPC->V_ref*dacResValue);
    return dacValue;
}

uint16_t get_DAC_final_voltage(ASPC *_ASPC){
    uint16_t dacValue;
    uint16_t dacResValue = (uint16_t) pow(2,_ASPC->DAC_RES)-1;
    float voltage=(_ASPC->V_ref/2)+_ASPC->V_final; 
    dacValue =(uint16_t) (voltage/_ASPC->V_ref*dacResValue);
    return dacValue;
}

uint16_t get_dac_desired_voltage(uint16_t vRef, int16_t vTarget){
    uint16_t dacValue;
    //convert the voltage relative to the reference
    uint16_t voltage = (vRef/2) + vTarget;
    dacValue = (uint16_t) voltage*4095/vRef;
    return dacValue;
}

float get_DAC_step_value(ASPC *_ASPC){
    float stepValue;
    uint16_t dacResValue = (uint16_t) pow(2,_ASPC->DAC_RES)-1;
    stepValue = (float) _ASPC->V_scanRate/(_ASPC->V_ref)*dacResValue/_ASPC->rate;
    return stepValue;
}

float get_voltage(ASPC *_ASPC,uint16_t DACValue){
    float volt;
    uint16_t dacResValue = (uint16_t) pow(2,_ASPC->DAC_RES)-1;
    volt = (float) DACValue/dacResValue*_ASPC->V_ref;
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
    
  if (_ASPC->mode == LINEAR_SWEEP_VOLTAMMETRY_FWD) { //linear forward
    uint16_t dacResValue = (uint16_t) pow(2,_ASPC->DAC_RES)-1;
    uint16_t halfDacResValue = (uint16_t) pow(2,_ASPC->DAC_RES)/2-2;
    //from negative to zero
    if (_ASPC->V_start<0)  size = (uint16_t) multiplier*(get_DAC_initial_voltage(_ASPC)-halfDacResValue)/get_DAC_step_value(_ASPC);
    //from zero or positive to max
    if (_ASPC->V_start>0 || _ASPC->V_start==0) size = (uint16_t) (dacResValue-get_DAC_initial_voltage(_ASPC))/get_DAC_step_value(_ASPC); 
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
    else if (_ASPC->mode == LINEAR_SWEEP_VOLTAMMETRY_RV) { //linear reverse
      if (_ASPC->V_start<0)  size = (uint16_t) get_DAC_initial_voltage(_ASPC)/get_DAC_step_value(_ASPC); //from negative to zero
      else if (_ASPC->V_start>0) size = (uint16_t) get_DAC_initial_voltage(_ASPC)/get_DAC_step_value(_ASPC); //from negative to zero or positive to max
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
    
    else if ((_ASPC->mode ==CYCLIC_VOLTAMMETRY) && (_ASPC->V_initial==_ASPC->V_ref)){ //updated CV
      uint16_t range;
      if ((_ASPC->V_start==_ASPC->V_final) && (_ASPC->V_start!=0)) size = (uint16_t) 2*2*multiplier*_ASPC->V_start*(_ASPC->rate)/(_ASPC->V_scanRate);
      else if ((_ASPC->V_start)>(_ASPC->V_final)) size = (uint16_t) 2*((_ASPC->V_start)-(_ASPC->V_final))*(_ASPC->rate)/(_ASPC->V_scanRate);
      else size = (uint16_t) 2*(-_ASPC->V_start+_ASPC->V_final)*(_ASPC->rate)/(_ASPC->V_scanRate);
      uint16_t mid;
        
      if (size%2!=0) {
          size++;
          mid=size/2;
          _ASPC->_dac_size=size+1;//yet to debug
          dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
          }

      else{
          size++;
          mid=size/2;
          _ASPC->_dac_size=size;
          dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
      }
      printf("size:%hu\n",size);

      float val=get_DAC_initial_voltage(_ASPC);
      float step_val = get_DAC_step_value(_ASPC);
      printf("Step value: %f",step_val);
      //if Vstart <0 DAC value increases, then from 1 should be multiplied by -1 otherwise changed from -1
      if (_ASPC->V_start < 0 || _ASPC->V_start>_ASPC->V_final ) multiplier*=-1;

      for (int i=0;i<mid;i++){
          *(dac_seq+i)=(uint16_t) val;
          *(dac_seq+size-1-i)=*(dac_seq+i);
          val+=step_val*multiplier;
      } 
      *(dac_seq+mid)=get_DAC_final_voltage(_ASPC);
    }
    return dac_seq;
}

uint16_t *sequence_generator(ASPC *_ASPC, int16_t V_start,int16_t V_final, uint8_t cyclic){
    uint16_t range;
    uint16_t size;
    uint16_t *dac_seq = NULL;
    uint16_t mid;
    int8_t multiplier = 1;
    if ((V_start)>(V_final)) size = (uint16_t) ((V_start)-(V_final))*(_ASPC->rate)/(_ASPC->V_scanRate);
    else size = (uint16_t) (-V_start+V_final)*(_ASPC->rate)/(_ASPC->V_scanRate);
    if (cyclic) {
        size*=2;
        if (size%2!=0) {
            size++;
            mid=size/2;
            _ASPC->_dac_size=size+1;//yet to debug
            dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
            }

        else{
            size++;
            mid=size/2;
            _ASPC->_dac_size=size;
            dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
        }
    }
    else {
      mid=size;
      dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
      }
      _ASPC->_dac_size=size;
      printf("size:%hu\n",size);

      float val=get_dac_desired_voltage(_ASPC->V_ref,V_start);
      float step_val = get_DAC_step_value(_ASPC);
      printf("Step value: %f\n",step_val);
      //if Vstart <0 DAC value increases, then from 1 should be multiplied by -1 otherwise changed from -1
      if (V_start < 0 || V_start>V_final ) multiplier*=-1;

      for (int i=0;i<mid;i++){
          *(dac_seq+i)=(uint16_t) val;
          if (cyclic) *(dac_seq+size-1-i)=*(dac_seq+i);
          val+=step_val*multiplier;
      } 
      if (cyclic) *(dac_seq+mid)=get_dac_desired_voltage(_ASPC->V_ref,V_final);
      else *(dac_seq+size-1)=get_dac_desired_voltage(_ASPC->V_ref,V_final);
    
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

uint8_t isDAQEnabled(ASPC* _ASPC){
    if(_ASPC->raw_data==NULL) return 0;
    else return 1;
}

void enableDataAcquisition(ASPC* _ASPC){
    if (_ASPC->raw_data!=NULL) free(_ASPC->raw_data);
    _ASPC->raw_data=(int16_t *) malloc((_ASPC->_dac_size)*sizeof(int16_t));
}

void disableDataAcquisition(ASPC* _ASPC){
    if (_ASPC->raw_data!=NULL) free(_ASPC->raw_data);
    _ASPC->raw_data=NULL;
    
}

/*
get data*/
void get_raw_data(ASPC* _ASPC){
    for (int i=0;i<_ASPC->_dac_size;i++){
        printf("DAC Value: %d\t",*(_ASPC->dac_sequence+i));
        printf("I-Voltage: %d\n",*(_ASPC->raw_data+i));
    }
}