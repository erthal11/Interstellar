/*
  ==============================================================================

    LPF.h
    Created: 28 Sep 2022 10:40:21pm
    Author:  Nathan Erthal

  ==============================================================================
*/

// APF lpf filter

#pragma once
 
#include <iostream>
#include <cmath>
 
using namespace std;
 
class LPF {
public:
 
      LPF() = default;
      ~LPF() = default;
 
      LPF(const LPF& lpf) = default;
    
    void updateCoeffs(int fc, int fs){
        wc = 2*float(fc)/float(fs);
        c = (tan(M_PI*wc/2)-1)/(tan(M_PI*wc/2)+1);
    }
 
    float processAudioSample(float x)
      {
          xh_new = x - c*xh;
          ap_y = c* xh_new + xh;
          xh = xh_new;
          return 0.5*(x+ap_y);
      }
 
 
private:
    
    float xh_new;
    float ap_y;
    float xh=0;
    float c;
    float wc;
};

