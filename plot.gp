
set style fill transparent solid 0.5
rgb(r,g,b) = int(r)*65536 + int(g)*256 + int(b)
plot 'slicepnts.txt' u 1:2:(rgb($1,$2,$2)) w filledcurves lt -1 lc rgb variable
pause -1 "Press return to continue"