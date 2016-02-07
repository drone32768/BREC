
% LTC data sheet values
Gdatasheet=[
SmagdegToGamma(0.605, -0.7),
SmagdegToGamma(0.605, -3.7),
SmagdegToGamma(0.606, -7.3),
SmagdegToGamma(0.608, -14.6),
SmagdegToGamma(0.614, -28.9),
SmagdegToGamma(0.619, -35.8),
SmagdegToGamma(0.636, -55.6),
SmagdegToGamma(0.643, -61.8),
SmagdegToGamma(0.650, -67.7),
SmagdegToGamma(0.685, -94.3),
SmagdegToGamma(0.700, -105.3), % 1750 interp
SmagdegToGamma(0.715, -116),
SmagdegToGamma(0.721, -119.9),
SmagdegToGamma(0.739, -134.1),
SmagdegToGamma(0.756, -149.6),
];

F=[
10e6, 
50e6,  
100e6, 
200e6, 
400e6, 
500e6, 
800e6, 
900e6, 
1000e6, 
1500e6, 
1750e6,  % interp
2000e6, 
2100e6, 
2500e6, 
3000e6, 
]

% Setup freqency span
%f=10e6:50e6:2000e6;
%F=f';

% LTC5587
Rin=205;
Cin=1.6e-12;

% Terminating network
Rt=68;

%MainTitle="2.2nH, - pF";
%Lm=2.2e-9;
%Cm=0.1e-12;

MainTitle="3.3nH,1.8pF";
Lm=3.3e-9;
Cm=1.8e-12;

%Lm=2.2e-9;
%Cm=1.5e-12;

%Lm=5.7e-9;
%Cm=2.8e-12;

%Lm=7.2e-9;
%Cm=3.2e-12;

% Calculate the device input impedance based on model
w=F.*2*pi();
Zc= 1./(j*w*Cin);
Zmodel= (Rin.*Zc) ./ (Rin+Zc);

% Pick a device input impedance
%Zin=Zmodel;
Zin=gamma2z( Gdatasheet );

% Impedance with terminating resistor
Z2= (Rt.*Zin) ./ (Rt+Zin);

% Impedance with matching inductor
L1= j.*w*Lm;
Z3= Z2 + L1;

% Impedance with matching capcitor
C1= 1./(j*w*Cm);
Z4= (Z3 .* C1) ./ (Z3+C1);

close all;

figure(1);
hold on;
grid on;
ylim( [25 75] );
xlabel("F(Hz)");
ylabel("Zin(ohms)");
plot( F, abs(Z4), 'b' );
plot( F, abs(Z3), 'g' );
plot( F, abs(Z2), 'r' );
%plot( F, abs(Zin), 'm' );
legend('Z4=Shunt C', 'Z3=Series L', 'Z2=Shunt R');
title( MainTitle );
print -dpng fig1.png

ZinTable = [ F, Zin];
Z4Table  = [ F, Z4];

ZinTable;

figure(2);
jkSmith();
jkSmithContours(2);
G4=z2gamma(Z4);
G3=z2gamma(Z3);
G2=z2gamma(Z2);
Gmodel=z2gamma(Zmodel);

plot( G4, "color", "blue") 
jkSmithMarker( G4(int32(length(G4))), "Shunt C", "blue" );

plot( G3, "color", "green") 
jkSmithMarker( G3(int32(length(G3))), "Series L", "green" );

plot( G2, "color", "red") 
jkSmithMarker( G2(int32(length(G2))), "Shunt R", "red" );

plot( Gmodel, "color", "magenta") 
jkSmithMarker( Gmodel(int32(length(Gmodel))), "Device Model", "magenta" );

plot( Gdatasheet, "color", "cyan") 
jkSmithMarker( Gdatasheet(int32(length(Gdatasheet))), "Device Datasheet", "cyan" );

title( MainTitle );
print -dpng fig2.png

figure(3);
hold on;
grid on;
%ylim( [25 75] );
xlabel("F(Hz)");
ylabel("Return Loss(dB)");
plot( F, 20*log10( abs(G4)), 'b'  );
plot( F, 20*log10( abs(G3)), 'g'  );
plot( F, 20*log10( abs(G2)), 'r'  );
plot( F, 20*log10( abs(Gmodel)), 'm'  );
plot( F, 20*log10( abs(Gdatasheet)), 'c'  );
legend("Z4=Shunt C", "Z3=Series L", "Z2=Shunt R", "DevModel", "DevData", "location", "southeast" );
title( MainTitle );
print -dpng fig3.png

