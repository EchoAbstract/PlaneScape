#include <cassert>
#include <string>
#include <vector>

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/MotionManager.h"

#include "TouchUI.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ImageAssetMap {
private:
  static const std::vector<std::string> map;
  static constexpr int numImages = 24;

public:
  static constexpr int numberOfImages(){
    return numImages;
  }

  static std::string getImageName( int index ){
    assert( numImages == map.size() );
    if (index < 0 || index > numImages-1){
      return map[0];
    } else {
      return map[index];
    }
  }
};

const std::vector<std::string> ImageAssetMap::map {
  "794px-1800s_Texas_House.JPG",
    "800px-1000_cubes.jpg",
    "800px-3d-wallpaper-widescreen-3.jpg",
    "800px-3spheres_1pyramid.jpg",
    "800px-88829-wallpaper06.jpg",
    "800px-A_bench_for_resting.jpg",
    "800px-A_close_shot_of_wind_turbines_wind_farm.jpg",
    "800px-Airbus_Wing_01798_changed.jpg",
    "800px-Airplane_reflection.jpg",
    "800x600_Wallpaper_Blue_Sky.png",
    "800px-Clipboard_scr3.jpg",
    "800px-Dolphin_and_venus.jpg",
    "Eye_farm_01.jpg",
    "800px-Erodedcity_01.jpg",
    "799px-Emirgan_04496.jpg",
    "800px-Emeralda_Marsh-Island_looking_west01.jpg",
    "800px-Fanning_Springs_Park_springs05.jpg",
    "800px-Clingmans_Dome-27527-1.jpg",
    "800px-Eastern_Bluebird-27527.jpg",
    "800px-Downtown_skyline_at_night.jpg",
    "800px-Great_Egret_Inching_Closer.JPG",
    "800px-Grayson_Highlands_Ponies-27527-6.jpg",
    "800px-Huella.jpg",
    "800px-Bubbles_in_the_dark.jpg"
};



class ImagePlane {
  geom::Plane plane;
  gl::BatchRef batch;
  vec2 planeSize;
  gl::GlslProgRef shader;
  ColorA color;
  gl::TextureRef texture;
  ImageSourceRef img;

public:
  ImagePlane(){}

  ImagePlane( vec2 size, ColorA color, ImageSourceRef img ) : planeSize(size), color(color), img(img) {

    texture = gl::Texture::create( img );
    texture->bind();
    
    auto rot = geom::Rotate( M_PI, vec3( 0, 1, 0 ) );

    auto c = geom::Constant( geom::COLOR, color );
    auto lambert = gl::ShaderDef().texture().lambert();
    shader = gl::getStockShader( lambert );
    plane = geom::Plane().size( size );
    batch = gl::Batch::create( plane >> rot, shader );
  }
  
  void draw(){
    gl::ScopedModelMatrix scpModelMatrix;
    batch->draw();
  }

  gl::TextureRef getTexture(){
    return texture;
  }
};


class PlaneScapeApp : public App {
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
  void resize() override;

  static const int NUM_PLANES = ImageAssetMap::numberOfImages();
  static constexpr int rows = 4;
  static constexpr int cols = NUM_PLANES / rows;
  static constexpr float padding = .5f;
  float mPlaneWidth = 800.f;
  float mPlaneHeight = 600.f;
  

  CameraPersp mCam;
  vec3 mCameraPos;
  vec3 mLookingAt;

  ImagePlane mPlanes[NUM_PLANES];

  int beenDone = 1;

  TouchUi mTouchUi;
};

void PlaneScapeApp::setup(){
  mPlaneHeight = 2.f;
  mPlaneWidth = 2.f;

  MotionManager::enable();

  mCameraPos = vec3( 5, 5, 5 );
  mLookingAt = vec3( 0 );
  mCam.setNearClip( .25 );
  mCam.setFarClip( 1000 );
  

  mTouchUi.connect( getWindow() );
  mTouchUi.setPanSpeed( vec2( 0.01f ) );
  mTouchUi.setScaleMin( vec2( 0.25f ) );

  for (int i = 0; i < NUM_PLANES; i++){
    // Get color relative index
    float rel = i / static_cast<float>(NUM_PLANES);
    
    std::string imgName = ImageAssetMap::getImageName( i );
    auto img = loadImage( loadAsset( imgName ) );

    mPlanes[i] = ImagePlane(vec2( mPlaneWidth, mPlaneHeight ), ColorA( CM_HSV, rel, 1, 1, .75f ), img );
  }

  gl::enableDepthRead();
  gl::enableDepthWrite();

}

void PlaneScapeApp::mouseDown( MouseEvent event ){
}

void PlaneScapeApp::update(){
}


void PlaneScapeApp::resize(){
  mCam.setAspectRatio( getWindowAspectRatio() );
  mCam.setFov( 65.f );
  mCam.setWorldUp( vec3( 0, 1, 0 ) );
  mCam.lookAt( mCameraPos, mLookingAt );

  mTouchUi.setMask( getWindowBounds() );

}


void PlaneScapeApp::draw(){
	gl::clear( Color( 0, 0, 0 ) ); 

  const float scale = mTouchUi.getScale().x;
  const float xPan = -mTouchUi.getPan().x;
  const float yPan = -mTouchUi.getPan().y;

  vec3 camPos( mCameraPos.x + xPan, mCameraPos.y, mCameraPos.z + yPan );
  mCam.lookAt( scale * camPos, mLookingAt );

  mCam.setOrientation( MotionManager::getRotation() );


  gl::setMatrices( mCam );
  gl::ScopedModelMatrix scpModelMatrix;

  for (int i = 0; i < NUM_PLANES; i++){
    int row = floor( i / cols );
    int col = i % cols;
    // Compute offsets, NUM_PLANES divided into 3 rows
    float xOffset = (mPlaneWidth + padding) * col;
    float zOffset = (mPlaneHeight + padding) * row;
    mat4 model( 1.0 );
    model[3] = vec4( xOffset, 0.f, zOffset, 1 );

    gl::setModelMatrix( model );
    gl::rotate( mTouchUi.getRotation(), vec3( 1.0f ) );

    mPlanes[i].getTexture()->bind();
    mPlanes[i].draw();

    if (beenDone > 0){
      console() << i << " (I): <<<" << std::endl
                << "Model: " << gl::getModelMatrix() << std::endl
                << "View:  " << gl::getViewMatrix() << std::endl
                << "Proj:  " << gl::getProjectionMatrix() << std::endl
                << ">>>" << std::endl << std::endl;
    }

  }

  if (beenDone > 0)
    beenDone--;

}

CINDER_APP( PlaneScapeApp, RendererGl, []( App::Settings* settings){
    settings->setHighDensityDisplayEnabled( true );
    settings->setMultiTouchEnabled( true );
} )
