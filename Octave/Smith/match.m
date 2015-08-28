
% LTC data sheet values
Gdev=[
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
SmagdegToGamma(0.715, -116),
SmagdegToGamma(0.721, -119.9),
SmagdegToGamma(0.739, -134.1),
SmagdegToGamma(0.756, -149.6),
];

Fdev=[
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
2000e6, 
2100e6, 
2500e6, 
3000e6, 
]

% Setup freqency span
f=10e6:50e6:3000e6;
F=f';
w=F.*2*pi();

% Create a datasheet S11 with interpolated points
Gdatasheet=interp1(Fdev,Gdev,F);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function z3 = jkShunt( z1, z2 )
   z3 = (z1.*z2) ./ (z1+z2);
end

function z3 = jkSerial( z1, z2 )
   z3 = z1 + z2;
end

function z3 = jkZc( w, C )
   z3 = -j * 1 ./ (w *C);
end

function z3 = jkZl( w, L )
   z3 = j * w *L;
end

% Calculate the device input impedance based on model
Zmodel=jkShunt( 205, jkZc(w, 1.6e-12) );
Gmodel=z2gamma(Zmodel);

Zdev=gamma2z( Gdatasheet );
Z1=jkShunt ( Zdev, 453 );
Z2=jkSerial( jkZl(w,2.2e-9), jkShunt( Zdev, 453 ) );
Z3=jkSerial( jkZl(w,2.2e-9), jkShunt( Zdev, 68 ) );
Z4=jkShunt ( jkZc(w,1.8e-12), jkSerial( jkZl(w,3.3e-9), jkShunt( Zdev, 68 ) ) );

G4=z2gamma(Z4);
G3=z2gamma(Z3);
G2=z2gamma(Z2);
G1=z2gamma(Z1);


close all;

figure(1);
hold on;
grid on;
xlabel("F(Hz)");
ylabel(" 20log10( |S11| )");

plot( F, 20*log10( abs(Gdatasheet)), 'c'  );
plot( F, 20*log10( abs(G4)), 'b'  );
plot( F, 20*log10( abs(G3)), 'g'  );
%plot( F, 20*log10( abs(G2)), 'r'  );
%plot( F, 20*log10( abs(G1)), 'm'  );
legend("None", "Z4=68,3.3nH,1.8pF", "Z3=68,2.2nH", "Z2", "Z1", "DevData", "location", "southeast" );
title( "Various matching networks" );
print -dpng fig1.png

