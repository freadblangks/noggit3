#pragma once

#include <QtCore/QSettings>

#include <memory>

namespace noggit
{
  class settings
  {
  public:
    settings()
    {
      values = std::make_unique<QSettings>("settings.ini", QSettings::Format::IniFormat);

      QString project_folder = values->value("project/path").toString();
      if (!(project_folder.endsWith('\\') || project_folder.endsWith('/')))
      {
        project_folder += "/";
      }

      values->setValue("project/path", project_folder);

      uids = std::make_unique<QSettings>(project_folder + "uid.ini", QSettings::Format::IniFormat);
    }

    QVariant value(const QString& key, const QVariant& default = QVariant()) const
    {
      return values->value(key, default);
    }

    void set_value(const QString& key, const QVariant& value)
    {
      values->setValue(key, value);
    }

    std::string project_path()
    {
      return values->value("project/path").toString().toStdString();
    }

    void sync()
    {
      values->sync();
    }

    std::unique_ptr<QSettings> values;
    std::unique_ptr<QSettings> uids;

    static settings& instance()
    {
      static settings inst;
      return inst;
    }
  };
}

#define NoggitSettings noggit::settings::instance()
