#ifndef GRAPES_PRECOND_HPP
#define GRAPES_PRECOND_HPP

#include "../utils/operator.hpp"

/*      R E A D M E 
 * 
 * 各种预条件子只要派生出Solver类就好了，则可在IterativeSolver中通过指针Solver * prec接入
 * 
 * 所以对于预条件，不必再分出一个类
 * 
 * 一般预条件只需要一种精度即可
 */

typedef enum {BACKWARD = -1, SYMMETRIC, FORWARD, RED_BLACK} SCAN_TYPE;

#endif