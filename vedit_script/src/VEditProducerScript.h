/*
 * VEditProducerScript.h
 *
 *  Created on: 2016��2��22��
 *      Author: li.lei@youku.com
 */

#ifndef VEDITPRODUCERSCRIPT_H_
#define VEDITPRODUCERSCRIPT_H_

#include "VEditScript.h"
#include "VEditMltRun.h"

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
	string uuid;
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
public:
	ImageScript(json_t* text):
		SingleResourceScript(text)
	{}

	virtual ~ImageScript();
	virtual json_t* compile() throw (Exception);
	virtual void pre_judge() throw (Exception);
};

class GifScript : public SingleResourceScript
{
public:
	GifScript(json_t* text):
		SingleResourceScript(text)
	{}

	virtual ~GifScript();
	virtual json_t* compile() throw (Exception);
	virtual void pre_judge() throw (Exception);
};

struct SingleResourceLoader : public MltLoader
{
	static int declare();

	SingleResourceLoader():producer_tmp(NULL){}
	virtual ~SingleResourceLoader();

	mlt_service get_video(JsonWrap js) throw (Exception);
	mlt_service get_audio(JsonWrap js) throw (Exception);
	mlt_service get_image(JsonWrap js) throw (Exception);
	mlt_service get_gif(JsonWrap js) throw (Exception);
private:
	void parse_filters() throw (Exception);
	mlt_filter get_filter(json_t* defines) throw (Exception);
	JsonWrap defines_tmp;
	mlt_producer producer_tmp;
};

NMSP_END(vedit)

#endif /* VEDITPRODUCERSCRIPT_H_ */
