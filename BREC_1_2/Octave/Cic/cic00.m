%close all;

N  = 1;  % order of filter
M  = 1;  % differential delay 1..2
R  = 4;  % decimation

f = linspace(1e-5,1,512);
fhz = 10e6 .* f ./ 2;

M = 1;
H = abs( sin( pi*M*f) ./ sin(pi*f/R) ) .^N;

M = 2;
HH = abs( sin( pi*M*f) ./ sin(pi*f/R) ) .^N;

M = 3;
HHH = abs( sin( pi*M*f) ./ sin(pi*f/R) ) .^N;


figure(1);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
subplot( 2, 1, 1);
plot( f, 10*log10( H ), f, 10*log10(HH), f, 10*log10(HHH) );
axis( [0, 1, -40, 40 ] );

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
subplot( 2, 1, 2);
plot( f, 10*log10( H ), f, 10*log10(HH), f, 10*log10(HHH)  );
axis( [0, 1/R, 0, 40 ] );

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
figure(2);
subplot( 2, 1, 1);
plot( fhz, 10*log10( H ), fhz, 10*log10(HH), fhz, 10*log10(HHH)  );
axis( [0, 10e6/2, -40, 40 ] );

subplot( 2, 1, 2);
plot( fhz, 10*log10( H ), fhz, 10*log10(HH), fhz, 10*log10(HHH)  );
axis( [0, (10e6/2)/R, 0, 40 ] );

