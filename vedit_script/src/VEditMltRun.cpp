/*
 * VEditMltRun.cpp
 *
 *  Created on: 2016-2-26
 *      Author: li.lei@youku.com
 */


#include "VEditMltRun.h"


NMSP_BEGIN(vedit)
MltRuntime::MltRuntime(json_t* serialize, int give):
	json_version(0),
	json_serialize(NULL),

{
}

MltRuntime::~MltRuntime()
{
}

void MltRuntime::run() throw (Exception)
{
}

void MltRuntime::stop() throw (Exception)
{
}

uint32_t MltRuntime::get_frame_length() throw (Exception)
{
}

uint32_t MltRuntime::get_frame_position() throw (Exception)
{
}

void MltRuntime::erase_uuid(const string& uuid)
{
}

void MltRuntime::seek() throw (Exception)
{
}

NMSP_END(vedit)


