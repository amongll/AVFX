/*
 * VEditProducerScript.h
 *
 *  Created on: 2016��2��22��
 *      Author: li.lei@youku.com
 */

#ifndef VEDITPRODUCERSCRIPT_H_
#define VEDITPRODUCERSCRIPT_H_

#include "VEditScript.h"

NMSP_BEGIN(vedit)

class SingleResourceScript: public Script
{
public:
	SingleResourceScript(json_t* text):
		Script(text)
	{}

	virtual ~SingleResourceScript(){}
	virtual void parse_specific() throw (Exception);

	string path;
};

class VideoScript : public SingleResourceScript
{
public:
	VideoScript(json_t* text):
		SingleResourceScript(text)
	{}

	virtual ~VideoScript();
	virtual json_t* compile() throw (Exception);
	virtual void pre_judge() throw (Exception);
};

class AudioScript : public SingleResourceScript
{
public:
	AudioScript(json_t* text):
		SingleResourceScript(text)
	{}

	virtual ~AudioScript();
	virtual json_t* compile() throw (Exception);
	virtual void pre_judge() throw (Exception);
};

class ImageScript : public SingleResourceScript
{
	ImageScript(json_t* text):
		SingleResourceScript(text)
	{}

	virtual ~ImageScript();
	virtual json_t* compile() throw (Exception);
	virtual void pre_judge() throw (Exception);
};

class GifScript : public SingleResourceScript
{
	GifScript(json_t* text):
		SingleResourceScript(text)
	{}

	virtual ~GifScript();
	virtual json_t* compile() throw (Exception);
	virtual void pre_judge() throw (Exception);
};

NMSP_END(vedit)

#endif /* VEDITPRODUCERSCRIPT_H_ */
