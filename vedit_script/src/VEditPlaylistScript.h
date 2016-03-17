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

private:
	struct SliceWrap
	{
		ScriptSerialized call_result;
		shared_ptr<ScriptCallable> call;
		ScriptPropsPtr ctrls;
		JsonWrap ctrl_result;
	};

	typedef vector<SliceWrap>::iterator SliceIter;
	vector<SliceWrap> slices;
};

struct PlaylistLoader : public MltLoader
{
	static void declare();

	mlt_service load_playlist(JsonWrap js) throw (Exception);
};

NMSP_END(vedit)

#endif /* VEDITPLAYLISTSCRIPT_H_ */
