/**
 * MltTokeniser.h - MLT Wrapper
 * Copyright (C) 2004-2015 Meltytech, LLC
 * Author: Charles Yates <charles.yates@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MLTPP_TOKENISER_H
#define MLTPP_TOKENISER_H

#include "config.h"

#include <framework/mlt.h>

namespace Mlt
{
	class MLTPP_DECLSPEC Tokeniser
	{
		private:
			mlt_tokeniser tokens;
		public:
			Tokeniser( char *text = NULL, char *delimiter = NULL );
			~Tokeniser( );
			int parse( char *text, char *delimiter = NULL );
			int count( );
			char *get( int index );
			char *input( );
	};
}

#endif
