/*
 * VEditPlaylistScript.h
 *
 *  Created on: 2016Äê2ÔÂ22ÈÕ
 *      Author: li.lei@youku.com
 */

#ifndef VEDITPLAYLISTSCRIPT_H_
#define VEDITPLAYLISTSCRIPT_H_


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
};

NMSP_END(vedit)

#endif /* VEDITPLAYLISTSCRIPT_H_ */
