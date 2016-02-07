function jkSmithMarker( anGam, asText, asColor )
  plot( real(anGam), imag(anGam), 'd', 'color', asColor );
  text( real(anGam)+0.01, imag(anGam)+0.01, asText, 'color', asColor  );
end
