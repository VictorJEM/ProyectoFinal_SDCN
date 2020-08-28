#!/bin/bash

num=(1 2 3 4)
name="HT"

while read line;
do  
 r=$(( $RANDOM % 4 ))
 name=$name${num[$r]} 
 ../cliente 127.0.0.1 45343 GET,$name,$line &
 name="HT"
done < ../../prueba/testGET_100000
