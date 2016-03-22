/*
 * VEditMultiTrackScript.h
 *
 *  Created on: 2016��2��22��
 *      Author: li.lei@youku.com
 */

#ifndef VEDITMULTITRACKSCRIPT_H_
#define VEDITMULTITRACKSCRIPT_H_

#include "VEditMltRun.h"
#include "VEditScript.h"

NMSP_BEGIN(vedit)

class MultitrackScript : public Script
{
public:
	MultitrackScript(json_t* text):
		Script(text)
	{}

	virtual ~MultitrackScript();
	virtual void pre_judge() throw(Exception);
	virtual json_t* compile() throw(Exception);
	virtual void parse_specific() throw (Exception);

private:
	struct TrackWrap
	{
		shared_ptr<ScriptCallable> track_calls;
		ScriptSerialized track_call_results;

		struct TransWrap
		{
			shared_ptr<ScriptCallable> trans_calls;
			ScriptSerialized trans_call_results;
		};

		list<TransWrap> transitions;
	};

	list<TrackWrap> tracks;
};

struct MultitrackLoader : public MltLoader
{
	static void declare();

	mlt_service get_tractro(JsonWrap js) throw (Exception);
};

NMSP_END(vedit)

#endif /* VEDITMULTITRACKSCRIPT_H_ */
