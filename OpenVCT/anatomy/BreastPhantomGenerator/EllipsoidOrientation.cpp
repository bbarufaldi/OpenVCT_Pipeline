// EllipsoidOrientation.cpp

#include "BreastPhantomGenerator.h"

#include <string>
#include <iostream>
#include <sstream>
#include <cmath>

matrix BreastPhantomGenerator::EllipsoidOrientation(rowVector Origin, float a, float bu, float bd, float c, rowVector pts)
{
    //% Compute the compartment orientation for current seed
    //% function R = EllipsoidOrientation(Origin,a,bu,bd,c,pts)
    //% INPUT:
    //% Origin:        Origin of the ellipsoids for breast outline
    //% a:             axis parameter along chest to nipple direction -->x 
    //% bu:            axis parameter vertical for upper ellipsoid  --->y+
    //% bd:            axis parameter vertical for lower ellipsoid  --->y-
    //% c:             axis parameter horizontally                  --->z
    //% pts:           seed location

    //% determine where is pts, upper breast, or lower breast?
    float b;
    
    if (pts[1] < Origin[1])
    {
        b = bd;
    }
    else
    {
        b = bu;
    }

    //% Origin
    float Ox = Origin[0]; 
    float Oy = Origin[1]; 
    float Oz = Origin[2];

    //% in order to get surface normal, first get the ellipsoid parameters that
    //% passes through ptc
    float x0 = pts[0], y0 = pts[1], z0 = pts[2];
//  float k  = ((y0 - Oy)*(y0 - Oy)/(b*b)+(z0 - Oz)*(z0 - Oz)/(c*c))/(1-(x0-Ox)*(x0-Ox)/(a*a));
    float k  = (float) std::sqrt((y0 - Oy)*(y0 - Oy)/(b*b)+(z0 - Oz)*(z0 - Oz)/(c*c))/(1-(x0-Ox)*(x0-Ox)/(a*a));

    //% virtual ellipsoid
//  float ap = a * a;
//  float bp = b * b * k;
//  float cp = c * c * k;
    float ap = a ;
    float bp = b * k;
    float cp = c * k;
    //% (n,u,v) is the new local coordinate system. n is the surface normal. (u,v)
    //% is in the tangent plane. u is the normal of the triangle OAP.
    //% n = [dfdx,dfdy,dfdz]

//  rowVector n(2 * (x0 - Ox) / ap, 2 * (y0 - Oy) / bp, 2 * (z0 - Oz) / cp);
    rowVector n(2 * (x0 - Ox) / (ap*ap), 2 * (y0 - Oy) / (bp*bp), 2 * (z0 - Oz) / (cp*cp));
    rowVector n_sq(n.square());
    n = n / std::sqrt(rowVector::sum(n_sq));

    //% OA --> origin to nipple, x axis
    //% OP --> origin to current seed

    rowVector A(a, 0, 0);

    rowVector OA = A - Origin;
    rowVector OP = pts - Origin;
    rowVector u = rowVector::cross(OA,OP);
    rowVector v = rowVector::cross(n,u);

    rowVector v_sq(v.square());
    v = v/std::sqrt(rowVector::sum(v_sq));
    u = rowVector::cross(v,n);
    rowVector u_sq(u.square()); // TAKE THE SQUARE AFTER THE CROSS!!!!!!
    u = u/std::sqrt(rowVector::sum(u_sq));

    matrix mat(n, u, v);

    return mat;
}
