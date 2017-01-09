#include "/usr/include/gdal/ogrsf_frmts.h"


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
		printf( "Cannot open layer.\n" );
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
