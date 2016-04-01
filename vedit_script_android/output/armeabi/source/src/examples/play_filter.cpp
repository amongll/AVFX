/*
 * player_filter.cpp
 *
 *  Created on: 2015Äê12ÔÂ8ÈÕ
 *      Author: L-F000000-PC
 */

#include <Mlt.h>
using namespace Mlt;

int main( int, char **argv )
{
	Factory::init( NULL );
	Profile profile;
	Producer producer( profile, argv[ 1 ] );
	Consumer consumer( profile );
	Filter filter(profile,"grayscale");
	consumer.set( "rescale", "none" );
	filter.connect_producer(producer,0);
	consumer.connect(filter);
	//consumer.connect( producer );
	consumer.run( );
	return 0;
}
