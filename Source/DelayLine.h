/*
  ==============================================================================

    Delay.h
    Created: 18 Sep 2022 8:02:07pm
    Author:  Nathan Erthal
 
    Delay buffer object. Takes in delay time in milliseconds

  ==============================================================================
*/

#pragma once
 
#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

class DelayLine {
public:
    
    DelayLine() = default;
    ~DelayLine() = default;
    
    DelayLine(const DelayLine& delayLine) = default;
    
    void setMaxDelay(const int sampleRate, const int maxDelay_ms)
    {
        this->sampleRate = sampleRate;
        writeIndex = 0;
        readIndex = 0;
        unsigned int s = sampleRate * maxDelay_ms/1000;
        
        //make buffersize equal to a power of two, for wrap-around logic to work with wire-AND-ing (&=)
        s--;
        s |= s >> 1;
        s |= s >> 2;
        s |= s >> 4;
        s |= s >> 8;
        s |= s >> 16;
        s++;
        buffer.resize(s);
    }
    
    void clearBuffer(){
        bufferClearing = true;
        indexWhereClearEnds = writeIndex;
    }
    
    void setParameters(float delay_ms, int algorithm){
        this->delay_ms = delay_ms;
        delayInSamples = delay_ms/1000.f * sampleRate;
        
        this->algorithm = algorithm;
    }
    
    float getDelayInMS(){
        return delay_ms;
    }
    
    
    float processAudioSample(float sample){
        writeIndex++;
        writeIndex &= buffer.size()-1;
        
        buffer[writeIndex] = sample;
        
        if (algorithm==0) //Default
        {
            readIndex = writeIndex - delayInSamples;
        }
        
        else if (algorithm==1) //Reverse
        {
            readIndex = writeIndex - delayInSamples - i;
            i+=2;
            if (delayInSamples + i > buffer.size()) i=0;
        }
        
        else if (algorithm==2) //Sped up
        {
            readIndex = writeIndex*2 - delayInSamples;
            if (readIndex > buffer.size())
                readIndex-=buffer.size();
        }
        
        else if (algorithm==3) //Slowed
        {
            readIndex = writeIndex/2 - delayInSamples;
        }
        
        if (readIndex==0) readIndex=1;
        
        // wrap readIndex around buffer (not using mod because its float)
        if (readIndex<0)
            readIndex += buffer.size();
        
        if (bufferClearing) {
            // "clear buffer" buy outputing zeros until readIndex catches up
            if (int(readIndex) != indexWhereClearEnds) {
                output = 0;
            }
            else{
                bufferClearing = false;
            }
        }
        
        else
            output = getFractionalDelay(readIndex);
        
        return output;
    }
    
    float getFeedbackSample(){
        return output;
    }
    
    
private:
    
    // return an interpolated value from a fractional read index
    float getFractionalDelay(float readIndex)
    {
        int index1 = floor(readIndex);
        
        if (index1==readIndex)
            return buffer[readIndex];
        
        int index2 = ceil(readIndex);
        
        float fraction = readIndex - index1;
        
        float sample1 = buffer[index1] * (1-fraction);
        
        if (index2 == buffer.size())
            index2 = 0;
        
        float sample2 = buffer[index2] * fraction;
        return sample1 + sample2;
    }
    
    std::vector<float> buffer;
    uint32_t sampleRate;
    int32_t writeIndex;
    float readIndex;
    float delay_ms;
    float delayInSamples;
    float output = 0;
    bool bufferClearing;
    int indexWhereClearEnds;
    int algorithm;
    int i=0;
};

