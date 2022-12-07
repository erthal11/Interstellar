%%
classdef Delay
    
    properties
        fs;
        delay_samples;
        buffer_size;
        buffer;
        writeIndex = 1;
        readIndex = 1;
        out = 0;
    end
    
    methods
        
        function obj = Delay(sampleRate, maxDelay_ms)
            obj.fs = sampleRate;
            obj.buffer_size = obj.fs * maxDelay_ms/1000;
            obj.buffer = zeros(1, obj.buffer_size);
        end
        
        function obj = setParameters(obj, delay_ms)
            obj.delay_samples = delay_ms /1000 * obj.fs;
        end
        
        function [obj,sample] = processAudioSample(obj, sample)
            
          obj.writeIndex = obj.writeIndex + 1;
            
          % Wrap write index around buffer
          if obj.writeIndex > obj.buffer_size
            obj.writeIndex = obj.writeIndex - obj.buffer_size;
          end

          % Set the new read index according to delay time
          obj.readIndex = obj.writeIndex - obj.delay_samples;
          
          % Wrap read index around buffer
          if (obj.readIndex < 1)
              obj.readIndex = obj.readIndex + obj.buffer_size;
          end
          
          % Write new sample to new write index
          obj.buffer(obj.writeIndex) = sample;
          
%           % Return sample at a fractional read index
%           index1 = floor(obj.readIndex);
%           index2 = ceil(obj.readIndex);
%           fraction = obj.readIndex - index1;
%           sample1 = obj.buffer(index1)*(1-fraction);
%           if index2 > obj.buffer_size
%             index2 = 1;
%           end
%           sample2 = obj.buffer(index2)*fraction;
%           sample = sample1 + sample2;

          % Return sample at floored read index
          sample = obj.buffer(floor(obj.readIndex));
          obj.out = sample;
           
        end
        
        
        function out = getFeedbackSample(obj)
            out = obj.out;
        end
        
    end
end

