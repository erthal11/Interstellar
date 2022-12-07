%%
classdef HPF
    
    properties
        xh_new;
        ap_y;
        xh=0;
        c;
        wc;
    end
    
    methods
        function obj = HPF()
        end
        
        function obj = updateCoeffs(obj,fc, fs)
           obj.wc = 2*fc/fs;
           obj.c = (tan(pi*obj.wc/2)-1)/(tan(pi*obj.wc/2)+1); 
        end
        
        function [obj, sample] = processAudioSample(obj, x)
          obj.xh_new = x - obj.c*obj.xh;
          obj.ap_y = obj.c* obj.xh_new + obj.xh;
          obj.xh = obj.xh_new;
          sample = 0.5*(x-obj.ap_y);
        end
    end
end

