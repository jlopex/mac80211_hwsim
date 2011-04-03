/*
 ============================================================================
 Name        : probability.h
 Author      : Javier Lopez
 Version     :
 Copyright   : 
 Description :
 ============================================================================
 */


#ifndef PROBABILITY_H_
#define PROBABILITY_H_

#include "mac_address.h"

void print_prob_matrix (double *aMatrix);
void fill_prob_matrix(double *aMatrix,double aValue);
int should_drop_frame(double *aMatrix,struct mac_address *src, struct mac_address *dst);
void init_probability(int size);
struct mac_address * get_mac_address(int pos);

#endif /* PROBABILITY_H_ */
