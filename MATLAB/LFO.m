%%
classdef LFO
    
    properties
        sampleRate
        frequency
        sample
        amplitude
        t = 0;
        minVal;
        maxVal;
    end
    
    methods
        
        function obj = LFO(sampleRate, minVal, maxVal)
            obj.sampleRate = sampleRate;
            obj.minVal = minVal;
            obj.maxVal = maxVal;
        end
        
        function obj = setParameters(obj, frequency, amplitude)
            obj.frequency = frequency;
            obj.amplitude = amplitude;
        end
        
        function obj = reset(obj)
            obj.t = 0;
        end
        
        function obj = update(obj)
            obj.sample = obj.amplitude*sin(2*pi*obj.frequency*(obj.t)/obj.sampleRate);
        end
        
        function output = getNextVal(obj, input)
            output = input + obj.sample;
            output = max(obj.minVal, min(obj.maxVal, output));
        end
        
        
        
    end
end

