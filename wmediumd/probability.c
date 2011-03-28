/*
 ============================================================================
 Name        : probability.c
 Author      : Javier Lopez
 Version     :
 Copyright   : 
 Description :
 ============================================================================
 */

/*
							Column Index
							SRC MAC Address

				[ -1.000000][  1.000000][  2.000000][  3.000000][  4.000000]
	Row Index		[  5.000000][ -1.000000][  6.000000][  7.000000][  8.000000]
	DST MAC			[  9.000000][ 10.000000][ -1.000000][ 11.000000][ 12.000000]
	Address			[ 13.000000][ 14.000000][ 15.000000][ -1.000000][ 16.000000]
				[ 17.000000][ 18.000000][ 19.000000][ 20.000000][ -1.000000]
*/


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "probability.h"

static int a_size = 0;
static struct mac_address *indexer;

/**
 * 	Returns the position of the address in the array.
 * 	If the mac_address is not found returns -1
 */

int find_pos_by_mac_address(struct mac_address *addr) {

	int i=0;

	void * ptr = indexer;

	while(memcmp(ptr,addr,sizeof(struct mac_address)) && i < a_size)
	{
		struct mac_address *a = (struct mac_address*)ptr;
		//printf("A:%02X:%02X:%02X:%02X:%02X:%02X\n",a->addr[0],a->addr[1],a->addr[2],a->addr[3],a->addr[4],a->addr[5]);
		//printf("B:%02X:%02X:%02X:%02X:%02X:%02X\n",addr->addr[0],addr->addr[1],addr->addr[2],addr->addr[3],addr->addr[4],addr->addr[5]);
		i++;
		ptr = ptr + sizeof(struct mac_address);
	}

	return ((i >= a_size) ?  -1 :  i);
}

/**
 * 	Prints the values of the Mac Adress Array
 */

void print_mac_address_array() {

	int i=0;
	void * ptr = indexer;

	while (i < a_size) {
		struct mac_address *a = malloc(sizeof(struct mac_address));
		memcpy(a,ptr,sizeof(struct mac_address));
		printf("A[%d]:%02X:%02X:%02X:%02X:%02X:%02X\n",i,a->addr[0],a->addr[1],a->addr[2],a->addr[3],a->addr[4],a->addr[5]);
		i++;
		ptr = ptr + sizeof(struct mac_address);
	}
}

/**
 * Fills the probability matrix with a given value 
 */

void fill_prob_matrix(double *aMatrix,double aValue) {
	int i,j,c=0;

	for (i = 0 ; i < a_size*a_size ; i=i+a_size ) {
		for (j = 0 ; j < a_size ; j++)
		{
			if (j==c) {
				aMatrix[j+i]=-1;
			} else {
				aMatrix[j+i]=aValue;
			}
		}
		c++;
	}
}

/**
 * 	Prints the values of the probability matrix
 */

void print_prob_matrix (double *aMatrix) {

	int i,j;

	for (i = 0 ; i < a_size*a_size; i=i+a_size) {
		for (j = 0 ; j < a_size ; j++)
		{
			int pos = j+i;
			printf("[%10f]",aMatrix[pos]);
		}
		printf("\n");
	}
}

/**
 * Returns the probability for a given matrix position
 */

double find_prob_by_pos (double *aMatrix, int column, int row) {

	return aMatrix[row+(column*a_size)];
}

/**
 * 	Returns loss probability for a desired link.
 * 	If an error occurs returns -1;
 */

double find_prob_by_addrs (double *aMatrix,struct mac_address *src, struct mac_address *dst) {

	int col = find_pos_by_mac_address(src);
	int row = find_pos_by_mac_address(dst);

	if (col == -1 || row ==-1)
		return -1;
	return find_prob_by_pos(aMatrix,col,row);
}

/**
 * 	Generates a random double value
 */

double generate_random_double() {

	return rand()/((double)RAND_MAX+1);
}

/**
 *  	Function to apply the loss probability to the packet
 * 	returns 1 if the frame should be dropped, 0 if not
 *  	In case of an unknown link, the frame wont be dropped
 */

int should_drop_frame(double *aMatrix,struct mac_address *src, struct mac_address *dst) {

	double prob_per_link = find_prob_by_addrs(aMatrix,src,dst);
	double random_double = generate_random_double();

	if (prob_per_link==-1)
	{
		printf("ERROR: unknown link!\n");
		return 0;
	}
//	printf("%f\n",random_double);

	if (random_double > prob_per_link)
		return 0;
	return 1;

}

void init_probability(int size) {
	
	int i;
	a_size = size;
	indexer = malloc(sizeof(struct mac_address)*a_size);
 	void *ptr = indexer;
	
	if (indexer==NULL) {
		printf("Problem allocating vector");
		exit(1);
	}

	// Fill the mac_addr array as mac80211_hwsim does
	struct mac_address a_addr;
	a_addr.addr[0] = 0x42;
	a_addr.addr[1] = 0x00;
	a_addr.addr[2] = 0x00;
	a_addr.addr[3] = 0x00;
	a_addr.addr[5] = 0x00;

 	for (i=0; i < a_size ; i++) {
 		a_addr.addr[4] = i;
 		memcpy(ptr, &a_addr, sizeof(struct mac_address));
 		ptr = ptr + sizeof(struct mac_address);
 	}

 	print_mac_address_array();

}


