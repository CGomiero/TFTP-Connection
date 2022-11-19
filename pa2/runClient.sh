#!/bin/bash
g++ client.cpp -o client
for nbufs in 15 30 60
do let "bufsize = $((1500/nbufs))"
    for typeVal in 1 2 3
        do  
            ./client 51531 10.158.82.32 20000 $nbufs $bufsize $typeVal
        done;
done;