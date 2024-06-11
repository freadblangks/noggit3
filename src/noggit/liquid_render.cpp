// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/liquid_layer.hpp>
#include <noggit/Log.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/World.h>
#include <opengl/context.hpp>
#include <opengl/scoped.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <string>


liquid_layer_ubo_data liquid_render::ubo_data(int liquid_id)
{
  auto& it = _liquids_ubo_data.find(liquid_id);

  if (it != _liquids_ubo_data.end())
  {
    return it->second;
  }
  else
  {
    std::string filename;
    liquid_layer_ubo_data data;
    try
    {
      DBCFile::Record lLiquidTypeRow = gLiquidTypeDB.getByID(liquid_id);

      data.liquid_type = lLiquidTypeRow.getInt(LiquidTypeDB::Type);
      data.param.x = lLiquidTypeRow.getFloat(LiquidTypeDB::AnimationX);
      data.param.y = lLiquidTypeRow.getFloat(LiquidTypeDB::AnimationY);
      data.param.z = 0.f;
      data.param.w = 0.f;

      // fix to not crash when using procedural water (id 100)
      if (lLiquidTypeRow.getInt(LiquidTypeDB::ShaderType) == 3)
      {
        filename = "XTextures\\river\\lake_a.%d.blp";
        // default param for water
        data.param.x = 1.f;
        data.param.y = 0.f;
      }
      else
      {
        filename = lLiquidTypeRow.getString(LiquidTypeDB::TextureFilenames);
      }
    }
    catch (...)
    {
      // Fallback, when there is no information.
      filename = "XTextures\\river\\lake_a.%d.blp";

      data.param = math::vector_4d(1.f, 0.f, 0.f, 0.f);
      data.liquid_type = 0;
    }

    filename = noggit::mpq::normalized_filename(filename);

    std::vector<std::string> textures;
    for (int i = 1;; ++i)
    {
      std::string file = boost::str(boost::format(filename) % i);

      if (MPQFile::exists(file))
      {
        textures.emplace_back(file);
      }
      else
      {
        break;
      }
    }

    if (textures.empty())
    {
      textures.emplace_back("textures/shanecube.blp");
    }

    data.texture_count = textures.size();

    blp_texture tex0(textures[0]);
    tex0.finishLoading();

    GLint format = tex0.texture_format();

    bool space_found = false;

    for (int i = 0; i < _texture_arrays.size(); ++i)
    {
      if (_textures_used_per_array[i] + data.texture_count <= textures_per_array && _arrays_format[i] == format)
      {
        data.array_id = i;
        data.id_start_in_array = _textures_used_per_array[i];
        space_found = true;

        break;
      }
    }

    if (!space_found)
    {
      data.array_id = _texture_arrays.size();
      data.id_start_in_array = 0;

      _textures_used_per_array[data.array_id] = 0;

      opengl::texture::set_active_texture(data.array_id);
      _texture_arrays.emplace_back();
      _texture_arrays[data.array_id].bind();
      _arrays_format.push_back(format);

      // todo: use a loop
      gl.texImage3D(GL_TEXTURE_2D_ARRAY, 0, format, 256, 256, textures_per_array, 0, GL_RGBA, GL_FLOAT, NULL);
      gl.texImage3D(GL_TEXTURE_2D_ARRAY, 1, format, 128, 128, textures_per_array, 0, GL_RGBA, GL_FLOAT, NULL);
      gl.texImage3D(GL_TEXTURE_2D_ARRAY, 2, format, 64, 64, textures_per_array, 0, GL_RGBA, GL_FLOAT, NULL);
      gl.texImage3D(GL_TEXTURE_2D_ARRAY, 3, format, 32, 32, textures_per_array, 0, GL_RGBA, GL_FLOAT, NULL);
      gl.texImage3D(GL_TEXTURE_2D_ARRAY, 4, format, 16, 16, textures_per_array, 0, GL_RGBA, GL_FLOAT, NULL);
      gl.texImage3D(GL_TEXTURE_2D_ARRAY, 5, format, 8, 8, textures_per_array, 0, GL_RGBA, GL_FLOAT, NULL);
      gl.texImage3D(GL_TEXTURE_2D_ARRAY, 6, format, 4, 4, textures_per_array, 0, GL_RGBA, GL_FLOAT, NULL);
      gl.texImage3D(GL_TEXTURE_2D_ARRAY, 7, format, 2, 2, textures_per_array, 0, GL_RGBA, GL_FLOAT, NULL);
      gl.texImage3D(GL_TEXTURE_2D_ARRAY, 8, format, 1, 1, textures_per_array, 0, GL_RGBA, GL_FLOAT, NULL);
      gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 8);
    }
    else
    {
      opengl::texture::set_active_texture(data.array_id);
      _texture_arrays[data.array_id].bind();
    }

    // todo: check for dimensions even though it should be 256x256
    for (int i = 0; i < data.texture_count; ++i)
    {
      // no need to re-load the same texture
      if (i == 0)
      {
        tex0.upload_to_currently_bound_array(data.id_start_in_array + i);
      }
      else
      {
        blp_texture tex(textures[i]);
        tex.finishLoading();
        tex.upload_to_currently_bound_array(data.id_start_in_array + i);
      }
    }

    _textures_used_per_array[data.array_id] += data.texture_count;
    _liquids_ubo_data[liquid_id] = data;

    return data;
  }
}

void liquid_render::bind_arrays()
{
  for (int i = 0; i < _texture_arrays.size(); ++i)
  {
    opengl::texture::set_active_texture(i);
    _texture_arrays[i].bind();
  }
}
