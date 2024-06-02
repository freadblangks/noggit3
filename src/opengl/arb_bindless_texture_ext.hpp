// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/types.hpp>

#include <QtGui/QOpenGLFunctions_4_1_Core>
#include <QtOpenGLExtensions/QOpenGLExtensions>

class QOpenGLExtension_ARB_bindless_texturePrivate : public QAbstractOpenGLExtensionPrivate
{
public:
  GLboolean(QOPENGLF_APIENTRYP IsImageHandleResidentARB)(GLuint64 handle);
  GLboolean(QOPENGLF_APIENTRYP IsTextureHandleResidentARB)(GLuint64 handle);
  void (QOPENGLF_APIENTRYP ProgramUniformHandleui64vARB)(GLuint program, GLint location, GLsizei count, const GLuint64* values);
  void (QOPENGLF_APIENTRYP ProgramUniformHandleui64ARB)(GLuint program, GLint location, GLuint64 value);
  void (QOPENGLF_APIENTRYP UniformHandleui64vARB)(GLint location, GLsizei count, const GLuint64* value);
  void (QOPENGLF_APIENTRYP UniformHandleui64ARB)(GLint location, GLuint64 value);
  void (QOPENGLF_APIENTRYP MakeImageHandleNonResidentARB)(GLuint64 handle);
  void (QOPENGLF_APIENTRYP MakeImageHandleResidentARB)(GLuint64 handle, GLenum access);
  GLuint64(QOPENGLF_APIENTRYP GetImageHandleARB)(GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format);
  void (QOPENGLF_APIENTRYP MakeTextureHandleNonResidentARB)(GLuint64 handle);
  void (QOPENGLF_APIENTRYP MakeTextureHandleResidentARB)(GLuint64 handle);
  GLuint64(QOPENGLF_APIENTRYP GetTextureSamplerHandleARB)(GLuint texture, GLuint sampler);
  GLuint64(QOPENGLF_APIENTRYP GetTextureHandleARB)(GLuint texture);
};

class QOpenGLExtension_ARB_bindless_texture : public QAbstractOpenGLExtension
{
public:
  QOpenGLExtension_ARB_bindless_texture();

  bool initializeOpenGLFunctions() Q_DECL_FINAL;

  GLboolean glIsImageHandleResidentARB(GLuint64 handle);
  GLboolean glIsTextureHandleResidentARB(GLuint64 handle);
  void glProgramUniformHandleui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* values);
  void glProgramUniformHandleui64ARB(GLuint program, GLint location, GLuint64 value);
  void glUniformHandleui64vARB(GLint location, GLsizei count, const GLuint64* value);
  void glUniformHandleui64ARB(GLint location, GLuint64 value);
  void glMakeImageHandleNonResidentARB(GLuint64 handle);
  void glMakeImageHandleResidentARB(GLuint64 handle, GLenum access);
  GLuint64 glGetImageHandleARB(GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format);
  void glMakeTextureHandleNonResidentARB(GLuint64 handle);
  void glMakeTextureHandleResidentARB(GLuint64 handle);
  GLuint64 glGetTextureSamplerHandleARB(GLuint texture, GLuint sampler);
  GLuint64 glGetTextureHandleARB(GLuint texture);

protected:
  Q_DECLARE_PRIVATE(QOpenGLExtension_ARB_bindless_texture)
};

inline GLboolean QOpenGLExtension_ARB_bindless_texture::glIsImageHandleResidentARB(GLuint64 handle)
{
  Q_D(QOpenGLExtension_ARB_bindless_texture);
  return d->IsImageHandleResidentARB(handle);
}

inline GLboolean QOpenGLExtension_ARB_bindless_texture::glIsTextureHandleResidentARB(GLuint64 handle)
{
  Q_D(QOpenGLExtension_ARB_bindless_texture);
  return d->IsTextureHandleResidentARB(handle);
}

inline void QOpenGLExtension_ARB_bindless_texture::glProgramUniformHandleui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* values)
{
  Q_D(QOpenGLExtension_ARB_bindless_texture);
  d->ProgramUniformHandleui64vARB(program, location, count, values);
}

inline void QOpenGLExtension_ARB_bindless_texture::glProgramUniformHandleui64ARB(GLuint program, GLint location, GLuint64 value)
{
  Q_D(QOpenGLExtension_ARB_bindless_texture);
  d->ProgramUniformHandleui64ARB(program, location, value);
}

inline void QOpenGLExtension_ARB_bindless_texture::glUniformHandleui64vARB(GLint location, GLsizei count, const GLuint64* value)
{
  Q_D(QOpenGLExtension_ARB_bindless_texture);
  d->UniformHandleui64vARB(location, count, value);
}

inline void QOpenGLExtension_ARB_bindless_texture::glUniformHandleui64ARB(GLint location, GLuint64 value)
{
  Q_D(QOpenGLExtension_ARB_bindless_texture);
  d->UniformHandleui64ARB(location, value);
}

inline void QOpenGLExtension_ARB_bindless_texture::glMakeImageHandleNonResidentARB(GLuint64 handle)
{
  Q_D(QOpenGLExtension_ARB_bindless_texture);
  d->MakeImageHandleNonResidentARB(handle);
}

inline void QOpenGLExtension_ARB_bindless_texture::glMakeImageHandleResidentARB(GLuint64 handle, GLenum access)
{
  Q_D(QOpenGLExtension_ARB_bindless_texture);
  d->MakeImageHandleResidentARB(handle, access);
}

inline GLuint64 QOpenGLExtension_ARB_bindless_texture::glGetImageHandleARB(GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format)
{
  Q_D(QOpenGLExtension_ARB_bindless_texture);
  return d->GetImageHandleARB(texture, level, layered, layer, format);
}

inline void QOpenGLExtension_ARB_bindless_texture::glMakeTextureHandleNonResidentARB(GLuint64 handle)
{
  Q_D(QOpenGLExtension_ARB_bindless_texture);
  d->MakeTextureHandleNonResidentARB(handle);
}

inline void QOpenGLExtension_ARB_bindless_texture::glMakeTextureHandleResidentARB(GLuint64 handle)
{
  Q_D(QOpenGLExtension_ARB_bindless_texture);
  d->MakeTextureHandleResidentARB(handle);
}

inline GLuint64 QOpenGLExtension_ARB_bindless_texture::glGetTextureSamplerHandleARB(GLuint texture, GLuint sampler)
{
  Q_D(QOpenGLExtension_ARB_bindless_texture);
  return d->GetTextureSamplerHandleARB(texture, sampler);
}

inline GLuint64 QOpenGLExtension_ARB_bindless_texture::glGetTextureHandleARB(GLuint texture)
{
  Q_D(QOpenGLExtension_ARB_bindless_texture);
  return d->GetTextureHandleARB(texture);
}

QOpenGLExtension_ARB_bindless_texture::QOpenGLExtension_ARB_bindless_texture()
  : QAbstractOpenGLExtension(*(new QOpenGLExtension_ARB_bindless_texturePrivate))
{
}

bool QOpenGLExtension_ARB_bindless_texture::initializeOpenGLFunctions()
{
  if (isInitialized())
    return true;

  QOpenGLContext* context = QOpenGLContext::currentContext();
  if (!context) {
    qWarning("A current OpenGL context is required to resolve OpenGL extension functions");
    return false;
  }

  // Resolve the functions
  Q_D(QOpenGLExtension_ARB_bindless_texture);

  d->IsImageHandleResidentARB = reinterpret_cast<GLboolean(QOPENGLF_APIENTRYP)(GLuint64)>(context->getProcAddress("glIsImageHandleResidentARB"));
  d->IsTextureHandleResidentARB = reinterpret_cast<GLboolean(QOPENGLF_APIENTRYP)(GLuint64)>(context->getProcAddress("glIsTextureHandleResidentARB"));
  d->ProgramUniformHandleui64vARB = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLint, GLsizei, const GLuint64*)>(context->getProcAddress("glProgramUniformHandleui64vARB"));
  d->ProgramUniformHandleui64ARB = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLint, GLuint64)>(context->getProcAddress("glProgramUniformHandleui64ARB"));
  d->UniformHandleui64vARB = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint, GLsizei, const GLuint64*)>(context->getProcAddress("glUniformHandleui64vARB"));
  d->UniformHandleui64ARB = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint, GLuint64)>(context->getProcAddress("glUniformHandleui64ARB"));
  d->MakeImageHandleNonResidentARB = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint64)>(context->getProcAddress("glMakeImageHandleNonResidentARB"));
  d->MakeImageHandleResidentARB = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint64, GLenum)>(context->getProcAddress("glMakeImageHandleResidentARB"));
  d->GetImageHandleARB = reinterpret_cast<GLuint64(QOPENGLF_APIENTRYP)(GLuint, GLint, GLboolean, GLint, GLenum)>(context->getProcAddress("glGetImageHandleARB"));
  d->MakeTextureHandleNonResidentARB = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint64)>(context->getProcAddress("glMakeTextureHandleNonResidentARB"));
  d->MakeTextureHandleResidentARB = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint64)>(context->getProcAddress("glMakeTextureHandleResidentARB"));
  d->GetTextureSamplerHandleARB = reinterpret_cast<GLuint64(QOPENGLF_APIENTRYP)(GLuint, GLuint)>(context->getProcAddress("glGetTextureSamplerHandleARB"));
  d->GetTextureHandleARB = reinterpret_cast<GLuint64(QOPENGLF_APIENTRYP)(GLuint)>(context->getProcAddress("glGetTextureHandleARB"));
  QAbstractOpenGLExtension::initializeOpenGLFunctions();
  return true;
}
