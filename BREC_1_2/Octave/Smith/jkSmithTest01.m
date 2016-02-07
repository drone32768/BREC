%
% This test creates a smith chare of a 205 ohm resistor shunted with a 1.8pF
% capacitor.
%
close all;

f =10e6:50e6:3000e6;
F =f';
w =F.*2*pi();
Z1=1./(j*w*1.8e-12);
Z2=(205.*Z1) ./ (205+Z1);

figure(2);

jkSmith();
jkSmithContours(2);
title( 'R=205||C=1.8pF' );
G2=z2gamma(Z2);

color="blue";
plt1 = plot( G2, "color", color );
jkSmithMarker(            G2(1), "10MHz", color );
jkSmithMarker(   G2(length(G2)), "3GHz", color );
jkSmithMarker( G2(length(G2)/2), "Z2", color );

print -dpng fig1.png
