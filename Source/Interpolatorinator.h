/*
  ==============================================================================

    Interpolatorinator.h
    Created: 22 Sep 2022 12:59:31am
    Author:  Nathan Erthal

  ==============================================================================
*/

#pragma once

#include<iostream>
#include<algorithm>
using namespace std;

class Interpolatorinator {
public:
    
    void setTargetValue(double targetValue){
        this->targetValue = targetValue;
    }
    
    float getNextValue(){
        if (currentValue == targetValue)
            return targetValue;
        
        stepSize = targetValue - currentValue;
        
        if (abs(stepSize) <= threshold){
            currentValue = targetValue;
            return targetValue;
        }
        stepSize /= logBase;
        currentValue += stepSize;
        return currentValue;
    }
    
    void reset(float logBase, double currentValue, float threshold){
        this->logBase = logBase;
        targetValue = currentValue;
        this->currentValue = currentValue;
        this->threshold = threshold;
    }
    
    
private:
    double targetValue;
    double currentValue;
    double logBase;
    
    float stepSize;
    float threshold;
};
