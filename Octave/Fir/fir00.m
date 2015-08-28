f0 = 0.80e6/5e6;   % norm. frequency
b = fir1(10, f0);  % FIR filter coefficients
freqz(b);          % Plot Frequency Response
