// dx9runtime.cpp
#include "dx9runtime.hpp"

#include "../../dx9/dx9window.hpp"
#include "../../dx9/dx9pixelshader.hpp"
#include "../../dx9/dx9vertexshader.hpp"
#include "../../dx9/dx9texture.hpp"

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

namespace brook
{
  static const char* kPassthroughVertexShaderSource =
    "vs.1.1\n"
    "dcl_position v0\n"
    "dcl_texcoord0 v1\n"
    "dcl_texcoord1 v2\n"
    "dcl_texcoord2 v3\n"
    "dcl_texcoord3 v4\n"
    "dcl_texcoord4 v5\n"
    "dcl_texcoord5 v6\n"
    "dcl_texcoord6 v7\n"
    "dcl_texcoord7 v8\n"
    "mov oPos, v0\n"
    "mov oT0, v1\n"
    "mov oT1, v2\n"
    "mov oT2, v3\n"
    "mov oT3, v4\n"
    "mov oT4, v5\n"
    "mov oT5, v6\n"
    "mov oT6, v7\n"
    "mov oT7, v8\n"
    ;

  static const char* kPassthroughPixelShaderSource =
    "ps_2_0\n"
    "dcl t0.xy\n"
    "dcl_2d s0\n"
    "texld r0, t0, s0\n"
    "mov oC0, r0\n"
    ;

  static const D3DVERTEXELEMENT9 kDX9VertexElements[] =
  {
    { 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 4*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
    { 0, 8*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
    { 0, 12*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
    { 0, 16*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },
    { 0, 20*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4 },
    { 0, 24*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 5 },
    { 0, 28*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 6 },
    { 0, 32*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 7 },
    D3DDECL_END()
  };

  struct DX9Vertex
  {
    float4 position;
    float4 texcoords[8]; // TIM: TODO: named constant
  };

  class GPUContextDX9 : public GPUContext
  {
  public:
    static GPUContextDX9* create();

    virtual bool isValidShaderNameString( const char* inNameString ) const;

    virtual float4 getStreamIndexofConstant( TextureHandle inTexture ) const;
    virtual float4 getStreamGatherConstant( TextureHandle inTexture ) const;

    virtual void
      get1DInterpolant( const float4 &start, 
      const float4 &end,
      const unsigned int outputWidth,
      GPUInterpolant &interpolant) const;

    virtual void 
      get2DInterpolant( const float2 &start, 
      const float2 &end,
      const unsigned int outputWidth,
      const unsigned int outputHeight, 
      GPUInterpolant &interpolant) const;

    virtual void 
      getStreamInterpolant( const TextureHandle texture,
      const unsigned int outputWidth,
      const unsigned int outputHeight, 
      GPUInterpolant &interpolant) const;

    virtual void
      getStreamOutputRegion( const TextureHandle texture,
      GPURegion &region) const; 

    virtual TextureHandle createTexture2D( size_t inWidth, size_t inHeight, TextureFormat inFormat );
    virtual void releaseTexture( TextureHandle inTexture );

    virtual void setTextureData( TextureHandle inTexture, const float* inData, size_t inStrideBytes, size_t inComponentCount );
    virtual void getTextureData( TextureHandle inTexture, float* outData, size_t inStrideBytes, size_t inComponentCount );

    virtual PixelShaderHandle createPixelShader( const char* inSource );

    VertexShaderHandle createVertexShader( const char* inSource );

    virtual VertexShaderHandle getPassthroughVertexShader() {
      return _passthroughVertexShader;
    }

    virtual PixelShaderHandle getPassthroughPixelShader() {
      return _passthroughPixelShader;
    }

    virtual void beginScene();
    virtual void endScene();

    virtual void bindConstant( size_t inIndex, const float4& inValue );
    virtual void bindTexture( size_t inIndex, TextureHandle inTexture );
    virtual void bindOutput( size_t inIndex, TextureHandle inTexture );
    virtual void disableOutput( size_t inIndex );

    virtual void bindPixelShader( PixelShaderHandle inPixelShader );
    virtual void bindVertexShader( VertexShaderHandle inVertexShader );

    virtual void drawRectangle(
      const GPURegion& outputRegion, 
      const GPUInterpolant* interpolants, 
      unsigned int numInterpolants );

  private:
    GPUContextDX9();
    bool initialize();

    DX9Window* _window;
    IDirect3D9* _direct3D;
    IDirect3DDevice9* _device;

    VertexShaderHandle _passthroughVertexShader;
    PixelShaderHandle _passthroughPixelShader;
    IDirect3DVertexBuffer9* _vertexBuffer;
    IDirect3DVertexDeclaration9* _vertexDecl;

    enum {
      kMaximumSamplerCount = 16,
      kMaximumOutputCount = 4
    };

    DX9Texture* _boundTextures[16];
    DX9Texture* _boundOutputs[4];
  };

  GPURuntimeDX9* GPURuntimeDX9::create()
  {
    GPUContextDX9* context = GPUContextDX9::create();
    if( !context )
      return NULL;

    GPURuntimeDX9* result = new GPURuntimeDX9();
    if( !result->initialize( context ) )
    {
      delete context;
      return NULL;
    }

    return result;
  }

  GPUContextDX9* GPUContextDX9::create()
  {
    GPUContextDX9* result = new GPUContextDX9();
    if( result->initialize() )
      return result;
    delete result;
    return NULL;
  }

  GPUContextDX9::GPUContextDX9()
  {
  }

  bool GPUContextDX9::initialize()
  {
    _window = DX9Window::create();
    if( _window == NULL )
    {
      DX9WARN << "Could not create offscreen window.";
      return false;
    }

    HWND windowHandle = _window->getWindowHandle();

    _direct3D = Direct3DCreate9( D3D_SDK_VERSION );
    if( _direct3D == NULL )
    {
      DX9WARN << "Could not create Direct3D interface.";
      return false;
    }

    D3DPRESENT_PARAMETERS deviceDesc;
    ZeroMemory( &deviceDesc, sizeof(deviceDesc) );

    deviceDesc.Windowed = TRUE;
    deviceDesc.SwapEffect = D3DSWAPEFFECT_DISCARD;
    deviceDesc.BackBufferFormat = D3DFMT_UNKNOWN;
    deviceDesc.EnableAutoDepthStencil = FALSE;
    deviceDesc.AutoDepthStencilFormat = D3DFMT_D24S8;

    HRESULT result = _direct3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, windowHandle,
      D3DCREATE_HARDWARE_VERTEXPROCESSING, &deviceDesc, &_device );
    if( FAILED(result) )
    {
      DX9WARN << "Could not create Direct3D device.";
      return false;
    }

    // create vertex buffer
    static const int kMaxVertexCount = 64;

    result = _device->CreateVertexBuffer(
      kMaxVertexCount*sizeof(DX9Vertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &_vertexBuffer, NULL );
    DX9AssertResult( result, "CreateVertexBuffer failed" );

    result = _device->CreateVertexDeclaration( kDX9VertexElements, &_vertexDecl );
    DX9AssertResult( result, "CreateVertexDeclaration failed" );


    // TIM: set up initial state
    result = _device->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
    GPUAssert( !FAILED(result), "SetRenderState failed" );

    _passthroughVertexShader = createVertexShader( kPassthroughVertexShaderSource );
    _passthroughPixelShader = createPixelShader( kPassthroughPixelShaderSource );

    for( size_t i = 0; i < kMaximumOutputCount; i++ )
      _boundOutputs[i] = NULL;
    for( size_t t = 0; t < kMaximumSamplerCount; t++ )
      _boundTextures[t] = NULL;

    return true;
  }

  bool GPUContextDX9::isValidShaderNameString( const char* inNameString ) const
  {
    return strcmp( "ps20", inNameString ) == 0;
  }

  float4 GPUContextDX9::getStreamIndexofConstant( TextureHandle inTexture ) const
  {
    DX9Texture* texture = (DX9Texture*)inTexture;
    int textureWidth = texture->getWidth();
    int textureHeight = texture->getHeight();

    return float4( (float)textureWidth, (float)textureHeight, 0, 0 );
  }

  float4 GPUContextDX9::getStreamGatherConstant( TextureHandle inTexture ) const
  {
    DX9Texture* texture = (DX9Texture*)inTexture;
    int textureWidth = texture->getWidth();
    int textureHeight = texture->getHeight();
    float scaleX = 1.0f / (textureWidth);
    float scaleY = 1.0f / (textureHeight);
    float offsetX = 1.0f / (1 << 15);//0.5f / width;
    float offsetY = 1.0f / (1 << 15);//0.5f / height;

    return float4( scaleX, scaleY, offsetX, offsetY );
  }

  void GPUContextDX9::get1DInterpolant( const float4 &start, 
    const float4 &end,
    const unsigned int outputWidth,
    GPUInterpolant &interpolant) const
  {
    float4 f1 = start;

    float4 f2;
    f2.x = end.x + end.x - start.x;
    f2.y = end.y + end.y - start.y;
    f2.z = end.z + end.z - start.z;
    f2.w = end.w + end.w - start.w;

    interpolant.vertices[0] = f1;
    interpolant.vertices[1] = f2; 
    interpolant.vertices[2] = f1;
  }

  void GPUContextDX9::get2DInterpolant( const float2 &start, 
    const float2 &end,
    const unsigned int outputWidth,
    const unsigned int outputHeight, 
    GPUInterpolant &interpolant) const
  {
    float4 f1 = float4( start.x, start.y, 0, 1 );

    float4 f2;
    f2.x = end.x + end.x - start.x;
    f2.y = start.y;
    f2.z = 0;
    f2.w = 1;

    float4 f3;
    f3.x = start.x;
    f3.y = end.y + end.y - start.y;
    f3.z = 0;
    f3.w = 1;


    interpolant.vertices[0] = f1;
    interpolant.vertices[1] = f2; 
    interpolant.vertices[2] = f3;
  }

  void GPUContextDX9::getStreamInterpolant( const TextureHandle texture,
    const unsigned int outputWidth,
    const unsigned int outputHeight, 
    GPUInterpolant &interpolant) const
  {
    interpolant.vertices[0] = float4(0,0,0.5,1);
    interpolant.vertices[1] = float4(2,0,0.5,1);
    interpolant.vertices[2] = float4(0,2,0.5,1);
  }

  void GPUContextDX9::getStreamOutputRegion( const TextureHandle texture,
    GPURegion &region) const
  {
    region.vertices[0] = float4(-1,1,0.5,1);
    region.vertices[1] = float4(3,1,0.5,1);
    region.vertices[2] = float4(-1,-3,0.5,1);
  }

  GPUContextDX9::TextureHandle GPUContextDX9::createTexture2D( size_t inWidth, size_t inHeight, TextureFormat inFormat )
  {
    int components;
    switch( inFormat )
    {
    case kTextureFormat_Float1:
      components = 1;
      break;
    case kTextureFormat_Float2:
      components = 2;
      break;
    case kTextureFormat_Float3:
      components = 3;
      break;
    case kTextureFormat_Float4:
      components = 4;
      break;
    default:
      GPUError("Unknown format for DX9 Texture");
      return 0;
      break;
    }
    DX9Texture* result = DX9Texture::create( _device, inWidth, inHeight, components );
    return result;
  }

  void GPUContextDX9::releaseTexture( TextureHandle inTexture )
  {
    DX9Texture* texture = (DX9Texture*)inTexture;
    delete texture;
  }

  void GPUContextDX9::setTextureData( TextureHandle inTexture, const float* inData, size_t inStrideBytes, size_t inComponentCount )
  {
    DX9Texture* texture = (DX9Texture*)inTexture;
    texture->setData( inData, inStrideBytes, inComponentCount );
    texture->markShadowDataChanged();
  }

  void GPUContextDX9::getTextureData( TextureHandle inTexture, float* outData, size_t inStrideBytes, size_t inComponentCount )
  {
    DX9Texture* texture = (DX9Texture*)inTexture;
    texture->getData( outData, inStrideBytes, inComponentCount );
  }

  GPUContextDX9::PixelShaderHandle GPUContextDX9::createPixelShader( const char* inSource )
  {
    IDirect3DPixelShader9* shader;
    HRESULT result;
    LPD3DXBUFFER codeBuffer;
    LPD3DXBUFFER errorBuffer;

    result = D3DXAssembleShader( inSource, strlen(inSource), NULL, NULL, 0, &codeBuffer, &errorBuffer );
    if( errorBuffer != NULL )
    {
      const char* errorMessage = (const char*)errorBuffer->GetBufferPointer();
      GPUWARN << "Pixel shader failed to compile:\n" << errorMessage;
      return NULL;
    }
    else if( FAILED(result) )
    {
      GPUWARN << "Pixel shader failed to compile.";
      return NULL;
    }

    result = _device->CreatePixelShader( (DWORD*)codeBuffer->GetBufferPointer(), &shader );
    codeBuffer->Release();

    if( FAILED(result) )
    {
      DX9WARN << "Failed to allocate pixel shader.";
      return NULL;
    }

    return (PixelShaderHandle)shader;
  }

  GPUContextDX9::VertexShaderHandle GPUContextDX9::createVertexShader( const char* inSource )
  {
    IDirect3DVertexShader9* shader;
    HRESULT result;
    LPD3DXBUFFER codeBuffer;
    LPD3DXBUFFER errorBuffer;

    result = D3DXAssembleShader( inSource, strlen(inSource), NULL, NULL, 0, &codeBuffer, &errorBuffer );
    if( errorBuffer != NULL )
    {
      const char* errorMessage = (const char*)errorBuffer->GetBufferPointer();
      GPUWARN << "Vertex shader failed to compile:\n" << errorMessage;
      return NULL;
    }
    else if( FAILED(result) )
    {
      GPUWARN << "Vertex shader failed to compile.";
      return NULL;
    }

    result = _device->CreateVertexShader( (DWORD*)codeBuffer->GetBufferPointer(), &shader );
    codeBuffer->Release();

    if( FAILED(result) )
    {
      DX9WARN << "Failed to allocate vertex shader.";
      return NULL;
    }

    return (VertexShaderHandle)shader;
  }

  void GPUContextDX9::beginScene()
  {
    HRESULT result = _device->BeginScene();
    GPUAssert( !FAILED(result), "BeginScene failed" );
  }

  void GPUContextDX9::endScene()
  {
    HRESULT result = _device->EndScene();
    GPUAssert( !FAILED(result), "BeginScene failed" );

    for( size_t i = 0; i < kMaximumSamplerCount; i++ )
      _boundTextures[i] = NULL;

    for( size_t j = 0; j < kMaximumOutputCount; j++ )
      _boundOutputs[j] = NULL;
  }


  void GPUContextDX9::bindConstant( size_t inIndex, const float4& inValue )
  {
    HRESULT result = _device->SetPixelShaderConstantF( inIndex, (const float*)&inValue, 1 );
    GPUAssert( !FAILED(result), "SetPixelShaderConstantF failed" );
  }

  void GPUContextDX9::bindTexture( size_t inIndex, TextureHandle inTexture )
  {
    DX9Texture* texture = (DX9Texture*)inTexture;
    _boundTextures[inIndex] = texture;
    HRESULT result = _device->SetTexture( inIndex, texture->getTextureHandle() );
    GPUAssert( !FAILED(result), "SetTexture failed" );
  }

  void GPUContextDX9::bindOutput( size_t inIndex, TextureHandle inTexture )
  {
    DX9Texture* texture = (DX9Texture*)inTexture;
    _boundOutputs[inIndex] = texture;
    HRESULT result = _device->SetRenderTarget( inIndex, texture->getSurfaceHandle() );
    GPUAssert( !FAILED(result), "SetRenderTarget failed" );
  }

  void GPUContextDX9::disableOutput( size_t inIndex )
  {
    HRESULT result = _device->SetRenderTarget( inIndex, NULL );
    GPUAssert( !FAILED(result), "SetRenderTarget failed" );
  }

  void GPUContextDX9::bindPixelShader( PixelShaderHandle inPixelShader )
  {
    HRESULT result = _device->SetPixelShader( (IDirect3DPixelShader9*)inPixelShader );
    GPUAssert( !FAILED(result), "SetPixelShader failed" );
  }

  void GPUContextDX9::bindVertexShader( VertexShaderHandle inVertexShader )
  {
    HRESULT result = _device->SetVertexShader( (IDirect3DVertexShader9*)inVertexShader );
    GPUAssert( !FAILED(result), "SetVertexShader failed" );
  }

  void GPUContextDX9::drawRectangle(
    const GPURegion& inOutputRegion, 
    const GPUInterpolant* inInterpolants, 
    unsigned int inInterpolantCount )
  {
    HRESULT result;

//    result = _device->Clear( 0, NULL, D3DCLEAR_TARGET, 0, 0.0, 0 );
//    DX9AssertResult( result, "Clear failed" );

    DX9Vertex* vertices;
    result = _vertexBuffer->Lock( 0, 0, (void**)&vertices, D3DLOCK_DISCARD );
    DX9AssertResult( result, "VB::Lock failed" );

    DX9Assert( inInterpolantCount <= 8,
      "Can't have more than 8 texture coordinate interpolators" );

    DX9Vertex vertex;
    for( size_t i = 0; i < 3; i++ )
    {
      float4 position = inOutputRegion.vertices[i];

      // TIM: bad
      vertex.position.x = position.x;
      vertex.position.y = position.y;
      vertex.position.z = 0.5f;
      vertex.position.w = 1.0f;

      for( size_t t = 0; t < inInterpolantCount; t++ )
        vertex.texcoords[t] = inInterpolants[t].vertices[i];

      *vertices++ = vertex;
    }
    result = _vertexBuffer->Unlock();
    DX9AssertResult( result, "VB::Unlock failed" );

    result = _device->SetVertexDeclaration( _vertexDecl );
    DX9AssertResult( result, "SetVertexDeclaration failed" );

    result = _device->SetStreamSource( 0, _vertexBuffer, 0, sizeof(DX9Vertex) );
    DX9AssertResult( result, "SetStreamSource failed" );

    for( size_t t = 0; t < kMaximumSamplerCount; t++ )
    {
      if( _boundTextures[t] )
        _boundTextures[t]->validateCachedData();
    }

    result = _device->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 1 );
    DX9AssertResult( result, "DrawPrimitive failed" );

    for( size_t j = 0; j < kMaximumOutputCount; j++ )
    {
      if( _boundOutputs[j] )
        _boundOutputs[j]->markCachedDataChanged();
    }
  }
}

