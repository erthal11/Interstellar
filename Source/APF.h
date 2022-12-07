/*
  ==============================================================================

    APF.h
    Created: 25 Nov 2022 10:44:56am
    Author:  Nathan Erthal

  ==============================================================================
*/

// All Pass Filter

#pragma once

#include "DelayLine.h"

class APF {
    
public:
 
    APF() = default;
      ~APF() = default;
 
    APF(const APF& Apf) = default;
    
    void setup(double sampleRate, float delay_ms)
    {
        delayLine.setMaxDelay(sampleRate, delay_ms);
        delayLine.setParameters(delay_ms, 0);
    }
    
    float processAudioSample(float input, float feedbackAmount)
    {
        feedForwardSample = input * feedbackAmount;
        feedbackSample = delayLine.getFeedbackSample() * feedbackAmount * -1;
        return delayLine.processAudioSample(input + feedbackSample) + feedForwardSample;
    }
    
    void clearBuffer(){
        delayLine.clearBuffer();
    }
    
 
private:
    DelayLine delayLine;
    float feedbackSample;
    float feedForwardSample;
};

