%
%     ------- 
% o--|  Zs   |--------
%     -------   |     |
%               |     |
%               -     -
%              | |   | |
%           Zp | |   | | Zo
%              | |   | |
%               -     -
%               |     |
%               V     V
%
f=100e6:20e6:2400e6;
F=f';
w=F.*2*pi();

function z3 = Zparallel( z1, z2 )
   z3 = (z1.*z2) ./ (z1+z2);
end

function z3 = Zserial( z1, z2 )
   z3 = z1 + z2;
end

function z3 = Zcap( w, C )
   z3 = -j * 1 ./ (w * C);
end

function z3 = Zind( w, L )
   z3 = j * w * L;
end

Zo = 75;
Zp = Zcap( w, 1.5e-12 );
Zs = Zind( w, 5.6e-9 );
Zt = Zserial( Zs, Zparallel(Zp,Zo) );
S11= (Zt - 50)./ (Zt+50);

figure(1);
plot( F, abs(Zt) );

figure(2);
plot( F, 20*log10( abs(S11) ) );

figure(3);
plot( F, abs(S11) );
