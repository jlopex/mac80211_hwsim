/*
 *	wmediumd, wireless medium simulator for mac80211_hwsim kernel module
 *	Copyright (C) 2011  Javier Lopez (jlopex@gmail.com)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version 2
 *	of the License, or (at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef PROBABILITY_H_
#define PROBABILITY_H_

#include "mac_address.h"

void init_probability(int size);
struct mac_address * get_mac_address(int pos);
void print_prob_matrix (double *aMatrix);
void fill_prob_matrix(double *aMatrix,double aValue);
int should_drop_frame(double *aMatrix,struct mac_address *src, struct mac_address *dst);
double generate_random_double();
double get_prob_per_link_with_rate_idx(double *aMatrix,struct mac_address *src, struct mac_address *dst, int rate_idx);



#endif /* PROBABILITY_H_ */
