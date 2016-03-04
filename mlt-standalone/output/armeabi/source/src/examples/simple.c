 #include <stdio.h>
       #include <unistd.h>
       #include <framework/mlt.h>

       int main( int argc, char *argv[] )
       {
           // Initialise the factory
           if ( mlt_factory_init( NULL )  )
           {
               // Create the default consumer
		mlt_profile profile = mlt_profile_init(NULL);
		
               mlt_consumer hello = mlt_factory_consumer( profile, NULL, NULL );

               // Create via the default producer
               mlt_producer world = mlt_factory_producer( profile, NULL, argv[ 1 ] );
		mlt_producer_set_speed(world,3.0);

		mlt_service joint = mlt_producer_service(world);

		if (argc>2) {
			mlt_filter filter = mlt_factory_filter(profile, argv[2], 0);
			mlt_filter_set_in_and_out(filter, 0, 300);
			mlt_filter_connect(filter, joint, 0 );	
			joint = mlt_filter_service(filter);
		}

               // Connect the producer to the consumer
               mlt_consumer_connect( hello, joint );

               // Start the consumer
               mlt_consumer_start( hello );

               // Wait for the consumer to terminate
               while( !mlt_consumer_is_stopped( hello ) )
                   sleep( 1 );

               // Close the consumer
               mlt_consumer_close( hello );

               // Close the producer
               mlt_producer_close( world );

               // Close the factory
               mlt_factory_close( );
           }
           else
           {
               // Report an error during initialisation
               fprintf( stderr, "Unable to locate factory modules\n" );
           }

           // End of program
           return 0;
       }

