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
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
 *	02110-1301, USA.
 */

#include <libconfig.h>
#include <string.h>
#include <stdlib.h>

#include "probability.h"

extern int jammer;
extern double *prob_matrix; 
extern int size;

/*
 *	Funtion to replace all ocurrences of a "old" string for a "new" string
 *	inside a "str" string
 */

char *str_replace(const char *str, const char *old, const char *new)
{
	char *ret, *r;
	const char *p, *q;
	size_t len_str = strlen(str);
	size_t len_old = strlen(old);
	size_t len_new = strlen(new);
	size_t count;

	for(count = 0, p = str; (p = strstr(p, old)); p += len_old)
		count++;

	ret = malloc(count * (len_new - len_old) + len_str + 1);
	if(!ret)
		return NULL;

	for(r = ret, p = str; (q = strstr(p, old)); p = q + len_old) {
		count = q - p;
		memcpy(r, p, count);
		r += count;
		strcpy(r, new);
		r += len_new;
	}
	strcpy(r, p);
	return ret;
}

/*
 *	Writes a char* buffer to a destination file
 */

int write_buffer_to_file(char *file, char *buffer)
{
	FILE *p = NULL;

	p = fopen(file, "w");
	if (p== NULL) {
		return 1;
	}

	fwrite(buffer, strlen(buffer), 1, p);
	fclose(p);

	return 0;
}

/*
 *	Writes a sample configuration with matrix filled with a value to a file
 */

int write_config(char *file, int ifaces, float value)
{
	FILE *out;
	char *ptr, *ptr2;
	size_t size;
	config_t cfg;
	config_setting_t *root, *setting, *group, *array, *list;
	int i, j, rates = 12;

	/*Init tmp file stream*/
	out = open_memstream(&ptr, &size);
	if (out == NULL) {
		printf("Error generating stream\n");
		exit(EXIT_FAILURE);
	}
	/*Init config*/
	config_init(&cfg);

	/*Create a sample config schema*/
	root = config_root_setting(&cfg);
	/* Add some settings to the ifaces group. */
	group = config_setting_add(root, "ifaces", CONFIG_TYPE_GROUP);
	setting = config_setting_add(group, "count", CONFIG_TYPE_INT);
	config_setting_set_int(setting, ifaces);
	array = config_setting_add(group, "ids", CONFIG_TYPE_ARRAY);

	for(i = 0; i < ifaces; ++i) {
		setting = config_setting_add(array, NULL, CONFIG_TYPE_STRING);
		char buffer[25];
		sprintf (buffer, "42:00:00:00:%02d:00", i);
		config_setting_set_string(setting, buffer);
	}

	/* Add some settings to the prob group. */
	group = config_setting_add(root, "prob", CONFIG_TYPE_GROUP);
	setting = config_setting_add(group, "rates", CONFIG_TYPE_INT);
	config_setting_set_int(setting, rates);
	list = config_setting_add(group, "matrix_list", CONFIG_TYPE_LIST);
	for (j = 0; j < rates ; j++) {
		array = config_setting_add(list, NULL, CONFIG_TYPE_ARRAY);
		int diag_count = 0;
		for(i = 0; i < ifaces*ifaces; ++i) {
			setting = config_setting_add(array, NULL,
						     CONFIG_TYPE_FLOAT);
			if (diag_count == 0)
				config_setting_set_float(setting, -1.0);
			else
				config_setting_set_float(setting, value);
			diag_count++;
			if (diag_count > ifaces)
				diag_count = 0;
		}
	}
	/* Write in memory out file */
	config_write(&cfg, out);
	config_destroy(&cfg);
	fclose(out);

	/* Let's do some post processing */
	ptr2 = str_replace(ptr, "], ", "],\n\t");
	free(ptr);
	ptr = str_replace(ptr2, "( ", "(\n\t");
	free(ptr2);
	/* Let's add comments to the config file */
	ptr2 = str_replace(ptr, "ifaces :", "#\n# wmediumd sample config file\n#\n\nifaces :");
	free(ptr);
	ptr = str_replace(ptr2, "prob :", "\n#\n# probability matrices are defined in a rowcentric way \n# probability matrices are ordered from slower to fastest, check wmediumd documentation for more info\n#\n\nprob :");
	printf("%s",ptr);

	/*write the string to a file*/
	if(write_buffer_to_file(file, ptr)) {
		printf("Error while writing file.\n");
		free(ptr);
		exit(EXIT_FAILURE);
	}
	printf("New configuration successfully written to: %s\n", file);

	/*free ptr*/
	free(ptr);
	exit(EXIT_SUCCESS);
}

/*
 *	Loads a config file into memory
 */

int load_config(const char *file)
{

	config_t cfg, *cf;
	const config_setting_t *ids, *prob_list, *mat_array;
	int count_ids, rates_prob, i, j;
	long int count_value, rates_value;
	const char *jammer_v;

	/*initialize the config file*/
	cf = &cfg;
	config_init(cf);

	/*read the file*/
	if (!config_read_file(cf, file)) {
		printf("Error loading file %s at line:%d, reason: %s\n",
		file,
		config_error_line(cf),
		config_error_text(cf));
		config_destroy(cf);
		exit(EXIT_FAILURE);
    	}

	config_lookup_string(cf, "jammer", &jammer_v);
	if (!strcmp(jammer_v, "on")) {
		jammer = 1;
	}

	/*let's parse the values*/
	config_lookup_int(cf, "ifaces.count", &count_value);
	ids = config_lookup(cf, "ifaces.ids");
	count_ids = config_setting_length(ids);

	/*cross check*/
	if (count_value != count_ids) {
		printf("Error on ifaces.count");
		exit(EXIT_FAILURE);
	}

	size = count_ids;
	printf("#_if = %d\n",count_ids);
	/*Initialize the probability*/
	prob_matrix = init_probability(count_ids);

	/*Fill the mac_addr*/
	for (i = 0; i < count_ids; i++) {
    		const char *str =  config_setting_get_string_elem(ids, i);
    		put_mac_address(string_to_mac_address(str),i);
    	}
	/*Print the mac_addr array*/
	print_mac_address_array();

	config_lookup_int(cf, "prob.rates", &rates_value);
	prob_list = config_lookup(cf,"prob.matrix_list");

	/*Get rates*/
	rates_prob = config_setting_length(prob_list);

	/*Some checks*/
	if(!config_setting_is_list(prob_list)
	   && rates_prob != rates_value) {
		printf("Error on prob_list");
		exit(EXIT_FAILURE);
	}

	/*Iterate all matrix arrays*/
	for (i=0; i < rates_prob ; i++) {
		int x = 0, y = 0;
		mat_array = config_setting_get_elem(prob_list,i);
		/*If any error break execution*/
		if (config_setting_length(mat_array) != count_ids*count_ids) {
    			exit(EXIT_FAILURE);
		}
		/*Iterate all values on matrix array*/
		for (j=0; j < config_setting_length(mat_array); ) {
			MATRIX_PROB(prob_matrix,count_ids,x,y,i) =
			config_setting_get_float_elem(mat_array,j);
			//printf("%f, ", config_setting_get_float_elem(mat_array,j));
			x++;
			j++;
			/* if we finalized this row */
			if (j%count_ids==0) {
				y++;
				x=0;
				//printf("*******j:%d,count_ids:%d \n",j,count_ids);
			}
		}
	}

	config_destroy(cf);
	return (EXIT_SUCCESS);
}
