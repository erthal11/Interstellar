/*
  ==============================================================================

    EffectMaster.h
    Created: 30 Sep 2022 9:31:24pm
    Author:  Nathan Erthal

  ==============================================================================
*/

#pragma once
 
#include <iostream>
#include <cmath>
#include <vector>

#include "DelayLine.h"
#include "Interpolatorinator.h"
#include "LFO.h"
#include "LPF.h"
#include "HPF.h"
#include "APF.h"

#define NUM_CHANNELS 2
#define DELAY_LINES_PER_CHANNEL 8
#define APFs_PER_CHANNEL 3
 
using namespace std;
 
class EffectMaster {
public:
 
    EffectMaster() = default;
    ~EffectMaster() = default;
 
    EffectMaster(const EffectMaster& effectMaster) = default;
    
    //initialize each delayline, filters and lfo
    void setMaxDelay(const double sampleRate, const int maxDelay_ms)
    {
        this->sampleRate=sampleRate;

        for (int delay = 0; delay < DELAY_LINES_PER_CHANNEL; delay++)
            for (int channel = 0; channel<NUM_CHANNELS; channel++)
            {
                delays[delay][channel].setMaxDelay(sampleRate, maxDelay_ms);
            }
        
        for (int channel = 0; channel<NUM_CHANNELS; channel++)
        {
            allPass[0][channel].setup(sampleRate, 347);
            allPass[1][channel].setup(sampleRate, 113);
            allPass[2][channel].setup(sampleRate, 37);
        }
        
        updateLPFs(lpfFreq, sampleRate);
        updateHPFs(hpfFreq, sampleRate);
        
        lfo.prepare(sampleRate, 0, maxDelay_ms);
        
        loadFeedbackMatrix();
    }
    
    void reset(float delay_ms, float warpAmount, float modFreq)
    {
        clearBuffer();
        
        lfo.reset();
        this->delay_ms = delay_ms;
        delaySmoother.reset(18000, delay_ms, 0.1);
        
        this->warpAmount = warpAmount;
        warpSmoother.reset(18000, warpAmount, 0.0001);
        
        warpDelayValues(delay_ms, warpAmount);
        
        modFreqSmoother.reset(30000, modFreq, 0.0001);
//        modFreqSmoother.reset(sampleRate, 0.1);
        
        modDepthSmoother.reset(sampleRate, 0.4);
        
        lpfSmoother.reset(sampleRate, 0.2);
        hpfSmoother.reset(sampleRate, 0.2);
        
        wetSmoother.reset(sampleRate, 0.1);
    }
    
    void clearBuffer()
    {
        for (int delay = 0; delay < DELAY_LINES_PER_CHANNEL; delay++)
            for (int channel = 0; channel<NUM_CHANNELS; channel++)
                delays[delay][channel].clearBuffer();
        
        for (int apf = 0; apf < APFs_PER_CHANNEL; apf ++)
            for (int channel = 0; channel<NUM_CHANNELS; channel++)
                allPass[apf][channel].clearBuffer();
    }
    
    void setParameters(float delay_ms, float feedbackAmount, float warpAmount, float densityAmount, float diffusionAmount, float lpfFreq, float hpfFreq, float modFreq, float modDepth, float wet_db, float dry_db, int algorithm)
    {
        this->densityAmount = densityAmount;
        this->feedbackAmount = feedbackAmount;
        this->diffusionAmount = diffusionAmount;
        this->algorithm = algorithm;
        
        if (this->modFreq != modFreq || this->modDepth != modDepth)
        {
            modFreqSmoother.setTargetValue(modFreq);
            modFreq = modFreqSmoother.getNextValue();
            modDepthSmoother.setTargetValue(modDepth);
            modDepth = modDepthSmoother.getNextValue();
            
            lfo.setParameters(modFreq, modDepth);
        }
        
        if (this->warpAmount != warpAmount || this->delay_ms != delay_ms)
        {
            warpSmoother.setTargetValue(warpAmount);
            this->warpAmount = warpSmoother.getNextValue();
            delaySmoother.setTargetValue(delay_ms);
            this->delay_ms = delaySmoother.getNextValue();
            
            warpDelayValues(this->delay_ms, this->warpAmount);
        }
        
        if (lpfFreq != this->lpfFreq)
        {
            lpfSmoother.setTargetValue(lpfFreq);
            this->lpfFreq = lpfSmoother.getNextValue();
            
            updateLPFs(this->lpfFreq, sampleRate);
        }
        
        if (hpfFreq != this->hpfFreq)
        {
            hpfSmoother.setTargetValue(hpfFreq);
            this->hpfFreq = hpfSmoother.getNextValue();
            
            updateHPFs(this->hpfFreq, sampleRate);
        }
        
        if (hpfFreq != this->hpfFreq)
        {
            hpfSmoother.setTargetValue(hpfFreq);
            this->hpfFreq = hpfSmoother.getNextValue();
            
            updateHPFs(this->hpfFreq, sampleRate);
        }
        
        if (this->wet_db != wet_db)
        {
            wetSmoother.setTargetValue(wet_db);
            this->wet_db = wetSmoother.getNextValue();
            this->wetVolume = dbToLinear(this->wet_db);
        }
        
        if (this->dry_db != dry_db)
        {
            drySmoother.setTargetValue(dry_db);
            this->dry_db = drySmoother.getNextValue();
            this->dryVolume = dbToLinear(this->dry_db);
        }
        
    }
    
    
    float processAudioSample(float sample, int channel)
    {
        // lfo updates once/sample
        if (channel==0)
            lfo.update();
                
        wetOut = 0;
        dryOut = sample;
        
        // all-pass filtering, diffusionAmount is feedback coeff for each APF
        for (int i = 0; i<APFs_PER_CHANNEL; i++)
            sample = allPass[i][channel].processAudioSample(sample, diffusionAmount*0.7);
        // diffusion amount also serves as mix value for APF effect
        sample = dryOut + sample*diffusionAmount;
        
        for (int delayLine = 0; delayLine < DELAY_LINES_PER_CHANNEL; delayLine++)
        {
            // only have output from 1 delayline so signal isnt multiplied (TODO: fade instead of hardset to 0)
            if (warpAmount <0.05 || delay_ms == 0)
                if (delayLine !=0) sample = 0;
            
            // update the delaytime param to it's value with modulation
            float delayValue = lfo.getNextVal(delayValues[delayLine][channel]);
            delays[delayLine][channel].setParameters(delayValue, algorithm);
            
//            // density (cross-feedback) samples only come from 2 delaylines to others
//            if (delayLine!=1)
//                densitySample = delays[1][0].getFeedbackSample() + delays[1][1].getFeedbackSample();
//            densitySample*=-1;
//            else densitySample = 0;
            
            // density (cross-feedback): set the feedback and density sample for this delayline depending on the feedback coefficient matrix
            densitySample=0;
            for (int i = 0; i < DELAY_LINES_PER_CHANNEL; i++)
                for (int j = 0; j < NUM_CHANNELS; j++) {
                    if (i==delayLine && j == channel)
                        feedbackSample = delays[i][j].getFeedbackSample() * feedbackMatrix[i][j][delayLine][channel];
                    else
                        densitySample += delays[i][j].getFeedbackSample() * feedbackMatrix[i][j][delayLine][channel];
                }
            
            // add feedback + density to input
            float input = sample + feedbackSample*feedbackAmount + densitySample*densityAmount;
            
            // filter input
            input = lowPass[delayLine][channel].processAudioSample(input);
            input = highPass[delayLine][channel].processAudioSample(input);
            
            // process input for each delayline and sum to output
            wetOut += delays[delayLine][channel].processAudioSample(input);
        }
        
        // mix dry + wet
        return (wetOut * wetVolume) + (dryOut * dryVolume);
    }
    
 
private:
    
    void updateLPFs(int freq, int sampleRate){
        for (int delay = 0; delay < DELAY_LINES_PER_CHANNEL; delay++)
            for (int channel = 0; channel<2; channel++)
                lowPass[delay][channel].updateCoeffs(freq, sampleRate);
    }
    
    void updateHPFs(int freq, int sampleRate){
        for (int delay = 0; delay < DELAY_LINES_PER_CHANNEL; delay++)
            for (int channel = 0; channel<NUM_CHANNELS; channel++)
                highPass[delay][channel].updateCoeffs(freq, sampleRate);
    }
    
    void warpDelayValues(float delayValue, float warpValue) {
        
        float newDelayValue;
        
        for (int delayLine = 0; delayLine < DELAY_LINES_PER_CHANNEL; delayLine++) {
            for (int channel = 0; channel < NUM_CHANNELS; channel++) {
                
                // first delayline will always have time set to delayValue
                if (channel == 0 && delayLine == 0)
                    newDelayValue = delayValue;
                
                else // make the following delay value less than the previous by subtracting a fraction of 1/2.83 from the previous
                    newDelayValue = newDelayValue - (newDelayValue/2.83)*(pow(warpValue,3));
                // 2.83 was picked after analyzing supermassive's spread in delaylines.
                // Warp size brings them closer/further away, and is cubed for more drastic effect
                
                delayValues[delayLine][channel] = newDelayValue;
            }
        }
    }
    
    float dbToLinear(float val)
    {
        return pow(10.0, (val / 20.0));
    }
    
    DelayLine delays[DELAY_LINES_PER_CHANNEL][NUM_CHANNELS];
    LPF lowPass[DELAY_LINES_PER_CHANNEL][NUM_CHANNELS];
    HPF highPass[DELAY_LINES_PER_CHANNEL][NUM_CHANNELS];
    APF allPass[APFs_PER_CHANNEL][NUM_CHANNELS];
    LFO lfo;
    Interpolatorinator delaySmoother;
    Interpolatorinator warpSmoother;
    Interpolatorinator modFreqSmoother;
    juce::LinearSmoothedValue<float> modDepthSmoother;
    juce::LinearSmoothedValue<float> lpfSmoother;
    juce::LinearSmoothedValue<float> hpfSmoother;
    juce::LinearSmoothedValue<float> wetSmoother;
    juce::LinearSmoothedValue<float> drySmoother;
    float wetOut;
    float dryOut;
    float feedbackAmount;
    float densityAmount;
    float diffusionAmount;
    float delayValues[DELAY_LINES_PER_CHANNEL][NUM_CHANNELS];
    float warpAmount;
    float delay_ms;
    float lpfFreq;
    float hpfFreq;
    int sampleRate;
    float dryVolume;
    float wetVolume;
    float wet_db;
    float dry_db;
    float densitySample;
    float feedbackSample;
    float modFreq;
    float modDepth;
    int algorithm;
    
    float feedbackMatrix[DELAY_LINES_PER_CHANNEL][NUM_CHANNELS][DELAY_LINES_PER_CHANNEL][NUM_CHANNELS];
    
    
    void loadFeedbackMatrix(){
        
        for (int fdelay = 0; fdelay < DELAY_LINES_PER_CHANNEL; fdelay++)
            for (int fchannel = 0; fchannel<NUM_CHANNELS; fchannel++)
                for (int tdelay = 0; tdelay < DELAY_LINES_PER_CHANNEL; tdelay++)
                    for (int tchannel = 0; tchannel<NUM_CHANNELS; tchannel++)
                    {
                        feedbackMatrix[fdelay][fchannel][tdelay][tchannel]=0;
                    }
        
        // Schroeder Reverberator feedback matrix for 16 delaylines

        feedbackMatrix[0][0][0][0]= 1;
        feedbackMatrix[0][0][4][0]= 0.510915870982404;
        feedbackMatrix[0][0][4][1]= -0.435753337685657;
        feedbackMatrix[0][0][5][0]= 0.163403416765558;
        feedbackMatrix[0][0][5][1]= -0.198975357273277;
        feedbackMatrix[0][0][6][0]= 0.112664805605849;
        feedbackMatrix[0][0][6][1]= -0.0801054761233991;
        feedbackMatrix[0][0][7][0]= 0.0580984307570677;
        feedbackMatrix[0][0][7][1]= -0.0376694821418418;

        feedbackMatrix[0][1][0][1]= 1;
        feedbackMatrix[0][1][4][0]= 0.510915870982404;
        feedbackMatrix[0][1][4][1]= -0.435753337685657;
        feedbackMatrix[0][1][5][0]= 0.163403416765558;
        feedbackMatrix[0][1][5][1]= -0.198975357273277;
        feedbackMatrix[0][1][6][0]= 0.112664805605849;
        feedbackMatrix[0][1][6][1]= -0.0801054761233991;
        feedbackMatrix[0][1][7][0]= 0.0580984307570677;
        feedbackMatrix[0][1][7][1]= -0.0376694821418418;

        feedbackMatrix[1][0][1][0]= 1;
        feedbackMatrix[1][0][4][0]= 0.510915870982404;
        feedbackMatrix[1][0][4][1]= -0.435753337685657;
        feedbackMatrix[1][0][5][0]= 0.163403416765558;
        feedbackMatrix[1][0][5][1]= -0.198975357273277;
        feedbackMatrix[1][0][6][0]= 0.112664805605849;
        feedbackMatrix[1][0][6][1]= -0.0801054761233991;
        feedbackMatrix[1][0][7][0]= 0.0580984307570677;
        feedbackMatrix[1][0][7][1]= -0.0376694821418418;

        feedbackMatrix[1][1][1][1]= 1;
        feedbackMatrix[1][1][4][0]= 0.510915870982404;
        feedbackMatrix[1][1][4][1]= -0.435753337685657;
        feedbackMatrix[1][1][5][0]= 0.163403416765558;
        feedbackMatrix[1][1][5][1]= -0.198975357273277;
        feedbackMatrix[1][1][6][0]= 0.112664805605849;
        feedbackMatrix[1][1][6][1]= -0.0801054761233991;
        feedbackMatrix[1][1][7][0]= 0.0580984307570677;
        feedbackMatrix[1][1][7][1]= -0.0376694821418418;

        feedbackMatrix[2][0][2][0]= 1;
        feedbackMatrix[2][0][4][0]= 0.510915870982404;
        feedbackMatrix[2][0][4][1]= -0.435753337685657;
        feedbackMatrix[2][0][5][0]= 0.163403416765558;
        feedbackMatrix[2][0][5][1]= -0.198975357273277;
        feedbackMatrix[2][0][6][0]= 0.112664805605849;
        feedbackMatrix[2][0][6][1]= -0.0801054761233991;
        feedbackMatrix[2][0][7][0]= 0.0580984307570677;
        feedbackMatrix[2][0][7][1]= -0.0376694821418418;

        feedbackMatrix[2][1][2][1]= 1;
        feedbackMatrix[2][1][4][0]= 0.510915870982404;
        feedbackMatrix[2][1][4][1]= -0.435753337685657;
        feedbackMatrix[2][1][5][0]= 0.163403416765558;
        feedbackMatrix[2][1][5][1]= -0.198975357273277;
        feedbackMatrix[2][1][6][0]= 0.112664805605849;
        feedbackMatrix[2][1][6][1]= -0.0801054761233991;
        feedbackMatrix[2][1][7][0]= 0.0580984307570677;
        feedbackMatrix[2][1][7][1]= -0.0376694821418418;

        feedbackMatrix[3][0][3][0]= 1;
        feedbackMatrix[3][0][4][0]= 0.510915870982404;
        feedbackMatrix[3][0][4][1]= -0.435753337685657;
        feedbackMatrix[3][0][5][0]= 0.163403416765558;
        feedbackMatrix[3][0][5][1]= -0.198975357273277;
        feedbackMatrix[3][0][6][0]= 0.112664805605849;
        feedbackMatrix[3][0][6][1]= -0.0801054761233991;
        feedbackMatrix[3][0][7][0]= 0.0580984307570677;
        feedbackMatrix[3][0][7][1]= -0.0376694821418418;

        feedbackMatrix[3][1][3][1]= 1;
        feedbackMatrix[3][1][4][0]= 0.510915870982404;
        feedbackMatrix[3][1][4][1]= -0.435753337685657;
        feedbackMatrix[3][1][5][0]= 0.163403416765558;
        feedbackMatrix[3][1][5][1]= -0.198975357273277;
        feedbackMatrix[3][1][6][0]= 0.112664805605849;
        feedbackMatrix[3][1][6][1]= -0.0801054761233991;
        feedbackMatrix[3][1][7][0]= 0.0580984307570677;
        feedbackMatrix[3][1][7][1]= -0.0376694821418418;

        feedbackMatrix[4][1][4][0]= 0.699345500462823;
        feedbackMatrix[4][1][4][0]= 0.623087354386749;
        feedbackMatrix[4][1][4][1]= -0.233651916910052;
        feedbackMatrix[4][1][5][0]= 0.284516533160786;
        feedbackMatrix[4][1][5][1]= -0.161100351015754;
        feedbackMatrix[4][1][6][0]= 0.114543492551801;
        feedbackMatrix[4][1][6][1]= -0.0830754336999645;
        feedbackMatrix[4][1][7][0]= 0.0538639086358780;
        feedbackMatrix[4][1][7][1]= 0.623087354386749;

        feedbackMatrix[4][0][4][1]= 0.613932118082489;
        feedbackMatrix[4][0][4][1]= 0.380582657313683;
        feedbackMatrix[4][0][5][0]= -0.463433211556717;
        feedbackMatrix[4][0][5][1]= 0.262407432793911;
        feedbackMatrix[4][0][6][0]= -0.186573546452592;
        feedbackMatrix[4][0][6][1]= 0.135316969503789;
        feedbackMatrix[4][0][7][0]= -0.0877359353736250;
        feedbackMatrix[4][0][7][1]= 0.380582657313683;

        feedbackMatrix[5][0][5][0]= 0.787030712670298;
        feedbackMatrix[5][0][5][1]= 0.588837518150144;
        feedbackMatrix[5][0][6][0]= -0.333414476169037;
        feedbackMatrix[5][0][6][1]= 0.237060058075206;
        feedbackMatrix[5][0][7][0]= -0.171933531087592;
        feedbackMatrix[5][0][7][1]= 0.111477143091339;

        feedbackMatrix[5][1][5][1]= 0.641219527034117;
        feedbackMatrix[5][1][6][0]= 0.519969311775649;
        feedbackMatrix[5][1][6][1]= -0.369701869766348;
        feedbackMatrix[5][1][7][0]= 0.268135207738994;
        feedbackMatrix[5][1][7][1]= -0.173851759641449;

        feedbackMatrix[6][0][6][0]= 0.692842469991809;
        feedbackMatrix[6][0][6][1]= 0.533601627756333;
        feedbackMatrix[6][0][7][0]= -0.387007464686978;
        feedbackMatrix[6][0][7][1]= 0.250925379391803;

        feedbackMatrix[6][1][6][1]= 0.682933651421327;
        feedbackMatrix[6][1][7][0]= 0.566683840928815;
        feedbackMatrix[6][1][7][1]= -0.367422777995454;

        feedbackMatrix[7][0][7][0]= 0.658267543686597;
        feedbackMatrix[7][0][7][1]= 0.558166328447123;

        feedbackMatrix[7][1][7][1]= 0.664705702964009;
    }
};
 


