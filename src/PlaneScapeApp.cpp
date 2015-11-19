#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/MotionManager.h"

#include "TouchUI.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ImagePlane {
  geom::Plane plane;
  gl::BatchRef batch;
  vec2 planeSize;
  gl::GlslProgRef shader;
  ColorA color;

public:
  ImagePlane(){}

  ImagePlane( vec2 size, ColorA color ) : planeSize(size), color(color) {
    auto c = geom::Constant( geom::COLOR, color );
    auto lambert = gl::ShaderDef().lambert().color();
    shader = gl::getStockShader( lambert );
    plane = geom::Plane().size( size );
    batch = gl::Batch::create( plane >> c, shader );
  }
  
  void draw(){
    gl::ScopedModelMatrix scpModelMatrix;
    gl::color( color );
    batch->draw();
  }
};


class PlaneScapeApp : public App {
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
  void resize() override;

  static const int NUM_PLANES = 12;
  static constexpr int rows = 3;
  static constexpr int cols = NUM_PLANES / rows;
  static constexpr float padding = .5f;
  float mPlaneWidth = 8.f;
  float mPlaneHeight = 6.f;
  

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

  mCameraPos = vec3( 10, 9, 15 );
  mLookingAt = vec3( 0 );
  //  mCam.lookAt( vec3( 3, 3, 3 ), vec3( 0, 0, 0 ) );
  mCam.setNearClip( 2 );
  mCam.setFarClip( 1000 );
  

  mTouchUi.connect( getWindow() );
  mTouchUi.setPanSpeed( vec2( 0.0067f ) );
  mTouchUi.setScaleMin( vec2( 0.5f ) );

  for (int i = 0; i < NUM_PLANES; i++){
    float rel = i / static_cast<float>(NUM_PLANES);
    mPlanes[i] = ImagePlane(vec2( mPlaneWidth, mPlaneHeight ), ColorA( CM_HSV, rel, 1, 1, .75f ) );
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
  const float xPan = mTouchUi.getPan().y;  // Landscape mode swaps x and y
  const float yPan = -mTouchUi.getPan().x; // and Swiping is negative y motion

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

    mPlanes[i].draw();

    if (beenDone > 0){
      console() << i << " (I): <<<" << std::endl
                << "Model: " << gl::getModelMatrix() << std::endl
                << "View:  " << gl::getViewMatrix() << std::endl
                << "Proj:  " << gl::getProjectionMatrix() << std::endl
                << ">>>" << std::endl << std::endl;
    }

  }

  // for (int i = 0; i < NUM_PLANES; i++){
  //   if (i == NUM_PLANES / NUM_ROWS){
  //     mat4 model = mat4( 1.0 );
  //     model[3] = vec4( mTouchUi.getPan().x, 0, mTouchUi.getPan().y * (4 + padding), 1 );
  //     gl::setModelMatrix( model );
  //   }

  //   gl::translate( offset );
  //   mPlanes[i].draw();

  //   if (!beenDone){
  //     console() << i << " (I): <<<" << std::endl
  //               << "Model: " << gl::getModelMatrix() << std::endl
  //               << "View:  " << gl::getViewMatrix() << std::endl
  //               << "Proj:  " << gl::getProjectionMatrix() << std::endl
  //               << ">>>" << std::endl << std::endl;
  //   }

  // }
  if (beenDone > 0)
    beenDone--;

}

CINDER_APP( PlaneScapeApp, RendererGl, []( App::Settings* settings){
    settings->setHighDensityDisplayEnabled( true );
    settings->setMultiTouchEnabled( true );
} )
