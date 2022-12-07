%%
classdef Interpolator
    
    properties
        targetValue;
        currentValue;
        sampleRate;
        logBase;
        threshold;
        step;
        timeInSamples;
        stepSize;
    end
    
    methods
        
        function obj = Interpolator(sampleRate)
            obj.sampleRate = sampleRate;
        end
        
        function obj = reset(obj, logBase, currentValue, threshold)
            obj.logBase = logBase;
            obj.threshold = threshold;
            obj.currentValue = currentValue;
            obj.targetValue = obj.currentValue;
        end
        
        function obj = setTargetValue(obj, targetValue)
            obj.targetValue = targetValue;
        end
        
        function [obj, currentValue] = getNextValue(obj)
            obj.stepSize = obj.targetValue - obj.currentValue;
        
            if (abs(obj.stepSize) <= obj.threshold)
                obj.currentValue = obj.targetValue;
            end
            obj.stepSize = obj.stepSize / obj.logBase;
            obj.currentValue = + obj.currentValue +obj.stepSize;
            currentValue = obj.currentValue;
        end
        
    end
end

