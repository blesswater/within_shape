#include <iostream>
#include <string>
#include <libgen.h>
#include <getopt.h>

#include "/usr/include/gdal/ogrsf_frmts.h"

#include "within-shape.h"

using namespace std;


void usage()
{
	printf( "\n" );
	printf( "Usage:\n" );
	printf( "\twithin-shape [--latlon|--wkt] shapefile.shp\n" );
	printf( "\t\t--latlon latitude,longitude\n" );
	printf( "\t\t--wkt WKT\n" );
	printf( "\n" );
}

bool withinGetOpts( withinOpts *opts, int argc, char **argv )
{

	static struct option long_options[] = {
		{"latlon", required_argument, 0, 'l'},
		{"wkt", required_argument, 0, 'w'},
		{0,0,0,0}
	};

	int arg;
	int index = 0;

	char *pos;

	memset( opts, 0x00, sizeof(withinOpts) );
	opts->inType = within_Error;

	if( argc != 4 )
	{
		return WITHIN_BAD;
	}

	while( (arg = getopt_long( argc, argv, "l:w:", long_options, &index )) != -1 )
	{

		switch( arg )
		{

			case 'l':
				pos = strchr( optarg, ',' );
				if( pos != NULL )
				{
					*pos = 0x00;
					opts->lat = std::stod( optarg );
					opts->lon = std::stod( (pos + 1) );
					opts->inType = within_LatLon;
				}
			break;

			case 'w':
				opts->wkt = (char *)malloc( strlen( optarg ) + 1 );
				memset( opts->wkt, 0x00, strlen( optarg ) + 1 );
				strcpy( opts->wkt, optarg );
				opts->inType = within_Wkt;
			break;

			case '?':
			default:
			break;
		}
	}

	if( optind < argc )
	{
		opts->shapeFilename = (char *)malloc( strlen( argv[optind] ) );
		strcpy( opts->shapeFilename, argv[optind] );
	}
	else
	{
		opts->inType = within_Error;
		if( opts->wkt != NULL )
		{
			free( opts->wkt );
			opts->wkt = NULL;
		}
	}

	return (opts->inType != within_Error) ? WITHIN_GOOD : WITHIN_BAD;
}

int main( int argc, char **argv )
{

	char *layerName;

	GDALDataset *gdalDataSet;
	OGRLayer *layer;
	OGRFeature *feature;
	OGRGeometry *geom = NULL;
	OGRPoint *testPoint;

	withinOpts opts;

	if( withinGetOpts( &opts, argc, argv ) == WITHIN_BAD )
	{
		usage();
		exit( 1 );
	}

	OGRRegisterAll();

	gdalDataSet = (GDALDataset*)GDALOpenEx( opts.shapeFilename, GDAL_OF_VECTOR, NULL, NULL, NULL );
	if( gdalDataSet == NULL )
	{
		printf( "Could not open shapefile %s\n", opts.shapeFilename );
		exit( 1 );
	}

	layerName = (char *)malloc( strlen( opts.shapeFilename ) + 1 );
	strncpy( layerName, basename( (char *)&opts.shapeFilename[0] ), strlen( opts.shapeFilename ) );
	char *dotPos = strchr( layerName, '.' );
	if( dotPos != NULL )
	{
		*dotPos = 0x00;
	}

	layer = gdalDataSet->GetLayerByName( layerName );
	if( layer == NULL )
	{
		printf( "Could not open layer %s\n", layerName );
		free( layerName );
		exit( 1 );
	}
	layer->ResetReading();
	free( layerName );

	while( (geom == NULL) && (feature = layer->GetNextFeature()) != NULL )
	{
		if( (geom = feature->GetGeometryRef()) == NULL )
			OGRFeature::DestroyFeature( feature );
	}

	if( geom == NULL )
	{
		printf( "Could not find geometry.\n" );
		exit( 1 );
	}

	if( opts.inType == within_LatLon )
	{
	    testPoint = new OGRPoint( opts.lon, opts.lat );
	}
	else
	{
		printf( "NOT IMPLEMENTED\n" );
		exit( 1 );
	}

	// if( testPoint->Within( geom ) )
	if( testPoint->Intersects( geom ) )
	{
		printf( "Inside\n" );
	}
	else
	{
		printf( "Outside\n" );
	}

	testPoint->~OGRPoint();

	OGRFeature::DestroyFeature( feature );
	GDALClose( gdalDataSet );

	exit( 0 );
	
}

/*
int main()
{

	GDALDataset *gdalDataSet;
	OGRLayer *layer;
	OGRFeature *feature;
	OGRGeometry *geom = NULL;
	OGRPoint insideUsa( -95.19, 38.16 );
	OGRPoint outsideUsa( -18.61, 15.49 );
	char *result = "Unk";

	// GDALAllRegister();
	OGRRegisterAll();

	gdalDataSet = (GDALDataset*)GDALOpenEx( "../data/USAMap/USA_adm0.shp", GDAL_OF_VECTOR, NULL, NULL, NULL );
	if( gdalDataSet == NULL ) 
	{
		printf( "Open failed.\n" );
		return( 1 );
	}

	layer = gdalDataSet->GetLayerByName( "USA_adm0" );
	if( layer == NULL )
	{
		return( 1 );
	}

	layer->ResetReading();
	while( (feature = layer->GetNextFeature()) != NULL )
	{
		printf( "Found a feature.\n" );
		geom = feature->GetGeometryRef();
		
		if( outsideUsa.Within( geom ) )
		{
			printf( "Inside.\n" );
		}
		else
		{
			printf( "Outside\n" );
		}

		if( insideUsa.Within( geom ) )
		{
			printf( "Inside.\n" );
		}
		else
		{
			printf( "Outside\n" );
		}

		OGRFeature::DestroyFeature( feature );
	}



	return( 0 );
}
*/
