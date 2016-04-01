/*
 * factory.c
 *
 *  Created on: 2016��1��25��
 *      Author: li.lei
 */
#include <string.h>
#include <limits.h>
#include <framework/mlt.h>

extern mlt_consumer consumer_apreview_init( mlt_profile profile, mlt_service_type type, const char *id, char *arg );

MLT_REPOSITORY
{
	MLT_REGISTER( consumer_type, "android_surface_preview", consumer_apreview_init );
}



