/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: Victor
 *
 * Created on 25 de agosto de 2020, 11:11
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

/*
 * 
 */
int main(int argc, char** argv) {
    
    umask(0);
    //int fd = open("./lat/latencias.txt", O_RDONLY, 0666);
    
    int* acum = (int*)calloc( 60, sizeof(int) );
    double* perc = (double*)calloc( 60, sizeof(double) );
    
    for(int x = 0 ; x < 60 ; x++){
        
        int fd = open("./lat/P2_GET_latencias_SO.txt", O_RDONLY, 0666);
        
        int flag = 0;
        char buf[9];
        
        printf("Comienza %d\n",x);
        while(flag < 100000){
            
            read(fd, buf, 9);
            /*if(res<0){
                    printf("¡ERROR CON READ!\n");
                    return -1;
            }*/
            
            //ya tengo un número
            double numero = atof(buf);

            if(numero<0.005 && x==0 ){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.010 && x==1){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.015 && x==2){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.020 && x==3){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.025 && x==4){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.030 && x==5){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.035 && x==6){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.040 && x==7){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.045 && x==8){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.050 && x==9){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.055 && x==10){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.060 && x==11){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.065 && x==12){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.070 && x==13){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.075 && x==14){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.080 && x==15){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.085 && x==16){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.090 && x==17){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.095 && x==18){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.100 && x==19){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.105 && x==20){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.110 && x==21){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.115 && x==22){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.120 && x==23){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.125 && x==24){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.130 && x==25){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.135 && x==26){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.140 && x==27){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.145 && x==28){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.150 && x==29){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.155 && x==30){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.160 && x==31){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.165 && x==32){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.170 && x==33){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.175 && x==34){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.180 && x==35){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.185 && x==36){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.190 && x==37){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.195 && x==38){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.200 && x==39){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.205 && x==40){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.210 && x==41){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.215 && x==42){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.220 && x==43){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.225 && x==44){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.230 && x==45){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.235 && x==46){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.240 && x==47){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.245 && x==48){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.250 && x==49){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.255 && x==50){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.260 && x==51){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.265 && x==52){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.270 && x==53){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.275 && x==54){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.280 && x==55){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.285 && x==56){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.290 && x==57){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.295 && x==58){
                    acum[x] = acum[x] + 1;
            }else if(numero<0.300 && x==59){
                    acum[x] = acum[x] + 1;
            }

            memset(buf,0,9);
            flag++;
            
        }
        
        printf("Termina %d\n",x);
        //lseek(fd,0,SEEK_SET);
        close(fd);
        
    }
    
    int fd2 = open("P2_GET_percentiles_SO.txt", O_CREAT | O_APPEND | O_RDWR ,0666);
    char buffer2[12];
    
    for(int y = 0; y < 60 ; y++){
        perc[y] = ((double)acum[y])/1000;
        sprintf(buffer2,"%f\n",perc[y]);
        write(fd2,buffer2,strlen(buffer2));
        printf("%f\n",perc[y]);
    }
    
    close(fd2);

    return (EXIT_SUCCESS);
}

