// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtCore/QSettings>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

namespace util
{
  struct file_line_edit : public QWidget
  {
    enum mode
    {
      files,
      directories,
    };

    file_line_edit (mode, QString browse_title, QWidget* parent);

    QLineEdit* actual;
  };
}

namespace noggit
{
  namespace ui
  {
    class settings : public QDialog
    {
    private:
      util::file_line_edit* gamePathField;
      util::file_line_edit* wodPathField;
      util::file_line_edit* projectPathField;
      util::file_line_edit* importPathField;
      util::file_line_edit* wmvLogPathField;
      QDoubleSpinBox* viewDistanceField;
      QDoubleSpinBox* farZField;

      QCheckBox* tabletModeCheck;

      QGroupBox* _mysql_box;
      QLineEdit* _mysql_server_field;
      QLineEdit* _mysql_user_field;
      QLineEdit* _mysql_pwd_field;
      QLineEdit* _mysql_db_field;

      QSettings* _settings;
    public:
      settings();
      void discard_changes();
      void save_changes();
    };
  }
}
