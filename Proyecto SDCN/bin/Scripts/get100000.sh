#!/bin/bash

while read line;
do  
 ../cliente GET,$line &
done < ../../prueba/testGET_100000
