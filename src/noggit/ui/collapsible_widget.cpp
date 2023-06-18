// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/collapsible_widget.hpp>
#include <noggit/ui/clickable_label.hpp>

#include <QtWidgets/QGridLayout>


namespace noggit::ui
{
  collapsible_widget::collapsible_widget(std::string const& header, bool collasped, QWidget* parent)
    : QGroupBox(parent)
    , _collasped(collasped)
  {
    QGridLayout* layout = new QGridLayout(this);

    layout->setSpacing(1);
    layout->setContentsMargins(1, 1, 1, 1);

    _toggle_btn = new QPushButton(collasped ? ">" : "-", this);
    _toggle_btn->setFixedWidth(20);
    _toggle_btn->setFixedHeight(20);
    _toggle_btn->setStyleSheet("text-align:center; background-color:transparent; border-color:transparent;");

    clickable_label* header_label = new clickable_label(this);
    header_label->setText(QString::fromStdString(header));

    layout->addWidget(_toggle_btn, 0, 0);
    layout->addWidget(header_label, 0, 1);

    _widget = new QWidget(this);
    _layout = new QVBoxLayout(_widget);

    layout->addWidget(_widget, 1, 0, 1, 2);

    if (_collasped)
    {
      _toggle_btn->setText(">");
      _widget->setVisible(false);
    }

    connect( _toggle_btn, &QPushButton::clicked
           , [=]()
             {
               toggle();
             }
           );
    connect( header_label, &clickable_label::clicked
           , [=]
             {
               toggle();
             }
           );
  }

  void collapsible_widget::add_layout(QLayout* layout)
  {
    _layout->addLayout(layout);
  }
  void collapsible_widget::add_widget(QWidget* widget)
  {
    _layout->addWidget(widget);
  }

  void collapsible_widget::toggle()
  {
    _toggle_btn->setText(_collasped ? "-" : ">");
    _widget->setVisible(_collasped);
    _collasped = !_collasped;
  }
}
