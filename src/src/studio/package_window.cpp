#include <studio.hpp>
#include <connection.hpp>
#include <evb.hpp>

namespace studio {


PackageWindow::PackageWindow(EvbWindow& w, const std::string& name)
: owner_(w)
, name_(name)
, unassigned_list_(1)
, assigned_list_(1)
{
  this->set_default_size(1024, 600);

  std::string title;
  title = "Package '";
  title += name_;
  title += "'";
  set_title(title);

  //unassigned_box_.set_border_width(20);
  //assigned_box_.set_border_width(20);
  //Gdk::RGBA black;
  //black.set_rgba(0, 0, 0, 1.);
  //unassigned_box_.override_background_color(black);
  //assigned_box_.override_background_color(Gdk::RGBA("#000"));
  unassigned_box_.add(unassigned_scroll_);
  assigned_box_.add(assigned_scroll_);
  unassigned_scroll_.add(unassigned_list_);
  assigned_scroll_.add(assigned_list_);
  unassigned_scroll_.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  assigned_scroll_.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

  unassigned_list_.set_column_title(0, "Unassigned Modules");
  assigned_list_.set_column_title(0, "Assigned Modules");

  name_label_.set_text("Package Name");
  name_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  name_box_.pack_start(name_label_, Gtk::PACK_SHRINK);
  name_box_.pack_start(name_entry_, Gtk::PACK_EXPAND_WIDGET);
  name_box_.set_spacing(5);

  exclude_label_.set_text("Exclude List (e.g. '708,709,10300')");
  exclude_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  exclude_box_.pack_start(exclude_label_, Gtk::PACK_SHRINK);
  exclude_box_.pack_start(exclude_entry_, Gtk::PACK_EXPAND_WIDGET);
  exclude_box_.set_spacing(5);

  //Pango::FontDescription fdesc;
  //fdesc.set_family("Consolas,monospace");
  //fdesc.set_size(14 * PANGO_SCALE);
  //text_.override_font(fdesc);
  //comment_text_.override_font(fdesc);

  // buttons
  button_box_.set_border_width(5);
  button_box_.set_layout(Gtk::BUTTONBOX_END);

  add_button_.set_label("<<");
  remove_button_.set_label(">>");
  up_button_.set_label("up");
  down_button_.set_label("down");

  button_vbox_.pack_start(add_button_, Gtk::PACK_SHRINK);
  add_button_.signal_clicked().connect(sigc::mem_fun(*this, &PackageWindow::on_button_add) );
  button_vbox_.pack_start(remove_button_, Gtk::PACK_SHRINK);
  remove_button_.signal_clicked().connect(sigc::mem_fun(*this, &PackageWindow::on_button_remove) );
  button_vbox_.pack_start(up_button_, Gtk::PACK_SHRINK);
  up_button_.signal_clicked().connect(sigc::mem_fun(*this, &PackageWindow::on_button_up) );
  button_vbox_.pack_start(down_button_, Gtk::PACK_SHRINK);
  down_button_.signal_clicked().connect(sigc::mem_fun(*this, &PackageWindow::on_button_down) );

  ok_button_.set_label("OK");
  button_box_.pack_start(ok_button_, Gtk::PACK_SHRINK);
  ok_button_.signal_clicked().connect(sigc::mem_fun(*this, &PackageWindow::on_button_ok) );
    
  revert_button_.set_label("Revert");
  button_box_.pack_start(revert_button_, Gtk::PACK_SHRINK);
  revert_button_.signal_clicked().connect(sigc::mem_fun(*this, &PackageWindow::on_button_revert) );
    
  cancel_button_.set_label("Cancel");
  button_box_.pack_start(cancel_button_, Gtk::PACK_SHRINK);
  cancel_button_.signal_clicked().connect(sigc::mem_fun(*this, &PackageWindow::on_button_cancel) );

  list_hbox_.pack_start(assigned_box_, Gtk::PACK_EXPAND_WIDGET);
  list_hbox_.pack_start(button_vbox_, Gtk::PACK_SHRINK);
  list_hbox_.pack_start(unassigned_box_, Gtk::PACK_EXPAND_WIDGET);

  boot_method_label_.set_text("Boot Method");
  boot_method_label_.set_alignment(Gtk::JUSTIFY_LEFT);
  boot_button_.set_label("Boot");
  boot_button_.signal_clicked().connect(sigc::mem_fun(*this, &PackageWindow::on_button_boot) );

  boot_method_.append("H708", "Host async(708)");
  boot_method_.set_active_id("H708");

  boot_box_.pack_start(boot_method_label_, Gtk::PACK_SHRINK);
  boot_box_.pack_start(boot_method_, Gtk::PACK_EXPAND_WIDGET);
  boot_box_.pack_start(boot_button_, Gtk::PACK_SHRINK);
  boot_box_.set_spacing(5);

  vbox_.set_spacing(5);
  vbox_.pack_start(name_box_, Gtk::PACK_SHRINK);
  vbox_.pack_start(exclude_box_, Gtk::PACK_SHRINK);
  vbox_.pack_start(list_hbox_, Gtk::PACK_EXPAND_WIDGET);
  vbox_.pack_start(boot_box_, Gtk::PACK_SHRINK);
  vbox_.pack_start(button_box_, Gtk::PACK_SHRINK);

  vbox_.set_border_width(5);

  add(vbox_);
  show_all_children();

}

PackageWindow::~PackageWindow()
{
}

void PackageWindow::setup()
{
  setup(package_);
}

void PackageWindow::setup(evb::Package_ptr ptr)
{
  evb::Board_ptr board = owner_.view().model();
  package_ = ptr;
  if (package_.is_null()) {
    package_ = new evb::Package(**board, "");
  }
  else {
    name_ = package_->name();
    name_entry_.set_text(name_);
  }
  assigned_list_.clear_items();
  unassigned_list_.clear_items();
  assigned_map_.clear();
  size_t i, limit = package_->size();
  for (i=0; i<limit; ++i) {
    evb::Module_ptr mod = package_->module(i);
    if (mod.is_not_null()) {
      assigned_map_[mod->name()] = i;
      assigned_list_.append(mod->name());
    }
  }
  if (board.is_not_null()) {
    evb::Board::module_map_type& modules = board->modules();
    for (evb::Board::module_map_type::iterator it = modules.begin(); it != modules.end(); ++it) {
      std::map<std::string, size_t>::iterator ass_it = assigned_map_.find(it->first);
      if (ass_it == assigned_map_.end()) {
        unassigned_list_.append(it->first);
      }
    }
  }
  const evb::node_list_type& excl_list = package_->exclude_list();
  std::string excl_txt;
  limit = excl_list.size();
  for (i=0; i<limit; ++i) {
    if (i>0) {excl_txt.push_back(',');}
    excl_txt += boost::lexical_cast<std::string>(excl_list[i]);
  }
  exclude_entry_.set_text(excl_txt);
}

void PackageWindow::on_button_add()
{
  Gtk::ListViewText::SelectionList sel_list = unassigned_list_.get_selected();
  size_t limit = sel_list.size();
  if (!sel_list.empty()) {
    name_ = name_entry_.get_text();
    package_->rename(name_);
    evb::Board_ptr board = owner_.view().model();
    evb::Board::module_map_type& modules = board->modules();
    for (size_t i=0; i<limit; ++i) {
      std::string name = unassigned_list_.get_text(sel_list.at(i));
      evb::Board::module_map_type::iterator it = modules.find(name);
      if (it != modules.end()) {
        package_->append(it->second);
      }
    }
    setup();
  }
}

void PackageWindow::on_button_remove()
{
  Gtk::ListViewText::SelectionList sel_list = assigned_list_.get_selected();
  size_t limit = sel_list.size();
  if (!sel_list.empty()) {
    name_ = name_entry_.get_text();
    package_->rename(name_);
    evb::Board_ptr board = owner_.view().model();
    evb::Board::module_map_type& modules = board->modules();
    for (size_t i=0; i<limit; ++i) {
      std::string name = assigned_list_.get_text(sel_list.at(i));
      size_t pos;
      if (package_->find(name, pos)) {
        package_->remove(pos);
      }
    }
    setup();
  }
}

void PackageWindow::on_button_up()
{
  Gtk::ListViewText::SelectionList sel_list = assigned_list_.get_selected();
  size_t limit = sel_list.size();
  if (!sel_list.empty()) {
    name_ = name_entry_.get_text();
    package_->rename(name_);
    evb::Board_ptr board = owner_.view().model();
    evb::Board::module_map_type& modules = board->modules();
    if (package_->move_up(sel_list.at(0))) {
      setup();
      Gtk::TreeModel::iterator it = assigned_list_.get_model()->children().begin();
      size_t pos = sel_list.at(0)-1;
      while (pos > 0) {
        ++it;
        --pos;
      }
      Gtk::TreeModel::Path path = assigned_list_.get_model()->get_path(it);
      assigned_list_.set_cursor(path);
    }
  }
}

void PackageWindow::on_button_down()
{
  Gtk::ListViewText::SelectionList sel_list = assigned_list_.get_selected();
  size_t limit = sel_list.size();
  if (!sel_list.empty()) {
    name_ = name_entry_.get_text();
    package_->rename(name_);
    evb::Board_ptr board = owner_.view().model();
    evb::Board::module_map_type& modules = board->modules();
    if (package_->move_down(sel_list.at(0))) {
      setup();
      Gtk::TreeModel::iterator it = assigned_list_.get_model()->children().begin();
      size_t pos = sel_list.at(0)+1;
      while (pos > 0) {
        ++it;
        --pos;
      }
      Gtk::TreeModel::Path path = assigned_list_.get_model()->get_path(it);
      assigned_list_.set_cursor(path);
    }
  }
}

void PackageWindow::on_button_ok()
{
  evb::Board_ptr board = owner_.view().model();

  evb::node_list_type excl_list;
  std::string excl_txt = exclude_entry_.get_text();
  if (!excl_txt.empty()) {
    std::string::size_type pos = 0;
    std::string::size_type found_pos = excl_txt.find(',', pos);
    while (found_pos != std::string::npos) {
      try {
        excl_list.push_back(boost::lexical_cast<size_t>(excl_txt.substr(pos, found_pos-pos)));
      }
      catch (...) {
      }
      pos = found_pos+1;
      found_pos = excl_txt.find(',', pos);
    }
    try {
      excl_list.push_back(boost::lexical_cast<size_t>(excl_txt.substr(pos, excl_txt.size()-pos)));
    }
    catch (...) {
    }
  }
 
  name_ = name_entry_.get_text();
  board->remove_package(package_->name());
  package_->rename(name_);
  package_->exclude_list(excl_list);
  board->set_package(package_);
  hide();
  owner_.view().modify();
  owner_.on_update_packages();
}

void PackageWindow::on_button_revert()
{
  name_ = package_->name();
  name_entry_.set_text(name_);
  evb::Board_ptr board = owner_.view().model();
  evb::Package_ptr ptr = board->find_package(name_);
  if (ptr.is_not_null()) {
    setup(ptr->clone());
  }
}

void PackageWindow::on_button_cancel()
{
  hide();
}

void PackageWindow::on_button_boot()
{
  boot::Path::Enum path = boot::Path::limit;
  std::string bm = boot_method_.get_active_id();
  if (bm == "H708") {
    path = boot::Path::chip_via_708;
  }
  boot::Stream bs(path);
  evb::Board_ptr board = owner_.view().model();
  board->create_boot_stream(bs, package_);
  size_t len = bs.data().size();
  owner_.manager()->do_boot_via_708(bs.data());
}





}
