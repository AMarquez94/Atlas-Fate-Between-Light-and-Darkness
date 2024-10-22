#pragma once

#include "mcv_platform.h"

namespace GUI
{
  struct TParams
  {
    bool _visible = true;
    VEC2 _size;
    VEC2 _position;
    float _rotation = 0.f;
    VEC2 _scale;
  };

  struct TImageParams
  {
    const CTexture* _texture = nullptr;
    VEC4 _color;
    VEC2 _minUV;
    VEC2 _maxUV;
  };

  struct TTextParams
  {
    enum EAlignment { Left, Center, Right, Top, Bottom };
    EAlignment _vAlign = Left;
    EAlignment _hAlign = Top;
    float _size;
    //const CFont* _font = nullptr;
    std::string _text;
    VEC4 _color;
  };

  struct TBarParams
  {
    enum EDirection { Horizontal, Vertical };
    EDirection _direction;
    float _processValue;
	std::string _variable;
	float _ratio = 1;
  };

  struct TSpriteParams 
  {
	  std::vector<VEC2> _frame_size;
	  std::vector<VEC2> _original_image_size;
	  std::vector<int> _frames_per_second;
	  std::vector<int> _num_frames;
	  int _playing_sprite = 0;
	  std::vector<const CTexture*> _textures;
  };

  struct TButtonParams
  {
    TParams _params;
    TImageParams _imageParams;
    TTextParams _textParams;
    float ratio = 0.0f;
  };

  struct TSubtitlesParams
  {
	  TParams _params;
	  TImageParams _imageParams;
	  int _current_subtitles = 0;
	  const CTexture* _current_texture;
	  std::vector<const CTexture*> _map_to_textures;
  };

  using GUICallback = std::function<void()>;
}