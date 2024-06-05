// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtCore/QObject>

namespace noggit
{
  struct float_property : QObject
  {
  private:
    Q_OBJECT

    float _value;

  signals:
    void changed (float);

  public slots:
    void set (float v)
    {
      if (_value != v)
      {
        _value = v;
        emit changed (v);
      }
    }
    void change(float v)
    {
      _value += v;
      emit changed(_value);
    }
    float get() const
    {
      return _value;
    }

  public:
    float_property (float value)
      : _value (value)
    {}
  };
}
