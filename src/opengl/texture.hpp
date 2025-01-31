// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/types.hpp>

#include <optional>

namespace opengl
{
  class texture
  {
  public:
    texture();
    ~texture();

    texture (texture const&) = delete;
    texture (texture&&);
    texture& operator= (texture const&) = delete;
    texture& operator= (texture&&);

    virtual void bind();

    static void set_active_texture (size_t num = 0);
    static size_t current_active_texture;

  protected:
    typedef GLuint internal_type;

    internal_type _id;
  };

  class texture_array : texture
  {
  public:
    virtual void bind() override;

#ifdef USE_BINDLESS_TEXTURES
    GLuint64 get_resident_handle();
#endif
  private:
    std::optional<GLuint64> _handle;
  };
}
