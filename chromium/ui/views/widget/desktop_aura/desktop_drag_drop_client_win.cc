// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/widget/desktop_aura/desktop_drag_drop_client_win.h"

#include "base/metrics/histogram_macros.h"
#include "ui/base/dragdrop/drag_drop_types.h"
#include "ui/base/dragdrop/drag_source_win.h"
#include "ui/base/dragdrop/drop_target_event.h"
#include "ui/base/dragdrop/os_exchange_data_provider_win.h"
#include "ui/views/widget/desktop_aura/desktop_drop_target_win.h"
#include "ui/views/widget/desktop_aura/desktop_window_tree_host_win.h"

#include "app/vivaldi_apptools.h"
#include "ui/dragging/custom_drag_source_win.h"

namespace views {

DesktopDragDropClientWin::DesktopDragDropClientWin(
    aura::Window* root_window,
    HWND window)
    : drag_drop_in_progress_(false),
      drag_operation_(0),
      weak_factory_(this) {
  drop_target_ = new DesktopDropTargetWin(root_window, window);
}

DesktopDragDropClientWin::~DesktopDragDropClientWin() {
  if (drag_drop_in_progress_)
    DragCancel();
}

int DesktopDragDropClientWin::StartDragAndDrop(
    const ui::OSExchangeData& data,
    aura::Window* root_window,
    aura::Window* source_window,
    const gfx::Point& screen_location,
    int operation,
    ui::DragDropTypes::DragEventSource source,
    bool& cancelled) {
  drag_drop_in_progress_ = true;
  drag_operation_ = operation;

  base::WeakPtr<DesktopDragDropClientWin> alive(weak_factory_.GetWeakPtr());

  if (vivaldi::IsVivaldiRunning()) {
    drag_source_ = Microsoft::WRL::Make<vivaldi::CustomDragSourceWin>(
        vivaldi::IsTabDragInProgress());
  } else {
    drag_source_ = Microsoft::WRL::Make<ui::DragSourceWin>();
  }
  Microsoft::WRL::ComPtr<ui::DragSourceWin> drag_source_copy = drag_source_;
  drag_source_copy->set_data(&data);
  ui::OSExchangeDataProviderWin::GetDataObjectImpl(data)
      ->set_in_drag_loop(true);

  DWORD effect;

  UMA_HISTOGRAM_ENUMERATION("Event.DragDrop.Start", source,
                            ui::DragDropTypes::DRAG_EVENT_SOURCE_COUNT);

  HRESULT result = DoDragDrop(
      ui::OSExchangeDataProviderWin::GetIDataObject(data), drag_source_.Get(),
      ui::DragDropTypes::DragOperationToDropEffect(operation), &effect);
  drag_source_copy->set_data(nullptr);

  if (alive)
    drag_drop_in_progress_ = false;

  cancelled = (result == DRAGDROP_S_CANCEL);

  if (result != DRAGDROP_S_DROP)
    effect = DROPEFFECT_NONE;

  int drag_operation = ui::DragDropTypes::DropEffectToDragOperation(effect);

  if (drag_operation == ui::DragDropTypes::DRAG_NONE) {
    UMA_HISTOGRAM_ENUMERATION("Event.DragDrop.Cancel", source,
                              ui::DragDropTypes::DRAG_EVENT_SOURCE_COUNT);
  } else {
    UMA_HISTOGRAM_ENUMERATION("Event.DragDrop.Drop", source,
                              ui::DragDropTypes::DRAG_EVENT_SOURCE_COUNT);
  }

  return drag_operation;
}

void DesktopDragDropClientWin::DragCancel() {
  drag_source_->CancelDrag();
  drag_operation_ = 0;
}

bool DesktopDragDropClientWin::IsDragDropInProgress() {
  return drag_drop_in_progress_;
}

void DesktopDragDropClientWin::OnNativeWidgetDestroying(HWND window) {
  if (drop_target_.get()) {
    RevokeDragDrop(window);
    drop_target_ = NULL;
  }
}

}  // namespace views
