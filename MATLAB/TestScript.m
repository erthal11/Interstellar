fs = 44100;     %sample rate


%% setup variables for a process block
LENGTH = 0.1;     %length of buffer in seconds
LENGTH = LENGTH + 1/fs; %add 1 more sample to see delays set to time length
CHANNELS = 2;
out_buffer1 = zeros(fs*LENGTH, CHANNELS);   %make several out buffers for the purpose of having several plots
out_buffer2 = zeros(fs*LENGTH, CHANNELS);
out_buffer3 = zeros(fs*LENGTH, CHANNELS);
out_buffer4 = zeros(fs*LENGTH, CHANNELS);
out_buffer5 = zeros(fs*LENGTH, CHANNELS);
input_buffer = zeros(fs*LENGTH, CHANNELS);
buff_size = length(out_buffer1);


% input audio is all 0's except for a 1 for the first sample
ir = zeros(fs*LENGTH,1);
ir(1) = 1;
input_buffer(:,1) = ir; %left
input_buffer(:,2) = ir; %right


%% make an object of effect with set parameters for each out_buffer
MAX_DELAY_MS = 2000;

delayTime1 = 100;
feedbackAmount1 = 0;
warpAmount1 = 0;
densityAmount1 = 0;
lpfFreq1 = 20000;
hpfFreq1 = 20;
modFreq1 = 0;
modDepth1 = 0;

delayTime2 = 10;
feedbackAmount2 = 0.7;

warpAmount3 = 1;

warpAmount4 = 0.7;

densityAmount5 = 1;

effect1 = EffectMaster(fs, MAX_DELAY_MS);
effect1 = effect1.reset(delayTime1, warpAmount1, modFreq1);
effect1 = effect1.setParameters(delayTime1, feedbackAmount1, warpAmount1, densityAmount1, lpfFreq1, hpfFreq1, modFreq1, modDepth1);

effect2 = EffectMaster(fs, MAX_DELAY_MS);
effect2 = effect2.reset(delayTime2, warpAmount1, modFreq1);
effect2 = effect2.setParameters(delayTime2, feedbackAmount2, warpAmount1, densityAmount1, lpfFreq1, hpfFreq1, modFreq1, modDepth1);

effect3 = EffectMaster(fs, MAX_DELAY_MS);
effect3 = effect3.reset(delayTime1, warpAmount3, modFreq1);
effect3 = effect3.setParameters(delayTime1, feedbackAmount1, warpAmount3, densityAmount1, lpfFreq1, hpfFreq1, modFreq1, modDepth1);

effect4 = EffectMaster(fs, MAX_DELAY_MS);
effect4 = effect4.reset(delayTime1, warpAmount4, modFreq1);
effect4 = effect4.setParameters(delayTime1, feedbackAmount1, warpAmount4, densityAmount1, lpfFreq1, hpfFreq1, modFreq1, modDepth1);

effect5 = EffectMaster(fs, MAX_DELAY_MS);
effect5 = effect5.reset(delayTime2, warpAmount3, modFreq1);
effect5 = effect5.setParameters(delayTime2, feedbackAmount2, warpAmount3, densityAmount5, lpfFreq1, hpfFreq1, modFreq1, modDepth1);


%% Process block
for sample = 1:buff_size
    for channel = 1:CHANNELS
    
        input_sample = input_buffer(sample, channel);
    
        [effect1, out_buffer1(sample, channel)] = effect1.processAudioSample(input_sample, channel);
        
        [effect2, out_buffer2(sample, channel)] = effect2.processAudioSample(input_sample, channel);
        
        [effect3, out_buffer3(sample, channel)] = effect3.processAudioSample(input_sample, channel);
        
        [effect4, out_buffer4(sample, channel)] = effect4.processAudioSample(input_sample, channel);
        
        [effect5, out_buffer5(sample, channel)] = effect5.processAudioSample(input_sample, channel);
    
    end
end


%% Use objects of just the filters for plotting their FR's
lpf = LPF();
hpf = HPF();

lpf_cf = 10000; % lpf cutoff Hz
hpf_cf = 1000; % hpf cutoff Hz

lpf = lpf.updateCoeffs(fs,lpf_cf);
hpf = hpf.updateCoeffs(fs,hpf_cf);

% generate sinesweep
sweepLength = 1; % seconds
sweepLengthSamples = sweepLength*fs;
startF = 20; % Hz
endF = 20000; % Hz
[sweep, invsweepfft, sweepRate] = synthSweep(sweepLength, fs, startF, endF);

% filter the sweep
filteredSweep = zeros(1, sweepLengthSamples);
for sample = 1:sweepLengthSamples
    [lpf, filteredSweep(sample)] = lpf.processAudioSample(sweep(sample));
    [hpf, filteredSweep(sample)] = hpf.processAudioSample(filteredSweep(sample));
end

extraFilteredSweep = filteredSweep;
orders = 3;
for order = 1:orders
    for sample = 1:sweepLengthSamples
        [lpf, extraFilteredSweep(sample)] = lpf.processAudioSample(extraFilteredSweep(sample));
        [hpf, extraFilteredSweep(sample)] = hpf.processAudioSample(extraFilteredSweep(sample));
    end
end

% get filter's IR
[irLin, irNonLin] = extractIR(filteredSweep, invsweepfft);
filterIR = irLin;

[irLin, irNonLin] = extractIR(extraFilteredSweep, invsweepfft);
filterIR2 = irLin;

% fft
Npts = length(filterIR);
fft_IR = fft(filterIR,Npts);  
fft_IR = fft_IR(1:(Npts/2)); % FFT is symmetric, throw away second half

Npts = length(filterIR2);
fft_IR2 = fft(filterIR2,Npts);  
fft_IR2 = fft_IR2(1:(Npts/2)); % FFT is symmetric, throw away second half

% plot the FFT
figure; hold on;
plot(db(abs(fft_IR)));
plot(db(abs(fft_IR2)));
xlim([startF endF]);
title("Frequency Response of hpf at "+hpf_cf+"Hz and lpf at "+lpf_cf+"Hz");
legend('Cascade order: 1',"Cascade order: "+ orders)
xlabel('Frequency (Hz)');
ylabel('magnitude (db)');
hold off;

%% plots

figure;
hold on;
t = linspace(0, LENGTH, (buff_size));
plot(t, input_buffer(:,1));
plot(t, out_buffer5(:,1));
plot(t, out_buffer5(:,2));
legend("input", "ouput L", "ouput R");
xlabel('time (s)');
ylabel('amplitude');
title("10ms delay, 100% warp, 70% feedback, 100% density");

figure;
hold on;
t = linspace(0, LENGTH, (buff_size));
plot(t, input_buffer(:,1));
plot(t, out_buffer4(:,1));
plot(t, out_buffer4(:,2));
legend("input", "ouput L", "ouput R");
xlabel('time (s)');
ylabel('amplitude');
title("100ms delay, 70% warp");

figure;
hold on;
t = linspace(0, LENGTH, (buff_size));
plot(t, input_buffer(:,1));
plot(t, out_buffer3(:,1));
plot(t, out_buffer3(:,2));
legend("input", "ouput L", "ouput R");
xlabel('time (s)');
ylabel('amplitude');
title("100ms delay, 100% warp");

figure;
hold on;
t = linspace(0, LENGTH, (buff_size));
plot(t, input_buffer(:,1));
plot(t, out_buffer2(:,1));
legend("input", "ouput");
xlabel('time (s)');
ylabel('amplitude');
title("10ms delay, 70% feedback");

figure;
hold on;
t = linspace(0, LENGTH, (buff_size)); % linspace(start, end, size)
plot(t, input_buffer(:,1));
plot(t, out_buffer1(:,1));
legend("input", "ouput");
xlabel('time (s)');
ylabel('amplitude');
title("100ms delay");