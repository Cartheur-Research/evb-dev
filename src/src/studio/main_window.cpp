#include <studio.hpp>
#include "boost/filesystem.hpp"
#include <evb001.hpp>
#include <evb002.hpp>

namespace studio {

//
//MainView::MainView(MainWindow& w)
//: window_(w)
//{
//}
//
//MainView::~MainView()
//{
//}

//=========================================================================


MainWindow::MainWindow()
//: view_main_(*this)
//, connection_window_(0)
: is_modified_(false)
{
}

MainWindow::~MainWindow()
{
}

void MainWindow::modify()
{
  if (!is_modified_) {
    std::string title;
    title = get_title();
    title.insert(0, 1, '*');
    set_title(title); 
    //save_.set_sensitive(!filename_.empty());
    is_modified_ = true;
  }
}

void MainWindow::unmodify()
{
  if (is_modified_) {
    std::string title;
    title = get_title();
    if (title[0] == '*') {
      title.erase(0, 1);
    }
    set_title(title); 
    is_modified_ = false;
  }
}

void MainWindow::save()
{
  //if (is_modified_) {
    std::string title;
    title = get_title();
    if (!title.empty() && (title[0] == '*')) {
      title.erase(0, 1);
      set_title(title);
    }
    if (!filename_.empty()) {
      store(filename_);
    }
    //save_.set_sensitive(!filename_.empty());
    is_modified_ = false;
  //}
}

void MainWindow::load(const std::string& filename)
{
	std::ifstream in(filename.c_str(), ::std::ios::binary);
	xml::Iterator it(in);
  it.begin();
	read(it);
	in.close();
}

void MainWindow::store(const std::string& filename)
{
	std::ofstream out(filename.c_str(), std::ios_base::out | std::ios_base::binary);
  xml::Writer w(out);
	write(w);
	out.close();
}

void MainWindow::setup(const std::string& file)
{
  filename_ = file;

  int w = 1000, h = 700;
  set_default_size(w, h); 
  set_title("GreenArrays Studio");

  //view_event_.add(view_main_);
  view_event_.set_hexpand(true);
  view_event_.set_vexpand(true);
  view_event_.add_events(Gdk::BUTTON_MOTION_MASK);
  view_event_.add_events(Gdk::SCROLL_MASK);
  //view_event_.signal_button_press_event().connect(sigc::mem_fun(view_, &EvbView::on_button_press_event) );
  //view_event_.signal_scroll_event().connect(sigc::mem_fun(view_, &EvbView::on_scroll_event) );

  content_.pack_start(menu_, Gtk::PACK_SHRINK);
  //content_.pack_start(view_event_, Gtk::PACK_EXPAND_WIDGET);
  content_.pack_start(board_bar_, Gtk::PACK_EXPAND_WIDGET);

  menu_.append(file_); file_.set_label("File"); file_.set_submenu(file_menu_);
  menu_.append(edit_); edit_.set_label("Edit"); edit_.set_submenu(edit_menu_);
  menu_.append(view_); view_.set_label("View"); view_.set_submenu(view_menu_);


  file_menu_.append(new_); new_.set_label("New"); new_.set_submenu(new_menu_);
  new_menu_.append(new_evb001_); new_evb001_.set_label("New EVB001");
  new_menu_.append(new_evb002_); new_evb002_.set_label("New EVB002");


  file_menu_.append(file_sep1_);
  file_menu_.append(save_); save_.set_label("Save");
  file_menu_.append(save_as_); save_as_.set_label("Save as...");
  file_menu_.append(file_sep2_);
  file_menu_.append(open_); open_.set_label("Open...");
  file_menu_.append(file_sep3_);
  file_menu_.append(quit_); quit_.set_label("Quit");

  edit_menu_.append(connection_); connection_.set_label("Connection...");

  quit_.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_menu_file_quit));
  save_.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_save));
  new_evb001_.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_new_001));
  new_evb002_.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_new_002));
  //connection_.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_connection_window));

  //quit_accel_ = new Gtk::AccelKey('q', Gdk::CONTROL_MASK, "<MainWindow>/File/Quit");

  accel_ = Gtk::AccelGroup::create();
  quit_.add_accelerator("activate", accel_, 'q', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
  //new_.add_accelerator("activate", accel_, 'n', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
  save_.add_accelerator("activate", accel_, 's', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
  this->add_accel_group(accel_);

  add(content_);
  show_all_children();

  if (filename_.empty()) {
    save_.set_sensitive(false);
  }
  else {
    try {
      load(filename_);
    }
    catch (...) {
      modify();
    }
  }
  //(new Evb001Window())->show();
}

void MainWindow::open_new(const std::string& name, bool show, evb::Board_ptr ptr)
{
  Gtk::ToolButton* button;
  if (ptr.is_null()) {
    ptr = new evb::Board;
  }
  //std::string file = "pict/evb001_board.png";
  std::string file = ptr->picture_file();
  if (boost::filesystem::exists(file)) {
    Gtk::Image* image = new Gtk::Image(file);
    image->reference();
    button = new Gtk::ToolButton(*image, name);
  }
  else {
    button = new Gtk::ToolButton(name);
  }

  board_bar_.add(*button);


  EvbWindow* curr = new EvbWindow(*this, ptr);
  curr->view().model()->register_modify_dispatcher(modify_dispatcher_);
  button->signal_clicked().connect(sigc::mem_fun(*curr, &EvbWindow::show));
  curr->name(name);
  board_list_.push_back(curr);
  modify();
  queue_draw();
  show_all_children();

  button->signal_clicked().connect(sigc::bind<EvbWindow&>( sigc::mem_fun(*this, &MainWindow::on_click_board), *curr));

  evb_window_list_.push_back(curr);

  if (show) {
    curr->show();
  }
}

void MainWindow::on_new_001()
{
  open_new("<unnamed 001>", true, new evb::Board001());
  modify();
}

void MainWindow::on_new_002()
{
  open_new("<unnamed 002>", true, new evb::Board002());
  modify();
}

void MainWindow::on_save()
{
  save();
}

void MainWindow::on_click_board(EvbWindow& w)
{
  w.show();
  w.present();
}

void MainWindow::on_menu_file_quit()
{
  hide(); 
}

void MainWindow::reset()
{
  for (size_t i=0; i<board_list_.size(); ++i) {
    if (board_list_[i]) {
      board_list_[i]->hide();
      board_list_[i]->reset();
      delete board_list_[i];
    }
  }
}

//void MainWindow::on_connection_window()
//{
//  if (connection_window_ == 0) {
//    connection_window_ = new ConnectionWindow();
//  }
//  connection_window_->open();
//}

void MainWindow::read(xml::Iterator& it)
{
  evb::Board_ptr model;
  reset();
	if (it.current() != xml::Element::tag) {throw xml_exception("MainWindow::read invalid file");}
	std::string tag = it.tag();
	it.next_tag();
	while (it.current() == xml::Element::tag) {
  	std::string evb_tag = it.tag();
		if (evb_tag == "evb") {
      model = new evb::Board;
      //std::string version;
      //if (it.tag() == "version") {
      //  if (it.next() == xml::Element::content) {
      //    version = it.value();
      //    if ((it.next_tag() == xml::Element::end) && (it.tag() == "version")) {
      //      it.next_tag();
      //    }
      //  }
      //}

      //if (model.is_null()) {
      //  if ((version == "001") || version.empty()) {
      //    model = new evb::Board001();
      //  }
      //  else {
      //    throw xml_exception("EvbView::read evalboard version not supported");
      //  }
      //}
      //else if (!version.empty()) {
      //  if (version == "001") {
      //    model = new evb::Board001();
      //  }
      //  else {
      //    throw xml_exception("EvbView::read evalboard version not supported");
      //  }
      //}

      //EvbWindow* curr = new EvbWindow();
      //board_list_.push_back(curr);
      //curr->read(it);
    }
    else if (evb_tag == "evb001") {
      model = new evb::Board001;
    }
    else if (evb_tag == "evb002") {
      model = new evb::Board002;
    }
    if (model.is_null()) {
      throw xml_exception("Context::read invalid evb tag");
    }
    it.next_tag();
    open_new("", false, model);
    board_list_.back()->read(it);
    board_list_.back()->on_update_modules();
    board_list_.back()->on_update_packages();
    if ((it.current() != xml::Element::end) || (it.tag() != evb_tag)) {throw xml_exception("Context::read evb end tag");}
    it.next_tag();
	}

	if ((it.current() != xml::Element::end) || (it.tag() != tag)) {throw xml_exception("Context::read invalid file");}
  unmodify();
}

void MainWindow::write(xml::Writer& w)
{
	w.out() << "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>";
	//w.out() << "<?xml-stylesheet type='text/xsl' href='lib.xsl'?>";
	//w.newline();
	w.newline();
  w.open("ga", false, true);
  w.indent(1);
  for (size_t i=0; i<board_list_.size(); ++i) {
    if (board_list_[i]) {
      std::string version = board_list_[i]->view().model()->version();
    	w.newline();
      w.open("evb" + version, false, true);
      w.indent(1);
      board_list_[i]->write(w);
      w.newline(-1);
      w.open("evb" + version, true, true);
    }
  }
  w.newline(-1);
  w.open("ga", true, true);
}




}
