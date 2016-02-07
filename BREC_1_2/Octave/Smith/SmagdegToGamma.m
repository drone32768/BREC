function Z = SmagdegToGamma( R, angDeg )
angRad=pi*angDeg/180.0;
Z= R * exp( j* angRad );
end
