/*
  ==============================================================================

    LFO.h
    Created: 30 Sep 2022 7:01:30pm
    Author:  Nathan Erthal

  ==============================================================================
*/

#pragma once
 
#include <iostream>
#include <cmath>
#include <vector>

 
using namespace std;
 
class LFO {
public:
 
    LFO() = default;
      ~LFO() = default;
 
    LFO(const LFO& lfo) = default;
    
    void prepare(double sampleRate, float minVal, float maxVal){
        this->sampleRate = float(sampleRate);
        this->minVal = minVal;
        this->maxVal = maxVal;
    }
    
    void setParameters(float modFreq,float modDepth){
        this->modFreq = modFreq;
        this->modDepth = modDepth;
    }
    
    void reset(){
        t=0;
    }
    
    void update(){
        sineWave = modDepth*sin(2*M_PI*modFreq*(t++)/sampleRate);
        sineWave+=modDepth;
    }
    
    float getNextVal(float value){
        value += sineWave;
        return max(minVal, min(maxVal, value));
    }
 
 
 
private:
    int t=0;
    float sampleRate;
    float minVal;
    float maxVal;
    float sineWave;
    float modFreq;
    float modDepth;
};

