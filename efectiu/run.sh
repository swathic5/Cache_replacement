#!/bin/bash
product=1
i=0
head="Trace"
printf "%-40s\t" $head;
printf "**********LRU***********|*********MY POLICY*********|*******SPEEDUP*******\n";
for entry in "traces"/*
do
  export DAN_POLICY=0
  result=$(./efectiu ./$entry);
  y1=(${result//$'\n'/ })
  export DAN_POLICY=2
  result=$(./efectiu ./$entry);
  y2=(${result//$'\n'/ })
  x=$(echo "${y2[-2]} / ${y1[-2]}" | bc -l);
  product=$(echo "${x} * ${product}" | bc -l);
  printf "%-40s\t" $entry;
  printf "MPKI: %0.3f IPC: %0.3f\t| " ${y1[-5]} ${y1[-2]};
  printf "MPKI: %0.3f IPC: %0.3f\t| " ${y2[-5]} ${y2[-2]};
  printf "Speed up: %0.8f\n" $x;
  i=$((i+1))
done
gm=$(echo "e( l($product)/($i) )" | bc -l)
printf "_______________________________________________________________________________________________________________________\n";
printf "Geometric Mean: %0.8f\n" $gm;//
//
//
//
//
