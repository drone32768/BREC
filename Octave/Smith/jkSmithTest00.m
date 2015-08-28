%
% The jkSmith is just a u,v plot of -1,+1 and -1i,+1i rectilinear with
% a set of impedance contours overlayed in black.  It is a normal figure
% with hold on appied to create the contours.
%

close all;

figure(1);
jkSmith();
jkSmithContours(2);

