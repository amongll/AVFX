/*
 * VEditPlaylistScript.h
 *
 *  Created on: 2016-2-22
 *      Author: li.lei@youku.com
 */

#ifndef VEDITPLAYLISTSCRIPT_H_
#define VEDITPLAYLISTSCRIPT_H_

#include "VEditMltRun.h"
#include "VEditScript.h"

NMSP_BEGIN(vedit)

struct ProducerWrap
{
	ProducerWrap(mlt_producer o=NULL, int giv=0):
		obj(NULL)
	{
		obj = o;
		if (obj && giv==0) {
			mlt_properties_inc_ref(mlt_producer_properties(obj));
		}
	}
	~ProducerWrap() {
		if (obj) mlt_producer_close(obj);
	}

	ProducerWrap(const ProducerWrap& r) :
		obj(r.obj) {
		if (obj) mlt_properties_inc_ref(mlt_producer_properties(obj));
	}

	ProducerWrap& operator=(const ProducerWrap& r) {
		if (obj) mlt_producer_close(obj);
		obj = r.obj;
		if (obj) mlt_properties_inc_ref(mlt_producer_properties(obj));
		return *this;
	}
	mlt_producer obj;
};

class PlaylistScript : public Script
{
public:
	PlaylistScript(json_t* text):
		Script(text)
	{}

	virtual ~PlaylistScript();
	virtual json_t* compile() throw(Exception);
	virtual void parse_specific() throw (Exception);
	virtual void pre_judge() throw(Exception);
};

class PlaylistLoader : public MltLoader
{
	static void declare();

	mlt_service load_playlist(JsonWrap js) throw (Exception);
};

NMSP_END(vedit)

#endif /* VEDITPLAYLISTSCRIPT_H_ */
