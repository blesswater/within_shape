import sys
import sqlite3
from osgeo import ogr
from osgeo import gdal
import math
import os.path

def usage():
	print( "\n" )
	print( "removeShape <shape file (shp)> <sqlite_db>\n" )
	print( "\n" )

class GeoTrans:

	def deg2num(self, lat_deg, lon_deg, zoom):

		"""Return the (x, y) index for the tile corresponding with a (lat,lon) point"""

		lat_rad = math.radians(lat_deg)

		n = 2.0 ** zoom

		xtile = int((lon_deg + 180.0) / 360.0 * n)

		ytile = int((1.0 - math.log(math.tan(lat_rad) + (1 / math.cos(lat_rad))) / math.pi) / 2.0 * n)

		return (xtile, ytile)



	def num2deg(self, ytile, xtile, zoom):

		"""Return the lat and lon coordinates of the center of the tile with index (xtile, ytile)"""
		n = 2.0 ** zoom

		lon_deg = xtile / n * 360.0 - 180.0

		lat_rad = math.atan(math.sinh(math.pi * (1 - 2 * ytile / n)))

		lat_deg = math.degrees(lat_rad)

		return (lat_deg, lon_deg)



	def tile2long(self, x, z):

		"""Return the longitude for tile with index x and zoom z"""

		return (x / math.pow(2, z) * 360 - 180)



	def tile2lat(self, y, z):

		n = math.pi - 2 * math.pi * y / math.pow(2, z)

		return (180 / math.pi * math.atan(0.5 * (math.exp(n) - math.exp(-n))))

	def rowcol2deg( self, row, col, zoom ):

		ty = (1 << zoom) - 1 - row

		return self.num2deg( ty, col, zoom )




if __name__=="__main__":

	if( len(sys.argv) != 3 ):
		usage()
		quit()

	mbTileFilename = str(sys.argv[2])
	shapeFile = str( sys.argv[1] )
	xlate = GeoTrans()

	

	try:
		conn = sqlite3.connect( mbTileFilename )
		cursor = conn.cursor()
	except err:
		print( "Cannot open %s\n", mbTileFilename )
		quit()

	driver = ogr.GetDriverByName( 'ESRI Shapefile' )
	filterFile = driver.Open( shapeFile )
	layerName = os.path.basename( shapeFile ).split('.')[0]
	layer = filterFile.GetLayerByName( layerName )
	featCnt = layer.GetFeatureCount()
	if( featCnt != 1 ):
		print( "Only handles 1 feature. featureCon = %d\n", featCnt )
		exit()


	layer.ResetReading()
	# geom = layer.GetNextFeature().GetGeometryRef()
	feat = layer.GetNextFeature()
	geom = feat.GetGeometryRef()

	sql = "SELECT tile_row AS tile_y, tile_column AS tile_x, zoom_level AS zoom FROM tiles"

	cnt = 0
	for row in cursor.execute( sql ):

		cnt = cnt + 1
		(lat, lon) = xlate.rowcol2deg( row[0], row[1], row[2] )

		pt = ogr.CreateGeometryFromWkt( "POINT( " + str(lon) + " " + str(lat) + ")" )

		# if( pt.Within( geom ) ):
		# if( geom.Contains( pt ) ):
		if( pt.Intersects( geom ) ):
			print( str(cnt) + " " + str(lat) + "," + str(lon) + " Inside" )
			sqlDelete = "DELETE FROM tiles WHERE "
			sqlDelete += "tile_row=" + str(row[0]) + " and "
			sqlDelete += "tile_column=" + str(row[1])

			conn.execute( sqlDelete );
		else:
			print( str(cnt) + " " + str(lat) + "," + str(lon) + " Outside" )

	conn.commit()
	conn.execute("VACUUM")
	conn.close()

	


	
