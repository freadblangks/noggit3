// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>

namespace noggit::ui
{
  class collapsible_widget : public QGroupBox
  {
    Q_OBJECT

  public:
    collapsible_widget(std::string const& header, bool collapsed, QWidget* parent = nullptr);

    void add_layout(QLayout* layout);
    void add_widget(QWidget* widget);

    // to avoid using the groupbox's title/checkbox as it's only
    // used to get the border, the title is inside the widget
    void setTitle(const QString& title) { }
    void setCheckable(bool checkable) { }

  private:
    void toggle();

    QWidget* _widget;
    QVBoxLayout* _layout;
    QPushButton* _toggle_btn;

    bool _collasped = false;
  };
}
