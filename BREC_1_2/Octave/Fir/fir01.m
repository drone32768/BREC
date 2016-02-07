% Calculate the filter
fc = 10e6/2;
f0 = 0.05e6/fc;   % norm. frequency
b = fir1(100, f0);  % FIR filter coefficients

%freqz(b);          % Plot Frequency Response

h  = freqz(b);         % Plot Frequency Response
nf = ( 0:pi/(length(h)-1):pi )';
f  = nf*fc/pi;

plot( f, 20*log10( abs(h) ) );
xlabel( "MHz" );
ylabel( "dB" );
title( "FIR filter response" );
legend( sprintf("order=%d", length(b)-1) );
axis( [0,fc,-60,1] );
grid on;

% saveas seems to be coming up unresolved, but is in documenation
%saveas( 1, "plot1.png", "png" ); % Save figure 1 as png
print -dpng fig1.png;

