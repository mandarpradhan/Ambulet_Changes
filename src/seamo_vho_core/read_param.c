/*
 *  read_param.c
 *  Reads various types of data obtained from the D-Bus message
 *  output
 * 
 *  (c) SeaMo, version 0.1, 2011, ECE Department, IISc, Bangalore &
 *  Department of IT, MCIT, Government of India
 *
 *  Copyright (c) 2009 - 2011
 *  MIP Project group, ECE Department, Indian
 *  Institute of Science, Bangalore and Department of IT, Ministry of
 *  Communications and IT, Government of India. All rights reserved.
 * 
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *     
 *  Authors: Seema K   <seema at eis.iisc.ernet.in>
 *           Anand SVR <anand at eis.iisc.ernet.in>
 *          
 *  See the file "license.terms" for information on usage and redistribution
 *  of this file.
 */     

/***************************** INCLUDES *****************************/
#include "read_param.h"


/**************************** SUBROUTINES ******************************/

/* This function reads the integer value in the @param1
 * and stores it in the @param2.
 * @param1: Contains the reply returned by D-Bus method call
 * @param2: Buffer to store the read integer value
 */

void read_int_byte(DBusMessageIter * args, int *value)
{
	int byte;
	int type = dbus_message_iter_get_arg_type(args);
	if (type == DBUS_TYPE_INT64 || type == DBUS_TYPE_INT32
	    || type == DBUS_TYPE_INT16 || type == DBUS_TYPE_UINT64
	    || type == DBUS_TYPE_UINT32 || type == DBUS_TYPE_UINT16
	    || type == DBUS_TYPE_BYTE) {
		dbus_message_iter_get_basic(args, &byte);
		*value = byte;
	}
}

/* This function reads the string data in the @param1
 * and stores it in the @param2.
 * @param1: Contains the reply returned by D-Bus method call
 * @param2: Buffer to store the read string data
 */

void read_string(DBusMessageIter * args, char *str_value)
{
	const char **str = malloc(100 * sizeof(char));
	int type = dbus_message_iter_get_arg_type(args);
	if (type == DBUS_TYPE_STRING)	//115
	{
		dbus_message_iter_get_basic(args, str);
		strcpy(str_value, *str);
	}
	free(str);
}

/* This function reads the array data which contains the object
 * paths in the @param1 and stores the array of objects in @param2
 */

int read_array(DBusMessageIter * args, char *list[30])
{
	DBusMessageIter subiter;
	int num_of_dev = 0;
	int type = dbus_message_iter_get_arg_type(args);
	if (type == DBUS_TYPE_ARRAY) {
		dbus_message_iter_recurse(args, &subiter);
		int sub_type = dbus_message_iter_get_arg_type(&subiter);
		if (sub_type == DBUS_TYPE_OBJECT_PATH)
			num_of_dev = read_obj(&subiter, list);
	}
return(num_of_dev);
}

/* This function gets the essid by recursively reading the reply
 * obtained from the D-Bus method call.
 * @param1: Contains the reply returned by D-Bus method call
 * @param2: Buffer to store the essid
 */

void read_essid(DBusMessageIter * args, char *name)
{
	DBusMessageIter dict_iter, subiter;
	char str_ptr[50];
	int type = dbus_message_iter_get_arg_type(args);
	if (type != DBUS_TYPE_ARRAY) {
		return;
	}
	dbus_message_iter_recurse(args, &subiter);
	do {
		int sub_type = dbus_message_iter_get_arg_type(&subiter);
		if (sub_type != DBUS_TYPE_DICT_ENTRY)
			return;
		dbus_message_iter_recurse(&subiter, &dict_iter);
		if (dbus_message_iter_get_arg_type(&dict_iter) ==
		    DBUS_TYPE_STRING) {
			read_string(&dict_iter, str_ptr);
			if (!strcmp(str_ptr, "connection")) {
				dbus_message_iter_next(&dict_iter);
				read_connection(&dict_iter, name);
			}
		}
	} while (dbus_message_iter_next(&subiter));
}

void read_connection(DBusMessageIter * iter, char *name)
{
	DBusMessageIter dict_iter, var_iter, subiter;
	char str_ptr[50];
	int type = dbus_message_iter_get_arg_type(iter);
	if (type != DBUS_TYPE_ARRAY) {
            return;
	}
	dbus_message_iter_recurse(iter, &subiter);
	do {
		int sub_type = dbus_message_iter_get_arg_type(&subiter);
		if (sub_type != DBUS_TYPE_DICT_ENTRY)
			return;
		dbus_message_iter_recurse(&subiter, &dict_iter);
		if (dbus_message_iter_get_arg_type(&dict_iter) ==
		    DBUS_TYPE_STRING) {
			read_string(&dict_iter, str_ptr);
			if (!strcmp(str_ptr, "id")) {
				dbus_message_iter_next(&dict_iter);
				int dict_type =
				    dbus_message_iter_get_arg_type(&dict_iter);
				if (dict_type == DBUS_TYPE_VARIANT) {
					dbus_message_iter_recurse(&dict_iter,
								  &var_iter);
					read_string(&var_iter, name);
				}
			}
		}
	} while (dbus_message_iter_next(&subiter));
}


void read_spn(DBusMessageIter * iter, char *spn)
{
        DBusMessageIter dict_iter, var_iter, subiter;
        char str_ptr[50];
        int type = dbus_message_iter_get_arg_type(iter);
        if (type != DBUS_TYPE_ARRAY) {
            return;
        }
        dbus_message_iter_recurse(iter, &subiter);
        do {
                int sub_type = dbus_message_iter_get_arg_type(&subiter);
                if (sub_type != DBUS_TYPE_DICT_ENTRY)
                        return;
                dbus_message_iter_recurse(&subiter, &dict_iter);
                if (dbus_message_iter_get_arg_type(&dict_iter) ==
                    DBUS_TYPE_STRING) {
                        read_string(&dict_iter, str_ptr);
                        if (!strcmp(str_ptr, "m3gpp-operator-name")) {
                                dbus_message_iter_next(&dict_iter);
                                int dict_type =
                                    dbus_message_iter_get_arg_type(&dict_iter);
                                if (dict_type == DBUS_TYPE_VARIANT) {
                                        dbus_message_iter_recurse(&dict_iter,
                                                                  &var_iter);
                                        read_string(&var_iter, spn);
					printf("\n spn1 is : %s\n",spn);
                                }
                        }
                }
        } while (dbus_message_iter_next(&subiter));

}

/* This function reads the object path in the @param1
 * and stores it in the @param2.
 * @param1: Contains the reply returned by D-Bus method call
 * @param2: Buffer to store the array of objects
 */

int read_obj(DBusMessageIter * subiter, char *list[30])
{	
	int num_of_dev = 0;
	while (dbus_message_iter_get_arg_type(subiter) == DBUS_TYPE_OBJECT_PATH) {
		*list = malloc(100);
		const char *path;
		dbus_message_iter_get_basic(subiter, &path);
		strcpy(*list, path);
		dbus_message_iter_next(subiter);
		list++;
		num_of_dev++;
	}
return(num_of_dev);
}

int read_obj_single_path(DBusMessageIter * subiter, char **list)
{
                *list = malloc(100);
                const char *path;
                dbus_message_iter_get_basic(subiter, &path);
                strcpy(*list, path);
}

/* This function reads the variant value in the @param1
 * and stores it in the @param2.
 * @param1: Contains the reply returned by D-Bus method call
 * @param2: Buffer to store the read variant value
 */

void read_variant(DBusMessageIter * args, int *value)
{
	DBusMessageIter subiter;
	int type = dbus_message_iter_get_arg_type(args);
	if (type == DBUS_TYPE_VARIANT) {
		dbus_message_iter_recurse(args, &subiter);
                read_int_byte(&subiter, value);
	}
}

int read_variant_array(DBusMessageIter * args, char *connections[30])
{
	DBusMessageIter subiter;
	int num_of_conn;
	int type = dbus_message_iter_get_arg_type(args);
	if (type == DBUS_TYPE_VARIANT) {
		dbus_message_iter_recurse(args, &subiter);
            int sub_type = dbus_message_iter_get_arg_type(&subiter);
            if (sub_type == DBUS_TYPE_ARRAY)
                 num_of_conn = read_array(&subiter, connections);
	}
return(num_of_conn);     
}

void read_variant_string(DBusMessageIter * args, char *connections)
{
        DBusMessageIter subiter;
        int type = dbus_message_iter_get_arg_type(args);

        if (type == DBUS_TYPE_VARIANT) {
                dbus_message_iter_recurse(args, &subiter);
            int sub_type = dbus_message_iter_get_arg_type(&subiter);
            if (sub_type == DBUS_TYPE_STRING)
                  read_string(&subiter, connections);
        }

}

void read_variant_objpath(DBusMessageIter * args, char **connections)
{
        DBusMessageIter subiter;
        int type = dbus_message_iter_get_arg_type(args);
        const char *path;
        if (type == DBUS_TYPE_VARIANT) {
                dbus_message_iter_recurse(args, &subiter);
            int sub_type = dbus_message_iter_get_arg_type(&subiter);
            if (sub_type == DBUS_TYPE_OBJECT_PATH)
                  read_obj_single_path(&subiter, connections);
        }

}


bool read_bool(DBusMessageIter * args)
{
	bool status;

	int type = dbus_message_iter_get_arg_type(args);
	if (type == DBUS_TYPE_BOOLEAN)
		dbus_message_iter_get_basic(args, &status);

	return status;
}

void read_gsm(DBusMessageIter * args, char *name)
{
        DBusMessageIter dict_iter, subiter;
        char str_ptr[50];
        int type = dbus_message_iter_get_arg_type(args);
        if (type != DBUS_TYPE_ARRAY) {
                return;
        }
        dbus_message_iter_recurse(args, &subiter);
        do {
                int sub_type = dbus_message_iter_get_arg_type(&subiter);
                if (sub_type != DBUS_TYPE_DICT_ENTRY)
                        return;
                dbus_message_iter_recurse(&subiter, &dict_iter);
                if (dbus_message_iter_get_arg_type(&dict_iter) ==
                    DBUS_TYPE_STRING) {
                        read_string(&dict_iter, str_ptr);
                        if (!strcmp(str_ptr, "gsm")) {
                                dbus_message_iter_next(&dict_iter);
                                read_apn(&dict_iter, name);
                        }
                }
        } while (dbus_message_iter_next(&subiter));
}

void read_apn(DBusMessageIter * iter, char *name)
{
        DBusMessageIter dict_iter, var_iter, subiter;
        char str_ptr[50];
        int type = dbus_message_iter_get_arg_type(iter);
        if (type != DBUS_TYPE_ARRAY) {
            return;
        }
        dbus_message_iter_recurse(iter, &subiter);
        do {
                int sub_type = dbus_message_iter_get_arg_type(&subiter);
                if (sub_type != DBUS_TYPE_DICT_ENTRY)
                        return;
                dbus_message_iter_recurse(&subiter, &dict_iter);
                if (dbus_message_iter_get_arg_type(&dict_iter) ==
                    DBUS_TYPE_STRING) {
                        read_string(&dict_iter, str_ptr);
                        if (!strcmp(str_ptr, "apn")) {
                                dbus_message_iter_next(&dict_iter);
                                int dict_type =
                                    dbus_message_iter_get_arg_type(&dict_iter);
                                if (dict_type == DBUS_TYPE_VARIANT) {
                                        dbus_message_iter_recurse(&dict_iter,
                                                                  &var_iter);
                                        read_string(&var_iter, name);
                                }
                        }
                }
	} while (dbus_message_iter_next(&subiter));
}


