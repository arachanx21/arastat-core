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

void ASPC_configure(ASPC *_ASPC,int16_t *data){
    _ASPC->mode=*(data);
    _ASPC->V_start=*(data+1);
    _ASPC->V_final=*(data+2);
    _ASPC->V_scanRate=*(data+3);
    _ASPC->V_ref=*(data+4);
    _ASPC->rate=SAMPLERATE;
    _ASPC->V_initial=_ASPC->V_ref;
    _ASPC->dac_sequence=get_dac_sequence(_ASPC);
}

void ASPC_deinit(ASPC *_ASPC){
	//free the dac sequence
	if (_ASPC->dac_sequence !=NULL) free(_ASPC->dac_sequence);

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

void set_ASPC_initial_voltage(ASPC *_ASPC,int16_t Vinit){
    if (Vinit>(_ASPC->V_ref)/2 || Vinit<-(_ASPC->V_ref)/2) {
        printf("Initial voltage is out of range\n");    
        return;
    }
    //if the scan is increasing, the initial voltage should be less than the final voltage and more than the starting one
    else if ((_ASPC->V_start>_ASPC->V_final) && (Vinit>_ASPC->V_final || Vinit<_ASPC->V_start)){
        printf("Initial voltage is out of range\n");
        return;
    }
    //if the scan is decreasing, the initial voltage should be more than the final voltage and less than the starting one
    else if ((_ASPC->V_start<_ASPC->V_final) && (Vinit<_ASPC->V_final || Vinit>_ASPC->V_start)){
        printf("Initial voltage is out of range\n");
        return;
    }
    else{
        _ASPC->V_initial=Vinit;
        _ASPC->dac_sequence=get_dac_sequence(_ASPC);
        return;
    }
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
    
  if (_ASPC->mode == LINEAR_SWEEP_VOLTAMMETRY) { //linear forward
    if (_ASPC->V_start==_ASPC->V_final) return NULL;
    return sequence_generator(_ASPC,_ASPC->V_start,_ASPC->V_final,0);
  }
    
    else if (_ASPC->mode==CYCLIC_VOLTAMMETRY){ 
      //if the initial voltage isn't set
      if (_ASPC->V_initial==_ASPC->V_ref){
        dac_seq =  sequence_generator(_ASPC,_ASPC->V_start,_ASPC->V_final,1);
      
      }
      else{
        uint16_t *seq1 = sequence_generator(_ASPC,_ASPC->V_initial,_ASPC->V_final,1);
        size = _ASPC->_dac_size;
        uint16_t *seq2 = sequence_generator(_ASPC,_ASPC->V_initial,_ASPC->V_start,1);
        
        //when sequence 1 and 2 are added, there is one same value in the start of sequence 2
        //the size is reduced by 1 to remove redundancy
        dac_seq = (uint16_t *) malloc((size-1+_ASPC->_dac_size)*sizeof(uint16_t));

        for (int i=0;i<size;i++){
            *(dac_seq+i)=*(seq1+i);
        }
        //the first value of seq2 is the same as the last value of seq1, skipped 1 increment.
        for (int i=1;i<_ASPC->_dac_size;i++){
            *(dac_seq+size+i-1)=*(seq2+i);
        }
        _ASPC->_dac_size+=size-1;
        free(seq1);
        free(seq2);
      }
      
        
    }
    return dac_seq;
}

uint16_t *sequence_generator(ASPC *_ASPC, int16_t V_start,int16_t V_final, uint8_t cyclic){
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
      printf("Step value: %f\n",(uint16_t)step_val);
      //if Vstart <0 DAC value increases, then from 1 should be multiplied by -1 otherwise changed from -1 to reverse the increment.
      if (V_start>V_final ) multiplier*=-1;

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

uint16_t voltage_to_dac(int16_t voltage,uint16_t vRef, uint16_t dacResolution){
    uint16_t dacVal;
    dacVal = (uint16_t) (voltage-vRef/2)*dacResolution/vRef;
    return dacVal;    
}
