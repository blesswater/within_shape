
#define WITHIN_BAD   0
#define WITHIN_GOOD  1

typedef enum withinInputType_e {
	within_LatLon,
	within_Wkt,
	within_Error
} withinInputType;

typedef struct withinOpts_s {
	withinInputType inType;
	
	double lat, lon;
	char *wkt;
	char *shapeFilename;

} withinOpts;
	
