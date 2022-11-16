/*
 * quaternion.c
 *
 *  Created on: 30 set 2022
 *      Author: Dello Iacono
 */
#include "math.h"


void quatern2euler(float q0, float q1, float q2, float q3, float *phi, float *theta, float *psi) {
    float r11, r21, r31, r32, r33;

    r11 = 2.0*q0*q0 - 1.0 + 2.0*q1*q1;
    r21 = 2.0*( q1*q2 - q0*q3 );
    r31 = 2.0*( q1*q3 + q0*q2 );
    r32 = 2.0*( q2*q3 - q0*q1 );
    r33 = 2.0*( q0*q0 - 1.0 + 2.0*q3*q3 );

    *phi = atan2( r32, r33 );
    *theta = -atan( r31 / sqrt( 1- r31*r31 ) );
    *psi = atan2( r21, r11 );

    //euler = [phi(1,:)' theta(1,:)' psi(1,:)'];
}
