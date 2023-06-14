// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTreeWidget>

#include <functional>
#include <optional>
#include <string>
#include <vector>

struct asset_tree_node
{
  asset_tree_node(std::string name) : name(name) {}
  asset_tree_node(asset_tree_node const& other) : name(other.name), children(other.children) {}

  asset_tree_node& add_child(std::string const& child_name)
  {
    for (asset_tree_node& child : children)
    {
      if (child_name == child.name)
      {
        return child;
      }
    }
    children.emplace_back(child_name);

    return children.back();
  }

  bool operator==(asset_tree_node const& other) { return name == other.name; }
  asset_tree_node const& operator=(asset_tree_node const& other)
  {
    name = other.name;
    children = other.children;

    return *this;
  }
  asset_tree_node const& operator=(asset_tree_node&& other)
  {
    std::swap(name, other.name);
    std::swap(children, other.children);

    return *this;
  }
  std::vector<asset_tree_node> children;
  std::string name;
};

namespace noggit::ui
{
  class object_editor;

  class asset_browser : public QWidget
  {
    Q_OBJECT
  public:
    asset_browser(QWidget* parent = nullptr, noggit::ui::object_editor* object_editor = nullptr);

  private:
    void create_tree(std::string filter = "");

    void add_children(asset_tree_node const& data_node, QTreeWidgetItem* parent);

    bool _expanded = false;

    QLineEdit* _search_bar;
    QTreeWidget* _asset_tree;
  };
}
