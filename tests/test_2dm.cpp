/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"
#include "mdal_utils.hpp"

TEST( Mesh2DMTest, MissingFile )
{
  MeshH m = MDAL_LoadMesh( "non/existent/path.2dm" );
  EXPECT_EQ( nullptr, m );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_FileNotFound, s );
}

TEST( Mesh2DMTest, WrongFile )
{
  std::string path = test_file( "/2dm/not_a_mesh_file.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_EQ( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_UnknownFormat, s );
}

TEST( Mesh2DMTest, MeshWithNumberingGaps )
{
  //https://github.com/lutraconsulting/MDAL/issues/51
  std::string path = test_file( "/2dm/mesh_with_numbering_gaps.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 5 );
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 2, f_count );
  MDAL_CloseMesh( m );
}

TEST( Mesh2DMTest, MeshWithUnorderedIds )
{
  std::string path = test_file( "/2dm/unordered_ids.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_EQ( m, nullptr );
}

void _test_QuadAndTriangleFile( const std::string  &path )
{
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int maxCount = MDAL_M_faceVerticesMaximumCount( m );
  EXPECT_EQ( maxCount, 4 );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "2DM" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 5 );
  double x = getVertexXCoordinatesAt( m, 0 );
  double y = getVertexYCoordinatesAt( m, 0 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 1000.0, x );
  EXPECT_DOUBLE_EQ( 2000.0, y );
  EXPECT_DOUBLE_EQ( 20.0, z );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 2, f_count );

  int f_v_count = getFaceVerticesCountAt( m, 0 );
  EXPECT_EQ( 4, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 0, 0 );
  EXPECT_EQ( 0, f_v );

  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( 1000, minX );
  EXPECT_DOUBLE_EQ( 3000, maxX );
  EXPECT_DOUBLE_EQ( 2000, minY );
  EXPECT_DOUBLE_EQ( 3000, maxY );

  f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( f_v_count, 3 ); //triangle
  f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 1, f_v );

  // Bed elevation dataset
  ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  bool onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( true, onVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  double value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 30, value );

  MDAL_CloseMesh( m );
}

TEST( Mesh2DMTest, QuadAndTriangleFile )
{
  std::string path = test_file( "/2dm/quad_and_triangle.2dm" );

  _test_QuadAndTriangleFile( path );
}

TEST( Mesh2DMTest, RegularGridFile )
{
  std::string path = test_file( "/2dm/regular_grid.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );

  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 1976 );
  double x = getVertexXCoordinatesAt( m, 1000 );
  double y = getVertexYCoordinatesAt( m, 1000 );
  EXPECT_DOUBLE_EQ( 381473.785, x );
  EXPECT_DOUBLE_EQ( 168726.985, y );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 1875, f_count );

  int f_v_count = getFaceVerticesCountAt( m, 0 );
  EXPECT_EQ( 4, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 0, 0 );
  EXPECT_EQ( 0, f_v );

  MDAL_CloseMesh( m );
}

TEST( Mesh2DMTest, Basement3CellElevationTest )
{
  std::string path = test_file( "/xdmf/basement3/SimpleChannel/SimpleChannel.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int maxCount = MDAL_M_faceVerticesMaximumCount( m );
  EXPECT_EQ( maxCount, 4 );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "2DM" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 54 );
  double x = getVertexXCoordinatesAt( m, 4 );
  double y = getVertexYCoordinatesAt( m, 4 );
  double z = getVertexZCoordinatesAt( m, 4 );
  EXPECT_DOUBLE_EQ( 8.0, x );
  EXPECT_DOUBLE_EQ( 0.0, y );
  EXPECT_DOUBLE_EQ( 0.0, z );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 77, f_count );

  int f_v_count = getFaceVerticesCountAt( m, 0 );
  EXPECT_EQ( 3, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 0, 0 );
  EXPECT_EQ( 29, f_v );

  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( 0, minX );
  EXPECT_DOUBLE_EQ( 20, maxX );
  EXPECT_DOUBLE_EQ( 0, minY );
  EXPECT_DOUBLE_EQ( 5, maxY );

  f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( f_v_count, 3 ); //triangle
  f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 0, f_v );

  // Bed elevation dataset
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  {
    DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    bool onVertices = MDAL_G_isOnVertices( g );
    EXPECT_EQ( true, onVertices );

    ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    bool active = getActive( ds, 0 );
    EXPECT_EQ( true, active );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 54, count );

    double value = getValue( ds, 1 );
    EXPECT_DOUBLE_EQ( 0, value );
  }

  // Bed elevation dataset and face elevation dataset
  {

    DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Bed Elevation (Face)" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    bool onVertices = MDAL_G_isOnVertices( g );
    EXPECT_EQ( false, onVertices );

    ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    bool active = getActive( ds, 0 );
    EXPECT_EQ( true, active );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 77, count );

    double value = getValue( ds, 1 );
    EXPECT_DOUBLE_EQ( 0.19500000000000001, value );
  }

  MDAL_CloseMesh( m );
}


TEST( Mesh2DMTest, SaveMeshToFile )
{
  //test driver capability
  EXPECT_TRUE( MDAL_DR_SaveMeshCapability( MDAL_driverFromName( "2DM" ) ) );

  //open a mesh
  std::string pathSource = test_file( "/2dm/quad_and_triangle.2dm" );
  MeshH meshToSave = MDAL_LoadMesh( pathSource.c_str() );

  //save the mesh
  std::string fileNameToSave = tmp_file( "/quad_and_triangle_saveTest.2dm" );
  MDAL_SaveMesh( meshToSave, fileNameToSave.c_str(), "2DM" );

  MDAL_CloseMesh( meshToSave );

  //open the saved mesh and test it (same as QuadAndTriangleFile test)
  _test_QuadAndTriangleFile( fileNameToSave );

  std::remove( fileNameToSave.c_str() );
}


int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret =  RUN_ALL_TESTS();
  finalize_test();
  return ret;
}

