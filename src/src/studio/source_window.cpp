#include <studio.hpp>
#include <connection.hpp>
#include <evb.hpp>

namespace studio {


SourceWindow::SourceWindow(evb::Board& board, bool is_rom)
: board_(board)
, view_(0)
, pos_(0)
, selected_module_(0)
, is_rom_(is_rom)
{
  this->set_default_size(1024, 600);

  name_label_.set_text("This source belongs to module ");
  name_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  name_box_.pack_start(name_label_, Gtk::PACK_SHRINK);
  //name_box_.pack_start(name_, Gtk::PACK_EXPAND_WIDGET);
  name_box_.pack_start(name_list_, Gtk::PACK_SHRINK);


  source_label_.set_text("Source");
  source_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  comment_label_.set_text("Comment");
  comment_label_.set_alignment(Gtk::JUSTIFY_LEFT);


  buffer_ = Gtk::TextBuffer::create();
  comment_buffer_ = Gtk::TextBuffer::create();
  text_.set_buffer(buffer_);
  comment_text_.set_buffer(comment_buffer_);

  source_frame_.add(source_scroll_);
  comment_frame_.add(comment_scroll_);
  source_scroll_.add(text_);
  comment_scroll_.add(comment_text_);
  source_scroll_.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  comment_scroll_.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

  //this->set_modal(true);
  //this->set_deletable(false);

  Pango::FontDescription fdesc;
  fdesc.set_family("Consolas,monospace");
  fdesc.set_size(14 * PANGO_SCALE);
  text_.override_font(fdesc);
  comment_text_.override_font(fdesc);

  // buttons
  button_box_.set_border_width(5);
  button_box_.set_layout(Gtk::BUTTONBOX_END);

  assemble_button_.set_label("Assemble");
  button_box_.pack_start(assemble_button_, Gtk::PACK_SHRINK);
  assemble_button_.signal_clicked().connect(sigc::mem_fun(*this, &SourceWindow::on_button_assemble) );

  ok_button_.set_label("OK");
  button_box_.pack_start(ok_button_, Gtk::PACK_SHRINK);
  ok_button_.signal_clicked().connect(sigc::mem_fun(*this, &SourceWindow::on_button_ok) );
    
  cancel_button_.set_label("Cancel");
  button_box_.pack_start(cancel_button_, Gtk::PACK_SHRINK);
  cancel_button_.signal_clicked().connect(sigc::mem_fun(*this, &SourceWindow::on_button_cancel) );

  //table_.set_column_homogeneous(false);
  //table_.set_border_width(10);
  //table_.set_row_spacing(5);
  //table_.set_column_spacing(5);

  //table_.attach(source_label_,            0, 0, 1, 1);
  //table_.attach(comment_label_,           2, 0, 1, 1);
  //table_.attach(source_box_,              0, 1, 1, 1);
  //table_.attach(sep_,                     1, 1, 1, 1);
  //table_.attach(comment_box_,             2, 1, 1, 1);

  source_box_.pack_start(source_label_, Gtk::PACK_SHRINK);
  source_box_.pack_start(source_frame_, Gtk::PACK_EXPAND_WIDGET);
  comment_box_.pack_start(comment_label_, Gtk::PACK_SHRINK);
  comment_box_.pack_start(comment_frame_, Gtk::PACK_EXPAND_WIDGET);

  text_box_.pack_start(source_box_, Gtk::PACK_EXPAND_WIDGET);
  text_box_.pack_start(sep_, Gtk::PACK_SHRINK);
  text_box_.pack_start(comment_box_, Gtk::PACK_EXPAND_WIDGET);

  vbox_.pack_start(name_box_, Gtk::PACK_SHRINK);

  vbox_.pack_start(text_box_, Gtk::PACK_EXPAND_WIDGET);


  //vbox_.pack_start(comment_label_, Gtk::PACK_SHRINK);
  //vbox_.pack_start(comment_text_, Gtk::PACK_EXPAND_WIDGET, 10);
  //vbox_.pack_start(source_label_, Gtk::PACK_SHRINK);
  //vbox_.pack_start(text_, Gtk::PACK_EXPAND_WIDGET);

  vbox_.pack_start(button_box_, Gtk::PACK_SHRINK);

  vbox_.set_border_width(5);

  add(vbox_);
  show_all_children();

  name_list_.signal_changed().connect( sigc::mem_fun(*this, &SourceWindow::on_module_changed) );
}

SourceWindow::~SourceWindow()
{
}

void SourceWindow::setup(F18aView& view, size_t pos)
{
  pos_ = pos;
  view_ = &view;
  std::string title = view.window().get_title();
  if (is_rom_) {
    title += " - ROM";
    set_title(title);
    module_name_.clear();
    old_module_name_.clear();

    name_list_.append("0", "ROM");
    name_list_.set_active(0);
    name_list_.set_sensitive(false);

    f18a::Module_ptr module = view.model()->rom_module();
    if (module.is_null()) {
      module = new f18a::Module();
      module->reset();
    }
    buffer_->set_text(module->rom().source());
    comment_buffer_->set_text(module->rom().comment());
  }
  else {
    size_t config_limit = view.model()->config_size();
    while (pos_ >= config_limit) {
      view.model()->append_config();
      config_limit = view.model()->config_size();
    }
    f18a::Module_ptr module = view.model()->config(pos_);
    old_module_name_= module_name_ = module->name();
    set_title(title);

    name_list_.set_sensitive(true);
    name_list_.append("0", "-");
    std::map<std::string, bool> used_selection;
    for (evb::Board::module_map_type::iterator it = board_.modules().begin(); it != board_.modules().end(); ++it) {
      used_selection[it->first] = false;
    }
    for (size_t i=0; i<config_limit; ++i) {
      f18a::Module_ptr mod = view.model()->config(i);
      used_selection[mod->name()] = true;
    }
    size_t selected = 0;
    size_t size = 1;
    if (!module_name_.empty()) {
      used_selection[module_name_] = false;
    }
    for (std::map<std::string, bool>::iterator it = used_selection.begin(); it != used_selection.end(); ++it) {
      if (!it->second) {
        if (it->first == module_name_) {
          selected = size;
        }
        name_list_.append(boost::lexical_cast<std::string>(size), it->first);
        ++size;
      }
    }
    name_list_.set_active(selected);
    buffer_->set_text(module->ram().source());
    comment_buffer_->set_text(module->ram().comment());
  }
}

void SourceWindow::text(const std::string& txt)
{
  buffer_->set_text(txt);
}

void SourceWindow::open()
{
  show();
}

void SourceWindow::on_button_assemble()
{
  if (view_) {
    f18a::State state;
    f18a::Processor_ptr proc = view_->model();

    if (
      !state.assemble(
        module_name_,
        buffer_->get_text(),
        is_rom_,
        proc,
        view_->window().owner().window().owner().model()
      )
    ) {
      Gtk::MessageDialog dialog(*this, "F18A Assembly failed");
      std::string err = state.error_;
      dialog.set_secondary_text(err);
      dialog.show();
      dialog.run();
      this->present();
      this->grab_focus();
      //text_.move_visually(buffer_->begin(), state.last_token_pos());
      text_.set_cursor_visible();
      //buffer_->place_cursor(buffer_->get_iter_at_offset(state.last_token_pos()));
      buffer_->select_range(
        buffer_->get_iter_at_offset(state.last_token_pos()),
        buffer_->get_iter_at_offset(state.last_token_pos() + state.token().size())
      );
      text_.grab_focus();
      return;
    }
    hide();
    //if (module->name() != name_.get_text()) {
    //  module->name(name_.get_text());
    //  view_->window().name(pos_, name_.get_text());
    //}
    if (is_rom_) {
      f18a::Module_ptr module = proc->rom_module();
      module->rom().source(buffer_->get_text());
      module->rom().comment(comment_buffer_->get_text());
    }
    else {
      f18a::Module_ptr module = proc->assert_config(pos_);
      module->ram().source(buffer_->get_text());
      module->ram().comment(comment_buffer_->get_text());
    }
    state.write(view_->model(), pos_, is_rom_);
    view_->queue_draw();
    view_->modify();
  }
}

void SourceWindow::on_button_ok()
{
  hide();
  if (view_) {
    //module_name_ = name_list_.get_active_text();
    //if (module_name_ == "-") {
    //  module_name_.clear();
    //}
    f18a::Processor_ptr proc = view_->model();
    ga144::Chip_ptr chip = view_->window().owner().model();
    evb::Board_ptr board = view_->window().owner().window().owner().model();
    if (module_name_ != old_module_name_) {
      // module has changed
      size_t node = board->join_node(chip, view_->model()->node());
      f18a::Module_ptr module = view_->model()->assert_config(pos_);
      if (!old_module_name_.empty()) {
        evb::Module_ptr old_module = board_.find_module(old_module_name_);
        if (old_module.is_not_null()) {
          // remove node from old module
          old_module->remove(node);
        }
      }
      if (!module_name_.empty()) {
        // add to new module
        evb::Module_ptr new_module = board_.find_module(module_name_);
        if (new_module.is_not_null()) {
          // insert node into new module
          new_module->add(node, module);
        }
      }
      old_module_name_ = module_name_;
      module->name(module_name_);
    }
    if (is_rom_) {
      f18a::Module_ptr module = proc->rom_module();
      module->rom().source(buffer_->get_text());
      module->rom().comment(comment_buffer_->get_text());
    }
    else {
      f18a::Module_ptr module = proc->assert_config(pos_);
      module->ram().source(buffer_->get_text());
      module->ram().comment(comment_buffer_->get_text());
    }
    view_->modify();
  }
}

void SourceWindow::on_button_cancel()
{
  hide();
  if (view_) {
    if (view_->model()->config_size() > pos_) {
      f18a::Processor_ptr proc = view_->model();
      if (is_rom_) {
        buffer_->set_text(proc->rom_module()->rom().source());
        comment_buffer_->set_text(proc->rom_module()->rom().comment());
      }
      else {
        buffer_->set_text(proc->config(pos_)->ram().source());
        comment_buffer_->set_text(proc->config(pos_)->ram().comment());
      }
    }
    else {
      buffer_->set_text("");
      comment_buffer_->set_text("");
    }
  }
}

void SourceWindow::on_module_changed()
{
  if (is_rom_ || (view_ == 0)) {
    return;
  }
  module_name_ = name_list_.get_active_text();;
  if (module_name_ == "-") {
    module_name_.clear();
  }

}


}
