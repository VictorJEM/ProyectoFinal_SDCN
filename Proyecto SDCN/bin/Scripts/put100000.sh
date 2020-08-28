#!/bin/bash

while read line;
do 
 ../cliente PUT,$line &
done < ../../prueba/testPUT_100000
