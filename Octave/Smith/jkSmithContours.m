%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function jkSmithContours()

   N=100;
   text_delta=0.03;

   X=linspace( 0+1.0i, 10+1.0i ,N);
   G=(X-1)./(X+1);
   plot(G,"k");
   text( real(G(1)) - text_delta , imag(G(1)) + text_delta, '+1i');

   X=linspace( 0-1.0i, 10-1.0i ,N);
   G=(X-1)./(X+1);
   plot(G,"k");
   text( real(G(1)) - text_delta , imag(G(1)) - text_delta, '-1i');

   X=linspace( 0+0.3i, 10+0.3i ,N);
   G=(X-1)./(X+1);
   plot(G,"k");
   text( real(G(1)) - 4*text_delta, imag(G(1)), '+0.3i');

   X=linspace( 0-0.3i, 10-0.3i ,N);
   G=(X-1)./(X+1);
   plot(G,"k");
   text( real(G(1)) - 4*text_delta, imag(G(1)), '-0.3i');

   X=linspace( 0+3.0i, 10+3.0i ,N);
   G=(X-1)./(X+1);
   plot(G,"k");
   text( real(G(1)), imag(G(1)), '+3i');

   X=linspace( 0-3.0i, 10-3.0i ,N);
   G=(X-1)./(X+1);
   plot(G,"k");
   text( real(G(1)), imag(G(1)), '-3i');

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   R=10;
   Z=50i;
   N=300;

   R=0.3;
   X=linspace( R-Z, R+0i ,2*N);
   G=(X-1)./(X+1);
   plot(G,"k");

   X=linspace( R-0i, R+Z ,2*N);
   G=(X-1)./(X+1);
   plot(G,"k");
   text( real(G(1)) + text_delta, imag(G(1)), '0.3');

   R=1.0;
   X=linspace( R-Z, R+0.0i ,N);
   G=(X-1)./(X+1);
   plot(G,"k");

   X=linspace( R-0.0i, R+Z ,N);
   G=(X-1)./(X+1);
   plot(G,"k");
   text( real(G(1)) + text_delta, imag(G(1)), '1');

   R=3;
   X=linspace( R-Z, R+0i ,N);
   G=(X-1)./(X+1);
   plot(G,"k");

   X=linspace( R-0i, R+Z ,N);
   G=(X-1)./(X+1);
   plot(G,"k");
   text( real(G(1)) + text_delta, imag(G(1)), '3');

   R=10;
   X=linspace( R-Z, R+0.0i ,N);
   G=(X-1)./(X+1);
   plot(G,"k");

   X=linspace( R-0.0i, R+Z ,N);
   G=(X-1)./(X+1);
   plot(G,"k");
   text( real(G(1)) + text_delta, imag(G(1)), '10');
end

