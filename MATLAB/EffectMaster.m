%%
classdef EffectMaster
    
    properties
        fs = 44100;
        CHANNELS = 2;
        DELAY_LINES_PER_CHANNEL = 4;
        delays(4,2) = Delay(44100,2000);
        lfo = LFO(44100, 0, 2000);
        delaySmoother = Interpolator(44100);
        warpSmoother = Interpolator(44100);
        modFreqSmoother = Interpolator(44100);
        lpf(4,2) = LPF
        hpf(4,2) = HPF
        delay_ms
        warpAmount
        crossFeedbackSampleFor = zeros(4,2)
        crossFeedbackAmount
        feedbackAmount;
    end
    
    methods
        
        function obj = EffectMaster(sampleRate, maxDelay_ms)
            
            obj.fs = sampleRate;
            
            % initialize every delay line
            for delay = 1:obj.DELAY_LINES_PER_CHANNEL
                for channel = 1:obj.CHANNELS
                    obj.delays(delay, channel) = Delay(obj.fs, maxDelay_ms);
                end
            end
            
            obj.delaySmoother = Interpolator(obj.fs);
            obj.warpSmoother = Interpolator(obj.fs);
            obj.modFreqSmoother = Interpolator(obj.fs);
            
            obj.lfo = LFO(obj.fs, 0, maxDelay_ms);
            
        end
        
        function obj = reset(obj, delay_ms, warpAmount, modFreq)
            obj.lfo = obj.lfo.reset();
            obj.delaySmoother = obj.delaySmoother.reset(18000, delay_ms, 0.1);
            obj.warpSmoother = obj.warpSmoother.reset(18000, warpAmount, 0.01);
            obj.modFreqSmoother = obj.modFreqSmoother.reset(30000, modFreq, 0.0001);
            
        end
        
        function obj = setParameters(obj, delay_ms, feedbackAmount, warpAmount, crossFeedbackAmount, lpfFreq, hpfFreq, modFreq, modDepth)
            
            obj.crossFeedbackAmount = crossFeedbackAmount*0.485;  %0.485 = max coefficient for delay matrix before reverb bcomes unstable
            obj.feedbackAmount = feedbackAmount;
            
%             obj.delaySmoother = obj.delaySmoother.setTargetValue(delay_ms);
%             [obj.delaySmoother, delay_ms] = obj.delaySmoother.getNextValue();
            
%             obj.modFreqSmoother = obj.modFreqSmoother.setTargetValue(modFreq);
%             [obj.modFreqSmoother, modFreq] = obj.modFreqSmoother.getNextValue();
            obj.lfo = obj.lfo.setParameters(modFreq, modDepth);
            
%             obj.warpSmoother = obj.warpSmoother.setTargetValue(warpAmount);
%             [obj.warpSmoother, warpAmount] = obj.warpSmoother.getNextValue();
            obj.warpAmount = warpAmount;
            obj.delay_ms = delay_ms;
            
            % warpDelayValues
            newDelayTime = 0;
            % for each delay line, make the following delay value less than the previous
            for delayLine = 1:obj.DELAY_LINES_PER_CHANNEL
                for channel = 1:obj.CHANNELS
                    
                    % first delayline will always have time set to delayValue, and only lfo mod once/sample
                    if (channel == 1 && delayLine == 1)
                        newDelayTime = delay_ms;
                        obj.lfo = obj.lfo.update();
                    else
                        % 2.83 was picked after analyzing supermassive's spread in delaylines. Warp brings them closer/further away, and is cubed for more drastic effect
                        newDelayTime = newDelayTime - (newDelayTime/2.83*(warpAmount^3));
                    end
                    
                    % cross-feedback samples only come from 1st 2 delaylines to others
                    if (delayLine~=1)
                        obj.crossFeedbackSampleFor(delayLine, channel) = obj.delays(1, 1).getFeedbackSample() + obj.delays(1, 2).getFeedbackSample();
                    end
                    
                    % set warped delay times to each delay line after  modulating
                    newDelayTimeWithLFO = obj.lfo.getNextVal(newDelayTime);
                    obj.delays(delayLine, channel) = obj.delays(delayLine, channel).setParameters(newDelayTimeWithLFO);
                    
                end
            end
            
            % updateLPFs
            for delay = 1:obj.DELAY_LINES_PER_CHANNEL
                for channel = 1:obj.CHANNELS
                    obj.lpf(delay, channel) = obj.lpf(delay, channel).updateCoeffs(lpfFreq, obj.fs);
                end
            end
            
            % updateHPFs
            for delay = 1:obj.DELAY_LINES_PER_CHANNEL
                for channel = 1:obj.CHANNELS
                    obj.hpf(delay, channel) = obj.hpf(delay, channel).updateCoeffs(hpfFreq, obj.fs);
                end
            end
            
        end
        
        function [obj,output] = processAudioSample(obj, sample, channel)
            
            output = 0;
            for delay = 1:obj.DELAY_LINES_PER_CHANNEL
                
                if obj.warpAmount == 0 | obj.delay_ms == 0
                    if (delay ~=1)
                        sample = 0;  % only have output from 1 delayline so signal isnt multiplied (TODO: fade instead of hardset to 0)
                    end
                end
                
                % add feedback + crossfeedback to input
                input = sample + obj.delays(delay, channel).getFeedbackSample() * obj.feedbackAmount + obj.crossFeedbackSampleFor(delay, channel)*obj.crossFeedbackAmount;
                 
%                 %filter input
%                 [obj.lpf(delay, channel), input] = obj.lpf(delay, channel).processAudioSample(input);
%                 [obj.hpf(delay, channel), input] = obj.hpf(delay, channel).processAudioSample(input);
                
                %process input for each delayline and sum to output
                [obj.delays(delay, channel), delayOut] = obj.delays(delay, channel).processAudioSample(input);
                output = output + delayOut;
            end
            
            %limit at 1
            if (output >=1)
                output = 1;
            else
                if (output <=-1)
                    output = -1;
                end
            end
            
        end
    end
end

